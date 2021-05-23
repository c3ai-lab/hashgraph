#ifndef HASHGRAPH_PROTOCOL_SUPER_PERSON_HPP
#define HASHGRAPH_PROTOCOL_SUPER_PERSON_HPP

#include <string>
#include <thread>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift::concurrency;

namespace hashgraph {
namespace protocol {

class Event;

/**
 * SuperPerson class
 */
class SuperPerson {

	private:

        /**
         * Public identifier of the node
         */
        std::string identifier;

        /**
         * Thread pool manager
         */
		std::shared_ptr<ThreadManager> manager;

        /**
         * SQLite database path
         */
        std::string databasePath;
	
	protected:

        /**
         * Return the database path
         */
		std::string getDatabasePath() const;

        /**
         * Set the nodes' identifier
         */
        void setIdentifier(const std::string identifier);

	public:

        /**
         * Constructor
         */
        SuperPerson();

        /**
         * Destructor
         */
        ~SuperPerson();

        /**
         * Initialize the database
         * 
         * @param databasePath Path of the database file
         */
        void initDatabase(const std::string databasePath);

        /**
         * Return the nodes' thread pool manager
         */
        std::shared_ptr<ThreadManager> getManager() const;

        /**
         * Return the nodes' identifier
         */
		std::string getIdentifier() const;
};

};
};
#endif
