#ifndef HASHGRAPH_NETWORKER_HPP
#define HASHGRAPH_NETWORKER_HPP

#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>

#include "../types/Endpoint.hpp"
#include "../message/Gossip.h"

using namespace apache::thrift::server;

namespace hashgraph {
namespace protocol {

/**
 * PersonNetworker
 */
class PersonNetworker : virtual public message::GossipIf {

    private:

        /**
         * Required to solve a race condition
         */
        bool force_close;

        /**
         * Server
         */
        std::shared_ptr<TSimpleServer> server;

        /**
         * Server thread
         */
        std::shared_ptr<std::thread> thread;

        /**
         * Server starter function
         * 
         * @param ctx This context
         */
        static void *serverStarter(PersonNetworker *ctx);   

    public:

        /**
		 * Endpoint of this node
		 */
		types::Endpoint *ep;

        /**
         * Vector of hashgraph endpoints
         */
        std::vector<types::Endpoint*> *endpoints;

        /**
         * Mutex for shared resources
         */
        std::mutex mutex;

        /**
         * Constructor
         * 
         * @param ep
         * @param endpoints Vector of hashgraph endpoints
         */
        PersonNetworker(types::Endpoint *ep, std::vector<types::Endpoint*> *endpoints);

        /**
         * Destructor
         */
        ~PersonNetworker();

        /**
         * Start the server for incoming messages
         */
        void startServer();

        /**
         * Gossip a message
         * 
         * @param gossiper
         * @param target
         * @param gossip
         */
        bool sendGossip(types::Endpoint *gossiper, types::Endpoint *target, std::vector<message::Data> const &gossip);

        /**
		 * Handle incoming gossip data
		 *
		 * @param gossiper Creator index of the gossip data
		 * @param gossip Gossip data vector
         */
        virtual void recieveGossip(const int32_t gossiper, const std::vector<message::Data> &gossip);

        /**
         * Called on incoming transfer request
         * 
         * @param payload
         * @param target
         */
        virtual void transfer(const int32_t payload, const int32_t target);
};


};
};
#endif