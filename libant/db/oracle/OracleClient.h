/**************************************************************
 *  Filename:    OracleClient.h
 *
 *  Description: 封装Oracle数据库操作
 *
 **************************************************************/

#ifndef LIBANT_DB_ORACLE_CLIENT_H_
#define LIBANT_DB_ORACLE_CLIENT_H_

#include <string>
#include <set>
#include <unordered_map>
#include <occi.h>

#include "OracleConnectionConfig.h"

namespace occi = oracle::occi;

/**
 * @brief 封装Oracle数据库操作。非线程安全，如果需要在多线程环境下使用，请每个线程创建一个对象。
 */
class OracleClient {
public:
	enum ErrorCode {
		kSuccess			= 0,      // 成功
		kDupKey				= 1,      // 违反唯一约束条件
		kNotConnected		= 3114,
		kConnectionLost		= 3135,   // 丢失连接
		kConnectTimeout		= 12170,  // 连接超时
		kHostNotAvailable	= 12541,  // 远端地址不存在
		kCantConnectToHost	= 12543,  // 无法连接目标主机

		kUnknownError		= 2000000001, // 未知的错误
		kErrorNeedNotRetry	= 2000000002, // 不需要重试的错误
	};

	enum ClobType {
		kClobTypeClob	= 0,
		kClobTypeNClob	= 1,
	};

	class Clob {
	public:
		Clob(ClobType type)
			: Type(type)
		{
		}

		Clob(ClobType type, std::string&& content)
			: Type(type), Content(content)
		{
		}

	public:
		const ClobType	Type;
		std::string		Content;
	};

public:
	/**
	 * @brief 构造Oracle操作对象
	 * @throw occi::SQLException
	 */
	OracleClient(const std::string& host, const std::string& user, const std::string& passwd);
	~OracleClient();

	/**
	 * @brief Runs a SQL statement that returns a ResultSet. Should not be called for a statement which is not a query, has streamed parameters.
	 *        用完之后，调用CloseResultSet()释放资源。
	 * @return ResultSet that contains the data produced by the query on success, nullptr othrewise.
	 */
	occi::ResultSet* ExecuteQuery(const std::string& sql)
	{
		occi::ResultSet* rs = 0;
		auto err = executeQuery(sql, rs);
		if (err == kSuccess || err == kErrorNeedNotRetry) {
			return rs;
		}

		// SQL执行失败，重试一次
		// 如果是丢失连接，则尝试重连
		if (err == kConnectionLost || err == kNotConnected) {
			closeConnection();
		}

		executeQuery(sql, rs);
		return rs;
	}

	/**
	 * @brief Executes a non-query statement such as a SQL INSERT, UPDATE, DELETE statement, a DDL statement such as CREATE/ALTER and so on, or a stored procedure call.
	 * @param affectedCnt will hold either the row count for INSERT, UPDATE or DELETE or 0 for SQL statements that return nothing on success.
	 * @return true on success, false otherwise.
	 * @note If the number of rows processed as a result of this call exceeds UB4MAXVAL, it may throw an exception.
	 *       In such scenarios, use execute() instead, followed by getUb8RowCount() to obtain the number of rows processed.
	 */
	bool ExecuteUpdate(const std::string& sql, const std::set<ErrorCode>* ignoreErrs = 0, unsigned int* affectedCnt = 0)
	{
		auto err = executeUpdate(sql, ignoreErrs, affectedCnt);
		if (err == kSuccess) {
			return true;
		}
		if (err == kErrorNeedNotRetry) {
			return false;
		}

		// SQL执行失败，重试一次
		// 如果是丢失连接，则尝试重连
		if (err == kConnectionLost || err == kNotConnected) {
			closeConnection();
		}

		return executeUpdate(sql, ignoreErrs, affectedCnt) == kSuccess;
	}

	/**
	* @brief Atomically executes a batch of non-query statements such as INSERT, UPDATE, DELETE statements, DDL statements such as CREATE/ALTER and so on, or stored procedure calls.
	* @return true on success, false otherwise.
	* @note If the number of rows processed as a result of this call exceeds UB4MAXVAL, it may throw an exception.
	*       In such scenarios, use execute() instead, followed by getUb8RowCount() to obtain the number of rows processed.
	*/
	bool BatchUpdate(const std::vector<std::string>& sqls, const std::set<ErrorCode>* ignoreErrs = 0)
	{
		auto err = batchUpdate(sqls, ignoreErrs);
		if (err == kSuccess) {
			return true;
		}
		if (err == kErrorNeedNotRetry) {
			return false;
		}

		// SQL执行失败，重试一次
		// 如果是丢失连接，则尝试重连
		if (err == kConnectionLost || err == kNotConnected) {
			closeConnection();
		}

		return batchUpdate(sqls, ignoreErrs) == kSuccess;
	}

