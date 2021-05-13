
namespace cpp hashgraph.message

// payload data
struct Payload {
	1: string senderId;
	2: string receiverId;
	3: i32    amount;
}

// gossip data
struct Data {
  	1: optional Payload payload;
  	2: string  	selfHash;
  	3: string  	gossipHash;
	4: i64	   	timestamp;
  	5: string  	owner;
}

// service definition
service Gossip {
	// exchange gossip data
	void receiveGossip(1:string gossiper, 2:list<Data> gossip)
	// initiate transfer (with crypto)
	void crypto_transfer(1:binary ownerPkDer, 2:i32 amount, 3:string receiverId, 4:binary challenge, 5:binary sigDer)
	// initiate transfer (with crypto)
	i32 user_amount(1:string ownerId)
}