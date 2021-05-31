namespace cpp hashgraph.message

///////////////////////////////////////////////////////////
// Defines a balance transfer from a sender to a receiver
///////////////////////////////////////////////////////////

struct BalanceTransfer {
    1:string senderId;
    2:string receiverId;
    3:string pkDer;
    4:string sigDer;
    5:i32    amount;
    6:i64    timestamp;
}

///////////////////////////////////////////////////////////
// Payload of the gossiping data
///////////////////////////////////////////////////////////

struct GossipPayload {
    1:binary senderPkDer;
    2:i32    amount;
    3:string receiverId;
    4:binary challenge; 
    5:binary sigDer;
}

///////////////////////////////////////////////////////////
// Data to be exchanged through node gossiping
///////////////////////////////////////////////////////////

struct GossipData {
    1:optional GossipPayload payload;
    2:string   selfHash;
    3:string   gossipHash;
    4:i64      timestamp;
    5:string   owner;
}

struct GossipPacket {
    1:string           gossiper;
    2:list<GossipData> data;
}

///////////////////////////////////////////////////////////
// Hashgraph service definition
///////////////////////////////////////////////////////////

service Hashgraph {

    // exchange gossip data
    void receiveGossip(1:GossipPacket packet, 2:string sigDer)

    // initiate a balance transfer
    void crypto_transfer(1:binary ownerPkDer, 2:i32 amount, 3:string receiverId, 4:binary challenge, 5:binary sigDer)

    // return the balance of the given owner
    i32 balance(1:string ownerId)

    // return the balance history of the given owner
    list<BalanceTransfer> balance_history(1:string ownerId)

    // return a challenge to be used within a transfer
    binary challenge()
}