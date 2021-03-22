#ifndef HASHGRAPH_UTILS_SHA384_HPP
#define HASHGRAPH_UTILS_SHA384_HPP

#include <string>
#include "openssl/sha.h"

namespace hashgraph {
namespace utils {


const std::string sha384_string(std::string const str) {
	std::string buffer;

    unsigned char hash[SHA384_DIGEST_LENGTH];
    SHA512_CTX ctx;
    SHA384_Init(&ctx);
    SHA384_Update(&ctx, str.c_str(), str.size());
    SHA384_Final(hash, &ctx);

	char hex[3];
    for(int i = 0; i < SHA384_DIGEST_LENGTH; i++) {
        sprintf(hex, "%02x", hash[i]);
		buffer.append(hex);
    }

	return buffer;
}


};
};
#endif
