#ifndef LIBANT_KAFKACONSUMER_H_
#define LIBANT_KAFKACONSUMER_H_

#include <vector>

#include "Common.h"

/**
 * @brief RdKafka::KafkaConsumer的简单封装
 */
class KafkaConsumer {
public:
	using Message = RdKafka::Message;

public:
	/**
	 * @brief 构造函数
	 * @param brokers 逗号分隔的ip:port列表
	 * @param groupID
	 * @param topics 逗号分隔的topic列表
	 */
	KafkaConsumer(const std::string& brokers, const std::string& groupID, const std::string& topics, RdKafka::OffsetCommitCb* offsetCb = nullptr, RdKafka::EventCb* evcb = nullptr);

	~KafkaConsumer()
	{
		//consumer_->close(); // 会卡死，先注释掉，后续看情况调整
		delete consumer_;
	}

	/**
	 * @brief 获取一条消息或者错误事件,触发回调函数
	 * @remark 使用delete释放消息内存
	 * @returns One of:
	 *  - proper message (Message::err() is ERR_NO_ERROR)
	 *  - error event (Message::err() is != ERR_NO_ERROR)
	 *  - timeout due to no message or event in timeoutMS
	 *    (RdKafka::Message::err() is ERR__TIMED_OUT)
	 */
	Message* Consume(int timeoutMS)
	{
		return consumer_->consume(timeoutMS);
	}

	/**
	 * @brief 异步提交Offset
	 */
	void AsyncCommit(Message* msg)
	{
		consumer_->commitAsync(msg);
	}

private:
	RdKafka::KafkaConsumer*	consumer_;
};

#endif // !LIBANT_KAFKACONSUMER_H_
