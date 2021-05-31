#include <string>
#include <cstdint>
#include <vector>
#include <cstdio>
#include "sqlite3.h"
#include "../message/Hashgraph.h"

namespace hashgraph {
namespace utils {

void createDatabaseTables(const std::string databasePath) {

    sqlite3 *db;

    // open database
    if (sqlite3_open_v2(databasePath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // enable WAL mode
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    // set query timeout
    sqlite3_exec(db, "PRAGMA busy_timeout = 30000;", NULL, NULL, NULL);

    // create database tables
    sqlite3_exec(db, 
        "CREATE TABLE IF NOT EXISTS Log ( "
        " id         INTEGER       PRIMARY KEY "
        "                          NOT NULL, "
        " owner      VARCHAR (100) NOT NULL, "
        " round      INTEGER       NOT NULL, "
        " time       INTEGER       NOT NULL, "
        " cnsTime    INTEGER       NOT NULL, "
        " selfHash   VARCHAR (100) NOT NULL, "
        " gossipHash VARCHAR (100) NOT NULL, "
        " payload    BLOB);"

        "CREATE TABLE IF NOT EXISTS User ( "
        "id         INTEGER       PRIMARY KEY "
        "                         NOT NULL, "
        "identifier VARCHAR (100) NOT NULL "
        "                         UNIQUE ON CONFLICT IGNORE, "
        "pkDer      BLOB          UNIQUE ON CONFLICT IGNORE);"

        "CREATE TABLE IF NOT EXISTS Transfer ( "
        "id        INTEGER PRIMARY KEY "
        "                  NOT NULL, "
        "sId       INTEGER REFERENCES User (id) "
        "                  NOT NULL, "
        "rId       INTEGER REFERENCES User (id) "
        "                  NOT NULL, "
        "sigDer    BLOB    NOT NULL, "
        "amount    INTEGER NOT NULL, "
        "timestamp INTEGER NOT NULL);", NULL, NULL, NULL);

    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

void storeTransferData(const std::string databasePath, const std::string senderId, const std::string receiverId, const std::string pkDer, const std::string sigDer, int32_t amount, int64_t timestamp) {

    sqlite3 *db;
    sqlite3_stmt* stmt;

    // open database
    if (sqlite3_open_v2(databasePath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // enable WAL mode
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    // set query timeout
    sqlite3_exec(db, "PRAGMA busy_timeout = 30000;", NULL, NULL, NULL);

    // start transaction
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    // insert sender and receiver if not existent
    if (sqlite3_prepare_v2(db, "INSERT INTO User (identifier) VALUES (?);", -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    sqlite3_bind_text(stmt, 1, receiverId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    sqlite3_reset(stmt);

    sqlite3_bind_text(stmt, 1, senderId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // update sender public key
    if (sqlite3_prepare_v2(db, "UPDATE User SET pkDer = ? WHERE identifier = ?;", -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    sqlite3_bind_blob(stmt, 1, pkDer.c_str(), pkDer.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, senderId.c_str(), -1, SQLITE_TRANSIENT);  

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // insert transfer request
    if (sqlite3_prepare_v2(db, 
        "INSERT INTO Transfer (sId, rId, sigDer, amount, timestamp) "
        "VALUES ((SELECT id FROM User WHERE identifier = ?), (SELECT id FROM User WHERE identifier = ?),  ?,  ?, ?);", -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // https://stackoverflow.com/questions/16043734/sqlite3-bind-text-sqlite-static-vs-sqlite-transient-for-c-string
    sqlite3_bind_text(stmt, 1, senderId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, receiverId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, sigDer.c_str(), sigDer.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, amount);
    sqlite3_bind_int64(stmt, 5, timestamp);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // end transaction
    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);

    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

void getTransferHistory(const std::string databasePath, const std::string identifier, std::vector<message::BalanceTransfer> &history) {

    sqlite3 *db;
    sqlite3_stmt* stmt;
    int rc;

    // open database
    if (sqlite3_open_v2(databasePath.c_str(), &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // enable WAL mode
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    // set query timeout
    sqlite3_exec(db, "PRAGMA busy_timeout = 30000;", NULL, NULL, NULL);

    // build transfers history
    if (sqlite3_prepare_v2(db, 
        "SELECT su.identifier, ru.identifier, su.pkDer, t.sigDer, t.amount, t.timestamp FROM Transfer AS t "
        "LEFT JOIN User AS su ON t.sId = su.id "
        "LEFT JOIN User AS ru ON t.rId = ru.id "
        "WHERE su.identifier = ? OR ru.identifier = ?;", -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }
        
    sqlite3_bind_text(stmt, 1, identifier.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, identifier.c_str(), -1, SQLITE_TRANSIENT);

    do {
        rc = sqlite3_step(stmt);
        switch (rc) {
            /** No more data */
            case SQLITE_DONE:
            break;
            /** New data */
            case SQLITE_ROW: {
            
                message::BalanceTransfer bt;
                bt.senderId.assign((char*)sqlite3_column_text(stmt, 0));
                bt.receiverId.assign((char*)sqlite3_column_text(stmt, 1));
                bt.pkDer.assign((char*)sqlite3_column_blob(stmt, 2), sqlite3_column_bytes(stmt, 2));
                bt.sigDer.assign((char*)sqlite3_column_blob(stmt, 3), sqlite3_column_bytes(stmt, 3));
                bt.amount = sqlite3_column_int(stmt, 4);
                bt.timestamp = sqlite3_column_int64(stmt, 5);
                history.push_back(bt);
            }
            break;
            default: break;
        }
    } 
    while (rc == SQLITE_ROW);

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

void writeToLog(const std::string databasePath, const std::string owner, int round, int64_t time, int64_t cnsTime, const std::string selfHash, const std::string gossipHash, const std::string payload) {

    sqlite3 *db;
    sqlite3_stmt* stmt;

    // open database
    if (sqlite3_open_v2(databasePath.c_str(), &db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // enable WAL mode
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    // set query timeout
    sqlite3_exec(db, "PRAGMA busy_timeout = 30000;", NULL, NULL, NULL);

    // log sql query
    if (sqlite3_prepare_v2(db, 
        "INSERT INTO Log (owner, round, time, cnsTime, selfHash, gossipHash, payload) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);", -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    sqlite3_bind_text(stmt, 1, owner.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, round);
    sqlite3_bind_int64(stmt, 3, time);
    sqlite3_bind_int64(stmt, 4, cnsTime);
    sqlite3_bind_text(stmt, 5, selfHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, gossipHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 7, payload.c_str(), payload.length(), SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

};
};
