#include <sstream>
#include "PersonApplication.hpp"
#include "Event.hpp"
#include "../runner/SQLiteLogRunner.hpp"
#include "../runner/SQLiteTransferRunner.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {

PersonApplication::PersonApplication() : 
    logEvents(false) {
}

void PersonApplication::setEventLogging(bool logEvents) {
    this->logEvents = logEvents;
}

void PersonApplication::storeTransferData(const protocol::Event *event) {
    this->getManager()->add(std::make_shared<runner::SQLiteTransferRunner>(
		this->getDatabasePath(),
        utils::encodeIdentifier(event->getData().payload.senderPkDer),
        event->getData().payload.receiverId,
        event->getData().payload.amount, 
        event->getConsensusTimestamp()
    ));
}

void PersonApplication::writeEventToLog(const protocol::Event *event) {
    if (!this->logEvents) return;

    std::stringstream payload;
	if (event->getData().__isset.payload) {
        event->getData().payload.printTo(payload);
    }

    this->getManager()->add(std::make_shared<runner::SQLiteLogRunner>(
        this->getDatabasePath(), 
        event->getData().owner,
        event->getRoundRecieved(),  
        event->getData().timestamp, 
        event->getConsensusTimestamp(),
        event->getData().selfHash, 
        event->getData().gossipHash, 
        payload.str()
    ));
}

};
};