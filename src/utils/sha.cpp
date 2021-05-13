#ifndef HASHGRAPH_UTILS_SHA_HPP
#define HASHGRAPH_UTILS_SHA_HPP

#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>

namespace hashgraph {
namespace utils {

const std::string SHA384(const std::string data) {

    unsigned char hash[SHA384_DIGEST_LENGTH];
    SHA512_CTX ctx;
    SHA384_Init(&ctx);
    SHA384_Update(&ctx, data.c_str(), data.size());
    SHA384_Final(hash, &ctx);

    return std::string(hash, hash + sizeof(hash) / sizeof(hash[0]));
}

const std::string SHA256(const std::string data) {

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data.c_str(), data.size());
    SHA256_Final(hash, &ctx);

    return std::string(hash, hash + sizeof(hash) / sizeof(hash[0]));
}

const std::string SHA3(const std::string data) {
    unsigned int len = 32;
    unsigned char digest[len];

    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, EVP_sha3_256(), NULL);
    EVP_DigestUpdate(mdctx, data.c_str(), data.size());
    EVP_DigestFinal_ex(mdctx, digest, &len);
    EVP_MD_CTX_destroy(mdctx);

    return std::string(digest, digest + sizeof(digest) / sizeof(digest[0]));
}

};
};
#endif
