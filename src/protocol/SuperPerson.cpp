#include "SuperPerson.hpp"

namespace hashgraph {
namespace protocol {

SuperPerson::SuperPerson()  { 
    // setup the nodes' thread pool manager
    this->manager = ThreadManager::newSimpleThreadManager(5, 20);
    manager->threadFactory(std::make_shared<ThreadFactory>());
    manager->start();
}

SuperPerson::~SuperPerson()  {
    // join all threads from the pool
    this->manager->stop();
}

std::shared_ptr<ThreadManager> SuperPerson::getManager() const {
    return this->manager;
}

std::string SuperPerson::getDatabasePath() const {
    return this->databasePath;
}

void SuperPerson::initDatabase(const std::string databasePath) {
    this->databasePath = databasePath;
    utils::createDatabaseTables(databasePath);
}

void SuperPerson::setIdentifier(const std::string identifier) {
    this->identifier = identifier;
}

std::string SuperPerson::getIdentifier() const {
    return this->identifier;
}

};
};
