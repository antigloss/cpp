/**
 * redis_client.h
 *
 *  Created on: 2016-09-08
 *      Author: antigloss
 */
#ifndef LIBANT_DB_REDIS_REDIS_CLIENT_H_
#define LIBANT_DB_REDIS_REDIS_CLIENT_H_

#include <cstdint>
#include <sstream>
#include <string>
#include <map>
#include <type_traits>
#include <vector>

#include <hiredis/hiredis.h>

namespace ant {

/**
 * @brief Scoped pointer wrapper for redisReply*
 */
class ScopedReplyPointer {
private:
	// safe bool idiom
	typedef void (ScopedReplyPointer::*SafeBool)() const;
	void comparison_not_supported() const
	{
	}

public:
	ScopedReplyPointer(void* reply = 0)
	{
		m_reply = reinterpret_cast<redisReply*>(reply);
	}
	~ScopedReplyPointer()
	{
		free_reply();
	}

	void operator=(void* reply)
	{
		if (m_reply != reply) {
			free_reply();
			m_reply = reinterpret_cast<redisReply*>(reply);
		}
	}
	redisReply* operator->()
	{
		return m_reply;
	}
	const redisReply* operator->() const
	{
		return m_reply;
	}

	// safe bool idiom
	operator SafeBool() const
	{
		return m_reply ? &ScopedReplyPointer::comparison_not_supported : 0;
	}

private:
	void free_reply()
	{
		if (m_reply) {
			freeReplyObject(m_reply);
		}
	}

private:
	redisReply* m_reply;
};

// safe bool idiom
template <typename T>
bool operator!=(const ScopedReplyPointer& lhs, const T& rhs)
{
	lhs.comparison_not_supported();
	return false;
}

template <typename T>
bool operator!=(const T& lhs, const ScopedReplyPointer& rhs)
{
	rhs.comparison_not_supported();
	return false;
}

template <typename T>
bool operator==(const ScopedReplyPointer& lhs, const T& rhs)
{
	lhs.comparison_not_supported();
	return false;
}

template <typename T>
bool operator==(const T& lhs, const ScopedReplyPointer& rhs)
{
	rhs.comparison_not_supported();
	return false;
}

/**
 * @brief Redis Client
 */
class RedisClient {
public:
	enum SetOpType {
		kSetAnyhow		= 0, // Set the key no matter it already exist or not
		kSetIfNotExist	= 1, // Only set the key if it does not already exist
		kSetIfExist		= 2, // Only set the key if it already exist

		kSetOpCnt
	};

	class ExpirationTime {
	public:
		ExpirationTime(bool is_abs_ts, time_t expire_tm)
		{
			m_is_abs_ts = is_abs_ts;
			m_expire_tm = expire_tm;
		}

		time_t remaining_seconds() const
		{
			if (!m_is_abs_ts) {
				return m_expire_tm;
			}
			return m_expire_tm - time(0);
		}

	private:
		bool	m_is_abs_ts;
		time_t	m_expire_tm;
	};

private:
	enum Consts {
		kVecArgMaxSize		= 2000000000,

		kStaticArgvMaxSize	= 500,
	};

	typedef const char* const_char_ptr;

public:
	/**
	 * @brief
	 * @param ip_port 127.0.0.1:6379
	 */
	RedisClient(const std::string& ip_port);

	~RedisClient()
	{
		free_context();
	}

	const std::string& server_ip() const
	{
		return m_ip;
	}

	const int server_port() const
	{
			return m_port;
	}

	const std::string& server_ip_port() const
	{
		return m_ip_port;
	}

	const std::string& last_error_message() const
	{
		return m_errmsg;
	}

	bool select_db(int dbidx);
	/**
	 * @brief
	 * @param key
	 * @param seconds
	 */
	bool expire(const std::string& key, unsigned int seconds);
	/**
	 * @brief
	 * @param key
	 * @param expired_tm
	 */
	bool expire_at(const std::string& key, time_t expired_tm);
	/**
	 * @brief
	 * @param key
	 * @param ttl TTL in seconds. -1 if key exists but has no associated expire, -2 if key does not exist.
	 */
	bool ttl(const std::string& key, long long& ttl);
	/**
	 * @brief
	 * @param key
	 * @param val
	 * @param expired_tm
	 */
	bool set(const std::string& key, const std::string& val,
				const ExpirationTime* expire_tm = 0, SetOpType op_type = kSetAnyhow);
	/**
	 * @brief
	 * @param key
	 * @param val
	 * @param key_exists
	 */
	bool get(const std::string& key, std::string& val, bool* key_exists = 0);

