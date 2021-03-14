#include <chrono>
#include "Person.hpp"

#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {


/**
 * Find unique famous witnesses 
 *
 * @param witnesses Vector of witnesses to search through
 * @param numNodes Number of network persons
 */
static std::vector<Event*> findUFW(std::vector<Event*> const &witnesses, std::size_t numNodes) {

	std::vector<Event*> vec(numNodes);
	for (std::size_t i = 0; i < numNodes; i++)
		vec[i] = NULL;

	std::vector<int> b(numNodes);
	std::fill(b.begin(), b.end(), 0);

	for (std::size_t i = 0; i < witnesses.size(); i++) {
		if (witnesses[i]->getFamous() == true) {
			int32_t num = witnesses[i]->getData().owner;
			if (b[num] == 1) {
				b[num] = -1;
				vec[num] = NULL;
			}
			if (b[num] == 0) {
				b[num] = 1;
				vec[num] = witnesses[i];
			}
		}
	}
	return vec;
}

/**
 * Compare which of two events occurance later
 *
 * @param lhs First event for the comparision
 * @param rhs Second event for the comparision
 */
bool compareEventsGreater(const Event* lhs, const Event* rhs) {

	// compare events by timestamp
	if (lhs->getData().timestamp != rhs->getData().timestamp) 
		return lhs->getData().timestamp > rhs->getData().timestamp;

	// compare events by owner
	if (lhs->getData().owner != rhs->getData().owner)
		return lhs->getData().owner > rhs->getData().owner;

	// compare events by sequence number
	return lhs->getData().seqNum > rhs->getData().seqNum;
}

/**
 * Compare which of two events occurance earlier
 *
 * @param lhs First event for the comparision
 * @param rhs Second event for the comparision
 */
bool compareEventsLesser(const Event* lhs, const Event* rhs) {

	// compare events by timestamp
	if (lhs->getData().timestamp != rhs->getData().timestamp) 
		return lhs->getData().timestamp < rhs->getData().timestamp;

	// compare events by owner
	if (lhs->getData().owner != rhs->getData().owner)
		return lhs->getData().owner < rhs->getData().owner;

	// compare events by sequence number
	return lhs->getData().seqNum < rhs->getData().seqNum;
}

int64_t Person::dataSequenceNumber = 0;

Person::Person(message::Endpoint const &ep, std::vector<message::Endpoint> *endpoints) : PersonNetworker(ep, endpoints), currentRound(0) {

	// open file descriptor for logging
	if (WRITE_LOG) {
		std::ostringstream filename;
		filename << "Log" << ep.index << ".log";
		ofs.open(filename.str(), std::ofstream::out | std::ofstream::trunc);
	}

	// initial event data
	message::Data d;
	d.owner 	 = ep.index;
	d.target 	 = -1;
	d.payload 	 = 0;
	d.timestamp  = 0;
	d.selfHash 	 = "\0";
	d.gossipHash = "\0";
	
	// starter event
	hashgraph.insert(hashgraph.begin(), new Event(*this, d));

	// initial worth for every person
	for (std::size_t i = 0; i < endpoints->size(); i++)
		networth.push_back(100000);

	// start server
    startServer();
}

Person::~Person() {
	ofs.close();
}

Person&	Person::operator=(Person const &){
	return *this;
}

bool Person::operator==(Person const &rhs) {
	return this->ep.index == rhs.ep.index;
}

std::vector<Event*>	Person::findWitnesses(int const &round) const {
	std::size_t size = this->getHashgraph().size();

	std::vector<Event*> witnesses;
	const std::vector<Event*> &hashRef = this->getHashgraph();
	for (std::size_t i = 0; i < size && hashRef[i]->getRound() >= round - 1; i++) {
		if (hashRef[i]->getRound() == round && hashRef[i]->getWitness() == true)
			witnesses.push_back(hashRef[i]);
	}
	return witnesses;
}

void Person::insertEvent(Event const &event) {
	Event* p;
	p = const_cast<Event*>(&event);

	std::size_t i;
	for (i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getRoundRecieved() != -1 && hashgraph[i]->getRoundRecieved() <= event.getRoundRecieved())
			break;
	}
	while (i != hashgraph.size() && (hashgraph[i]->getRoundRecieved() == -1 || hashgraph[i]->getRoundRecieved() == event.getRoundRecieved()) && hashgraph[i]->getConsensusTimestamp() <= event.getConsensusTimestamp())
		i++;
	while (i != hashgraph.size() && (hashgraph[i]->getRoundRecieved() == -1 || hashgraph[i]->getRoundRecieved() == event.getRoundRecieved()) && hashgraph[i]->getConsensusTimestamp() == event.getConsensusTimestamp() && hashgraph[i]->getData().owner < hashgraph[i]->getData().owner)
		i++;

	hashgraph.insert(hashgraph.begin() + i, p);
}

