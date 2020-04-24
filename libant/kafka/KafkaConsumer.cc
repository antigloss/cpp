#include <memory>
#include <boost/algorithm/string.hpp>

#include "KafkaConsumer.h"

using namespace std;

//===========================================================================
// Public methods
//===========================================================================
KafkaConsumer::KafkaConsumer(const std::string& brokers, const std::string& groupID, const std::string& topics, RdKafka::OffsetCommitCb* offsetCb, RdKafka::EventCb* evcb)
{
	if (brokers.empty() || groupID.empty() || topics.empty()) {
		throw runtime_error("Invalid parameters!");
	}

	auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	unique_ptr<RdKafka::Conf> confForAutoDel(conf); // for auto delete when throwing exception

	setConfig(conf, "metadata.broker.list", brokers);
	setConfig(conf, "group.id", groupID);
	setConfig(conf, "auto.offset.reset", "smallest");
	setConfig(conf, "enable.auto.commit", "false");
	setConfig(conf, "enable.auto.offset.store", "false");
	if (evcb) {
		setConfig(conf, "event_cb", evcb);
	}
	if (offsetCb) {
		setConfig(conf, "offset_commit_cb", offsetCb);
	}

	string errstr;
	unique_ptr<RdKafka::KafkaConsumer> consumer(RdKafka::KafkaConsumer::create(conf, errstr));
	if (!consumer) {
		throw std::runtime_error("Failed to create consumer! err=" + errstr);
	}

	vector<string> allTopics;
	boost::split(allTopics, topics, boost::is_any_of(","));
	auto err = consumer->subscribe(allTopics);
	if (err) {
		throw std::runtime_error("Failed to subscribe to '" + topics + "'! err=" + RdKafka::err2str(err));
	}

	consumer_ = consumer.release();
}
