#ifndef LIBANT_CONTAINER_SMALL_STRING_H_
#define LIBANT_CONTAINER_SMALL_STRING_H_

#include <cstdint>
#include <cstring>
#include <string>

namespace ant {

class small_string {
public:
	typedef size_t size_type;

public:
	small_string()
	{
		len_ = 0;
		str_ = 0;
	}

	small_string(const small_string& rhs)
	{
		construct(rhs.str_, rhs.len_);
	}

	small_string(const char* s)
	{
		auto len = strlen(s);
		if (len > 255) {
			len = 255;
		}
		construct(s, static_cast<uint8_t>(len));
	}

	small_string(const std::string& s)
	{
		auto len = s.size();
		if (len > 255) {
			len = 255;
		}
		construct(s.c_str(), static_cast<uint8_t>(len));
	}

	~small_string()
	{
		delete[] str_;
	}

	const small_string& operator=(const small_string& rhs)
	{
		if (&rhs != this) {
			assign(rhs.str_, rhs.len_);
		}
		return *this;
	}

	const small_string& operator=(const std::string& rhs)
	{
		auto len = rhs.size();
		if (len > 255) {
			len = 255;
		}
		assign(rhs.c_str(), static_cast<uint8_t>(len));
		return *this;
	}

	const small_string& operator=(const char* rhs)
	{
		auto len = strlen(rhs);
		if (len > 255) {
			len = 255;
		}
		assign(rhs, static_cast<uint8_t>(len));
		return *this;
	}

	size_type size() const
	{
		return len_;
	}

	const char* c_str() const
	{
		return str_;
	}

public:
	friend bool operator==(const small_string& lhs, const small_string& rhs)
	{
		return (&lhs == &rhs) || ((lhs.len_ == rhs.len_) && ((lhs.len_ == 0) || (strcmp(lhs.str_, rhs.str_) == 0)));
	}

	friend bool operator<(const small_string& lhs, const small_string& rhs)
	{
		return &lhs != &rhs && rhs.len_ != 0 && (lhs.len_ == 0 || strcmp(lhs.str_, rhs.str_) < 0);
	}

	friend bool operator>(const small_string& lhs, const small_string& rhs)
	{
		return &lhs != &rhs && lhs.len_ != 0 && (rhs.len_ == 0 || strcmp(lhs.str_, rhs.str_) > 0);
	}

	friend std::ostream& operator<<(std::ostream& out, const small_string& s)
	{
		out << ((s.len_ > 0) ? s.str_ : "");
		return out;
	}

	friend std::istream& operator>>(std::istream& in, small_string& s)
	{
		std::string tmp;
		in >> tmp;
		s = tmp;
		return in;
	}

private:
	inline void construct(const char* s, uint8_t len)
	{
		if (len > 0) {
			len_ = len;
			size_t n = len_ + 1;
			str_ = new char[n];
			memcpy(str_, s, len_);
			str_[len_] = '\0';
		} else {
			len_ = 0;
			str_ = 0;
		}
	}

	void assign(const char* s, uint8_t len)
	{
		if (len != len_) {
			delete str_;
			str_ = 0;
		}
		if (len > 0) {
			size_t n = len + 1;
			if (!str_) {
				str_ = new char[n];
			}
			len_ = len;
			memcpy(str_, s, len_);
			str_[len_] = '\0';
		} else {
			len_ = 0;
		}
	}

private:
	uint8_t		len_;
	char*		str_;
};

inline size_t unaligned_load(const char* p)
{
	size_t result;
	memcpy(&result, p, sizeof(result));
	return result;
}

// Loads n bytes, where 1 <= n < 8.
inline size_t load_bytes(const char* p, int n)
{
	size_t result = 0;
	--n;
	do {
		result = (result << 8) + static_cast<unsigned char>(p[n]);
	} while (--n >= 0);
	return result;
}

inline size_t shift_mix(size_t v)
{
	return v ^ (v >> 47);
}

size_t hash_bytes_def(const void* ptr, size_t len, size_t seed);
size_t hash_bytes_32(const void* ptr, size_t len, size_t seed);
size_t hash_bytes_64(const void* ptr, size_t len, size_t seed);

inline size_t hash_bytes(const void* ptr, size_t len, size_t seed)
{
	auto sz = sizeof(size_t);
	switch (sz) {
	case 4:
		return hash_bytes_32(ptr, len, seed);
	case 8:
		return hash_bytes_64(ptr, len, seed);
	default:
		return hash_bytes_def(ptr, len, seed);
	}
}

}

namespace std {

template<>
struct hash<ant::small_string> {
	size_t operator()(const ant::small_string& s) const
	{
		return ant::hash_bytes(s.c_str(), s.size(), 0xc70f6907UL);
	}
};

}

#endif // LIBANT_CONTAINER_SMALL_STRING_H_
