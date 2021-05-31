#include <unordered_map>
#include "Event.hpp"
#include "Person.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {

Event::Event(Person &p, message::GossipData const &data) : 
	graph(const_cast<std::vector<Event *>&>(p.getHashgraph())),
	d(data),
	selfParent(NULL), 
	gossiperParent(NULL), 
	consensusTimestamp(-1), 
	roundRecieved(-1),
	round(0), 
	witness(d.selfHash == "\0" ? true : false), 
	famous(-1) {
	
	hash = makeHash();
}

Event::~Event() {
}

std::string Event::makeHash() {
	std::ostringstream s;
	s << *this;
	return utils::byteToHex(utils::SHA384(s.str()));
}

void Event::divideRounds(Person &person) {

	if (!this->selfParent || !this->gossiperParent) {
		this->round = 0;
		return;
	}

	this->round = this->selfParent->getRound();
	if (this->gossiperParent->getRound() > round)
		this->round = this->gossiperParent->getRound();

	std::size_t numStrongSee = 0;

	std::vector<Event*> witnesses = person.findWitnesses(this->round);
	for (std::size_t i = 0; i < witnesses.size(); i++) {
		if (numStrongSee > 2 * person.endpoints->size() / 3)
			break;
		if (this->stronglySee(*witnesses[i], person.endpoints))
			numStrongSee++;
	}
	if (numStrongSee > 2 * person.endpoints->size() / 3) {
		this->round = this->round + 1;
		if (person.getCurRound() < this->round)
			person.incCurRound();
	}

	this->witness = (this->getSelfParent() == NULL || this->getSelfParent()->getRound() < this->round);
	
}

void Event::decideFame(Person &person) {

	std::vector<Event*> s;
	std::size_t count;
	std::size_t countNo;
	int d;

	if (!this->witness || this->round < 2)
		return ;

	for (std::size_t x = this->graph.size() - 1; x < this->graph.size(); x--) {
		if (this->graph[x]->getWitness() && this->graph[x]->getFamous() == -1 && this->graph[x]->getRound() <= round - 2) {
			s = person.findWitnesses(this->graph[x]->getRound() + 1);
			count = 0;
			countNo = 0;
			for (std::size_t y = 0; y < s.size(); y++) {
				if (!this->stronglySee(*(s[y]), person.endpoints)) {
					s.erase(s.begin() + y);
				}
				else {
					if (s[y]->see(*(this->graph[x])))
						count++;
					else
						countNo++;
				}
			}
			d = this->round - this->graph[x]->getRound();
			if (count > 2 * person.endpoints->size() / 3)
				this->graph[x]->setFamous(1);
			else if (countNo > 2 * person.endpoints->size() / 3)
				this->graph[x]->setFamous(0);
			else if (!(d % 2)) // TODO: check C=2
				this->graph[x]->setFamous(this->graph[x]->getHash()[16] % 2);
		}
	}

}

bool Event::operator==(Event const &rhs) const {
	return (hash == rhs.getHash());
}

bool Event::seeRecursion(Event const &y, std::vector<Event*> &forkCheck, bool &done, std::vector<Event*> &visited) {
	if (std::find(visited.begin(), visited.end() , this) != visited.end())
		return false;
	visited.push_back(this);

	if (d.owner == y.getData().owner)
		forkCheck.push_back(this);
	if (done)
		return true;
	if (*this == y) {
		done = true;
		return true;
	}
	if (d.timestamp < y.getData().timestamp)
		return false;
	if (!this->getSelfParent() || !this->getGossiperParent())
		return false;
	return this->getSelfParent()->seeRecursion(y, forkCheck, done, visited) || this->getGossiperParent()->seeRecursion(y, forkCheck, done, visited);
}

