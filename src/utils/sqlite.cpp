#ifndef HASHGRAPH_UTILS_SQLITE_HPP
#define HASHGRAPH_UTILS_SQLITE_HPP

#include <sstream>
#include <string>
#include <cstdint>
#include <vector>
#include "sqlite3.h"
#include "../message/Gossip.h"

namespace hashgraph {
namespace utils {

sqlite3* prepareDatabase(std::string databasePath) {

    sqlite3 *db;
    char *err = NULL;
    int rc;
    std::stringstream sql;

	// open database
	rc = sqlite3_open(databasePath.c_str(), &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return NULL;
	}

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
   
   rc = sqlite3_exec(db, sql.str().c_str(), NULL, NULL, &err);
   if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", err);
      sqlite3_free(err);
   }

   return db;
}

void closeDatabase(sqlite3 *db) {
   sqlite3_close(db);
}

void storeBalanceTransfer(sqlite3 *db, std::string sender, std::string receiver, int amount, int64_t timestamp) {

   int rc;
   std::stringstream sql;

   // start transaction
   rc = sqlite3_exec(db, "BEGIN TRANSACTION; ", NULL, NULL, NULL);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
      return;
   }

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

   sqlite3_stmt* stmt;
   rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
      return;
   }
   
   // https://stackoverflow.com/questions/16043734/sqlite3-bind-text-sqlite-static-vs-sqlite-transient-for-c-string
   sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 3, amount);
   sqlite3_bind_int64(stmt, 4, timestamp);

   rc = sqlite3_step(stmt);
   if (rc != SQLITE_DONE){
      printf("SQL error: %s \n", sqlite3_errmsg(db));
   }

   rc = sqlite3_finalize(stmt);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
   }

   // end transaction
   rc = sqlite3_exec(db, "END TRANSACTION; ", NULL, NULL, NULL);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
   }
}

void getTransferHistory(sqlite3 *db, std::string identifier, std::vector<message::BalanceTransfer> &history) {

   int rc;
   std::stringstream sql;

   sql << "SELECT su.identifier, ru.identifier, t.amount, t.timestamp FROM Transfer AS t ";
   sql << "LEFT JOIN User AS su ON t.sender = su.id ";
   sql << "LEFT JOIN User AS ru ON t.receiver = ru.id ";
   sql << "WHERE su.identifier = ? OR ru.identifier = ? ";

   sqlite3_stmt* stmt;
   rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
      return;
   }
      
   sqlite3_bind_text(stmt, 1, identifier.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, identifier.c_str(), -1, SQLITE_TRANSIENT);

   do {
      rc = sqlite3_step(stmt);
      switch(rc) {
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

   sqlite3_finalize(stmt);
}

void writeToLog(sqlite3 *db, std::string owner, int round, int64_t time, int64_t cnsTime, std::string selfHash, std::string gossipHash, std::string payload) {

   int rc;
   std::stringstream sql;

   sql << "INSERT INTO Log (owner, round, time, cnsTime, selfHash, gossipHash, payload) VALUES (?, ?, ?, ?, ?, ?, ?)";

   sqlite3_stmt* stmt;
   rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, 0);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
      return;
   }
      
   sqlite3_bind_text(stmt, 1, owner.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 2, round);
   sqlite3_bind_int64(stmt, 3, time);
   sqlite3_bind_int64(stmt, 4, cnsTime);
   sqlite3_bind_text(stmt, 5, selfHash.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 6, gossipHash.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 7, payload.c_str(), -1, SQLITE_TRANSIENT);

   rc = sqlite3_step(stmt);
   if (rc != SQLITE_DONE){
      printf("SQL error: %s \n", sqlite3_errmsg(db));
   }

   rc = sqlite3_finalize(stmt);
   if (rc != SQLITE_OK) {
      printf("SQL error: %s \n", sqlite3_errmsg(db));
   }
}

};
};
#endif
