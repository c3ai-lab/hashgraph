#ifndef HASHGRAPH_UTILS_HPP
#define HASHGRAPH_UTILS_HPP

#include <string>
#include <openssl/ec.h>

namespace hashgraph {
namespace utils {

/**
 * Returns an sha384 hash
 *
 * @param str The string to hash
 */
const std::string sha384_string(std::string const str);

/**
 * Get an EC private key object from the given PEM
 *
 * @param pem
 */
EC_KEY* getPrivFromPEM(std::string const pem);

/**
 * Get an EC public key object from the given certificate PEM
 *
 * @param pem
 */
EC_KEY* getPubFromCertPEM(std::string const pem);

/**
 * Creates a ECDSA signature from a message
 *
 * @param key
 * @param msg
 * @param r
 * @param s
 */
void ecdsa_sign(EC_KEY *key, std::string const msg, char **r, char **s);

/**
 * Verifies a ECDSA signature of a message
 *
 * @param key
 * @param msg
 * @param r
 * @param s
 */
void ecdsa_verify(EC_KEY *key, std::string const msg, char *r, char *s);


};
};
#endif
