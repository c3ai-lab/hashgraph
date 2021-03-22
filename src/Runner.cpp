
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <streambuf>

#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "Runner.hpp"
#include "utils/hashgraph_utils.hpp"

namespace hashgraph {

    std::atomic<bool> quit;

    Runner::Runner() {
        // set thrift global output
        apache::thrift::GlobalOutput.setOutputFunction([](const char* msg) {
            if (DEBUG) {
                apache::thrift::TOutput::errorTimeWrapper(msg);
            }
        });

        // detect signals
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa) );
        sa.sa_handler = [](int) {
            quit.store(true);
        };
        sigfillset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);
    }

    Runner::~Runner() {
        // free persons memory
        std::vector<protocol::Person*>::iterator it;
        for (it = this->persons.begin(); it != this->persons.end(); it++) {
            delete (*it);
        }
    }

    void Runner::initHashgraphRunner(std::string configFile) {
        std::ifstream t(configFile.c_str());
        if (!t.good())
            throw std::invalid_argument("No config file found");

        
        // read config file into string
        std::string configStr((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        // read json protocol
        std::shared_ptr<apache::thrift::transport::TMemoryBuffer> transport(new apache::thrift::transport::TMemoryBuffer((uint8_t*)configStr.c_str(), configStr.size()));
        std::shared_ptr<apache::thrift::protocol::TJSONProtocol> protocol(new apache::thrift::protocol::TJSONProtocol(transport));
        this->settings.read((protocol.get()));
    }

    void Runner::startHashgraph(int interval) {
        if (this->settings.endpoints.size() == 0)
            throw std::invalid_argument("No nodes given");

        std::vector<message::Endpoint>::iterator it;
        for (it = this->settings.endpoints.begin(); it != this->settings.endpoints.end(); it++) {
            // check whether the node should be run locally
            if (it->isLocal) {
                this->persons.push_back(
                    new protocol::Person(*it, &(this->settings.endpoints))
                );
            }
        }

        // limits networth prints
        int printerval = 0;

        while (true) {

            // select a random person from the local persons list
            protocol::Person *p = this->persons.at(std::rand() % this->persons.size());

            // select a random target index from the list of known nodes
            message::Endpoint tar;
            while ((tar = this->settings.endpoints.at(std::rand() % this->settings.endpoints.size())).index == p->ep.index);

            // gossip to target index
            p->gossip(tar);
            
            // limit gossip interval
            usleep(interval);

            // print nodes worth
            if (!DEBUG) {
                if (printerval > 1000000) {
                    printerval = 0;
                    std::cout << "\rNetworth (view of " << p->ep.index <<"): ";
                    for (std::size_t i = 0; i < p->networth.size(); i++) {
                        std::cout << p->networth.at(i) << " ";
                    }
                    std::cout << std::flush;
                }
                printerval += interval;
            }

            // exit normally after SIGINT
            if (quit.load()) 
                break;    
        }
    }
}