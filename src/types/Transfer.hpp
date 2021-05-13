#ifndef HASHGRAPH_TYPES_TRANSFER_HPP
#define HASHGRAPH_TYPES_TRANSFER_HPP

#include <string>
#include <cstdint>

namespace hashgraph {
namespace types {

/**
 * Transfer
 */
struct Transfer {

	/**
	 * The transfered amount
	 */
	int32_t amount;

	/**
	 * Consensus time
	 */
    int64_t timestamp;

	/**
	 * Identifier of the sender
	 */
    std::string senderId;

	/**
	 * Identifier of the receiver
	 */
    std::string receiverId;

};

};
};
#endif
