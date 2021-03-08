
namespace cpp hashgraph.message

// gossip data
struct Data {
  1: i64    seqNum;
  2: i32    payload;
	3: string selfHash;
	4: string gossipHash;
	5: i64	  timestamp;
  6: i32    target;
	7: i32    owner;
}

// Service definition
service Gossip {

    // exchange gossip data
    void recieveGossip(1:i32 gossiper, 2:list<Data> gossip),
}