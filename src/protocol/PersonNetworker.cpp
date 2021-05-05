#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TSSLServerSocket.h>

#include "PersonNetworker.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hashgraph {
namespace protocol {


PersonNetworker::PersonNetworker(types::Endpoint *ep, std::vector<types::Endpoint*> *endpoints) : ep(ep), endpoints(endpoints) {
    this->force_close = false;
}

PersonNetworker::~PersonNetworker() {
    this->force_close = true;

    // stop the server
    if (this->server) {
        this->server->stop();
    }
    // wait for receiver thread to exit
    if (this->thread && this->thread->joinable()) {
        this->thread->join();
    }
}

void *PersonNetworker::serverStarter(PersonNetworker* ctx) {
    if (ctx->force_close) return NULL;

    // https://stackoverflow.com/questions/28523035/best-way-to-create-a-fake-smart-pointer-when-you-need-one-but-only-have-a-refere
    std::shared_ptr<PersonNetworker> handler(std::shared_ptr<PersonNetworker>(std::shared_ptr<PersonNetworker>(), ctx));
    std::shared_ptr<TProcessor> processor(new message::GossipProcessor(handler));
    std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // ssl transport
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    sslSocketFactory->loadCertificateFromBuffer(ctx->ep->certificatePEM.c_str());
    sslSocketFactory->loadPrivateKeyFromBuffer(ctx->ep->privKeyPEM.c_str());
    sslSocketFactory->authenticate(false);
    std::shared_ptr<TServerTransport> serverTransport(new TSSLServerSocket(ctx->ep->port, sslSocketFactory));

    // start server
    ctx->server = std::make_shared<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);

    // wait for incoming messages
    ctx->server->serve();

    return NULL;
}

void PersonNetworker::startServer() {
    this->thread = std::make_shared<std::thread>(
        std::bind(&PersonNetworker::serverStarter, this)
    );
}

bool PersonNetworker::sendGossip(types::Endpoint *gossiper, types::Endpoint *target, std::vector<message::Data> const &gossip) {

    // ssl transport
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    sslSocketFactory->loadTrustedCertificatesFromBuffer(target->certificatePEM.c_str());

	std::shared_ptr<TSSLSocket> socket            = sslSocketFactory->createSocket(target->address, target->port);
	std::shared_ptr<TBufferedTransport> transport = std::make_shared<TBufferedTransport>(socket);
	std::shared_ptr<TBinaryProtocol> protocol     = std::make_shared<TBinaryProtocol>(transport);

	message::GossipClient client(protocol);

	try {
        // connect to remote server
		transport->open();
        // exchange gossip data
		client.recieveGossip(gossiper->index, gossip);
        // close connection
		transport->close();
	}
	catch (TException& tx) {
        return false;
	}
    return true;
}


};
};