#ifndef HASHGRAPH_TYPES_ENDPOINT_HPP
#define HASHGRAPH_TYPES_ENDPOINT_HPP

#include <string>
#include <memory>
#include "../message/Gossip.h"

namespace hashgraph {
namespace types {

/**
 * Endpoint class
 */
class Endpoint {

    private:

        /**
         * Client to communicate
         */
        std::unique_ptr<message::GossipClient> client;

	public:

        /**
         * Unique identifier of the node
         */
		std::string identifier;

        /**
         * Constructor
         */
		Endpoint(const std::string host, int port, const std::string certPath);

        /**
         * Destructor
         */
		~Endpoint();

        /**
         * Exchange gossip data with other nodes
         * 
         * @param senderId
         * @param gossipData
         */
        bool exchangeGossipData(const std::string senderId, const std::vector<message::Data> &gossipData);
};

};
};
#endif