void Person::outputOrder(std::size_t n) {

	ofs << "Node owner: " 	  << this->hashgraph[n]->getData().owner 	   	 << std::endl;
	ofs << "Round Received: " << this->hashgraph[n]->getRoundRecieved()	   	 << std::endl;
	ofs << "Timestamp: "  	  << this->hashgraph[n]->getData().timestamp 	 << std::endl;
	ofs << "Consensus Time: " << this->hashgraph[n]->getConsensusTimestamp() << std::endl;
	ofs << "Self Parent: " 	  << this->hashgraph[n]->getData().selfHash      << std::endl;
	ofs << "Gossip Parent: "  << this->hashgraph[n]->getData().gossipHash    << std::endl;

	if (this->hashgraph[n]->getData().payload) {
		ofs << "Payload: " << this->hashgraph[n]->getData().payload << " to " << this->hashgraph[n]->getData().target << std::endl;
		ofs << "Current Networth: ";
		for (std::size_t i = 0; i < this->networth.size(); i++)
			ofs << this->networth[i] << " ";
		ofs << std::endl;
	}
	ofs << std::endl;
}

int	Person::finalizeOrder(std::size_t n, int const &r, std::vector<Event*> const &w) {
	std::vector<int64_t> s;
	std::vector<Event*> ufw;
	Event *tmp;

	ufw = findUFW(w, this->endpoints->size());
	
	std::size_t j;
	for (j = 0; j < this->endpoints->size() && (!ufw[j] || ufw[j]->ancestor(*(hashgraph[n]))); j++)
		;
	if (j == this->endpoints->size()) {
		for (j = 0; j < this->endpoints->size(); j++) {
			if (ufw[j]) {
				tmp = ufw[j];
				while (tmp->getSelfParent() && tmp->getSelfParent()->ancestor(*(hashgraph[n])))
					tmp = tmp->getSelfParent();
				s.push_back(tmp->getData().timestamp);
			}
		}
		if (s.size() == 0)
			return (1);
		hashgraph[n]->setRoundReceived(r);
		std::sort(s.begin(),s.end());
		hashgraph[n]->setConsensusTimestamp(s[s.size() / 2]);
		if (hashgraph[n]->getData().payload != 0) {
			this->networth[hashgraph[n]->getData().owner]  -= hashgraph[n]->getData().payload;
			this->networth[hashgraph[n]->getData().target] += hashgraph[n]->getData().payload;
		}
		if (WRITE_LOG) {
			outputOrder(n);
		}
		return (1);
	}
	return (0);
}

void Person::findOrder() {
	std::size_t i;
	std::vector<Event*> w;

	for (std::size_t n = this->hashgraph.size() - 1; n < this->hashgraph.size(); n--) {
		if (hashgraph[n]->getRoundRecieved() == -1) {
			for (int r = this->hashgraph[n]->getRound(); r <= this->hashgraph[0]->getRound(); r++) {
				w = this->findWitnesses(r);
				for (i = 0; i < w.size() && w[i]->getFamous() != -1; i++)
					;
				if (i == w.size()) {
					if (this->finalizeOrder(n, r, w))
						break;
				}
			}
		}
	}
}

void Person::gossip(message::Endpoint const &target) {
	
	// sort the hashgraph
	this->mutex.lock();
	std::sort(hashgraph.begin(), hashgraph.end(), compareEventsGreater);
	Event* check = getTopNode(target.index);
	this->mutex.unlock();

	std::vector<bool> b(this->endpoints->size());
	std::fill(b.begin(), b.end(), false);

	// find an events in the hashgraph the target has not seen yet
	std::vector<message::Data> arr;
	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (!b[hashgraph[i]->getData().owner]) {
			if (check && check->see(*(hashgraph[i])))
				b[hashgraph[i]->getData().owner] = true;
			arr.push_back(hashgraph[i]->getData());
		}
	}

	this->sendGossip(this->ep, target, arr);
}

Event *Person::getTopNode(int32_t index) const {
	Event *top = NULL;
	int64_t t  = -1;

	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getData().owner == index && hashgraph[i]->getData().timestamp > t) {
			t   = hashgraph[i]->getData().timestamp;
			top = hashgraph[i];
		}
	}
	return top;
}

