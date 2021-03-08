#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>

#include "PersonNetworker.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hashgraph {
namespace protocol {


PersonNetworker::PersonNetworker(std::map<int, HashgraphNode> *nodes) : nodes(nodes) {
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

void *PersonNetworker::serverStarter(PersonNetworker* ctx, int port) {
    if (ctx->force_close) return NULL;

    // https://stackoverflow.com/questions/28523035/best-way-to-create-a-fake-smart-pointer-when-you-need-one-but-only-have-a-refere
    std::shared_ptr<PersonNetworker> handler(std::shared_ptr<PersonNetworker>(std::shared_ptr<PersonNetworker>(), ctx));
    std::shared_ptr<TProcessor> processor(new message::GossipProcessor(handler));
    std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // start server
    ctx->server = std::make_shared<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);

    // wait for incoming messages
    ctx->server->serve();

    return NULL;
}

void PersonNetworker::startServer(int32_t index) {
    this->thread = std::make_shared<std::thread>(
        std::bind(&PersonNetworker::serverStarter, this, this->nodes->at(index).port)
    );
}

bool PersonNetworker::sendGossip(int32_t gossiper, int32_t target, std::vector<message::Data> const &gossip) {

	auto socket    = std::make_shared<TSocket>(this->nodes->at(target).address, this->nodes->at(target).port);
	auto transport = std::make_shared<TBufferedTransport>(socket);
	auto protocol  = std::make_shared<TBinaryProtocol>(transport);
 
	message::GossipClient client(protocol);
 
	try {
        // connect to remote server
		transport->open();
        // exchange gossip data
		client.recieveGossip(gossiper, gossip);
        // close connection
		transport->close();
	}
	catch (TException& tx) {
        return false;
	}
    return true;
}

void PersonNetworker::recieveGossip(const int32_t gossiper, const std::vector<message::Data> &gossip) {
    // dummy
}


};
};