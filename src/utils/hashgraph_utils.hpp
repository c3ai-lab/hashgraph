#ifndef HASHGRAPH_UTILS_HPP
#define HASHGRAPH_UTILS_HPP

#include <string>
#include <openssl/ec.h>

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

};
};
#endif
