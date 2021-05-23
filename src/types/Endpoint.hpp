#ifndef HASHGRAPH_TYPES_ENDPOINT_HPP
#define HASHGRAPH_TYPES_ENDPOINT_HPP

#include <string>
#include <memory>
#include <mutex>
#include "../message/Hashgraph.h"

namespace hashgraph {
namespace types {

/**
 * Endpoint class
 */
class Endpoint {

    private:

        /**
         * Unique identifier of the node
         */
		std::string identifier;

        /**
         * Limits access to gossiping client
         */
        std::mutex gspMutex;

        /**
         * Client to communicate
         */
        std::unique_ptr<message::HashgraphClient> client;

	public:

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
        void exchangeGossipData(const std::string senderId, const std::vector<message::GossipData> &gossipData);

        /**
         * Return the nodes' identifier
         */
		std::string getIdentifier() const;
};

};
};
#endif
