
namespace cpp hashgraph.message

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