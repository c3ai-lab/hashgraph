#ifndef HASHGRAPH_PROTOCOL_PERSON_HPP
#define HASHGRAPH_PROTOCOL_PERSON_HPP

#include <list>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <ctime>

#include "Event.hpp"
#include "PersonNetworker.hpp"
#include "../message/Gossip.h"


#ifndef WRITE_LOG
#define WRITE_LOG 0
#endif

#ifndef MAKE_FORKS
#define MAKE_FORKS 0
#endif

namespace hashgraph {
namespace protocol {


class Event;

/**
 * Person class
 */
class Person : public PersonNetworker {
	
	private:
		/**
         * Counts the number of created events
         */
		static int64_t dataSequenceNumber;

		/**
         * Hashgraph
         */
		std::vector<Event*> hashgraph;

		/**
         * Current round of this node
         */
		int currentRound;

		/**
         * Output stream used for logging
         */
		std::ofstream ofs;

		/**
		 * Create an event
		 * 
		 * @param gossiper Index of the gossiper
		 */
		void createEvent(int32_t gossiper);

		/**
		 * Get the most recent event in the hashgraph that was created by the person with the given index
		 *
		 * @param index Person index for the event to search
		 */
		Event *getTopNode(int32_t index) const;

		/**
		 * THIS FUNCTION IS ONLY TO TEST FORKS, DO NOT USE THIS UNLESS YOU WANT TO CHEAT
		 *
		 * @param target
		 */
		Event *getForkNode(Person const &target) const;

	public:
	
		/**
		 * Index of this node
		 */
		int index;

		/**
		 * Contains the worth for every node
		 */
		std::vector<int64_t> networth;

		/**
		 * Constructor
		 * 
		 * @param ind Index of the person
		 * @param nodes List of all network nodes
		 */
		Person(int const &ind, std::map<int, HashgraphNode> *nodes);

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
		 * Prints event data for a hashgraph event
		 *
		 * @param n Index in the hashgraph vector
		 */
		void outputOrder(std::size_t n);

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
		 * Collect new events from the hashgraph and gossip them to another person
		 *
		 * @param target Index from the target person
		 */
		void gossip(int32_t target);

		/**
		 * Handle incoming gossip data
		 *
		 * @param gossiper Creator index of the gossip data
		 * @param gossip Gossip data vector
		 */
		void recieveGossip(const int32_t gossiper, const std::vector<message::Data> &gossip);

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
