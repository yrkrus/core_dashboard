#include "IVR.h"
#include "InternalFunction.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include "SQLRequest.h"
#include <iterator>

using namespace INTERNALFUNCTION;

// ����������
//IVR_OLD::Parsing::Parsing(const char *fileIVR)
//{
//	
//	std::ifstream fileivr;
//	
//	fileivr.open(fileIVR);
//
//	if (fileivr.is_open()) {
//		//std::cout << "open file... OK\n";
//		
//		std::string line;				
//
//		// ��������
//		while (std::getline(fileivr, line)) {
//			Pacients pacient;						
//
//			pacient.phone		= findParsing(line, IVR_OLD::Currentfind::phone_find);
//			pacient.waiting		= findParsing(line, IVR_OLD::Currentfind::waiting_find);
//			pacient.callerID	= getCallerID(findParsing(line, IVR_OLD::Currentfind::caller_id));
//
//			// ���������
//			if (pacient.phone != "null" && 
//				pacient.waiting != "null" &&
//				pacient.callerID != null_caller) {
//				pacient_list.push_back(pacient);
//			}				
//		}		
//	}
//
//	fileivr.close();
//}


//IVR_OLD::CallerID IVR_OLD::getCallerID(std::string str)
//{
//	if (str.find("ivr-13")				!= std::string::npos)	return domru_220000;
//	if (str.find("druOUT_220220")		!= std::string::npos)	return domru_220220;
//	if (str.find("Dru_220000")			!= std::string::npos)	return domru_220220;	
//	if (str.find("sts_")				!= std::string::npos)	return sts;
//	if (str.find("221122")				!= std::string::npos)	return comagic;
//	if (str.find("camaa")				!= std::string::npos)	return comagic;
//	return null_caller;
//}


//std::string IVR_OLD::getCallerID(const CallerID &callerID)
//{
//	switch (callerID)
//	{
//		case(domru_220220): {
//			return "220220";
//			break;
//		}
//		case(domru_220000): {
//			return "220000";
//			break;
//		}
//		case(sts):
//		{
//			return "STS";
//			break;
//		}
//		case(comagic):
//		{
//			return "COMAGIC";
//			break;
//		}
//		case(null_caller):
//		{
//			return "null_caller";
//			break;
//		}	
//	}
//}



// �������� ������ �� ������ � ��������
//bool IVR_OLD::Parsing::isExistList()
//{
//	return (pacient_list.empty() ? false : true);
//}


//void IVR_OLD::Parsing::show(bool silent)
//{	
//	std::ostringstream buffer;
//	
//	if (this->isExistList()) {
//		buffer << "Line IVR is (" << pacient_list.size() << ")\n";
//		
//		if (!silent) {
//			buffer << "trunk" << "\t    \t" << "phone" << "\t \t" << " wait time" << "\n";
//
//			for (std::vector<IVR_OLD::Pacients>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
//			{
//
//				buffer << getCallerID(it->callerID) << "\t >> \t" << it->phone << "\t (" << it->waiting << ")\n";
//
//				{ // test
//
//					std::string subString3 = "00:01:";
//					std::string subString4 = "00:02:";
//					std::string subString5 = "00:03:";
//					std::string subString6 = "00:04:";
//					std::string subString7 = "00:05:";
//
//
//
//					size_t found3 = it->waiting.find(subString3);
//					size_t found4 = it->waiting.find(subString4);
//					size_t found5 = it->waiting.find(subString5);
//					size_t found6 = it->waiting.find(subString6);
//					size_t found7 = it->waiting.find(subString7);
//
//
//
//					if (found3 != std::string::npos)
//					{
//						buffer << "�������";
//					}
//					if (found4 != std::string::npos)
//					{
//						buffer << "�������";
//					}
//					if (found5 != std::string::npos)
//					{
//						buffer << "�������";
//					}
//					if (found6 != std::string::npos)
//					{
//						buffer << "�������";
//					}
//					if (found7 != std::string::npos)
//					{
//						buffer << "�������";
//					}
//				}
//			}
//		}			
//	}
//	else {
//		buffer << "IVR is empty!\n";
//	}
//
//	std::cout << buffer.str();
//}


