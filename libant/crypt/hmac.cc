#include <openssl/hmac.h>
#include <boost/beast/core/detail/base64.hpp>
#include "hmac.h"

namespace ant {

bool hmac_sha1_64(const std::string& key, const std::string& input, std::string& output)
{
	unsigned char out[EVP_MAX_MD_SIZE];
	unsigned int outLen = 0;
	if (!HMAC(EVP_sha1(), key.c_str(), static_cast<int>(key.size()), reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), out, &outLen)) {
		return false;
	}
	output = boost::beast::detail::base64_encode(out, outLen);
	return true;
}

}