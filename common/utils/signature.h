#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>

namespace degen_crypto { namespace common { namespace utils {

inline auto generate_signature(const std::string& query_string, const std::string& secret_key) -> std::string {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    HMAC_CTX* hmac = HMAC_CTX_new();
    HMAC_Init_ex(hmac, secret_key.c_str(), secret_key.length(), EVP_sha256(), nullptr);
    HMAC_Update(hmac, reinterpret_cast<const unsigned char*>(query_string.c_str()), query_string.length());
    HMAC_Final(hmac, hash, &hash_len);
    HMAC_CTX_free(hmac);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

} // namespace utils
} // namespace common
} // namespace degen_crypto 