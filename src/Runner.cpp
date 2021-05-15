
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <cstring>

#include "yaml-cpp/yaml.h"

#include "Runner.hpp"

namespace hashgraph {

    std::atomic<bool> quit;

    Runner::Runner() {
        // set thrift global output
        apache::thrift::GlobalOutput.setOutputFunction([](const char* msg) {
            if (DEBUG) {
                apache::thrift::TOutput::errorTimeWrapper(msg);
            }
        });

		// prevents application crash when using ssl
		// https://thrift.apache.org/lib/cpp#sigpipe-signal
		signal(SIGPIPE, SIG_IGN);

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

        for (std::vector<types::Endpoint*>::iterator eit = this->endpoints.begin(); eit != this->endpoints.end(); ++eit)
            delete (*eit);
    }

    void Runner::runHashgraphProtocol(const std::string configPath) {

        // read node configuration file
        YAML::Node config = YAML::LoadFile(configPath);

        // create list of network nodes
        for (const auto& n: config["network"].as<std::vector<YAML::Node>>()) {
            this->endpoints.push_back(
                new types::Endpoint(n["host"].as<std::string>(), n["port"].as<int>(), n["certPath"].as<std::string>())
            );
        }

        // hashgraph node
        protocol::Person person(
            config["databasePath"].as<std::string>(),
            config["keyPath"].as<std::string>(), 
            config["certPath"].as<std::string>(), 
            &(this->endpoints));

		// start listening for requests
        person.startServer(config["port"].as<int>());

        // target endpoint
        types::Endpoint *tar;

        while (true) {

            // select a random target from the list of known nodes
            while ((tar = this->endpoints.at(std::rand() % this->endpoints.size()))->identifier == person.identifier);

            // gossip to target
            person.gossip(tar);
            
            // limit gossip interval
            usleep(config["interval"].as<int>());

            // exit normally after SIGINT
            if (quit.load()) 
                break;    
        }
    }
}