#include <iostream>
#include <atomic>
#include <cstring>
#include <vector>
#include <string>
#include <signal.h>
#include <stdlib.h>
#include "yaml-cpp/yaml.h"
#include "types/Endpoint.hpp"
#include "protocol/Person.hpp"

#include <iostream>

#ifndef DEBUG
#define DEBUG 0
#endif

std::atomic<bool> quit;

int main(int argc, char** argv) {

    quit = true; // initially stops the gossipping and ends the application normally

    if (argc <= 1) {
        std::cout << "Invalid number of arguments. Usage: ./hashgraph \"/path/to/tests/config/node0/settings.yaml\"" << std::endl;
        return EXIT_FAILURE;
    }

    // prevents application crash when using ssl
    // https://thrift.apache.org/lib/cpp#sigpipe-signal
    signal(SIGPIPE, SIG_IGN);

    // detect interrupt signal
    signal(SIGINT, [](int) {
        quit.store(true);
    });

    printf("Starting...");
    fflush(stdout);
    // read node configuration file
    YAML::Node config = YAML::LoadFile(argv[1]);

    // suppress thrift global output
    apache::thrift::GlobalOutput.setOutputFunction([](const char* msg) {
        if (DEBUG) apache::thrift::TOutput::errorTimeWrapper(msg);
    });

    // list of network nodes
    std::vector<hashgraph::types::Endpoint*> endpoints;
    for (const auto& n: config["network"].as<std::vector<YAML::Node>>()) {
        endpoints.push_back(new hashgraph::types::Endpoint(n["host"].as<std::string>(), n["port"].as<int>(), n["certPath"].as<std::string>()));
    }

    // local hashgraph node
    hashgraph::protocol::Person person(
        config["keyPath"].as<std::string>(), 
        config["certPath"].as<std::string>(), 
        &endpoints
    );


    // initialize database
    person.initDatabase(config["databasePath"].as<std::string>());


    // test
    std::vector<hashgraph::message::BalanceTransfer> transactions;
    person.get_transaction(transactions, 0, 9623202949011);

    for (size_t i=0; i<transactions.size(); i++) {
        // c++ method of printing text
        // requires #include <iostream>

        std::cout << "sID: "       << transactions.at(i).senderId   << std::endl;
        std::cout << "rID: "       << transactions.at(i).receiverId << std::endl;
        //std::cout << "sigDer: "    << transactions.at(i).sigDer     << std::endl;
        std::cout << "Amount: "    << transactions.at(i).amount     << std::endl;
        std::cout << "Timestamp: " << transactions.at(i).timestamp  << std::endl;

        std::cout << std::endl; // endl = newline
    }

    // enable/disable event logging
    person.setEventLogging(config["logEvents"].as<bool>());
    // start server process
    person.startServer(config["port"].as<int>());
    // start gossiping to other nodes
    person.startGossip(config["interval"].as<int>(), &quit);

    // free memory
    for (auto eit = endpoints.begin(); eit != endpoints.end(); ++eit)
        delete (*eit);

    return EXIT_SUCCESS;
}
