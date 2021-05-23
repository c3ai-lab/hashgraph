#ifndef HASHGRAPH_PROTOCOL_PERSON_APPLICATION_HPP
#define HASHGRAPH_PROTOCOL_PERSON_APPLICATION_HPP

#include <string>
#include "SuperPerson.hpp"
#include "../message/Hashgraph.h"


namespace hashgraph {
namespace protocol {

class Event;

/**
 * PersonApplication
 */
class PersonApplication : virtual public SuperPerson {

    private:

        /**
         * Indicate whether all commited should be logged
         */
        bool logEvents;

    public:

		/**
		 * Constructor
		 */
		PersonApplication();

        /**
         * Store the transfer data
         * 
         * @param event The event that contains the transfer
         */
        void storeTransferData(const protocol::Event *event);

        /**
         * Log a committed event
         *
         * @param event The event to log
         */
        void writeEventToLog(const protocol::Event *event);

        /**
		 * Constructor
		 * 
         * @param logEvents Flag that indicates whether all events should be logged
		 */
		void setEventLogging(bool logEvents);
};

};
};
#endif