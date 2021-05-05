#ifndef HASHGRAPH_TYPES_USER_HPP
#define HASHGRAPH_TYPES_USER_HPP

#include <string>
#include <cstdint>
#include <openssl/ec.h>

namespace hashgraph {
namespace types {

/**
 * User class
 */
class User {

	public:

		/**
         * Hashed public key
         */
		std::string identifier;

        /**
         * Networth
         */
		int64_t networth;
		
		/**
         * Certificate in PEM format
         */
		std::string certificatePEM;

        /**
         * Public key object
         */
		EC_KEY* pubKey;

        /**
         * Constructor
         */
		User(std::string certPath);

        /**
         * Destructor
         */
		~User();

};


};
};
#endif
