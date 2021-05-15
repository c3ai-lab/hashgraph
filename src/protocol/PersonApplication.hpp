#ifndef HASHGRAPH_APPLICATION_HPP
#define HASHGRAPH_APPLICATION_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "sqlite3.h"
#include "../message/Gossip.h"

namespace hashgraph {
namespace protocol {

/**
 * PersonApplication
 */
class PersonApplication {

    private:

        /**
         * Database handle
         */
        sqlite3* database;

    public:

        /**
         * Store a balance transfer
         */
        void storeBalanceTransfer(const std::string senderId, const std::string receiverId, int32_t amount, int64_t timestamp) ;

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
		 * Constructor
		 * 
		 * @param databasePath Path of the database file
		 */
		PersonApplication(const std::string databasePath);

		/**
		 * Destructor
		 */
		~PersonApplication();
};

};
};
#endif