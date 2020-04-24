#ifndef LIBANT_ORACLE_CONNECTION_CONFIG_H_
#define LIBANT_ORACLE_CONNECTION_CONFIG_H_

#include <string>

/**
 * @brief 数据库地址配置
 */
class OracleConnectionConfig {
public:
	OracleConnectionConfig()
	{
	}

	OracleConnectionConfig(const std::string& host, const std::string& user, const std::string& password)
		: Host(host), User(user), Password(password)
	{
	}

	void Swap(OracleConnectionConfig& cfg)
	{
		Host.swap(cfg.Host);
		User.swap(cfg.User);
		Password.swap(cfg.Password);
	}

public:
	friend bool operator<(const OracleConnectionConfig& lhs, const OracleConnectionConfig& rhs)
	{
		return lhs.toString() < rhs.toString();
	}

	friend bool operator==(const OracleConnectionConfig& lhs, const OracleConnectionConfig& rhs)
	{
		return (lhs.Host == rhs.Host) && (lhs.User == rhs.User) && (lhs.Password == rhs.Password);
	}

public:
	std::string	Host;
	std::string	User;
	std::string	Password;

private:
	const std::string& toString() const
	{
		tmpStr_.clear();
		tmpStr_.append(Host).append("|").append(User).append("|").append(Password);
		return tmpStr_;
	}

private:
	mutable std::string tmpStr_;
};

#endif // LIBANT_ORACLE_CONNECTION_CONFIG_H_
