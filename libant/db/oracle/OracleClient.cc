/**************************************************************
 *  Filename:    OracleClient.cc
 *
 *  Description: 
 *
 **************************************************************/

#include <stdexcept>
#include "OracleClient.h"

using namespace std;

//===========================================================================
// OracleClient公有函数
//===========================================================================
OracleClient::OracleClient(const std::string& host, const std::string& user, const std::string& passwd)
	: connCfg_(host, user, passwd)
{
	env_ = occi::Environment::createEnvironment("UTF8", "UTF8", occi::Environment::THREADED_UNMUTEXED);
	conn_ = 0;
	stmt_ = 0;
	autoCommit_ = false;
}

OracleClient::~OracleClient()
{
	try {
		if (stmt_) {
			conn_->terminateStatement(stmt_);
		}
		if (conn_) {
			env_->terminateConnection(conn_);
		}
		occi::Environment::terminateEnvironment(env_);
	} catch (...) {
		// do nothing
	}
}

bool OracleClient::CloseResultSet(occi::ResultSet*& resultSet)
{
	try {
		stmt_->closeResultSet(resultSet);
		return true;
	} catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	resultSet = 0; // 如果抛出异常，大不了泄漏些内存
	return false;
}

//===========================================================================
// OracleClient私有函数
//===========================================================================
bool OracleClient::createStatement()
{
	try {
		if (conn_) {
			if (!stmt_) {
				stmt_ = conn_->createStatement();
				autoCommit_ = false;
			}
		} else {
			conn_ = env_->createConnection(connCfg_.User, connCfg_.Password, connCfg_.Host);
			stmt_ = conn_->createStatement();
			autoCommit_ = false;
		}
		return true;
	} catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	return false;
}

OracleClient::ErrorCode OracleClient::getColumnsMetaData(const std::string& tableName, std::vector<occi::MetaData>& meta)
{
	if (!createStatement()) {
		return kErrorNeedNotRetry; // 建立连接失败，没必要重试
	}

	auto err = kUnknownError;
	try {
		auto tblMeta = conn_->getMetaData(tableName, occi::MetaData::PTYPE_TABLE);
		meta = tblMeta.getVector(occi::MetaData::ATTR_LIST_COLUMNS);
		return kSuccess;
	}
	catch (const occi::SQLException& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
		err = static_cast<ErrorCode>(e.getErrorCode());
	}
	catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	}
	catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	return err;
}

OracleClient::ErrorCode OracleClient::executeQuery(const std::string& sql, occi::ResultSet*& rs)
{
	if (!createStatement()) {
		return kErrorNeedNotRetry; // 建立连接失败，没必要重试
	}

	auto err = kUnknownError;
	try {
		rs = stmt_->executeQuery(sql);
		return kSuccess;
	} catch (const occi::SQLException& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
		err = static_cast<ErrorCode>(e.getErrorCode());
	} catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	return err;
}

