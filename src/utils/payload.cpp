#include <string>
#include <cstdint>
#include "hashgraph_utils.hpp"

namespace hashgraph {
namespace utils {

bool verifyGossipPayload(const std::string ownerPkDer, const int32_t amount, const std::string receiverId, const std::string challenge, const std::string sigDer) {

	if (ownerPkDer.empty() || receiverId.empty() || sigDer.empty() || challenge.empty()) 
		return false;

	// calculate owner identifier
	std::string ownerId = utils::encodeIdentifier(ownerPkDer);

	// verify request data
	return utils::verifyECDSASignature(ownerPkDer, sigDer, (challenge + "|" + std::to_string(amount) + "|" + receiverId));
}

};
};
