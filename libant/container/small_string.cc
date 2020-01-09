#include "small_string.h"

namespace ant {

size_t hash_bytes_def(const void* ptr, size_t len, size_t seed)
{
	size_t hash = seed;
	const char* cptr = reinterpret_cast<const char*>(ptr);
	for (; len; --len) {
		hash = (hash * 131) + *cptr++;
	}
	return hash;
}

size_t hash_bytes_32(const void* ptr, size_t len, size_t seed)
{
	const size_t m = 0x5bd1e995;
	size_t hash = seed ^ len;
	const char* buf = static_cast<const char*>(ptr);

	// Mix 4 bytes at a time into the hash.
	while (len >= 4) {
		size_t k = unaligned_load(buf);
		k *= m;
		k ^= k >> 24;
		k *= m;
		hash *= m;
		hash ^= k;
		buf += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array.
	switch (len) {
	case 3:
		hash ^= static_cast<unsigned char>(buf[2]) << 16;
	case 2:
		hash ^= static_cast<unsigned char>(buf[1]) << 8;
	case 1:
		hash ^= static_cast<unsigned char>(buf[0]);
		hash *= m;
	};

	// Do a few final mixes of the hash.
	hash ^= hash >> 13;
	hash *= m;
	hash ^= hash >> 15;
	return hash;
}

size_t hash_bytes_64(const void* ptr, size_t len, size_t seed)
{
	static const size_t mul = (((size_t)0xc6a4a793UL) << 32UL) + (size_t)0x5bd1e995UL;
	const char* const buf = static_cast<const char*>(ptr);

	// Remove the bytes not divisible by the sizeof(size_t).  This
	// allows the main loop to process the data as 64-bit integers.
	const size_t len_aligned = len & ~(size_t)0x7;
	const char* const end = buf + len_aligned;
	size_t hash = seed ^ (len * mul);
	for (const char* p = buf; p != end; p += 8) {
		const size_t data = shift_mix(unaligned_load(p) * mul) * mul;
		hash ^= data;
		hash *= mul;
	}
	if ((len & 0x7) != 0) {
		const size_t data = load_bytes(end, len & 0x7);
		hash ^= data;
		hash *= mul;
	}
	hash = shift_mix(hash) * mul;
	hash = shift_mix(hash);
	return hash;
}

}