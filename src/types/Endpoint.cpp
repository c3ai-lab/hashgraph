
#include <fstream>
#include <streambuf>

#include "Endpoint.hpp"
#include "../utils/hashgraph_utils.hpp"

namespace hashgraph {
namespace types {

Endpoint::Endpoint(const std::string host, int port, const std::string certPath) :
	port(port),
	host(host) {

	// read certificate from disk
	std::ifstream cert(certPath);
	this->certificatePEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());

	// build public identifier
	this->identifier = utils::getIdentifierFromCertPEM(this->certificatePEM);
}

Endpoint::~Endpoint() {}

};
};