#include "IClear.h"

IClear::IClear()
	: m_sql(std::make_shared<ISQLConnect>(false))
{
}

IClear::~IClear()
{
}
