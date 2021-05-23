#include <fstream>
#include <streambuf>
#include <stdio.h>
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

    // client socket setup
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->loadTrustedCertificatesFromBuffer(certificatePEM.c_str());
	std::shared_ptr<TSSLSocket> socket = sslSocketFactory->createSocket(host, port);
    socket->setRecvTimeout(30000);
    socket->setSendTimeout(30000);

	std::shared_ptr<TBufferedTransport> transport = std::make_shared<TBufferedTransport>(socket);
	std::shared_ptr<TBinaryProtocol> protocol     = std::make_shared<TBinaryProtocol>(transport);

	this->client = std::unique_ptr<message::HashgraphClient>(
		new message::HashgraphClient(protocol)
	);
}

Endpoint::~Endpoint() {
	if (this->client->getInputProtocol()->getTransport()->isOpen()) {
        this->client->getInputProtocol()->getTransport()->close();
    }		
}

void Endpoint::exchangeGossipData(const std::string senderId, const std::vector<message::GossipData> &gossipData) {
    if (this->gspMutex.try_lock()) {
        try {
            // connect to remote server
            if (!this->client->getInputProtocol()->getTransport()->isOpen()) {
                this->client->getInputProtocol()->getTransport()->open();
            }
            // send gossip data
            this->client->receiveGossip(senderId, gossipData);
            // close connection
            this->client->getInputProtocol()->getTransport()->close();
        }
        catch (TException& tx) {
            fprintf(stderr, "%s\n", tx.what());
        }
        this->gspMutex.unlock();
    }
}

std::string Endpoint::getIdentifier() const {
    return this->identifier;
}

};
};