
#include "PersonApplication.hpp"
#include "Event.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {

PersonApplication::PersonApplication(const std::string databasePath, bool logEvents) : logEvents(logEvents) {
	this->database = utils::prepareDatabase(databasePath);
}

PersonApplication::~PersonApplication() {
    utils::closeDatabase(this->database);
}

void PersonApplication::storeBalanceTransfer(const protocol::Event *event) {
    utils::storeBalanceTransfer(
		this->database,
        event->getData().payload.senderId,
        event->getData().payload.receiverId,
        event->getData().payload.amount, 
        event->getConsensusTimestamp()
    );
}

int32_t PersonApplication::getUserBalance(const std::string identifier) {
    
    std::vector<message::BalanceTransfer> history;
    utils::getTransferHistory(this->database, identifier, history);

    int32_t amount = 0;
    for(std::vector<message::BalanceTransfer>::iterator it = history.begin(); it != history.end(); ++it) {
        if (it->senderId   == identifier) amount -= it->amount;
        if (it->receiverId == identifier) amount += it->amount;
    }
    return amount;
}

void PersonApplication::getUserBalanceHistory(const std::string identifier, std::vector<message::BalanceTransfer> &history) {
    utils::getTransferHistory(this->database, identifier, history);
}

void PersonApplication::writeEventToLog(const protocol::Event *event) {
    if (!this->logEvents) return;

	std::string payload = "";
	if (event->getData().__isset.payload) {
		payload = event->getData().payload.senderId + " sent " + std::to_string(event->getData().payload.amount) + " to " + event->getData().payload.receiverId;
	}

    utils::writeToLog(
        this->database, 
        event->getData().owner,
        event->getRoundRecieved(),  
        event->getData().timestamp, 
        event->getConsensusTimestamp(),
        event->getData().selfHash, 
        event->getData().gossipHash, 
        payload
    );
}

};
};