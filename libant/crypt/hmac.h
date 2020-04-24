#ifndef LIBANT_COMMON_CRYPT_H_
#define LIBANT_COMMON_CRYPT_H_

#include <string>

namespace ant {

// sha1方法做hmac，然后转成base64编码
// 成功返回true
bool hmac_sha1_64(const std::string& key, const std::string& input, std::string& output);

}

#endif // !LIBANT_COMMON_CRYPT_H_

