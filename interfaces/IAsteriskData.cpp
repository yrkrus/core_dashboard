#include "IAsteriskData.h"


IAsteriskData::IAsteriskData(const std::string &_name, unsigned int _timer)
	: m_name(_name) 
	, m_rawData()
	, m_dispether(_name, _timer)
{
}

IAsteriskData::~IAsteriskData()
{
}

std::deque<std::string> IAsteriskData::GetRawAllData()
{
	return m_rawData.GetRawAll();
}

bool IAsteriskData::IsExistRawData()
{
	return m_rawData.IsExistRaw();
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
