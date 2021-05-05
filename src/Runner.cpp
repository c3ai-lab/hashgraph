
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstring>

#include "yaml-cpp/yaml.h"

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

        std::vector<protocol::Person*>::iterator pit;
        for (pit = this->persons.begin(); pit != this->persons.end(); pit++)
            delete (*pit);

        std::vector<types::Endpoint*>::iterator eit;
        for (eit = this->endpoints.begin(); eit != this->endpoints.end(); eit++)
            delete (*eit);
    }

    void Runner::initHashgraphRunner(std::string configFile) {

        // read config file
        YAML::Node config = YAML::LoadFile(configFile);

        if (!config["endpoints"]) 
            throw std::invalid_argument("No endpoints given");

        // parse hashgraph endpoints
        for (const auto& n: config["endpoints"].as<std::vector<YAML::Node>>()) {
            this->endpoints.push_back(
                new types::Endpoint(
                    n["index"].as<int>(),
                    n["address"].as<std::string>(),
                    n["port"].as<int>(),
                    n["isLocal"].as<int>(),
                    n["certPath"].as<std::string>(),
                    n["keyPath"].as<std::string>()
                )
            );
        }

        /*
        if (!config["users"]) 
            throw std::invalid_argument("No users given");

        // parse hashgraph user
        for (const auto& n: config["users"].as<std::vector<YAML::Node>>()) {
            this->users.push_back(
                new types::User(
                    n["index"].as<int>(),
                    n["certPath"].as<std::string>()
                )
            );
        }
        */

    }

    void Runner::startHashgraph(int interval) {
        if (this->endpoints.size() == 0)
            throw std::invalid_argument("No endpoints set");

        std::vector<types::Endpoint*>::iterator it;
        for (it = this->endpoints.begin(); it != this->endpoints.end(); it++) {
            // check whether the node should be run locally
            if ((*it)->isLocal) {
                this->persons.push_back(
                    new protocol::Person((*it), &(this->endpoints))
                );
            }
        }

        // limits networth prints
        int printerval = 0;
        // target endpoint
        types::Endpoint *tar;

        while (true) {

            // select a random person from the local persons list
            protocol::Person *p = this->persons.at(std::rand() % this->persons.size());

            // select a random target from the list of known nodes
            while ((tar = this->endpoints.at(std::rand() % this->endpoints.size()))->index == p->ep->index);

            // gossip to target
            p->gossip(tar);
            
            // limit gossip interval
            usleep(interval);

            // print nodes worth
            if (!DEBUG) {
                if (printerval > 1000000) {
                    printerval = 0;
                    std::cout << "\rNetworth (view of " << p->ep->index <<"): ";
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