bool Event::see(Event const &y) {
	std::string yHash = y.getHash();
	if (this->hashesSeen.find(yHash) != this->hashesSeen.end())
		return true;
	if (this->hashesNotSeen.find(yHash) != this->hashesSeen.end())
		return false;
	std::vector<Event*> forkCheck;
	std::vector<Event*> visited;
	bool done = false;
	bool b = this->seeRecursion(y, forkCheck, done, visited);
	if (b == false) {
		this->hashesNotSeen.insert(yHash);
		return false;
	}
	for (std::size_t i = 0; i < forkCheck.size(); i++) {
		for (std::size_t j = i + 1; j < forkCheck.size(); j++) {
			if (fork(forkCheck[i], forkCheck[j])) {
				this->hashesNotSeen.insert(yHash);
				return false;
			}
		}
	}
	this->hashesSeen.insert(yHash);
	return true;
}

bool Event::ancestorRecursion(Event const &y, bool &done, std::vector<Event*> &visited) {
	if (done)
		return true;
	if (*this == y) {
		done = true;
		return true;
	}
	if (std::find(visited.begin(), visited.end() , this) != visited.end())
		return false;
	visited.push_back(this);
	if (d.timestamp < y.getData().timestamp)
		return false;
	if (!this->getSelfParent() || !this->getGossiperParent())
		return false;
	return (this->getSelfParent()->ancestorRecursion(y, done, visited)) || (this->getGossiperParent()->ancestorRecursion(y, done, visited));
}

bool Event::ancestor(Event const &y) {
	bool b;
	std::vector<Event*> visited;
	bool done = false;
	std::string yHash = y.getHash();
	if (hashesSeen.find(yHash) != hashesSeen.end() || ancestorsSeen.find(yHash) != ancestorsSeen.end())
		return true;
	if (ancestorsNotSeen.find(yHash) != ancestorsNotSeen.end())
		return false;
	b = ancestorRecursion(y, done, visited);
	if (!b){
		ancestorsNotSeen.insert(yHash);
	}
	else
		ancestorsSeen.insert(yHash);
	return b;
}

bool Event::stronglySee(Event const &y, std::vector<types::Endpoint*> *endpoints) {

	std::size_t numSee = 0;

	std::unordered_map<std::string, bool> found;
	for (std::vector<types::Endpoint*>::iterator it = endpoints->begin(); it != endpoints->end(); ++it) {
		found[(*it)->getIdentifier()] = false;
	}

	for (std::size_t n = 0; n < graph.size(); n++) {
		if (found[graph[n]->getData().owner] == true || graph[n]->getRound() < y.getRound())
			continue ;
		if (this->see(*(graph[n])) && graph[n]->see(y)) {
			numSee++;
			found[graph[n]->getData().owner] = true;
			if (numSee > 2 * endpoints->size() / 3) {
				return true;
			}
		}
	}

	return false;
}

bool Event::fork(Event *x, Event *y) {
	Event *t;

	if (!(x->getData().owner == y->getData().owner))
		return 0;
	t = x;
	while (t) {
		if (t == y)
			return 0;
		t = t->getSelfParent();
	}
	t = y;
	while (t) {
		if (t == x)
			return 0;
		t = t->getSelfParent();
	}
	return 1;
}

std::vector<Event*>	&Event::getGraph() const {
	return this->graph;
}

const message::GossipData &Event::getData() const{
	return this->d;
}

int	Event::getRound() const {
	return this->round;
}

bool Event::getWitness() const {
	return this->witness;
}

std::string	Event::getHash() const{
	return this->hash;
}

char Event::getFamous() const {
	return this->famous;
}

void Event::setFamous(char const &fame) {
	this->famous = fame;
}

int	Event::getRoundRecieved() const {
	return this->roundRecieved;
}

void Event::setRoundReceived(int const &r) {
	this->roundRecieved = r;
}

int64_t	Event::getConsensusTimestamp() const {
	return this->consensusTimestamp;
}

void Event::setConsensusTimestamp(int64_t t) {
	this->consensusTimestamp = t;
}

Event *Event::getSelfParent() const {
	return this->selfParent;
}

void Event::setSelfParent(Event* const e) {
	this->selfParent = e;
}

Event *Event::getGossiperParent() const {
	return this->gossiperParent;
}

void Event::setGossiperParent(Event* const e) {
	this->gossiperParent = e;
}

std::ostream& operator<<(std::ostream& os, const Event& e) {
	os << e.getData().selfHash << e.getData().gossipHash
	<< e.getData().timestamp << e.getData().owner;
	return os;
}

};
};