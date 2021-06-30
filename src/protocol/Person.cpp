#include <chrono>
#include <unordered_map>
#include "Person.hpp"
#include "Event.hpp"
#include "../utils/hashgraph_utils.hpp"
#include "../runner/EndpointGossipRunner.hpp"

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
        v[(*it)->getIdentifier()] = NULL;
        b[(*it)->getIdentifier()] = 0;
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

Person::Person(const std::string skPath, const std::string certPath, std::vector<types::Endpoint*> *endpoints) : PersonNetworker(skPath, certPath), PersonApplication(), currentRound(0), endpoints(endpoints) {

    // set node identifier
    this->setIdentifier(utils::getIdentifierFromPrivatePEM(this->skPEM));

    types::Endpoint *tar;
    do {
        // select a random target from the list of known nodes
        tar = this->endpoints->at(std::rand() % this->endpoints->size());
    }
    while (tar->getIdentifier() == this->getIdentifier());

    message::BootstrapPacket packet;
    // retrieve bootstrap data from remote node
    tar->bootstrap(packet, this->getIdentifier());

    if (!packet.__isset.data) {
        
        // initial event data
        message::GossipData d;
        d.owner 	 = this->getIdentifier();
        d.timestamp  = 0;
        d.selfHash 	 = "\0";
        d.gossipHash = "\0";

        this->hashgraph.insert(this->hashgraph.begin(), new Event(*this, d));
    } 
    else {
        this->hashgraph.insert(this->hashgraph.begin(), new Event(*this, packet.data, 1, true));
    }
}

Person::~Person() {
}

Person&	Person::operator=(Person const &){
    return *this;
}

bool Person::operator==(Person const &rhs) {
    return this->getIdentifier() == rhs.getIdentifier();
}

std::vector<Event*> Person::findWitnesses(int const &round) const {
    std::size_t size = this->getHashgraph().size();

    std::vector<Event*> witnesses;
    const std::vector<Event*> &hashRef = this->getHashgraph();
    for (std::size_t i = 0; i < size && hashRef[i]->getRound() >= round - 1; i++) {
        if (hashRef[i]->getRound() == round && hashRef[i]->getWitness() == true)
            witnesses.push_back(hashRef[i]);
    }
    return witnesses;
}

int Person::finalizeOrder(std::size_t n, int const &r, std::vector<Event*> const &w) {
    std::vector<int64_t> s;
    std::unordered_map<std::string, Event*> ufw;
    Event *tmp;

    ufw = findUFW(w, this->endpoints);

    std::size_t j;
    for (j = 0; j < this->endpoints->size() && (!ufw[this->endpoints->at(j)->getIdentifier()] || ufw[this->endpoints->at(j)->getIdentifier()]->ancestor(*(this->hashgraph[n]))); j++)
        ;
    if (j == this->endpoints->size()) {
        for (j = 0; j < this->endpoints->size(); j++) {
            if (ufw[this->endpoints->at(j)->getIdentifier()]) {
                tmp = ufw[this->endpoints->at(j)->getIdentifier()];
                while (tmp->getSelfParent() && tmp->getSelfParent()->ancestor(*(this->hashgraph[n])))
                    tmp = tmp->getSelfParent();
                s.push_back(tmp->getData().timestamp);
            }
        }
        if (s.size() == 0)
            return 1;
        this->hashgraph[n]->setRoundReceived(r);
        std::sort(s.begin(),s.end());
        this->hashgraph[n]->setConsensusTimestamp(s[s.size() / 2]);

        // store transfer payload permanently (if any)
        this->storeTransferData(this->hashgraph[n]->getConsensusTimestamp(), this->hashgraph[n]->getData());
        // log event data
        this->writeEventToLog(this->hashgraph[n]);

        return 1;
    }
    return 0;
}

