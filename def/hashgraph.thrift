
namespace cpp hashgraph.message

// endpoint data
struct Endpoint {
  1: i32    index;
	2: string address;
  3: i32    port;
  4: i32    isLocal;
	5: string certPath;
  6: string keyPath;
}

// application settings
struct AppSettings {
  1:list<Endpoint> endpoints
}

// gossip data
struct Data {
  1: i32    payload;
	2: string selfHash;
	3: string gossipHash;
	4: i64	  timestamp;
  5: i32    target;
	6: i32    owner;
}

// service definition
service Gossip {
    // exchange gossip data
    void recieveGossip(1:i32 gossiper, 2:list<Data> gossip)
    // initiate transfer
    void transfer(1:i32 payload, 2:i32 target)
}