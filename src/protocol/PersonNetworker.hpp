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
		message::Endpoint ep;

        /**
         * Vector of hashgraph endpoints
         */
        std::vector<message::Endpoint> *endpoints;

        /**
         * Mutex for shared resources
         */
        std::mutex mutex;

        /**
         * Constructor
         * 
         * @param endpoints Vector of hashgraph endpoints
         */
        PersonNetworker(message::Endpoint const &ep, std::vector<message::Endpoint> *endpoints);

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
        bool sendGossip(message::Endpoint const &gossiper, message::Endpoint const &target, std::vector<message::Data> const &gossip);

        /**
         * Called on incoming data
         * 
         * @param gossiper
         * @param gossip
         */
        virtual void recieveGossip(const int32_t gossiper, const std::vector<message::Data> &gossip);
};


};
};
#endif