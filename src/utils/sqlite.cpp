#include <sstream>
#include <string>
#include <cstdint>
#include <vector>
#include <stdio.h>
#include "sqlite3.h"
#include "../message/Hashgraph.h"

namespace hashgraph {
namespace utils {

void createDatabaseTables(const std::string databasePath) {
    
    sqlite3 *db;
    std::stringstream sql;

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
    sql << "CREATE TABLE IF NOT EXISTS Log ( ";
    sql << " id         INTEGER       PRIMARY KEY ";
    sql << "                          NOT NULL, ";
    sql << " owner      VARCHAR (100) NOT NULL, ";
    sql << " round      INTEGER       NOT NULL, ";
    sql << " time       INTEGER       NOT NULL, ";
    sql << " cnsTime    INTEGER       NOT NULL, ";
    sql << " selfHash   VARCHAR (100) NOT NULL, ";
    sql << " gossipHash VARCHAR (100) NOT NULL, ";
    sql << " payload    VARCHAR (100)); ";

    sql << "CREATE TABLE IF NOT EXISTS User ( ";
    sql << "id         INTEGER       PRIMARY KEY ";
    sql << "                         NOT NULL, ";
    sql << "identifier VARCHAR (100) NOT NULL ";
    sql << "                         UNIQUE ON CONFLICT IGNORE); ";

    sql << "CREATE TABLE IF NOT EXISTS Transfer ( ";
    sql << "id        INTEGER PRIMARY KEY ";
    sql << "                  NOT NULL, ";
    sql << "sender    INTEGER REFERENCES User (id) ";
    sql << "                  NOT NULL, ";
    sql << "receiver  INTEGER REFERENCES User (id) ";
    sql << "                  NOT NULL, ";
    sql << "amount    INTEGER NOT NULL, ";
    sql << "timestamp INTEGER NOT NULL); ";

    sqlite3_exec(db, sql.str().c_str(), NULL, NULL, NULL);
    
    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

void storeTransferData(const std::string databasePath, const std::string sender, const std::string receiver, int amount, int64_t timestamp) {

    sqlite3 *db;
    sqlite3_stmt* stmt;
    std::stringstream sql;

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
    sqlite3_exec(db, "BEGIN TRANSACTION; ", NULL, NULL, NULL);

    // insert sender if not existent
    sql << "INSERT INTO User (identifier) VALUES ('" << sender << "'); ";
    sqlite3_exec(db, sql.str().c_str(), NULL, NULL, NULL);
    sql.str("");
    sql.clear();

    // insert receiver if not existent
    sql << "INSERT INTO User (identifier) VALUES ('" << receiver << "'); ";
    sqlite3_exec(db, sql.str().c_str(), NULL, NULL, NULL);
    sql.str("");
    sql.clear();

    // insert transfer
    sql << "INSERT INTO Transfer (sender, receiver, amount, timestamp) VALUES ( ";
    sql << "(SELECT id FROM User WHERE identifier = ?), (SELECT id FROM User WHERE identifier = ?),  ?,  ?); ";

    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // https://stackoverflow.com/questions/16043734/sqlite3-bind-text-sqlite-static-vs-sqlite-transient-for-c-string
    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, amount);
    sqlite3_bind_int64(stmt, 4, timestamp);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }

    // end transaction
    sqlite3_exec(db, "END TRANSACTION; ", NULL, NULL, NULL);

    // close database
    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "Can't close database\n");
    }
}

void getTransferHistory(const std::string databasePath, const std::string identifier, std::vector<message::BalanceTransfer> &history) {

    sqlite3 *db;
    sqlite3_stmt* stmt;
    std::stringstream sql;
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
    sql << "SELECT su.identifier, ru.identifier, t.amount, t.timestamp FROM Transfer AS t ";
    sql << "LEFT JOIN User AS su ON t.sender = su.id ";
    sql << "LEFT JOIN User AS ru ON t.receiver = ru.id ";
    sql << "WHERE su.identifier = ? OR ru.identifier = ? ";

    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0) != SQLITE_OK) {
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
                bt.senderId   = (char*)sqlite3_column_text(stmt, 0);
                bt.receiverId = (char*)sqlite3_column_text(stmt, 1);
                bt.amount     = sqlite3_column_int(stmt, 2);
                bt.timestamp  = sqlite3_column_int64(stmt, 3);
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
    std::stringstream sql;

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
    sql << "INSERT INTO Log (owner, round, time, cnsTime, selfHash, gossipHash, payload) VALUES (?, ?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0) != SQLITE_OK) {
        printf("SQL error: %s \n", sqlite3_errmsg(db));
    }
        
    sqlite3_bind_text(stmt, 1, owner.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, round);
    sqlite3_bind_int64(stmt, 3, time);
    sqlite3_bind_int64(stmt, 4, cnsTime);
    sqlite3_bind_text(stmt, 5, selfHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, gossipHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, payload.c_str(), -1, SQLITE_TRANSIENT);

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
