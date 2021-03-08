#ifndef HASHGRAPH_NODE_HPP
#define HASHGRAPH_NODE_HPP

#include <string>

namespace hashgraph {
namespace protocol {


/**
 * Hashgraph node
 */
struct HashgraphNode {

    /**
     * Index of this node
     */
    int index;

    /**
     * Indicates whether this is a local node
     */
    int isLocal;
    
    /**
     * IP address of the node
     */
    std::string address;

    /**
     * Port of the node
     */
    int port;
};


};
};
#endif