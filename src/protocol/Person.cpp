#include <chrono>
#include <unordered_map>
#include "Person.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {

/**
 * Find unique famous witnesses 
 *
 * @param witnesses Vector of witnesses to search through
 * @param endpoints Network endpoints
 */
static std::unordered_map<std::string, Event*> findUFW(std::vector<Event*> const &witnesses, std::vector<types::Endpoint*> *endpoints) {
	std::unordered_map<std::string, Event*> v;
	std::unordered_map<std::string, int> b;

	for (std::vector<types::Endpoint*>::iterator it = endpoints->begin(); it != endpoints->end(); ++it) {
		v[(*it)->identifier] = NULL;
		b[(*it)->identifier] = 0;
	}

	for (std::size_t i = 0; i < witnesses.size(); i++) {
		if (witnesses[i]->getFamous() == true) {
			std::string owner = witnesses[i]->getData().owner;
			if (b[owner] == 1) {
				b[owner] = -1;
				v[owner] = NULL;
			}
			if (b[owner] == 0) {
				b[owner] = 1;
				v[owner] = witnesses[i];
			}
		}
	}
	return v;
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

	// compare events by their hash
	return (lhs->getHash().compare(rhs->getHash()) > 0);
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

	// compare events by their hash
	return (lhs->getHash().compare(rhs->getHash()) < 0);
}

Person::Person(const std::string databasePath, const std::string privKeyPath, const std::string certPath, std::vector<types::Endpoint*> *endpoints) : PersonNetworker(privKeyPath, certPath), PersonApplication(databasePath), currentRound(0), endpoints(endpoints) {

	// open file descriptor for logging
	if (WRITE_LOG) {
		std::ostringstream filename;
		filename << "Log-" << identifier << ".log";
		ofs.open(filename.str(), std::ofstream::out | std::ofstream::trunc);
	}

	// initial event data
	message::Data d;
	d.owner 	 = identifier;
	d.timestamp  = 0;
	d.selfHash 	 = "\0";
	d.gossipHash = "\0";
	
	// starter event
	hashgraph.insert(hashgraph.begin(), new Event(*this, d));
}

Person::~Person() {
	ofs.close();
}

Person&	Person::operator=(Person const &){
	return *this;
}

bool Person::operator==(Person const &rhs) {
	return this->identifier == rhs.identifier;
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

	if (this->hashgraph[n]->getData().__isset.payload) {
		ofs << "Payload: " << this->hashgraph[n]->getData().payload.senderId << " sent " << this->hashgraph[n]->getData().payload.amount << " to " << this->hashgraph[n]->getData().payload.receiverId << std::endl;
	}
	ofs << std::endl;
}

