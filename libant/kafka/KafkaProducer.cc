#include <memory>

#include "KafkaProducer.h"

using namespace std;

//===========================================================================
// Public methods
//===========================================================================
KafkaProducer::KafkaProducer(const std::string& brokers, const std::string& topic, RdKafka::DeliveryReportCb* drcb, RdKafka::EventCb* evcb)
{
	auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	unique_ptr<RdKafka::Conf> confForAutoDel(conf); // for auto delete when throwing exception

	setConfig(conf, "metadata.broker.list", brokers);
	if (evcb) {
		setConfig(conf, "event_cb", evcb);
	}
	if (drcb) {
		setConfig(conf, "dr_cb", drcb);
	}

	string errstr;
	auto tmpProducer = RdKafka::Producer::create(conf, errstr);
	if (!tmpProducer) {
		throw std::runtime_error("Failed to create producer! err=" + errstr);
	}
	unique_ptr<RdKafka::Producer> producer(tmpProducer); // for auto delete when throwing exception

	topic_ = RdKafka::Topic::create(tmpProducer, topic, nullptr, errstr);
	if (!topic_) {
		throw std::runtime_error("Failed to create topic! topic=" + topic + " err=" + errstr);
	}

	producer_ = producer.release();
}
