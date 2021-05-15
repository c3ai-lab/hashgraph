#ifndef HASHGRAPH_NETWORKER_HPP
#define HASHGRAPH_NETWORKER_HPP

#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <cstdint>
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
        static void *serverStarter(PersonNetworker *ctx, int port);   

    public:

        /**
         * Public identifier
         */
        std::string identifier;

        /**
         * Private key in PEM format
         */
        std::string privKeyPEM;

        /**
         * Certificate in PEM format
         */
		std::string certifficatePEM;

        /**
         * Mutex for shared resources
         */
        std::mutex mutex;

        /**
         * Constructor
         */
        PersonNetworker(const std::string privKeyPat, const std::string certPath);

        /**
         * Destructor
         */
        ~PersonNetworker();

        /**
         * Start the server for incoming messages
         */
        void startServer(int port);
  
        /**
         * Gossip a message
         * 
         * @param target
         * @param gossipData
         */
        bool sendGossip(types::Endpoint *target, const std::vector<message::Data> &gossipData);

        /**
		 * Receives a gossip from another node
		 *
		 * @param gossiper Creator identifier of the gossip data
		 * @param gossip Gossip data vector
         */
        virtual void receiveGossip(const std::string& gossiper, const std::vector<message::Data> &gossip) = 0;

        /**
         * Transfer request from a user
         * 
         * @param ownerPkDer
		 * @param amount
         * @param receiverId
         * @param challenge
         * @param sigDer
         */
        virtual void crypto_transfer(const std::string& ownerPkDer, const int32_t amount, const std::string& receiverId, const std::string& challenge, const std::string& sigDer) = 0;

        /**
         * Returns the user balance
         * 
         * @param ownerId
         */
        virtual int32_t balance(const std::string& ownerId) = 0;

        /**
         * Returns the user balance history
         * 
         * @param _return
         * @param ownerId
         */
        virtual void balance_history(std::vector<message::BalanceTransfer> & _return, const std::string& ownerId) = 0;

};

};
};
#endif