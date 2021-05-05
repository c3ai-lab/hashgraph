
#include <fstream>
#include <streambuf>

#include "Endpoint.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace types {


Endpoint::Endpoint(int index, std::string address, int port, int isLocal, std::string certPath, std::string keyPath) :
	index(index),
	isLocal(isLocal),
	port(port),
	address(address) {

	// read cert from disk
	std::ifstream cert(certPath);
	this->certificatePEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());

	// read priv key from disk
	std::ifstream privKey(keyPath);
	this->privKeyPEM.assign(std::istreambuf_iterator<char>(privKey), std::istreambuf_iterator<char>());

	// openssl key objects
	this->privKey = utils::getPrivFromPEM(this->privKeyPEM);
	this->pubKey  = utils::getPubFromCertPEM(this->certificatePEM);
}

Endpoint::~Endpoint() {
	EC_KEY_free(this->privKey);
	EC_KEY_free(this->pubKey);
}


};
};