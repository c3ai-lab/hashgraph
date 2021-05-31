#ifndef HASHGRAPH_RUNNER_SQLITE_TRANSFER_HPP
#define HASHGRAPH_RUNNER_SQLITE_TRANSFER_HPP

#include <string>
#include <cstdint>
#include <thrift/concurrency/Thread.h>
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift::concurrency;

namespace hashgraph {
namespace runner {

/**
 * Endpoint class
 */
class SQLiteTransferRunner : public Runnable {

    private:

        /**
         * Path of the database file
         */
        const std::string dbPath;

        /**
         * Transfer receiver
         */
        const std::string receiver;

        /**
         * Byte string of the DER encoded public key
         */
        const std::string pkDer;

        /**
         * Byte string of the DER encoded signature
         */
        const std::string sigDer;

        /**
         * Transfer amount
         */
        const int amount;

        /**
         * Transfer time
         */
        const int64_t timestamp;

    public:

        /**
         * Constructor
         */
        SQLiteTransferRunner(const std::string dbPath, const std::string receiver, const std::string pkDer, const std::string sigDer, const int amount, const int64_t timestamp) :
            dbPath(dbPath),
            receiver(receiver),
            pkDer(pkDer),
            sigDer(sigDer),
            amount(amount),
            timestamp(timestamp) {};

        /**
         * Write transfer entry to database
         */
        void run() {

            // sender identifier from public key 
            const std::string identifier = utils::encodeIdentifier(this->pkDer);

            // write to database
            utils::storeTransferData(this->dbPath, identifier, this->receiver, this->pkDer, this->sigDer, this->amount, this->timestamp);
        }
    };

};
};
#endif