	template <typename T>
	bool get(const std::string& key, T& result, bool* key_exists = 0)
	{
		std::string tmp;
		if (get(key, tmp, key_exists)) {
			if (tmp.size()) {
				std::istringstream iss(tmp);
				iss >> result;
			} else {
				result = T();
			}
			return true;
		}
		return false;
	}

	// TODO template<typename CONTAINER>

	/**
	 * @brief
	 * @param key
	 * @param vals
	 * @param cnt
	 */
	bool sadd(const std::string& key, const std::vector<std::string>& vals, long long* cnt = 0);
	/**
	 * @brief Gets number of elements of the set stored at key
	 * @param key
	 * @param cnt
	 */
	bool scard(const std::string& key, long long& cnt);
	/**
	 * @brief Gets the members of the set resulting from the difference between
	 * 			the first set and all the successive sets specified by `keys`
	 * @param keys
	 * @param result
	 */
	bool sdiff(const std::vector<std::string>& keys, std::vector<std::string>& result);
	/**
	 * @brief Stores the result of the difference between the first set
	 * 			and all the successive sets specified by `keys` to `dest`.
	 * 			If `dest` already exists, it is overwritten.
	 * @param dest
	 * @param keys
	 * @param cnt
	 */
	bool sdiff_store(const std::string& dest, const std::vector<std::string>& keys, long long* cnt = 0);
	/**
	 * @brief Gets the members of the set resulting from the intersection of
	 * 			all the given sets specified by `keys`.
	 * @param keys
	 * @param result
	 */
	bool sinter(const std::vector<std::string>& keys, std::vector<std::string>& result);
	/**
	 * @brief Stores the result of the intersection of all the given sets
	 * 			specified by `keys` to `dest`. If `dest` already exists, it is overwritten.
	 * @param dest
	 * @param keys
	 * @param cnt
	 */
	bool sinter_store(const std::string& dest, const std::vector<std::string>& keys, long long* cnt = 0);
	/**
	 * @brief Determine if `val` is a member of the set stored at `key`.
	 * @param key
	 * @param val
	 * @param is_member true if is a member, false if not a member or not exist
	 */
	bool sismember(const std::string& key, const std::string& val, bool& is_member);

	bool hget(const std::string& key, std::map<std::string, std::string>& fields);
	bool hset(const std::string& key, const std::map<std::string, std::string>& fields);

private:
	bool alloc_context();

	bool realloc_context()
	{
		free_context();
		return alloc_context();
	}

	bool has_context() const
	{
		return m_context;
	}

	void free_context()
	{
		redisFree(m_context);
		m_context = 0;
	}

	redisReply* exec(const char* fmt, ...);

	template<typename STR_CONTAINER>
	redisReply* execv(const std::string& cmd, const std::string* key, const STR_CONTAINER* args)
	{
		static_assert(std::is_same<typename STR_CONTAINER::value_type, std::string>::value,
						"Element type of the container must be std::string!");

		if (!has_context() && !alloc_context()) {
			return 0;
		}

		const_char_ptr* argv;
		size_t* arglens;
		int plus;
		int argc;
		if (!alloc_argv(key, args, 1, argv, arglens, argc, plus)) {
			return 0;
		}

		argv[0] = cmd.c_str();
		arglens[0] = cmd.size();
		if (key) {
			argv[1] = key->c_str();
			arglens[1] = key->size();
		}
		if (args) {
			int i = 0;
			for (typename STR_CONTAINER::const_iterator it = args->begin();
					it != args->end(); ++it) {
				argv[i + plus] = it->c_str();
				arglens[i + plus] = it->size();
				++i;
			}
		}

		redisReply* reply = reinterpret_cast<redisReply*>(redisCommandArgv(m_context, argc, argv, arglens));
		if (!reply) {
			set_errmsg(m_context->errstr, " (", m_context->err, ')');
			free_context();
		}
		free_argv(argv, arglens, argc);

		return reply;
	}

