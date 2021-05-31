#ifndef HASHGRAPH_RUNNER_SQLITE_LOG_HPP
#define HASHGRAPH_RUNNER_SQLITE_LOG_HPP

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
class SQLiteLogRunner : public Runnable {

    private:

        /**
         * Path of the database file
         */
        const std::string dbPath;

        /**
         * Event owner
         */
        const std::string owner;

        /**
         * Event round
         */
        const int round;

        /**
         * Event time
         */
        const int64_t time;

        /**
         * Consensus timestamp
         */
        const int64_t cnsTime;

        /**
         * Event self hash
         */
        const std::string selfHash;

        /**
         * Event gossip hash
         */
        const std::string gossipHash;

        /**
         * Event payload
         */
        const std::string payload;

    public:

        /**
         * Constructor
         */
        SQLiteLogRunner(const std::string dbPath, const std::string owner, const int round, const int64_t time, const int64_t cnsTime, const std::string selfHash, const std::string gossipHash, const std::string payload) :
            dbPath(dbPath),
            owner(owner),
            round(round),
            time(time),
            cnsTime(cnsTime),
            selfHash(selfHash),
            gossipHash(gossipHash),
            payload(payload) {};

        /**
         * Write to log to database
         */
        void run() {
            utils::writeToLog(this->dbPath, this->owner, this->round, this->time, this->cnsTime, this->selfHash, this->gossipHash, this->payload);
        }
};

};
};
#endif
