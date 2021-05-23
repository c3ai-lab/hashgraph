#ifndef HASHGRAPH_UTILS_HPP
#define HASHGRAPH_UTILS_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <openssl/ec.h>
#include "sqlite3.h"
#include "../message/Hashgraph.h"

namespace hashgraph {
namespace utils {

/**
 * Return a SHA384 byte string of the given input
 *
 * @param data The string to hash
 */
const std::string SHA384(const std::string data);

/**
 * Return a SHA256 byte string of the given input
 *
 * @param data The string to hash
 */
const std::string SHA256(const std::string data);

/**
 * Return a SHA3 (256 bit) byte string of the given input
 *
 * @param data The string to hash
 */
const std::string SHA3(const std::string data);

/**
 * Encodes the public identifier from a DER encoded public key
 *
 * @param data The string to hash
 */
const std::string encodeIdentifier(const std::string pubKeyDER);

/**
 * Decodes a hex string to a byte string
 *
 * @param hex Hex string to be decoded
 */
const std::string hexToByte(const std::string hex);

/**
 * Encodes a raw byte string to its hex representation
 *
 * @param bytes Byte string to be encoded
 */
const std::string byteToHex(const std::string bytes);

/**
 * Verifies a ECDSA signature
 *
 * @param pubKeyDER A byte string of the DER encoded public key
 * @param sigDER A byte string of the DER encoded signature
 * @param msg The message to verify
 */
bool verifyECDSASignature(const std::string pubKeyDER, const std::string sigDER, const std::string msg);

/**
 * Get an EC private key object from the given PEM
 *
 * @param pem
 */
EC_KEY* getKeyFromPrivatePEM(const std::string pem);

/**
 * Get an EC public key object from the given certificate PEM
 *
 * @param pem
 */
EC_KEY* getKeyFromCertPEM(const std::string pem);

/**
 * DER encodes the public key part of the given key struct
 *
 * @param key
 */
std::string encodeKeyToPublicDER(EC_KEY *key);

/**
 * Builds the public identifier from a private key pem
 *
 * @param pem
 */
std::string getIdentifierFromPrivatePEM(const std::string pem);

/**
 * Builds the public identifier from a certificate pem
 *
 * @param pem
 */
std::string getIdentifierFromCertPEM(const std::string pem);

/**
 * Create nessessary database tables
 *
 * @param databasePath Path of the sqlite file
 */
void createDatabaseTables(const std::string databasePath);

/**
 * Store the transfer data
 *
 * @param databasePath
 * @param sender
 * @param receiver
 * @param amount
 * @param timestamp
 */
void storeTransferData(const std::string databasePath, const std::string sender, const std::string receiver, int amount, int64_t timestamp);

/**
 * Return the balance history
 *
 * @param databasePath
 * @param identifier
 * @param history
 */
void getTransferHistory(const std::string databasePath, const std::string identifier, std::vector<message::BalanceTransfer> &history);

/**
 * Write to log
 *
 * @param databasePath
 * @param owner
 * @param round
 * @param time
 * @param cnsTime
 * @param selfHash
 * @param gossipHash
 * @param payload
 */
void writeToLog(const std::string databasePath, const std::string owner, int round, int64_t time, int64_t cnsTime, const std::string selfHash, const std::string gossipHash, const std::string payload);

/**
 * Verifies gossip data
 *
 * @param ownerPkDer
 * @param amount
 * @param receiverId
 * @param challenge
 * @param cnsTime
 * @param sigDer
 */
bool verifyGossipPayload(const std::string ownerPkDer, const int32_t amount, const std::string receiverId, const std::string challenge, const std::string sigDer);

};
};
#endif
