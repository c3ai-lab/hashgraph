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

void PersonApplication::storeTransferData(int64_t cnsTime, const message::GossipData data) {
    if (!data.__isset.payload) return;

    this->getManager()->add(std::make_shared<runner::SQLiteTransferRunner>(
        this->getDatabasePath(),
        data.payload.receiverId,
        data.payload.senderPkDer,
        data.payload.sigDer,
        data.payload.amount, 
        cnsTime
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
