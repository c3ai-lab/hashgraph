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
         * DER encoded public key
         */
        std::string pkDer;

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
         * @param packet Gossip data
         * @param sigDer DER encoded signature of the packet
         */
        void exchangeGossipData(const message::GossipPacket packet, const std::string sigDer);

        /**
         * Get bootstrap data from remote node
         * 
         * @param _return
         * @param identifier
         */
        void bootstrap(message::BootstrapPacket &_return, const std::string &identifier);

        /**
         * Return the nodes' identifier
         */
        std::string getIdentifier() const;   
};

};
};
#endif
