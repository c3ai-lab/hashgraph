#ifndef HASHGRAPH_RUNNER_HPP
#define HASHGRAPH_RUNNER_HPP

#include <map>
#include <vector>
#include <string>

#include "types/Endpoint.hpp"
#include "types/User.hpp"
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

        /**
         * Network user
         */
        std::vector<types::User*> users;

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