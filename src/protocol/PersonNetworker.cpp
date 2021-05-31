#include <fstream>
#include <streambuf>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSSLServerSocket.h>
#include "PersonNetworker.hpp"
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hashgraph {
namespace protocol {

PersonNetworker::PersonNetworker(const std::string skPath, const std::string certPath) : 
    force_close(false) {

    // read private key from disk
    std::ifstream sk(skPath);
    this->skPEM.assign(std::istreambuf_iterator<char>(sk), std::istreambuf_iterator<char>());

    // read certificate from disk
    std::ifstream cert(certPath);
    this->certPEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());
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
    std::shared_ptr<TProcessor> processor(new message::HashgraphProcessor(handler));
    std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // ssl transport setup
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->loadCertificateFromBuffer(ctx->certPEM.c_str());
    sslSocketFactory->loadPrivateKeyFromBuffer(ctx->skPEM.c_str());
    sslSocketFactory->authenticate(false);

    // server socket setup
    TSSLServerSocket *socket = new TSSLServerSocket(port, 30000, 30000, sslSocketFactory);
    socket->setKeepAlive(false);

    // server transport setup
    std::shared_ptr<TServerTransport> transport(socket);

    // create server
    ctx->server = std::make_shared<TThreadPoolServer>(processor, transport, transportFactory, protocolFactory, ctx->getManager());

    // listen for incoming messages
    ctx->server->serve();

    return NULL;
}

void PersonNetworker::startServer(int port) {
    this->thread = std::make_shared<std::thread>(
        std::bind(&PersonNetworker::serverStarter, this, port)
    );
}

int32_t PersonNetworker::balance(const std::string& ownerId) {
    std::vector<message::BalanceTransfer> history;
    utils::getTransferHistory(this->getDatabasePath(), ownerId, history);

    std::string chlng;
    this->challenge(chlng);

    int32_t amount = 0;
    for(std::vector<message::BalanceTransfer>::iterator it = history.begin(); it != history.end(); ++it) {
        if (utils::verifyGossipPayload(it->pkDer, it->amount, it->receiverId, chlng, it->sigDer)) {
            if (it->senderId   == ownerId) amount -= it->amount;
            if (it->receiverId == ownerId) amount += it->amount;
        }
    }
    return amount;
}

void PersonNetworker::balance_history(std::vector<message::BalanceTransfer> & _return, const std::string& ownerId) {
    utils::getTransferHistory(this->getDatabasePath(), ownerId, _return);
}

void PersonNetworker::challenge(std::string& _return) {
    _return.assign("dummy");
}

};
};
