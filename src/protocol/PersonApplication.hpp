#ifndef HASHGRAPH_APPLICATION_HPP
#define HASHGRAPH_APPLICATION_HPP

#include <string>
#include <cstdint>
#include <vector>

#include "../types/Transfer.hpp"

namespace hashgraph {
namespace protocol {

/**
 * PersonApplication
 */
class PersonApplication {

    public:

        /**
         * Transfer history
         */
        std::vector<types::Transfer> history;

        /**
         * Updates the wealth for a user
         */
        void storeAmountTransfer(const std::string senderId, const std::string receiverId, int32_t amount, int64_t timestamp) ;

        /**
         * Returns the wealth for a user
         */
        int32_t getAmountForUser(const std::string identifier);
};

};
};
#endif