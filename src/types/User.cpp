#include <string>
#include <fstream>
#include <streambuf>

#include "User.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace types {


User::User(std::string certPath) : networth(0) {

	// read cert from disk
	std::ifstream cert(certPath);
	this->certificatePEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());

	// openssl key objects
	this->pubKey = utils::getPubFromCertPEM(this->certificatePEM);
}

User::~User() {
	EC_KEY_free(this->pubKey);
}


};
};