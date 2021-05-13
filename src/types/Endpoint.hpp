#ifndef HASHGRAPH_TYPES_ENDPOINT_HPP
#define HASHGRAPH_TYPES_ENDPOINT_HPP

#include <string>

namespace hashgraph {
namespace types {

/**
 * Endpoint class
 */
class Endpoint {

	public:

        /**
         * Remote port
         */
		int port;

        /**
         * Unique identifier of the node
         */
		std::string identifier;

		/**
         * Remote host
         */
		std::string host;
		
		/**
         * Certificate in PEM format
         */
		std::string certificatePEM;

        /**
         * Constructor
         */
		Endpoint(const std::string host, int port, const std::string certPath);

        /**
         * Destructor
         */
		~Endpoint();

};

};
};
#endif