	/**
	 * @brief 插入/更新包括大文本的数据
	 * @param sqls 需要插入/更新的数据（可以为空）
	 * @param blobSql 插入/更新成功后，通过blobSql把需要操作大文本的记录拉出来
	 * @param blobs 需要更新到记录里的大文本数据，必须和blobSql查询出来的记录顺序一致
	 * @param ignoreErrs 如果数据库操作出现这个set里面的错误码，不当作错误处理
	 * @return true on success, false otherwise.
	 */
	bool ExecuteClobUpdate(const std::vector<std::string>& sqls, const std::string& clobSql, const std::vector<std::vector<Clob>>& clobs, const std::set<ErrorCode>* ignoreErrs = 0)
	{
		auto err = executeClobUpdate(sqls, clobSql, clobs, ignoreErrs);
		if (err == kSuccess) {
			return true;
		}
		if (err == kErrorNeedNotRetry) {
			return false;
		}

		// SQL执行失败，重试一次
		// 如果是丢失连接，则尝试重连
		if (err == kConnectionLost || err == kNotConnected) {
			closeConnection();
		}

		return executeClobUpdate(sqls, clobSql, clobs, ignoreErrs) == kSuccess;
	}

	/**
	 * @brief 关闭resultSet，释放资源，同时把resultSet置为nullptr
	 * @return true成功，false异常。无论成功或者异常，resultSet都会被置为nullptr
	 */
	bool CloseResultSet(occi::ResultSet*& resultSet);

	/**
	 * @brief 重新连接到另外一台Oracle DB服务器
	 */
	void ReconnectToAnotherHost(const std::string& host, const std::string& user, const std::string& passwd)
	{
		if (connCfg_.Host != host || connCfg_.User != user || connCfg_.Password != passwd) {
			closeConnection();
			connCfg_.Host = host;
			connCfg_.User = user;
			connCfg_.Password = passwd;
		}
	}

	const OracleConnectionConfig& ConnectionConfig() const
	{
		return connCfg_;
	}

	const std::string& LastErrMsg() const
	{
		return errMsg_;
	}

private:
	bool createStatement();
	ErrorCode executeQuery(const std::string& sql, occi::ResultSet*& rs);
	ErrorCode executeUpdate(const std::string& sql, const std::set<ErrorCode>* ignoreErrs, unsigned int* affectedCnt);
	ErrorCode batchUpdate(const std::vector<std::string>& sqls, const std::set<ErrorCode>* ignoreErrs);
	ErrorCode executeClobUpdate(const std::vector<std::string>& sqls, const std::string& clobSql, const std::vector<std::vector<Clob>>& clobs, const std::set<ErrorCode>* ignoreErrs);
	void closeConnection();

private:
	enum {
		kDBAlertThreshold	= 3,
		kDBAlertInterval	= 60,
	};

private:
	OracleConnectionConfig	connCfg_;

	occi::Environment*	env_;
	occi::Connection*	conn_;
	occi::Statement*	stmt_;
	bool				autoCommit_;

	std::string			errMsg_;
};

/**
* @brief 管理OracleClient对象。非线程安全，如果需要在多线程环境下使用，请每个线程创建一个对象。
*/
class OracleClientMgr {
public:
	~OracleClientMgr()
	{
		Clear();
	}

	/**
	 * @brief 获取OracleClient对象。通过该接口得到的对象内存由OracleClientMgr管理，如果OracleClientMgr被析构了，则OracleClient也会被析构。
	 */
	OracleClient* Get(const OracleConnectionConfig& cfg)
	{
		return Get(cfg.Host, cfg.User, cfg.Password);
	}

	/**
	 * @brief 获取OracleClient对象。通过该接口得到的对象内存由OracleClientMgr管理，如果OracleClientMgr被析构了，则OracleClient也会被析构。
	 */
	OracleClient* Get(const std::string& host, const std::string& user, const std::string& passwd)
	{
		tmpKey_.clear();
		tmpKey_.append(host).append("|").append(user).append("|").append(passwd);
		auto& db = clients_[tmpKey_];
		if (!db) {
			db = new OracleClient(host, user, passwd);
		}
		return db;
	}

	/**
	 * @brief 释放除了cfgs里指定的连接配置以外的OracleClient对象。
	 */
	void ClearExcept(const std::set<OracleConnectionConfig>& cfgs)
	{
		auto it = clients_.begin();
		while (it != clients_.end()) {
			if (cfgs.count(it->second->ConnectionConfig()) > 0) {
				++it;
				continue;
			}

			delete it->second;
			clients_.erase(it++);
		}
	}

	/**
	 * @brief 释放所有OracleClient对象。
	 */
	void Clear()
	{
		for (const auto& v : clients_) {
			delete v.second;
		}
		clients_.clear();
	}

private:
	std::unordered_map<std::string, OracleClient*>	clients_;
	std::string	tmpKey_;
};

#endif // !LIBANT_DB_ORACLE_CLIENT_H_
