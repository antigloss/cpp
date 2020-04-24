#ifndef LIBANT_KAFKAPRODUCER_H_
#define LIBANT_KAFKAPRODUCER_H_

#include <mutex>
#include "Common.h"

/**
 * @brief RdKafka::Producer的简单封装
 */
class KafkaProducer {
public:
	/**
	 * @brief 构造函数
	 * @param brokers 逗号分隔的ip:port列表
	 * @param topic
	 */
	KafkaProducer(const std::string& brokers, const std::string& topic, RdKafka::DeliveryReportCb* drcb = nullptr, RdKafka::EventCb* evcb = nullptr);

	~KafkaProducer()
	{
		producer_->flush(10000);
		delete topic_;
		delete producer_;
	}

	/**
	 * @brief 异步往kafka broker发送一条消息
	 * @param payload 发送给kafka broker的消息内容
	 * @param msgOpaque 调用送达通知回调函数dr_cb时，传递给该函数
	 * @return 表示成功或者失败的错误码，取值如下：
	 *  - ERR_NO_ERROR           - 成功写入队列
	 *  - ERR__QUEUE_FULL        - 队列满：queue.buffering.max.message
	 *  - ERR_MSG_SIZE_TOO_LARGE - 消息太大：messages.max.bytes
	 *  - ERR__UNKNOWN_PARTITION
	 *  - ERR__UNKNOWN_TOPIC
	 */
	KafkaErrCode Produce(const std::string& payload, void* msgOpaque = nullptr)
	{
		// 只有当送达通知回调函数返回之后，该消息才会被移出队列。
		// 所以produce返回队列满时，不能马上重试，只能在定期调用Poll()函数后再重试。
		return producer_->produce(topic_, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
								  const_cast<char*>(payload.data()), payload.size(), nullptr, msgOpaque);
	}

	/**
	* @brief 同步往kafka broker发送一条消息
	* @param payload 发送给kafka broker的消息内容
	* @return 表示成功或者失败的错误码
	*/
	KafkaErrCode SyncProduce(const std::string& payload);

	/**
	 * @brief 触发已完成事件的回调函数。程序必须定期调用该函数，以触发事件回调函数。
	 * @param timeoutMS 函数等待事件的时间（毫秒），0表示不等待，-1表示永久等待
	 *
	 * 事件包括：
	 *   - 送达通知，回调dr_cb
	 *   - 其他事件，调用event_cb
	 * @returns 返回事件个数
	 */
	int Poll(int timeoutMS = 0)
	{
		return producer_->poll(timeoutMS);
	}

private:
	RdKafka::Producer*	producer_;
	RdKafka::Topic*		topic_;
};

/**
* @brief KafkaProducer对象池，线程安全。
*/
class KafkaProducerPool {
public:
	/**
	* @brief 构造Oracle操作对象
	* @throw occi::SQLException
	* @note 多线程环境下，使用者必须保证drcb和evcb是线程安全的
	*/
	KafkaProducerPool(size_t maxPooledProducers, const std::string& brokers, const std::string& topic, RdKafka::DeliveryReportCb* drcb = nullptr, RdKafka::EventCb* evcb = nullptr)
		: maxPooledProducers_(maxPooledProducers), brokers_(brokers), topic_(topic), drcb_(drcb), evcb_(evcb)
	{
	}

	~KafkaProducerPool()
	{
		for (auto cli : pooledProducers_) {
			delete cli;
		}
	}

	KafkaProducerPool(const KafkaProducerPool&) = delete;
	const KafkaProducerPool& operator=(const KafkaProducerPool&) = delete;

	/**
	* @brief 获取KafkaProducer对象，使用完毕后，要调用Put接口返还。
	*/
	KafkaProducer* Get();

	/**
	* @brief 返还通过Get接口获取的KafkaProducer对象。
	*/
	void Put(KafkaProducer* producer);

private:
	const size_t						maxPooledProducers_;
	const std::string					brokers_;
	const std::string					topic_;
	RdKafka::DeliveryReportCb* const	drcb_;
	RdKafka::EventCb* const				evcb_;

	std::mutex					lock_;
	std::list<KafkaProducer*>	pooledProducers_;
};

#endif // LIBANT_KAFKAPRODUCER_H_