int	Person::finalizeOrder(std::size_t n, int const &r, std::vector<Event*> const &w) {
	std::vector<int64_t> s;
	std::unordered_map<std::string, Event*> ufw;
	Event *tmp;

	ufw = findUFW(w, this->endpoints);

	std::size_t j;
	for (j = 0; j < this->endpoints->size() && (!ufw[this->endpoints->at(j)->identifier] || ufw[this->endpoints->at(j)->identifier]->ancestor(*(hashgraph[n]))); j++)
		;
	if (j == this->endpoints->size()) {
		for (j = 0; j < this->endpoints->size(); j++) {
			if (ufw[this->endpoints->at(j)->identifier]) {
				tmp = ufw[this->endpoints->at(j)->identifier];
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
		if (hashgraph[n]->getData().__isset.payload) {
			this->storeBalanceTransfer(
				hashgraph[n]->getData().payload.senderId,
				hashgraph[n]->getData().payload.receiverId,
				hashgraph[n]->getData().payload.amount, 
				hashgraph[n]->getConsensusTimestamp()
			);
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

void Person::gossip(types::Endpoint *target) {
	
	// sort the hashgraph
	this->mutex.lock();
	std::sort(hashgraph.begin(), hashgraph.end(), compareEventsGreater);
	Event* check = getTopNode(target->identifier);
	this->mutex.unlock();

	std::unordered_map<std::string, bool> b;
	for (std::vector<types::Endpoint*>::iterator it = this->endpoints->begin(); it != this->endpoints->end(); ++it) {
		b[(*it)->identifier] = false;
	}

	// find an events in the hashgraph the target has not seen yet
	std::vector<message::Data> gossipData;
	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (!b[hashgraph[i]->getData().owner]) {
			if (check && check->see(*(hashgraph[i])))
				b[hashgraph[i]->getData().owner] = true;
			gossipData.push_back(hashgraph[i]->getData());
		}
	}

	this->sendGossip(target, gossipData);
}

Event *Person::getTopNode(std::string identifier) const {
	Event *top = NULL;
	int64_t t  = -1;

	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getData().owner == identifier && hashgraph[i]->getData().timestamp > t) {
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
		if (hashgraph[i]->getData().owner == target.identifier && hashgraph[i]->getData().timestamp > t) {
			t = hashgraph[i]->getData().timestamp;
		}
	}
	for (std::size_t i = 0; i < hashgraph.size(); i++) {
		if (hashgraph[i]->getData().owner == target.identifier && hashgraph[i]->getData().timestamp > t2 && hashgraph[i]->getData().timestamp != t) {
			t2 = hashgraph[i]->getData().timestamp;
			fork = hashgraph[i];
		}
	}
	return fork;
}

void Person::createEvent(std::string gossiper) {

	// unix timestamp in milliseconds
	int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	message::Data d;
	d.owner      = this->identifier;
	d.timestamp  = timestamp;
	d.selfHash   = (this->getTopNode(this->identifier) ? this->getTopNode(this->identifier)->getHash() : "\0");
	d.gossipHash = (this->getTopNode(gossiper) ? this->getTopNode(gossiper)->getHash() : "\0");
	
	// process first transfer request in queue
	if (this->transferRequests.size() > 0) {
		d.__set_payload(this->transferRequests.front());
		this->transferRequests.pop();
	}
	
	// only for testing, force a fork every 100th message (approx.)
	if (MAKE_FORKS && !(std::rand() % 100) && this->getForkNode(*this)) {
		d.selfHash = this->getForkNode(*this)->getHash();
	}	
	
	hashgraph.insert(hashgraph.begin(), new Event(*this, d));
}

void Person::crypto_transfer(const std::string& ownerPkDer, const int32_t amount, const std::string& receiverId, const std::string& challenge, const std::string& sigDer) {
	std::lock_guard<std::mutex> guard(this->mutex);

	if (ownerPkDer.empty() || receiverId.empty() || sigDer.empty() || challenge.empty()) 
		return;

	// calculate owner identifier
	std::string ownerId = utils::encodeIdentifier(ownerPkDer);

	// verify request
	if (utils::verifyECDSASignature(ownerPkDer, sigDer, (challenge + "|" + std::to_string(amount) + "|" + receiverId))) {
		message::Payload p;
		p.senderId   = ownerId;
		p.receiverId = receiverId;
		p.amount     = amount;
		this->transferRequests.push(p);
	}
}

int32_t Person::balance(const std::string& ownerId) {
	return this->getUserBalance(ownerId); 
}

void Person::balance_history(std::vector<message::BalanceTransfer> & _return, const std::string& ownerId) {
	return this->getUserBalanceHistory(ownerId, _return); 
}

void Person::challenge(std::string& _return) {
	_return.assign("dummy");
}

void Person::receiveGossip(const std::string& gossiper, const std::vector<message::Data> &gossip) {
	std::lock_guard<std::mutex> guard(this->mutex);

	Event *tmp;
	std::size_t n;	
	std::vector<Event*> nEvents;

	for (std::size_t i=0; i < gossip.size(); i++) {

		// create event from gossip data
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