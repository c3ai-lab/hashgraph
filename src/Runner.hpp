#ifndef HASHGRAPH_RUNNER_HPP
#define HASHGRAPH_RUNNER_HPP

#include <map>
#include <vector>
#include <string>
#include <signal.h>
#include <stdlib.h>
#include "types/Endpoint.hpp"
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
         * Remote endpoints
         */
        std::vector<types::Endpoint*> endpoints;

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
         * Start the protocol execution
         */
        void runHashgraphProtocol(const std::string configPath);
};

};
#endif