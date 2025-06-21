#include "IAsteriskData.h"


IAsteriskData::IAsteriskData(unsigned int _timer)
	: m_rawData()
	, m_dispether(_timer)
{
}

IAsteriskData::~IAsteriskData()
{
}

std::deque<std::string> IAsteriskData::GetRawAllData()
{
	return m_rawData.GetRawAll();
}

std::string IAsteriskData::GetRawLastData()
{
	return m_rawData.GetRawLast();
}

std::string IAsteriskData::GetRawFirstData()
{
	return m_rawData.GetRawFirst();
}

void IAsteriskData::DeleteRawLastData()
{
	m_rawData.DeleteRawLast();
}

void IAsteriskData::DeleteRawAllData()
{
	m_rawData.DeleteRawAll();
}
