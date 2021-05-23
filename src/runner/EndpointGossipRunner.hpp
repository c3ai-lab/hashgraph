#ifndef HASHGRAPH_RUNNER_ENDPOINT_GOSSIP_HPP
#define HASHGRAPH_RUNNER_ENDPOINT_GOSSIP_HPP

#include <string>
#include <thrift/concurrency/Thread.h>
#include "../types/Endpoint.hpp"
#include "../message/Hashgraph.h"

using namespace apache::thrift::concurrency;

namespace hashgraph {
namespace runner {

/**
 * Endpoint class
 */
class EndpointGossipRunner : public Runnable {

    private:

        /**
         * Target endpoint
         */
        types::Endpoint *endpoint;

        /**
         * Identifier of the sender node
         */
        const std::string identifier;

        /**
         * Gossip data to send
         */
        const std::vector<message::GossipData> gossipData;

    public:

        /**
         * Constructor
         */
        EndpointGossipRunner(types::Endpoint *endpoint, const std::string identifier, const std::vector<message::GossipData> &gossipData) :
            endpoint(endpoint),
            identifier(identifier),
            gossipData(gossipData) {}

        /**
         * Exchange gossip data
         */
        void run() {
            this->endpoint->exchangeGossipData(this->identifier, this->gossipData);
        }
    };

};
};
#endif
