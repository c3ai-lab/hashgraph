#ifndef HASHGRAPH_PROTOCOL_EVENT_HPP
#define HASHGRAPH_PROTOCOL_EVENT_HPP

#include "../message/Gossip.h"

#include "Person.hpp"
#include <unordered_set>
#include <algorithm>    // std::find
#include <array>

namespace hashgraph {
namespace protocol {


class Person;

/**
 * Event class
 */
class Event{

	private:
		std::vector<Event*> &graph;
		std::unordered_set<std::string> ancestorsSeen;
		std::unordered_set<std::string> ancestorsNotSeen;
		std::unordered_set<std::string> hashesSeen;
		std::unordered_set<std::string> hashesNotSeen;
		std::string hash;
		message::Data d;
		Event *selfParent;
		Event *gossiperParent;
		int64_t consensusTimestamp;
		int roundRecieved;
		int round;
		bool witness;
		char famous;

		/**
		 * Hash this event
		 */
		std::string makeHash();
		
	public:
		// signature
		char *sigR;
		char *sigS;

		/**
		 * Constructor
		 */
		Event(Person &p, message::Data const &data);

		/**
		 * Destructor
		 */
		~Event();
		
		/**
		 * Compare the event against the given one
		 * 
		 * @param rhs
		 */
		bool operator==(Event const &rhs) const;

		/**
		 * Check whether this event is an ancestor of the given one
		 * 
		 * @param y Event to check against
		 */
		bool ancestor(Event const &y);

		/**
		 * Recursively check ancestors
		 * 
		 * @param y
		 * @param done
		 * @param visited
		 */
		bool ancestorRecursion(Event const &y, bool &done, std::vector<Event*> &visited);

		/**
		 * Checks whether the given event was already by this event
		 * 
		 * @param y The event to check
		 */
		bool see(Event const &y);

		/**
		 * Recursively check whether the given event was seen
		 * 
		 * @param y
		 * @param forkCheck
		 * @param done
		 * @param visited
		 */
		bool seeRecursion(Event const &y, std::vector<Event*> &forkCheck, bool &done, std::vector<Event*> &visited);

		/**
		 * Returns the witness
		 */
		bool getWitness() const;

		/**
		 * Sets the famous flag
		 * 
		 * @param fame
		 */
		void setFamous(char const &fame);

		/**
		 * Sets the round received
		 * 
		 * @param r
		 */
		void setRoundReceived(int const &r);

		/**
		 * Sets the consensus timestamp
		 * 
		 * @param t
		 */
		void setConsensusTimestamp(int64_t t);
		
		/**
		 * Sets the self parent
		 * 
		 * @param e
		 */
		void setSelfParent(Event* const e);

		/**
		 * Sets the gossiper parent
		 * 
		 * @param e
		 */
		void setGossiperParent(Event* const e);

		/**
		 * Returns the famous flag
		 */
		char getFamous() const;

		/**
		 * Returns the round
		 */
		int getRound() const;

		/**
		 * Returns the consensus timestamp
		 */
		int64_t getConsensusTimestamp() const;

		/**
		 * Returns the round received
		 */
		int getRoundRecieved() const;

		/**
		 * Returns the self parent
		 */
		Event *getSelfParent() const;

		/**
		 * Returns the gossiper parent
		 */
		Event *getGossiperParent() const;

		/**
		 * Returns the data
		 */
		const message::Data	&getData() const;

		/**
		 * Returns the event hash
		 */
		std::string getHash() const;

		/**
		 * Returns the graph
		 */
		std::vector<Event*>	&getGraph() const;

		/**
		 * Checks wether x is a fork of y
		 * 
		 * @param x
		 * @param y
		 */
		static bool fork(Event *x, Event *y);

		/**
		 * Devides rounds
		 * 
		 * @param person
		 */
		void divideRounds(Person &person);

		/**
		 * Decides whether the event is famous
		 * 
		 * @param person
		 */
		void decideFame(Person &person);

		/**
		 * Checks whether the given event was seen by events from a super majority of other nodes
		 * 
		 * @param y Event to check against
		 * @param numNodes Number of persons in the network
		 */
		bool stronglySee(Event const &y, std::size_t numNodes);
};

/**
 * Operator << implementation (used for hashing)
 *
 * @param os
 * @param e
 */
std::ostream& operator<<(std::ostream& os, const Event& e);


};
};
#endif