void Person::findOrder() {
    std::size_t i;
    std::vector<Event*> w;

    for (std::size_t n = this->hashgraph.size() - 1; n < this->hashgraph.size(); n--) {
        if (this->hashgraph[n]->getRoundRecieved() == -1) {
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

Event *Person::getTopNode(std::string identifier) const {
    Event *top = NULL;
    int64_t t  = -1;

    for (std::size_t i = 0; i < this->hashgraph.size(); i++) {
        if (this->hashgraph[i]->getData().owner == identifier && this->hashgraph[i]->getData().timestamp > t) {
            t   = this->hashgraph[i]->getData().timestamp;
            top = this->hashgraph[i];
        }
    }
    return top;
}

Event *Person::getForkNode(Person const &target) const {
    Event *fork = NULL;
    int64_t t   = -1;
    int64_t t2  = -1;

    for (std::size_t i = 0; i < this->hashgraph.size(); i++) {
        if (this->hashgraph[i]->getData().owner == target.getIdentifier() && this->hashgraph[i]->getData().timestamp > t) {
            t = this->hashgraph[i]->getData().timestamp;
        }
    }
    for (std::size_t i = 0; i < this->hashgraph.size(); i++) {
        if (this->hashgraph[i]->getData().owner == target.getIdentifier() && this->hashgraph[i]->getData().timestamp > t2 && this->hashgraph[i]->getData().timestamp != t) {
            t2 = this->hashgraph[i]->getData().timestamp;
            fork = this->hashgraph[i];
        }
    }
    return fork;
}

void Person::createEvent(std::string gossiper) {

    // unix timestamp in milliseconds
    int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    message::GossipData d;
    d.owner      = this->getIdentifier();
    d.timestamp  = timestamp;
    d.selfHash   = (this->getTopNode(this->getIdentifier()) ? this->getTopNode(this->getIdentifier())->getHash() : "\0");
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

    this->hashgraph.insert(this->hashgraph.begin(), new Event(*this, d));
}

void Person::crypto_transfer(const std::string &ownerPkDer, const int32_t amount, const std::string &receiverId, const std::string &challenge, const std::string &sigDer) {

    message::GossipPayload p;
    p.senderPkDer = ownerPkDer;
    p.amount      = amount;
    p.receiverId  = receiverId;
    p.challenge   = challenge;
    p.sigDer      = sigDer;

    this->hgMutex.lock();
    this->transferRequests.push(p);
    this->hgMutex.unlock();
}

void Person::bootstrap(message::BootstrapPacket &_return, const std::string &identifier) {
    std::lock_guard<std::mutex> guard(this->hgMutex);

    // find last witness of identifier
    Event *event = this->getTopNode(identifier);

    // no suitable event was found
    if (event == NULL)
        return;

    // build response
    _return.__set_data(event->getData());
}

void Person::startGossip(int interval, const std::atomic<bool> *quit) {
    // not enough nodes
    if (this->endpoints->size() < 2) return;

    // target node
    types::Endpoint *tar;

    while (!quit->load()) {

        // skip if queue is already filled
        if (this->getManager()->pendingTaskCount() <= this->getManager()->pendingTaskCountMax()) {

            do {
                // select a random target from the list of known nodes
                tar = this->endpoints->at(std::rand() % this->endpoints->size());
            }
            while (tar->getIdentifier() == this->getIdentifier());
            
            std::unordered_map<std::string, bool> b;
            for (std::vector<types::Endpoint*>::iterator it = this->endpoints->begin(); it != this->endpoints->end(); ++it) {
                b[(*it)->getIdentifier()] = false;
            }

            this->hgMutex.lock();

            // sort the hashgraph
            std::sort(hashgraph.begin(), hashgraph.end(), compareEventsGreater);
            Event* check = getTopNode(tar->getIdentifier());

            // find an events in the hashgraph the target has not seen yet
            message::GossipPacket packet;
            for (std::size_t i = 0; i < hashgraph.size(); i++) {
                if (!b[hashgraph[i]->getData().owner]) {
                    if (check && check->see(*(hashgraph[i])))
                        b[hashgraph[i]->getData().owner] = true;
                    packet.data.push_back(hashgraph[i]->getData());
                }
            }
            packet.gossiper = this->getIdentifier();
            
            this->hgMutex.unlock();

            // send data to remote endpoint
            this->getManager()->add(std::make_shared<runner::EndpointGossipRunner>(tar, packet, this->skPEM));
        }

        // limit gossip interval
        std::this_thread::sleep_for(std::chrono::microseconds(interval));
    }
}

void Person::receiveGossip(const message::GossipPacket &packet, const std::string &sigDer) {
    if (utils::verifyGossipPacket(this->endpoints, packet, sigDer)) {
        std::lock_guard<std::mutex> guard(this->hgMutex);

        Event *tmp;
        std::size_t n;	
        std::vector<Event*> nEvents;

        for (std::size_t i=0; i < packet.data.size(); i++) {

            // create event from gossip data
            tmp = new Event(*this, packet.data[i]);

            // check whether new event not already exists in hashgraph and its payload is valid
            for (n = 0; n < this->hashgraph.size(); n++) {
                if (this->hashgraph[n]->getHash() == tmp->getHash())
                    break;
            }
            if (n >= this->hashgraph.size()) {
                this->hashgraph.insert(this->hashgraph.begin(), tmp);
                nEvents.push_back(tmp);
            }
            else delete tmp;
        }

        // create a new transfer event
        this->createEvent(packet.gossiper);
        nEvents.push_back(this->hashgraph[0]);

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
    for (std::size_t i = 0; i < this->hashgraph.size(); i++) {

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
            // if (this->hashgraph[i])
            // {
            // 	delete this->hashgraph[i];
            // 	this->hashgraph[i] = NULL;
            // }
            this->hashgraph.erase(this->hashgraph.begin() + i);
            i--;
        }
    }
}

};
};