OracleClient::ErrorCode OracleClient::executeUpdate(const std::string& sql, const set<ErrorCode>* ignoreErrs, unsigned int* affectedCnt)
{
	if (!createStatement()) {
		return kErrorNeedNotRetry; // 建立连接失败，没必要重试
	}

	auto err = kUnknownError;
	try {
		if (!autoCommit_) {
			stmt_->setAutoCommit(true);
			autoCommit_ = true;
		}

		if (affectedCnt) {
			*affectedCnt = stmt_->executeUpdate(sql);
		} else {
			stmt_->executeUpdate(sql);
		}
		return kSuccess;
	} catch (const occi::SQLException& e) {
		err = static_cast<ErrorCode>(e.getErrorCode());
		if (ignoreErrs && ignoreErrs->count(err)) {
			return kSuccess;
		}

		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	return err;
}

OracleClient::ErrorCode OracleClient::batchUpdate(const vector<string>& sqls, const set<ErrorCode>* ignoreErrs)
{
	if (!createStatement()) {
		return kErrorNeedNotRetry; // 建立连接失败，没必要重试
	}

	auto err = kUnknownError;
	try {
		if (autoCommit_) {
			stmt_->setAutoCommit(false);
			autoCommit_ = false;
		}

		for (auto& sql : sqls) {
			try {
				stmt_->executeUpdate(sql);
			} catch (const occi::SQLException& e) {
				err = static_cast<ErrorCode>(e.getErrorCode());
				if (ignoreErrs && ignoreErrs->count(err)) {
					continue;
				}
				throw;
			}
		}

		conn_->commit();
		return kSuccess;
	} catch (const occi::SQLException& e) {
		err = static_cast<ErrorCode>(e.getErrorCode());
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	} catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	try {
		conn_->rollback();
	} catch (...) {
		// do nothing
	}

	return err;
}

OracleClient::ErrorCode OracleClient::executeClobUpdate(const std::vector<std::string>& sqls, const std::string& clobSql, const std::vector<std::vector<Clob>>& clobs, const std::set<ErrorCode>* ignoreErrs)
{
	if (!createStatement()) {
		return kErrorNeedNotRetry; // 建立连接失败，没必要重试
	}

	auto err = kUnknownError;
	try {
		if (autoCommit_) {
			stmt_->setAutoCommit(false); // 取消自动Commit
			autoCommit_ = false;
		}

		for (auto& sql : sqls) { // 先执行第一批SQL，插入初始数据，为后续插入大文本做准备
			try {
				stmt_->executeUpdate(sql);
			}
			catch (const occi::SQLException& e) {
				err = static_cast<ErrorCode>(e.getErrorCode());
				if (ignoreErrs && ignoreErrs->count(err)) {
					continue;
				}
				throw;
			}
		}

		decltype(clobs.size()) blobIdx = 0;
		occi::ResultSet* rs = stmt_->executeQuery(clobSql); // 开始更新大文本
		while (rs->next()) {
			if (blobIdx >= clobs.size()) {
				throw runtime_error("Result count is bigger than blobs.size()");
			}

			const auto& blob = clobs[blobIdx];
			unsigned int colIdx = 1;
			for (const auto& b : blob) {
				if (!b.Content.empty()) {
					occi::Clob clob = rs->getClob(colIdx);
					if (b.Type == kClobTypeNClob) {
						clob.setCharSetForm(occi::OCCI_SQLCS_NCHAR);
					}
					clob.write(static_cast<unsigned int>(b.Content.size()), (unsigned char*)b.Content.c_str(), static_cast<unsigned int>(b.Content.size()));
				}
				++colIdx;
			}

			++blobIdx;
		}

		conn_->commit();
		return kSuccess;
	}
	catch (const occi::SQLException& e) {
		err = static_cast<ErrorCode>(e.getErrorCode());
		errMsg_.clear();
		errMsg_.append(e.what());
	}
	catch (const std::exception& e) {
		errMsg_.clear();
		errMsg_.append(e.what());
	}
	catch (...) {
		errMsg_.clear();
		errMsg_.append("Unknown error!");
	}

	try {
		conn_->rollback();
	}
	catch (...) {
		// do nothing
	}

	return err;
}

void OracleClient::closeConnection()
{
	try {
		if (stmt_) {
			conn_->terminateStatement(stmt_);
		}
		if (conn_) {
			env_->terminateConnection(conn_);
		}
	} catch (...) {
		// do nothing
	}
	// 如果terminateXXX抛出异常，大不了就是泄漏些内存
	stmt_ = 0;
	conn_ = 0;
}

//=====================================================================================================================

OracleClient* OracleClientPool::Get()
{
	lock_.lock();
	if (!pooledClis_.empty()) {
		auto cli = pooledClis_.front();
		pooledClis_.pop_front();
		lock_.unlock();
		return cli;
	}
	lock_.unlock();

	try {
		return new OracleClient(host_, user_, passwd_);
	}
	catch (...) {
	}

	return nullptr;
}

void OracleClientPool::Put(OracleClient* cli)
{
	lock_.lock();
	if (pooledClis_.size() < maxPooledClis_) {
		pooledClis_.emplace_back(cli);
		lock_.unlock();
	}
	else {
		lock_.unlock();
		delete cli;
	}
}
