#ifndef HASHGRAPH_RUNNER_HPP
#define HASHGRAPH_RUNNER_HPP

#include <map>
#include <vector>
#include <string>

#include "protocol/Person.hpp"
#include "protocol/HashgraphNode.hpp"

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
         * List of hashgraph nodes in the network
         */
        std::map<int, protocol::HashgraphNode> nodes;

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
         * Add a node to the list
         */
        void addHashgraphNode(int index, int isLocal, std::string address, int port);

        /**
         * Start the hashgraph protocol
         */
        void startHashgraph(int interval);
};

};
#endif