Event *Person::getForkNode(Person const &target) const {
	Event *fork = NULL;
	int64_t t   = -1;
	int64_t t2  = -1;

	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getData().owner == target.ep.index && hashgraph[i]->getData().timestamp > t) {
			t = hashgraph[i]->getData().timestamp;
		}
	}
	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getData().owner == target.ep.index && hashgraph[i]->getData().timestamp > t2 && hashgraph[i]->getData().timestamp != t) {
			t2 = hashgraph[i]->getData().timestamp;
			fork = hashgraph[i];
		}
	}
	return fork;
}

void Person::createEvent(int32_t gossiper) {

	// unix timestamp in milliseconds
	int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	message::Data d;
	d.seqNum     =  Person::dataSequenceNumber++;
	d.payload    =  0;
	d.target     = -1;
	d.owner      = this->ep.index;
	d.timestamp  = timestamp;
	d.selfHash   = (this->getTopNode(this->ep.index) ? this->getTopNode(this->ep.index)->getHash() : "\0");
	d.gossipHash = (this->getTopNode(gossiper) ? this->getTopNode(gossiper)->getHash() : "\0");
	
	// gossip a payload every 10th message (approx.)
	if (!(std::rand() % 10)) {
		d.payload = std::rand() % 1000;
		d.target  = std::rand() % this->endpoints->size();
	}
	
	// only for testing, force a fork every 100th message (approx.)
	if (MAKE_FORKS && !(std::rand() % 100) && this->getForkNode(*this)) {
		d.selfHash = this->getForkNode(*this)->getHash();
	}	
	
	hashgraph.insert(hashgraph.begin(), new Event(*this, d));
}

void Person::recieveGossip(const int32_t gossiper, const std::vector<message::Data> &gossip) {
	std::lock_guard<std::mutex> guard(this->mutex);

	Event *tmp;
	std::size_t n;	
	std::vector<Event*> nEvents;

	for (std::size_t i=0; i < gossip.size(); i++) {

		// creatge event from gossip data
		tmp = new Event(*this, gossip[i]);

		// check whether new event already exists in hashgraph
		for (n = 0; n < hashgraph.size(); n++) {
			if (hashgraph[n]->getHash() == tmp->getHash())
				break;
		}
		if (n >= hashgraph.size()) {
			hashgraph.insert(hashgraph.begin(), tmp);
			nEvents.push_back(tmp);
		}
		else delete tmp;
	}

	// create a new transfer event
	this->createEvent(gossiper);
	nEvents.push_back(hashgraph[0]);

	// link new events, order them, detect round and decide famous property
	std::sort(nEvents.begin(), nEvents.end(), compareEventsLesser);
	this->linkEvents(nEvents);	
	for (std::size_t i = 0; i < nEvents.size(); i++)
		nEvents[i]->divideRounds(*this);
	this->removeOldBalls();
	for (std::size_t i = 0; i < nEvents.size(); i++)
		nEvents[i]->decideFame(*this);
	this->findOrder();
}

void Person::linkEvents(std::vector<Event*> const &nEvents) {
	for (std::size_t i = 0; i < nEvents.size(); i++) {
		if (nEvents[i]->getSelfParent() == NULL && nEvents[i]->getData().selfHash != "\0") {
			int c = 0;
			for (std::size_t j = 0; j < this->hashgraph.size(); j++) {
				if (this->hashgraph[j]->getHash() == nEvents[i]->getData().selfHash) {
					nEvents[i]->setSelfParent(this->hashgraph[j]);
					c++;
					if (c == 2) 
						break;
				}
				if (this->hashgraph[j]->getHash() == nEvents[i]->getData().gossipHash) {
					nEvents[i]->setGossiperParent(this->hashgraph[j]);
					c++;
					if (c == 2) 
						break;
				}
			}
		}
	}
}

const std::vector<Event*> &Person::getHashgraph() const {
	return this->hashgraph;
}

int Person::getCurRound() const {
	return this->currentRound;
}

void Person::incCurRound(){
	this->currentRound++;
}

void Person::removeOldBalls() {
	for (std::size_t i = 0; i < hashgraph.size(); i++) {

		// remove events from the hashgraph that
		// are not witnesses and doesn't have a 
		// consensus timestamp
		if (this->hashgraph[i]->getConsensusTimestamp() != -1 && this->hashgraph[i]->getWitness() == false) {
			this->hashgraph.erase(this->hashgraph.begin() + i);
			i--;
		}

		// remove events from the hashgraph that are 
		// older then 5 rounds from now
		if (this->hashgraph[i]->getRound() < this->currentRound - 5) {
			// if (hashgraph[i])
			// {
			// 	delete hashgraph[i];
			// 	hashgraph[i] = NULL;
			// }
			this->hashgraph.erase(hashgraph.begin() + i);
			i--;
		}
	}
}


};
};