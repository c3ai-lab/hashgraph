#ifndef HASHGRAPH_PROTOCOL_PERSON_APPLICATION_HPP
#define HASHGRAPH_PROTOCOL_PERSON_APPLICATION_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "sqlite3.h"

#include "../message/Hashgraph.h"

namespace hashgraph {
namespace protocol {

class Event;

/**
 * PersonApplication
 */
class PersonApplication {

    private:

        /**
         * Database handle
         */
        sqlite3* database;

        /**
         * Indicate whether all commited should be logged
         */
        bool logEvents;

    public:

        /**
         * Store a balance transfer
         * 
         * @param event The event that contains the transfer
         */
        void storeBalanceTransfer(const protocol::Event *event) ;

        /**
         * Return the user balance
         * 
         * @param identifier Identifier of the user
         */
        int32_t getUserBalance(const std::string identifier);

        /**
         * Returns the user balance history
         * 
         * @param identifier
         * @param history
         */
        void getUserBalanceHistory(const std::string identifier, std::vector<message::BalanceTransfer> &history);

        /**
         * Log a committed event
         *
         * @param event The event to log
         */
        void writeEventToLog(const protocol::Event *event);

		/**
		 * Constructor
		 * 
		 * @param databasePath Path of the database file
         * @param logEvents Write committed events to the log
		 */
		PersonApplication(const std::string databasePath, bool logEvents);

		/**
		 * Destructor
		 */
		~PersonApplication();
};

};
};
#endif