
#include "PersonApplication.hpp"

namespace hashgraph {
namespace protocol {

void PersonApplication::storeAmountTransfer(const std::string senderId, const std::string receiverId, int32_t amount, int64_t timestamp) {
    types::Transfer t;
    t.senderId   = senderId;
    t.receiverId = receiverId;
    t.amount     = amount;
    t.timestamp  = timestamp;
    
    this->history.push_back(t);
}

int32_t PersonApplication::getAmountForUser(const std::string identifier) {
    int32_t amount = 0;
	for (std::vector<types::Transfer>::iterator it = this->history.begin(); it != this->history.end(); ++it) {
		if (it->senderId   == identifier) amount -= it->amount;
        if (it->receiverId == identifier) amount += it->amount;
	}
    return amount;
}

};
};