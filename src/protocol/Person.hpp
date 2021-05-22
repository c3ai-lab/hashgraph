#ifndef HASHGRAPH_PROTOCOL_PERSON_HPP
#define HASHGRAPH_PROTOCOL_PERSON_HPP

#include <list>
#include <vector>
#include <string>
#include <atomic>
#include <queue>
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
		 * Collect new events from the hashgraph and gossip them to another person
		 *
		 * @param target Target endpoint to gossip
		 */
		void gossip(types::Endpoint *target);

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
		 * @param databasePath Path of the database file
		 * @param privKeyPath Path of the private key file
		 * @param certPath Path of the certificate file
		 * @param logEvents Write committed events to the log
		 * @param endpoints Vector of hashgraph endpoints
		 */
		Person(const std::string databasePath, const std::string privKeyPath, const std::string certPath, bool logEvents, std::vector<types::Endpoint*> *endpoints);

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
		 * Add a new event to the hashgraph
		 * 
		 * @param event Event to add
		 */
		void insertEvent(Event const &event);

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
		 * Handle incoming gossip data
		 *
		 * @param gossiper Creator identifier of the gossip data
		 * @param gossip Gossip data vector
		 */
		void receiveGossip(const std::string& gossiper, const std::vector<message::GossipData> &gossip);

        /**
         * Transfer request from a user
         * 
         * @param ownerPkDer
		 * @param amount
         * @param receiverId
         * @param challenge
         * @param sigDer
         */
		void crypto_transfer(const std::string& ownerPkDer, const int32_t amount, const std::string& receiverId, const std::string& challenge, const std::string& sigDer);

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
         * Generates a challenge
         * 
         * @param _return
         */
        void challenge(std::string& _return);

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

};

};
};
#endif
