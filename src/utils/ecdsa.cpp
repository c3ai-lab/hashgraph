#ifndef HASHGRAPH_UTILS_SHA384_HPP
#define HASHGRAPH_UTILS_SHA384_HPP

#include <string>
#include <iostream>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/ecdsa.h>

namespace hashgraph {
namespace utils {


EC_KEY* getPrivFromPEM(std::string const pem) {
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

EC_KEY* getPubFromCertPEM(std::string const pem) {
    
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

void ecdsa_sign(EC_KEY *key, std::string const msg, char **r, char **s) {

    if (NULL == key) {
        printf("Failed to generate EC Key\n");
    }

    ECDSA_SIG *signature = ECDSA_do_sign((const unsigned char*)msg.c_str(), msg.size(), key);
    if (NULL == signature) {
        printf("Failed to generate EC Signature\n");
    }

    (*r) = BN_bn2hex(ECDSA_SIG_get0_r(signature));
    (*s) = BN_bn2hex(ECDSA_SIG_get0_s(signature));

    ECDSA_SIG_free(signature);
}

void ecdsa_verify(EC_KEY *key, std::string const msg, char *r, char *s) {

    if (NULL == key) {
        printf("Failed to generate EC Key\n");
    }

    BIGNUM *bnR = BN_new();
    BN_hex2bn(&bnR, r);

    BIGNUM *bnS = BN_new();
    BN_hex2bn(&bnS, s);

    ECDSA_SIG *signature = ECDSA_SIG_new();
    ECDSA_SIG_set0(signature, bnR, bnS);

    int status = ECDSA_do_verify((const unsigned char*)msg.c_str(), msg.size(), signature, key);
    if (status != 1)
        printf("Failed to verify EC Signature\n");
    else
        printf("Verifed EC Signature\n");

    BN_free(bnR);
    BN_free(bnS);
    ECDSA_SIG_free(signature);
}


};
};
#endif
