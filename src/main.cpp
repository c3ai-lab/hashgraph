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

#ifndef DEBUG
#define DEBUG 0
#endif

std::atomic<bool> quit;

int main(int argc, char** argv) {

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
    printf("test0");
    fflush(stdout);
    person.initDatabase(config["databasePath"].as<std::string>());

    std::vector<hashgraph::message::BalanceTransfer> transactions;
    person.get_transaction(transactions, 1623190865829, 1623202949011);

    //person.balance_history(transactions, "11xa202effc3bb275689552d1ad1b0264c68dd036d4");
    printf("test2");
    fflush(stdout);

    //printf("Result %d", transactions);

    /*for (auto eit = transactions.begin(); eit != transactions.end(); ++eit){
        printf("Transaction sender %s", (*eit).senderId.c_str());
        fflush(stdout);
    }*/

    return EXIT_SUCCESS;

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
