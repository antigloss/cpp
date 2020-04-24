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

KafkaErrCode KafkaProducer::SyncProduce(const std::string& payload)
{
	KafkaErrCode err = static_cast<KafkaErrCode>(-12345);

	producer_->produce(topic_, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY, const_cast<char*>(payload.data()), payload.size(), nullptr, &err);
	while (err == static_cast<KafkaErrCode>(-12345)) {
		Poll(1000);
	}

	return err;
}

//=====================================================================================================================

KafkaProducer* KafkaProducerPool::Get()
{
	lock_.lock();
	if (!pooledProducers_.empty()) {
		auto cli = pooledProducers_.front();
		pooledProducers_.pop_front();
		lock_.unlock();
		return cli;
	}
	lock_.unlock();

	try {
		return new KafkaProducer(brokers_, topic_, drcb_, evcb_);
	}
	catch (...) {
	}

	return nullptr;
}

void KafkaProducerPool::Put(KafkaProducer* producer)
{
	lock_.lock();
	if (pooledProducers_.size() < maxPooledProducers_) {
		pooledProducers_.emplace_back(producer);
		lock_.unlock();
	}
	else {
		lock_.unlock();
		delete producer;
	}
}
