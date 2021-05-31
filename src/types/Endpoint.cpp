#include <fstream>
#include <streambuf>
#include <cstdio>
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
    std::string certPEM{std::istreambuf_iterator<char>{cert}, {}};

    // DER encoded public key
    this->pkDer = utils::getPublicDERFromCertPEM(certPEM);

    // build identifier
    this->identifier = utils::encodeIdentifier(this->pkDer);

    // socket factory setup
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->loadTrustedCertificatesFromBuffer(certPEM.c_str());

    // socket setup
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

void Endpoint::exchangeGossipData(const message::GossipPacket packet, const std::string sigDer) {
    if (this->gspMutex.try_lock()) {
        try {
            // connect to remote server
            if (!this->client->getInputProtocol()->getTransport()->isOpen()) {
                this->client->getInputProtocol()->getTransport()->open();
            }
            // send gossip data
            this->client->receiveGossip(packet, sigDer);
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
