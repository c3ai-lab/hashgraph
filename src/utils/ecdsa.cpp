#include <string>
#include <cstdio>
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

    BIO *bio = BIO_new_mem_buf((void*)pem.c_str(), pem.length());
    if (!bio) {
        fprintf(stderr, "BIO_new_mem_buf failed\n");
        return NULL;
    }
    key = PEM_read_bio_ECPrivateKey(bio, &key, NULL, NULL);
    BIO_free(bio);

    return key;
}

EC_KEY* getKeyFromCertPEM(const std::string pem) {

    BIO *bio = BIO_new_mem_buf((void*)pem.c_str(), pem.length());
    if (!bio) {
        fprintf(stderr, "BIO_new_mem_buf failed\n");
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
        fprintf(stderr, "Unexpected pub key length for generated key\n");
        return NULL;
    }
    std::string buffer((const char*)der, pub_key_len);
    OPENSSL_free(der);
    return buffer;
}

std::string getIdentifierFromPrivatePEM(const std::string pem) {

    // get pk/sk-key object from pem
    EC_KEY *key = getKeyFromPrivatePEM(pem);

    // get DER encoding of public key
    std::string pkDer = encodeKeyToPublicDER(key);

    // cleanup
    EC_KEY_free(key);

	// create identifier
	return utils::encodeIdentifier(pkDer);
}

EC_KEY* getKeyFromPublicDER(const std::string pkDer) {
	EC_KEY *pk = NULL;
	BIO *bio = BIO_new_mem_buf((void*)pkDer.c_str(), pkDer.length());
	if (!bio) {
       fprintf(stderr, "BIO_new_mem_buf failed\n");
	   return NULL;
	}
	d2i_EC_PUBKEY_bio(bio, &pk);
    BIO_free(bio);
    return pk;
}

std::string getPublicDERFromCertPEM(const std::string pem) {

    // get key object from pem
    EC_KEY *key = getKeyFromCertPEM(pem);

    // get DER encoding of public key
    std::string pkDer = encodeKeyToPublicDER(key);

    // cleanup
    EC_KEY_free(key);

	// create identifier
	return pkDer;
}

bool ecdsaVerifyMessage(const std::string pkDer, const std::string sigDer, const std::string msg) {

	// public key from DER encoded data
	EC_KEY* pk = getKeyFromPublicDER(pkDer);
 
	// hash message
	std::string digest = utils::SHA256(msg);
	
	// verify signature
   	bool valid = ECDSA_verify(0, (const unsigned char*)digest.c_str(), digest.length(), (const unsigned char*)sigDer.c_str(), sigDer.length(), pk) == 1;

    // cleanup
    EC_KEY_free(pk);

	return valid;
}

std::string ecdsaSignMessage(const std::string skDer, const std::string msg) {

    // secret key from DER encoded data
	EC_KEY* sk = getKeyFromPrivatePEM(skDer);

	// hash message
	std::string digest = utils::SHA256(msg);

    // signature length
    unsigned int siglen;

    // signature buffer
    unsigned char *buf = (unsigned char*)malloc(ECDSA_size(sk));

    if (ECDSA_sign(0, (const unsigned char*)digest.c_str(), digest.size(), buf, &siglen, sk) == 0) {
        fprintf(stderr, "Failed to generate EC Signature\n");
    }

    // DER encoded signature
    std::string sig = std::string(buf, buf+siglen);

    // cleanup
    free(buf);

    return sig;
}

};
};
