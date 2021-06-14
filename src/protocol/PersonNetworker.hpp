#ifndef HASHGRAPH_PROTOCOL_PERSON_NETWORKER_HPP
#define HASHGRAPH_PROTOCOL_PERSON_NETWORKER_HPP

#include <string>
#include <cstdint>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/Monitor.h>
#include <thrift/server/TThreadPoolServer.h>
#include "SuperPerson.hpp"
#include "../types/Endpoint.hpp"
#include "../message/Hashgraph.h"

using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

namespace hashgraph {
namespace protocol {

/**
 * PersonNetworker
 */
class PersonNetworker : virtual public SuperPerson, virtual public message::HashgraphIf {

    private:

        /**
         * Thread pool based server
         */
        std::shared_ptr<TThreadPoolServer> server;

        /**
         * Listener thread for new connections
         */
        std::shared_ptr<Thread> thread;

    protected:

        /**
         * PEM encoded secret key
         */
        std::string skPEM;

        /**
         * PEM encoded certificate
         */
        std::string certPEM;

    public:

        /**
         * Constructor
         * 
         * @param skPath
         * @param certPath
         */
        PersonNetworker(const std::string skPath, const std::string certPath);

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
         * @param packet Gossip data packet
         * @param sigDer Byte string of the DER encoded packet
         */
        virtual void receiveGossip(const message::GossipPacket &packet, const std::string &sigDer) = 0;

        /**
         * Transfer request from a user
         * 
         * @param ownerPkDer
         * @param amount
         * @param receiverId
         * @param challenge
         * @param sigDer
         */
        virtual void crypto_transfer(const std::string &ownerPkDer, const int32_t amount, const std::string &receiverId, const std::string &challenge, const std::string &sigDer) = 0;
};

};
};
#endif
