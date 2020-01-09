#ifndef LIBANT_KAFKACOMMON_H_
#define LIBANT_KAFKACOMMON_H_

#include <exception>
#include <string>

#include <librdkafka/rdkafkacpp.h>

using KafkaErrCode = RdKafka::ErrorCode;

inline void setConfig(RdKafka::Conf* conf, const std::string& key, const std::string& value)
{
	std::string errstr;
	if (conf->set(key, value, errstr) == RdKafka::Conf::CONF_OK) {
		return;
	}
	throw std::runtime_error("Failed to set '" + key + "' to '" + value + "'! err=" + errstr);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::EventCb* cbObj)
{
	std::string errstr;
	if (conf->set(key, cbObj, errstr) == RdKafka::Conf::CONF_OK) {
		return;
	}
	throw std::runtime_error("Failed to set '" + key + "'! err=" + errstr);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::OffsetCommitCb* cbObj)
{
	std::string errstr;
	if (conf->set(key, cbObj, errstr) == RdKafka::Conf::CONF_OK) {
		return;
	}
	throw std::runtime_error("Failed to set '" + key + "'! err=" + errstr);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::DeliveryReportCb* cbObj)
{
	std::string errstr;
	if (conf->set(key, cbObj, errstr) == RdKafka::Conf::CONF_OK) {
		return;
	}
	throw std::runtime_error("Failed to set '" + key + "'! err=" + errstr);
}

#endif // LIBANT_KAFKACOMMON_H_
