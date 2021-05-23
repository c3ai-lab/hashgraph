#ifndef HASHGRAPH_PROTOCOL_PERSON_NETWORKER_HPP
#define HASHGRAPH_PROTOCOL_PERSON_NETWORKER_HPP

#include <string>
#include <map>
#include <algorithm>
#include <memory>
#include <thread>
#include <cstdint>
#include <thrift/server/TThreadPoolServer.h>
#include "SuperPerson.hpp"
#include "../types/Endpoint.hpp"
#include "../message/Hashgraph.h"

using namespace apache::thrift::server;

namespace hashgraph {
namespace protocol {

/**
 * PersonNetworker
 */
class PersonNetworker : virtual public SuperPerson, virtual public message::HashgraphIf {

    private:

        /**
         * Required to resolve a race condition
         */
        bool force_close;

        /**
         * Thread pool based server
         */
        std::shared_ptr<TThreadPoolServer> server;

        /**
         * Listener thread for new connections
         */
        std::shared_ptr<std::thread> thread;

        /**
         * Private key in PEM format
         */
        std::string privKeyPEM;

        /**
         * Certificate in PEM format
         */
		std::string certifficatePEM;

        /**
         * Server starter function
         * 
         * @param ctx This context
         * @param port Port under which the server is accessable
         */
        static void *serverStarter(PersonNetworker *ctx, int port);   

    public:

        /**
         * Constructor
         * 
         * @param privKeyPath
         * @param certPath
         */
        PersonNetworker(const std::string privKeyPath, const std::string certPath);

        /**
         * Destructor
         */
        ~PersonNetworker();

        /**
         * Start the server for incoming messages
         * 
         * @param port Port under which the server is accessable
         */
        void startServer(int port);

        /**
         * Generates a challenge
         * 
         * @param _return
         */
        void challenge(std::string& _return);

        /**
         * Returns the user balance
         * 
         * @param ownerId
         */
        int32_t balance(const std::string& ownerId);

        /**
         * Returns the user balance history
         * 
         * @param _return
         * @param ownerId
         */
        void balance_history(std::vector<message::BalanceTransfer> & _return, const std::string& ownerId);

        /**
		 * Receives a gossip from another node
		 *
		 * @param gossiper Creator identifier of the gossip data
		 * @param gossip Gossip data vector
         */
        virtual void receiveGossip(const std::string& gossiper, const std::vector<message::GossipData> &gossip) = 0;

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
};

};
};
#endif