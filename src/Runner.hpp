#ifndef HASHGRAPH_RUNNER_HPP
#define HASHGRAPH_RUNNER_HPP

#include <map>
#include <vector>
#include <string>

#include "protocol/Person.hpp"
#include "message/Gossip.h"

#ifndef DEBUG
#define DEBUG 0
#endif

namespace hashgraph {

/**
 * Runner 
 */
class Runner {

    private:

        /**
         * Application settings
         */
        message::AppSettings settings;

        /**
         * List of locally running nodes
         */
        std::vector<protocol::Person*> persons;

    public:
        /**
         * Constructor
         */
        Runner();

        /**
         * Destructor
         */
        ~Runner();

        /**
         * Initializes the hashgraph runner
         */
        void initHashgraphRunner(std::string configFile);

        /**
         * Start the hashgraph protocol
         */
        void startHashgraph(int interval);
};

};
#endif