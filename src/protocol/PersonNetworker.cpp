#include <fstream>
#include <streambuf>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TSSLServerSocket.h>
#include <thrift/concurrency/ThreadFactory.h>
#include "PersonNetworker.hpp"
#include "../utils/hashgraph_utils.hpp"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace hashgraph {
namespace protocol {

PersonNetworker::PersonNetworker(const std::string skPath, const std::string certPath) {

    // read private key from disk
    std::ifstream sk(skPath);
    this->skPEM.assign(std::istreambuf_iterator<char>(sk), std::istreambuf_iterator<char>());

    // read certificate from disk
    std::ifstream cert(certPath);
    this->certPEM.assign(std::istreambuf_iterator<char>(cert), std::istreambuf_iterator<char>());
}

PersonNetworker::~PersonNetworker() {
    server->stop();
    thread->join();
}

void PersonNetworker::startServer(int port) {

    // https://stackoverflow.com/questions/28523035/best-way-to-create-a-fake-smart-pointer-when-you-need-one-but-only-have-a-refere
    std::shared_ptr<PersonNetworker> handler(std::shared_ptr<PersonNetworker>(std::shared_ptr<PersonNetworker>(), this));
    std::shared_ptr<TProcessor> processor(new message::HashgraphProcessor(handler));
    std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // ssl transport setup
    std::shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory(SSLProtocol::TLSv1_2));
    sslSocketFactory->loadCertificateFromBuffer(this->certPEM.c_str());
    sslSocketFactory->loadPrivateKeyFromBuffer(this->skPEM.c_str());
    sslSocketFactory->authenticate(false);

    // server transport setup
    std::shared_ptr<TServerTransport> transport(new TSSLServerSocket(port, 30000, 30000, sslSocketFactory));

    // create server and runner
    this->server = std::make_shared<TThreadPoolServer>(processor, transport, transportFactory, protocolFactory, this->getManager());
    std::shared_ptr<Runnable> runner(this->server);

    // detached thread factory
    ThreadFactory factory(false);

    // create and start server thread
    this->thread = factory.newThread(runner);
    this->thread->start();
}

int32_t PersonNetworker::balance(const std::string &ownerId) {
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

void PersonNetworker::balance_history(std::vector<message::BalanceTransfer> &_return, const std::string &ownerId) {
    utils::getTransferHistory(this->getDatabasePath(), ownerId, _return);
}

void PersonNetworker::challenge(std::string &_return) {
    _return.assign("dummy");
}

void PersonNetworker::get_transaction(std::vector<message::BalanceTransfer> &_return, const int64_t fromUnix, const int64_t toUnix) {
    printf("test1");
    fflush(stdout);
    utils::getTransactions(this->getDatabasePath(), fromUnix, toUnix, _return);
}

};
};
