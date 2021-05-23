#ifndef HASHGRAPH_RUNNER_SQLITE_TRANSFER_HPP
#define HASHGRAPH_RUNNER_SQLITE_TRANSFER_HPP

#include <string>
#include <cstdint>
#include <thrift/concurrency/Thread.h>
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift::concurrency;

namespace hashgraph {
namespace runner {

/**
 * Endpoint class
 */
class SQLiteTransferRunner : public Runnable {

	private:

		/**
		 * Path of the database file
		 */
		const std::string dbPath;

		/**
		 * Transfer sender
		 */
		const std::string sender;

		/**
		 * Transfer receiver
		 */
		const std::string receiver;

		/**
		 * Transfer amount
		 */
		const int amount;

		/**
		 * Transfer time
		 */
		const int64_t timestamp;

	public:

		/**
		 * Constructor
		 */
		SQLiteTransferRunner(const std::string dbPath, const std::string sender, const std::string receiver, const int amount, const int64_t timestamp) :
			dbPath(dbPath),
			sender(sender),
			receiver(receiver),
			amount(amount),
			timestamp(timestamp) {};

		/**
		 * Write transfer entry to database
		 */
		void run() {
			utils::storeTransferData(this->dbPath, this->sender, this->receiver, this->amount, this->timestamp);
		}
};

};
};
#endif