// ���������� ������ � ��
//void IVR_OLD::Parsing::insertData()
//{
//	if (this->isExistList()) {
//		SQL_REQUEST::SQL base;
//
//		for (std::vector<IVR_OLD::Pacients>::iterator it = pacient_list.begin(); it != pacient_list.end(); ++it)
//		{
//			if (base.isConnectedBD())
//			{
//				base.insertIVR(it->phone.c_str(), it->waiting.c_str(), getCallerID(it->callerID));
//			}
//		}
//	}	
//}

// ������� ������
//std::string IVR_OLD::Parsing::findParsing(std::string str, Currentfind find)
//{
//	std::vector<std::string> lines;
//	std::string current_str;
//
//	bool isNewLine{ false };
//
//	for (size_t i = 0; i != str.length(); ++i)
//	{
//
//		if (isNewLine)
//		{
//			if (!current_str.empty())
//			{
//				lines.push_back(current_str);
//				current_str.clear();
//			}
//		}
//
//		if (str[i] != ' ') // ���� ��������� (��������� ������ ������)
//		{
//			current_str += str[i];
//			isNewLine = false;
//		}
//		else
//		{
//			isNewLine = true;
//		}
//	}
//
//	if (!lines.empty())
//	{
//		switch (find)
//		{
//		case IVR_OLD::Currentfind::phone_find:
//		{
//			//return phoneParsing(lines[7]); TODO �������			
//			break;
//		}
//		case IVR_OLD::Currentfind::waiting_find:
//		{
//			return lines[8];
//			break;
//		}
//		case IVR_OLD::Currentfind::caller_id:
//		{
//			return lines[0]+","+ lines[1];
//			break;
//		}
//
//		default:
//		{
//			return "null";
//			break;
//		}
//		}
//	}
//	else {
//		return "null";
//	}	
//}

IVR::IVR()
	:IAsteriskData(CONSTANTS::TIMEOUT::IVR)
	, m_sql(std::make_shared<ISQLConnect>(false))
{
}

IVR::~IVR()
{
}

void IVR::Start()
{
	std::string error;
	auto func = [this, error = std::move(error)]() mutable
		{
			return m_rawData.CreateData(IVR_REQUEST, error);
		};

	m_dispether.Start(func);
}

void IVR::Stop()
{
	m_dispether.Stop();
}





void IVR::Parsing()
{
	m_listIvr.clear(); // ������� ������� ������	
	
	std::string rawLines = GetRawLastData();
	if (rawLines.empty()) 
	{
		return;
	}	

	std::istringstream ss(rawLines);
	std::string line;

	while (std::getline(ss, line))	
	{
		IvrCalls caller;

		if (CreateCallers(line, caller)) 
		{
			m_listIvr.push_back(caller);
		}		
	}

	// ��� �� ���� ����� ������ � �� ���������
	if (IsExistListIvr())
	{		
		InsertIvrCalls();
		
		// ������� �� ����� ������
		DeleteRawLastData();
	}
}

bool IVR::CreateCallers(const std::string &_lines, IvrCalls &_caller)
{
	bool status = false;
	
	std::vector<std::string> lines;
	std::string current_str;

	bool isNewLine = false;

	for (size_t i = 0; i != _lines.length(); ++i)
	{
		if (isNewLine)
		{
			if (!current_str.empty())
			{
				lines.push_back(current_str);
				current_str.clear();
			}
		}

		if (_lines[i] != ' ') // ���� ��������� (��������� ������ ������)
		{
			current_str += _lines[i];
			isNewLine = false;
		}
		else
		{
			isNewLine = true;
		}
	}

	if (!lines.empty())
	{
		_caller.phone = INTERNALFUNCTION::phoneParsing(lines[7]);
		_caller.waiting = lines[8];
		_caller.callerID = StringToEnum(lines[0] + "," + lines[1]);
		
		// TODO ��� � ��� ������ ���� �� ������ �� ����� �� ������� 
		if (!CheckCallers(_caller)) 
		{
			return false;
		}		

		status = true;
	}

	return status;
}

bool IVR::CheckCallers(const IvrCalls &_caller)
{
	return !((_caller.phone		== "null") &&
			 (_caller.waiting	== "null") &&
			 (_caller.callerID	== ecCallerId::eUnknown));
}

bool IVR::IsExistListIvr()
{
	return !m_listIvr.empty() ? true : false;
}

