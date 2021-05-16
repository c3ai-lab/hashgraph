
#include <fstream>
#include <streambuf>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSSLSocket.h>
#include "Endpoint.hpp"
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hashgraph {
namespace types {

Endpoint::Endpoint(const std::string host, int port, const std::string certPath) {

	// read certificate from disk
	std::ifstream cert(certPath);
	std::string certificatePEM{std::istreambuf_iterator<char>{cert}, {}};

	// build public identifier
	this->identifier = utils::getIdentifierFromCertPEM(certificatePEM);

	// create client
	std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->loadTrustedCertificatesFromBuffer(certificatePEM.c_str());

	std::shared_ptr<TSSLSocket> socket            = sslSocketFactory->createSocket(host, port);
	std::shared_ptr<TBufferedTransport> transport = std::make_shared<TBufferedTransport>(socket);
	std::shared_ptr<TBinaryProtocol> protocol     = std::make_shared<TBinaryProtocol>(transport);

	this->client = std::unique_ptr<message::GossipClient>(
		new message::GossipClient(protocol)
	);
}

bool Endpoint::exchangeGossipData(const std::string senderId, const std::vector<message::Data> &gossipData) {
	try {
		// connect to remote server
		if (!this->client->getInputProtocol()->getTransport()->isOpen())
			this->client->getInputProtocol()->getTransport()->open();

		// send gossip data
		this->client->receiveGossip(senderId, gossipData);
	}
	catch (TException& tx) {
        return false;
	}
    return true;
}

Endpoint::~Endpoint() {
	if (this->client->getInputProtocol()->getTransport()->isOpen())
		this->client->getInputProtocol()->getTransport()->close();
}

};
};