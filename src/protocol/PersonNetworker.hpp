#ifndef HASHGRAPH_NETWORKER_HPP
#define HASHGRAPH_NETWORKER_HPP


#include <string>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>



#include "../message/Gossip.h"
#include "HashgraphNode.hpp"

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
         * @param ctx
         * @param port
         */
        static void *serverStarter(PersonNetworker *ctx, int port);   

    public:

        /**
         * List of network nodes
         */
        std::map<int, HashgraphNode> *nodes;

        /**
         * Mutex for shared resources
         */
        std::mutex mutex;

        /**
         * Constructor
         * 
         * @param nodes
         */
        PersonNetworker(std::map<int, HashgraphNode> *nodes);

        /**
         * Destructor
         */
        ~PersonNetworker();

        /**
         * Start the server for incoming messages
         * 
         * @param index
         */
        void startServer(int32_t index);

        /**
         * Gossip a message
         * 
         * @param gossiper
         * @param target
         * @param gossip
         */
        bool sendGossip(int32_t gossiper, int32_t target, std::vector<message::Data> const &gossip);

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