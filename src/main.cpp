
#include <iostream>
#include "Runner.hpp"

int main(int argc, char** argv) {

	if (argc <= 1) {
		std::cout << "Invalid number of arguments. Usage: ./hashgraph \"/path/to/tests/config/node0/settings.yaml\"" << std::endl;
		return EXIT_FAILURE;
	}

	// hashgraph runner
	hashgraph::Runner runner;

	// start protocol
	runner.runHashgraphProtocol(argv[1]);

	return EXIT_SUCCESS;
}