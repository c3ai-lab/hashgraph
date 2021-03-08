#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "Runner.hpp"

int main(int argc, char** argv) {

    // hashgraph runner
	hashgraph::Runner runner;

    // message interval
    int interval = 100000;

    // handle command line inputs
    while (true) {
        int result = getopt(argc, argv, "n:i:");
        if (result == -1) break; /* end of list */
        switch (result) {
            case '?': /* unknown parameter */
                break;
            case ':': /* missing argument of a parameter */
                fprintf(stderr, "missing argument.\n");
                break;
            case 'n': { /* handle network nodes */
                    // node index
                    std::size_t seg0 = std::string(optarg).find(':');
                    if (seg0 != std::string::npos) {
                        // local or remote flag
                        std::size_t seg1 = std::string(optarg).find(':', seg0+1);
                        if (seg1 != std::string::npos) {
                            // address segment
                            std::size_t seg2 = std::string(optarg).find(':', seg1+1);
                            if (seg2 != std::string::npos) {
                                // add network node to the list
                                runner.addHashgraphNode(
                                    // index of the node
                                    std::stoi(std::string(optarg).substr(0, seg0)), 
                                    // flag that indicates whether this is a local node
                                    std::stoi(std::string(optarg).substr(seg0+1, seg1-seg0-1)),
                                    // extract the address
                                    std::string(optarg).substr(seg1+1, seg2-seg1-1),
                                    // extract the port 
                                    std::stoi(std::string(optarg).substr(seg2+1))
                                );
                            }
                        }
                    }
                }
                break;
            case 'i': /* message interval */
                interval = std::stoi(std::string(optarg));
                break;
            default: /* unknown */
                break;
        }
    }

	// start hashgraph protocol
	runner.startHashgraph(interval);
	
	return EXIT_SUCCESS;
}
