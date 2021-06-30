#ifndef HASHGRAPH_PROTOCOL_PERSON_HPP
#define HASHGRAPH_PROTOCOL_PERSON_HPP

#include <list>
#include <vector>
#include <string>
#include <atomic>
#include <queue>
#include <mutex>
#include <unistd.h>
#include "PersonNetworker.hpp"
#include "PersonApplication.hpp"
#include "../types/Endpoint.hpp"
#include "../message/Hashgraph.h"

#ifndef MAKE_FORKS
#define MAKE_FORKS 0
#endif

namespace hashgraph {
namespace protocol {

class Event;

/**
 * Person class
 */
class Person : public PersonNetworker, public PersonApplication {

    private:

        /**
         * Buffer for transfers to process
         */
        std::queue<message::GossipPayload> transferRequests;

        /**
         * Hashgraph
         */
        std::vector<Event*> hashgraph;

        /**
         * Protects the hashgraph vector
         */
        std::mutex hgMutex;

        /**
         * Current round of this node
         */
        int currentRound;

        /**
         * Create an event
         * 
         * @param gossiper Index of the gossiper
         */
        void createEvent(std::string gossiper);

        /**
         * Get the most recent event in the hashgraph that was created by the person with the given index
         *
         * @param identifier Person identifier for the event to search
         */
        Event *getTopNode(std::string identifier) const;

        /**
         * THIS FUNCTION IS ONLY TO TEST FORKS, DO NOT USE THIS UNLESS YOU WANT TO CHEAT
         *
         * @param target
         */
        Event *getForkNode(Person const &target) const;

    public:

        /**
         * Remote endpoints
         */
        std::vector<types::Endpoint*> *endpoints;

        /**
         * Constructor
         * 
         * @param skPath Path of the secret key file
         * @param certPath Path of the certificate file
         * @param endpoints Vector of hashgraph endpoints
         */
        Person(const std::string skPath, const std::string certPath, std::vector<types::Endpoint*> *endpoints);

        /**
         * Destructor
         */
        ~Person();

        /**
         * Assignment operator
         */
        Person&	operator=(Person const &p);

        /**
         * Compare two persons by their index
         *
         * @param rhs Person to compare agaist
         */
        bool operator==(Person const &rhs);

        /**
         * Returns the hashgraph
         * 
         * @return The hashgraph object
         */
        const std::vector<Event*> &getHashgraph() const;

        /**
         * 
         */
        void findOrder();

        /**
         * Determine consensus timestamp and update worth of persons
         *
         * @param n 
         * @param r
         * @param w
         */
        int finalizeOrder(std::size_t n, int const &r, std::vector<Event*> const &w);

        /**
         * Set SelfParent and GossiperParent references for every event in the vector
         *
         * @param nEvents Vector of events to find references for
         */
        void linkEvents(std::vector<Event*> const &nEvents);

        /**
         * Find all witnisses of the given round
         *
         * @param round Round to check against
         */ 
        std::vector<Event*>	findWitnesses(int const &round) const;

        /**
         * Starts the gossiping process
         *
         * @param interval Interval to wait after each gossip
         * @param quit Flag that indicates when to stop
         */ 
        void startGossip(int interval, const std::atomic<bool> *quit);

        /**
         * Gets the current round
         * 
         * @return The current round
         */
        int getCurRound() const;

        /**
         * Increase the current round
         */
        void incCurRound();

        /**
         * Remove outdated and insignificant events from the hashgraph
         */
        void removeOldBalls();

        /**
         * Handle incoming gossip data
         *
         * @param packet Gossip data packet
         * @param sigDer Byte string of the DER encoded packet
         */
        void receiveGossip(const message::GossipPacket &packet, const std::string &sigDer);

        /**
         * Transfer request from a user
         * 
         * @param ownerPkDer
         * @param amount
         * @param receiverId
         * @param challenge
         * @param sigDer
         */
        void crypto_transfer(const std::string &ownerPkDer, const int32_t amount, const std::string &receiverId, const std::string &challenge, const std::string &sigDer);

        /**
         * Bootstrap a remote node
         * 
         * @param _return
         * @param identifier
         */
        void bootstrap(message::BootstrapPacket &_return, const std::string &identifier);
};

};
};
#endif
