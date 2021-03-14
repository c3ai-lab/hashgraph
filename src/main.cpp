#include <string>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include "Runner.hpp"

int main(int argc, char** argv) {
    if (argc <= 2) {
        std::cout << "Invalid number of arguments. Usage: ./hashgraph \"/path/to/tests/config/sample.json\" 100000" << std::endl;
        return EXIT_FAILURE;
    }

    // prevents application crash when using ssl
    // https://thrift.apache.org/lib/cpp#sigpipe-signal
    signal(SIGPIPE, SIG_IGN);

    // hashgraph runner
	hashgraph::Runner runner;

    // init runner
    runner.initHashgraphRunner(argv[1]);

	// start hashgraph protocol
	runner.startHashgraph(std::stoi(std::string(argv[2])));
	
	return EXIT_SUCCESS;
}