#include <string>
#include <iostream>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/ecdsa.h>
#include "hashgraph_utils.hpp"

namespace hashgraph {
namespace utils {

EC_KEY* getKeyFromPrivatePEM(const std::string pem) {
    EC_KEY *key = NULL;

    BIO *bio = BIO_new_mem_buf((void*)pem.c_str(), pem.size());
    if (!bio) {
        std::cerr << "Error: BIO_new_mem_buf failed" << std::endl;
        return NULL;
    }
    key = PEM_read_bio_ECPrivateKey(bio, &key, NULL, NULL);
    BIO_free(bio);

    return key;
}

EC_KEY* getKeyFromCertPEM(const std::string pem) {

    BIO *bio = BIO_new_mem_buf((void*)pem.c_str(), pem.size());
    if (!bio) {
        std::cerr << "Error: BIO_new_mem_buf failed" << std::endl;
        return NULL;
    }

    X509* x509 = NULL;
    PEM_read_bio_X509(bio, &x509, NULL, NULL);

    EVP_PKEY *pkey = X509_get_pubkey(x509);
    EC_KEY* key    = EVP_PKEY_get1_EC_KEY(pkey);

    BIO_free(bio);
    X509_free(x509);
    EVP_PKEY_free(pkey);

    return key;
}

std::string encodeKeyToPublicDER(EC_KEY *key) {
    unsigned char *der = NULL;
    int pub_key_len = i2d_EC_PUBKEY(key, &der);
    if (pub_key_len <= 0) {
        printf("Unexpected pub key length for generated key");
        return NULL;
    }
    std::string buffer((const char*)der, pub_key_len);
    OPENSSL_free(der);
    return buffer;
}

std::string getIdentifierFromPrivatePEM(const std::string pem) {

    // get key object from pem
    EC_KEY *key = getKeyFromPrivatePEM(pem);

    // get DER encoding of public key
    std::string der = encodeKeyToPublicDER(key);

    // cleanup
    EC_KEY_free(key);

	// calculate identifier
	return utils::encodeIdentifier(der);
}

std::string getIdentifierFromCertPEM(const std::string pem) {

    // get key object from pem
    EC_KEY *key = getKeyFromCertPEM(pem);

    // get DER encoding of public key
    std::string der = encodeKeyToPublicDER(key);

    // cleanup
    EC_KEY_free(key);

	// calculate identifier
	return utils::encodeIdentifier(der);
}

EC_KEY* getKeyFromPublicDER(const std::string pubKeyDer) {
	EC_KEY *pk = NULL;
	BIO *bio = BIO_new_mem_buf((void*)pubKeyDer.c_str(), pubKeyDer.size());
	if (!bio) {
	   std::cerr << "Error: BIO_new_mem_buf failed" << std::endl;
	   return NULL;
	}
	d2i_EC_PUBKEY_bio(bio, &pk);
    BIO_free(bio);
    return pk;
}

bool verifyECDSASignature(const std::string pubKeyDER, const std::string sigDER, const std::string msg) {

	// public key from der encoded byte array
	EC_KEY* pubKey = getKeyFromPublicDER(pubKeyDER);
 
	// hash transfer message
	std::string digest = utils::SHA256(msg);
	
	// verify signature
   	bool valid = ECDSA_verify(0, (const unsigned char*)digest.c_str(), digest.size(), (const unsigned char*)sigDER.c_str(), sigDER.size(), pubKey) == 1;

    // cleanup
    EC_KEY_free(pubKey);

	return valid;
}

};
};