	template<typename STR_PAIR_CONTAINER>
	redisReply* execm(const std::string& cmd, const std::string& key,
						STR_PAIR_CONTAINER& args, bool packValue = false)
	{
		using FirstType = typename std::decay<decltype(args.begin()->first)>::type;
		using SecondType = typename std::decay<decltype(args.begin()->second)>::type;
		static_assert(std::is_same<FirstType, std::string>::value,
						"Key type of the container must be std::string!");
		static_assert(std::is_same<SecondType, std::string>::value,
						"Value type of the container must be std::string!");

		if (!has_context() && !alloc_context()) {
			return 0;
		}

		const_char_ptr* argv;
		size_t* arglens;
		int plus;
		int argc;
		if (!alloc_argv(&key, &args, (packValue ? 2 : 1), argv, arglens, argc, plus)) {
			return 0;
		}

		argv[0] = cmd.c_str();
		arglens[0] = cmd.size();
		argv[1] = key.c_str();
		arglens[1] = key.size();
		int i = 0;
		if (packValue) {
			for (typename STR_PAIR_CONTAINER::const_iterator it = args.begin();
					it != args.end(); ++it) {
				argv[i + 2] = it->first.c_str();
				arglens[i + 2] = it->first.size();
				argv[i + 3] = it->second.c_str();
				arglens[i + 3] = it->second.size();
				i += 2;
			}
		} else {
			for (typename STR_PAIR_CONTAINER::const_iterator it = args.begin();
					it != args.end(); ++it) {
				argv[i + 2] = it->first.c_str();
				arglens[i + 2] = it->first.size();
				++i;
			}
		}

		redisReply* reply = reinterpret_cast<redisReply*>(redisCommandArgv(m_context, argc, argv, arglens));
		if (!reply) {
			set_errmsg(m_context->errstr, " (", m_context->err, ')');
			free_context();
		}
		free_argv(argv, arglens, argc);

		return reply;
	}

	void arr_reply_to_vector(redisReply* const replies[], size_t reply_num, std::vector<std::string>& result);

// helpers
private:
	template<typename T, typename ... Tail>
	void set_errmsg(T head, Tail... tail)
	{
		m_oss.str("");
		m_oss << head;
		do_set_errmsg(tail...);
		m_errmsg = m_oss.str();
	}

	template<typename T, typename ... Tail>
	void do_set_errmsg(T head, Tail... tail)
	{
		m_oss << head;
		do_set_errmsg(tail...);
	}

	void do_set_errmsg()
	{
	}

	template<typename C>
	bool alloc_argv(const std::string* key, const C* args, typename C::size_type argsMul,
					const_char_ptr*& argv, size_t*& arglens, int& argc, int& plus)
	{
		typename C::size_type maxAllowArgSize = kVecArgMaxSize / argsMul;
		if (args && (!args->size() || args->size() > maxAllowArgSize)) {
			set_errmsg("Size of args is invalid (", args->size(), ')');
			return false;
		}

		argv = m_tmpArgv;
		arglens = m_tmpArgLens;
		argc = 1;
		plus = 1;
		if (key) {
			++argc;
			++plus;
		}
		if (args) {
			argc += (args->size() * argsMul);
		}
		if (argc > kStaticArgvMaxSize) {
			argv = new const_char_ptr[argc];
			arglens = new size_t[argc];
		}

		return true;
	}

	void free_argv(const_char_ptr* argv, size_t* arglens, int argc)
	{
		if (argc > kStaticArgvMaxSize) {
			delete [] argv;
			delete [] arglens;
		}
	}

private:
	std::string		m_ip_port;
	std::string		m_ip;
	int				m_port;
	redisContext*	m_context;

	std::ostringstream	m_oss;
	std::string			m_errmsg;

	const_char_ptr	m_tmpArgv[kStaticArgvMaxSize];
	size_t			m_tmpArgLens[kStaticArgvMaxSize];
};

}
#endif /* LIBANT_DB_REDIS_REDIS_CLIENT_H_ */
