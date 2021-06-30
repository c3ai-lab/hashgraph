#ifndef HASHGRAPH_PROTOCOL_EVENT_HPP
#define HASHGRAPH_PROTOCOL_EVENT_HPP

#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <array>
#include "../message/Hashgraph.h"
#include "../types/Endpoint.hpp"

namespace hashgraph {
namespace protocol {

class Person;

/**
 * Event class
 */
class Event {

    private:

        /**
         * Hashgraph reference
         */
        std::vector<Event*> &graph;

        /**
         * Ancestors seen
         */
        std::unordered_set<std::string> ancestorsSeen;

        /**
         * Ancestors not seen
         */
        std::unordered_set<std::string> ancestorsNotSeen;

        /**
         * Hashes seen
         */
        std::unordered_set<std::string> hashesSeen;

        /**
         * Hashes not seen
         */
        std::unordered_set<std::string> hashesNotSeen;

        /**
         * Gossip data
         */
        message::GossipData d;

        /**
         * Parent event
         */
        Event *selfParent;

        /**
         * Gossip event
         */
        Event *gossiperParent;

        /**
         * Event consensus timestamp
         */
        int64_t consensusTimestamp;

        /**
         * Round when the event was received
         */
        int roundRecieved;

        /**
         * Event round
         */
        int round;

        /**
         * Flag that indicates whether this event is a witness
         */
        bool witness;

        /**
         * Flag that indicates whether this event is famous
         */
        char famous;

        /**
         * Event hash
         */
        std::string hash;

        /**
         * Hash this event
         */
        std::string makeHash();
        
    public:

        /**
         * Constructor
         */
        Event(Person &p, message::GossipData const &data, int round = 0, bool witness = false);

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
        const message::GossipData &getData() const;

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
         * @param endpoints List of endpoints
         */
        bool stronglySee(Event const &y, std::vector<types::Endpoint*> *endpoints);
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
