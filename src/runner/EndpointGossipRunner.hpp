#ifndef HASHGRAPH_RUNNER_ENDPOINT_GOSSIP_HPP
#define HASHGRAPH_RUNNER_ENDPOINT_GOSSIP_HPP

#include <string>
#include <sstream>
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
         * Gossip packet
         */
        const message::GossipPacket packet;

        /**
         * PEM encoded secret key
         */
        const std::string skPEM;

    public:

        /**
         * Constructor
         */
        EndpointGossipRunner(types::Endpoint *endpoint, const message::GossipPacket packet, const std::string skPEM) :
            endpoint(endpoint),
            packet(packet),
            skPEM(skPEM) {}

        /**
         * Exchange gossip data
         */
        void run() {

            // serialize packet data
            std::stringstream stream;
            packet.printTo(stream);

            // send gossip data
            this->endpoint->exchangeGossipData(this->packet, utils::ecdsaSignMessage(this->skPEM, stream.str()));
        }
};

};
};
#endif
