#include "QueueOutgoing.h"
#include "system/Constants.h"

QueueOutgoing::QueueOutgoing()
    : IAsteriskData("QueueOutgoing",CONSTANTS::TIMEOUT::QUEUE_OUTGOING)
	, m_sql(std::make_shared<ISQLConnect>(false))
	, m_log(CONSTANTS::LOG::QUEUE_OUTGOING)
{
}

QueueOutgoing::~QueueOutgoing()
{
}

void QueueOutgoing::Start()
{
    // std::string error;
	// auto func = [this, error = std::move(error)]() mutable
	// 	{
	// 		return m_rawData.CreateData(QUEUE_OUTGOING_REQUEST, error);
	// 	};

	// m_dispether.Start(func);
}

void QueueOutgoing::Stop()
{
}

void QueueOutgoing::Parsing()
{
}
