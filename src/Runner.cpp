
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstring>

#include "Runner.hpp"
#include "utils/hashgraph_utils.hpp"

namespace hashgraph {

    std::atomic<bool> quit;

    Runner::Runner() {
        // initialize md5 utility
        utils::md5Init();
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

    void Runner::addHashgraphNode(int index, int isLocal, std::string address, int port) {
        protocol::HashgraphNode node;
        node.index         = index;
        node.isLocal       = isLocal;
        node.address       = address;
        node.port          = port;
        this->nodes[index] = node;
    }

    void Runner::startHashgraph(int interval) {
        if (this->nodes.size() == 0)
            throw std::invalid_argument("No nodes given");

        std::map<int, protocol::HashgraphNode>::iterator it;
        for (it = this->nodes.begin(); it != this->nodes.end(); it++) {
            // check whether the node should be run locally
            if (it->second.isLocal) {
                this->persons.push_back(
                    new protocol::Person(it->first, &(this->nodes))
                );
            }
        }

        // limits networth prints
        int printerval = 0;

        while (true) {

            // select a random person from the local persons list
            protocol::Person *p = this->persons.at(std::rand() % this->persons.size());

            // select a random target index from the list of known nodes
            int j;
            while ((j = this->nodes.at(std::rand() % this->nodes.size()).index) == p->index);

            // gossip to target index
            p->gossip(j);
            
            // limit gossip interval
            usleep(interval);

            // print nodes worth
            if (!DEBUG) {
                if (printerval > 1000000) {
                    printerval = 0;
                    std::cout << "\rNetworth (view of " << p->index <<"): ";
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