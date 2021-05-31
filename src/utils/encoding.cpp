#include <string>
#include <cstdio>
#include <openssl/bn.h>
#include "hashgraph_utils.hpp"

namespace hashgraph {
namespace utils {

const std::string hexToByte(const std::string hex) {

    // use big num to convert from hex string to byte string
    BIGNUM *input = BN_new();
    int len = BN_hex2bn(&input, hex.c_str());
    len = (len + 1) / 2; 
    unsigned char *buffer = (unsigned char*)malloc(len);
    BN_bn2bin(input, buffer);
    
    // byte string
    std::string bytes = std::string(buffer, buffer+len);

    // cleanup
    BN_free(input);
    free(buffer);

    return bytes;
}

const std::string byteToHex(const std::string bytes) {
	std::string buffer;
	char hex[3];
    for (size_t i=0; i < bytes.length(); i++) {
        sprintf(hex, "%02hhx", bytes.data()[i]);
        buffer.append(hex);
    }
	return buffer;
}

const std::string encodeIdentifier(const std::string pkDer) {
	return "11x" + utils::byteToHex(utils::SHA3(pkDer)).substr(24, std::string::npos);
}

};
};
