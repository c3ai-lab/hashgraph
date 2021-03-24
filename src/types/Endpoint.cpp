#include <string>
#include <fstream>
#include <streambuf>

#include "Endpoint.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace types {


Endpoint::Endpoint(message::Endpoint const &ep) {

	this->address = ep.address;
	this->port    = ep.port;
	this->index   = ep.index;
	this->isLocal = ep.isLocal;

	// read cert from disk
	std::ifstream cert(ep.certPath);
	this->certificatePEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());

	// read priv key from disk
	std::ifstream privKey(ep.keyPath);
	this->privKeyPEM.assign(std::istreambuf_iterator<char>(privKey), std::istreambuf_iterator<char>());

	// get openssl key objects
	this->privKey = utils::getPrivFromPEM(this->privKeyPEM);
	this->pubKey  = utils::getPubFromCertPEM(this->certificatePEM);
}

Endpoint::~Endpoint() {
	EC_KEY_free(this->privKey);
	EC_KEY_free(this->pubKey);
}


};
};