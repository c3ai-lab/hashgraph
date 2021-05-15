
#include "PersonApplication.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace protocol {

PersonApplication::PersonApplication(const std::string databasePath) {
	this->database = utils::prepareDatabase(databasePath);
}

PersonApplication::~PersonApplication() {
    utils::closeDatabase(this->database);
}

void PersonApplication::storeBalanceTransfer(const std::string senderId, const std::string receiverId, int32_t amount, int64_t timestamp) {
    utils::storeBalanceTransfer(this->database, senderId, receiverId, amount, timestamp);
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

};
};