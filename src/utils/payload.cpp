#include <string>
#include <sstream>
#include <cstdint>
#include "../types/Endpoint.hpp"
#include "../message/Hashgraph.h"
#include "hashgraph_utils.hpp"

namespace hashgraph {
namespace utils {

bool verifyGossipPacket(const std::vector<types::Endpoint*> *endpoints, const message::GossipPacket packet, const std::string sigDer) {

    types::Endpoint *ep = NULL;
    for (auto eit = endpoints->begin(); eit != endpoints->end(); ++eit) {
        // find packet sender within the gossip list
        if ((*eit)->getIdentifier() == packet.gossiper) ep = (*eit);
    }
    if (ep == NULL) return false;


    // serialize packet data
    std::stringstream stream;
    packet.printTo(stream);

    return utils::ecdsaVerifyMessage(ep->pkDer, sigDer, stream.str());
}

bool verifyGossipPayload(const std::string ownerPkDer, const int32_t amount, const std::string receiverId, const std::string challenge, const std::string sigDer) {

    if (ownerPkDer.empty() || receiverId.empty() || sigDer.empty() || challenge.empty()) 
        return false;

    // calculate owner identifier
    std::string ownerId = utils::encodeIdentifier(ownerPkDer);

    // verify request data
    return utils::ecdsaVerifyMessage(ownerPkDer, sigDer, (challenge + "|" + std::to_string(amount) + "|" + receiverId));
}

};
};
