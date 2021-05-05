#ifndef HASHGRAPH_TYPES_ENDPOINT_HPP
#define HASHGRAPH_TYPES_ENDPOINT_HPP

#include <string>
#include <openssl/ec.h>

namespace hashgraph {
namespace types {

/**
 * Endpoint class
 */
class Endpoint {

	public:

		/**
         * Unique index of the node
         */
		int index;

		/**
         * Flag that incidates a local node
         */
		int isLocal;

		/**
         * Port
         */
		int port;

		/**
         * IP address
         */
		std::string address;
		
		/**
         * Certificate in PEM format
         */
		std::string certificatePEM;

		/**
         * Private key in PEM format
         */
		std::string privKeyPEM;

        /**
         * Public key object
         */
		EC_KEY* pubKey;

		/**
         * Private key object
         */
		EC_KEY* privKey;

        /**
         * Constructor
         */
		Endpoint(int index, std::string address, int port, int isLocal, std::string certPath, std::string keyPath);

        /**
         * Destructor
         */
		~Endpoint();

};


};
};
#endif
