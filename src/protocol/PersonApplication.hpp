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
         * @param cnsTime The consensus time of the enclosing event
         * @param data Data of the gossip event
         */
        void storeTransferData(int64_t cnsTime, const message::GossipData data);

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