IVR::ecCallerId IVR::StringToEnum(const std::string &_str)
{
	if (_str.find("Dru_220000") != std::string::npos)		return ecCallerId::eDomru_220220;
	if (_str.find("druOUT_220220") != std::string::npos)	return ecCallerId::eDomru_220220;
	if (_str.find("ivr-13") != std::string::npos)			return ecCallerId::eDomru_220000;
	if (_str.find("sts_") != std::string::npos)				return ecCallerId::eSts;
	if (_str.find("221122") != std::string::npos)			return ecCallerId::eComagic;
	if (_str.find("camaa") != std::string::npos)			return ecCallerId::eComagic;
	if (_str.find("BeeIn") != std::string::npos)			return ecCallerId::eBeelineMih;
	
	return ecCallerId::eUnknown;
}

std::string IVR::EnumToString(ecCallerId _caller)
{
	static std::map<ecCallerId, std::string> callers =
	{
		{ecCallerId::eUnknown,		"Unknown"},
		{ecCallerId::eDomru_220220, "220220"},
		{ecCallerId::eDomru_220000, "220000"},
		{ecCallerId::eSts,			"STS"},
		{ecCallerId::eComagic,		"COMAGIC"},
		{ecCallerId::eBeelineMih,	"MIH" }
	};

	auto it = callers.find(_caller);
	if (it != callers.end()) 
	{
		return it->second;
	}

	return "Unknown";
}

void IVR::InsertIvrCalls()
{
	std::string _errorDescription = "";

	if (!IsExistListIvr()) 
	{
		_errorDescription = StringFormat("%s ivr list is empty", METHOD_NAME);
		printf("%s", _errorDescription.c_str());
		return;
	}	

	for (const auto &list : m_listIvr) 
	{
		// �������� ���� �� ����� ����� � ��
		if (IsExistIvrPhone(list, _errorDescription)) 
		{
			unsigned int id = GetPhoneIDIvr(list.phone);
			UpdateIvrCalls(id, list);
		}
		else  
		{ 
			// ������ ������ ��� ����� �������� � ��
			const std::string query = "insert into ivr (phone,waiting_time,trunk) values ('" 
									+ list.phone + "','" 
									+ list.waiting + "','" 
									+ EnumToString(list.callerID) + "')";

			if (!m_sql->Request(query, _errorDescription))
			{
				m_sql->Disconnect();
				return;
			}

			m_sql->Disconnect();
		}
	}
}

void IVR::UpdateIvrCalls(unsigned int _id, const IvrCalls &_caller)
{
	std::string error;
	const std::string query = "update ivr set waiting_time = '" 
										+ _caller.waiting 
										+ "' where phone = '" 
										+ _caller.phone 
										+ "' and id ='" + std::to_string(_id) + "'";

	if (!m_sql->Request(query, error))
	{
		printf("%s %s", METHOD_NAME, error.c_str());
		m_sql->Disconnect();
		return;
	}

	m_sql->Disconnect();
}


bool IVR::IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription)
{
	
	const std::string query = "select count(phone) from ivr where phone = '"
								+ std::string(_caller.phone) + "' and  date_time > '"
								+ getCurrentDateTimeAfterMinutes(2) + "' and to_queue = '0' order by date_time desc";

	if (!m_sql->Request(query, _errorDescription))
	{	
		// ���� ������ ������� ��� ���� ����� ����� � ��
		m_sql->Disconnect();
		return true;
	}

	// ���������
	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	bool existIvrPhone;
	std::stoi(row[0]) == 0 ? existIvrPhone = false : existIvrPhone = true;

	mysql_free_result(result);

	m_sql->Disconnect();

	return existIvrPhone;

}

unsigned int IVR::GetPhoneIDIvr(const std::string &_phone)
{
	const std::string query = "select id from ivr where phone = "
								+ _phone + " and date_time > '"
								+ getCurrentStartDay() + "' order by date_time desc limit 1";

	
	if (!m_sql->Request(query))
	{
		// ���� ������		
		m_sql->Disconnect();
		return 0;
	}
	

	MYSQL_RES *result = mysql_store_result(m_sql->Get());
	MYSQL_ROW row = mysql_fetch_row(result);

	unsigned int id = std::stoi(row[0]);

	mysql_free_result(result);

	m_sql->Disconnect();

	return id;
}



