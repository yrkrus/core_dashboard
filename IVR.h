#include <string>
#include <vector>
#include "IAsteriskData.h"
#include "ISQLConnect.h"

#ifndef IVR_H
#define IVR_H
#define MAX_IVR_PARSING_LINES 9		// ������������ �������� ��� �������� ����� ������ IVR

static std::string IVR_COMMANDS			= "Playback|lukoil|ivr-3";	// ���� ������ ��� ����� ��� ������������ IVR
static std::string IVR_COMMANDS_EXT1	= "IVREXT";					// ������� ���� ������
static std::string IVR_COMMANDS_EXT2	= "Spasibo";				// ������� ���� ������
static std::string IVR_COMMANDS_EXT3	= "recOfficeOffline";		// ������� ���� ������
static std::string IVR_COMMANDS_EXT4	= "noservice";				// ������� ���� ������ 
static std::string IVR_COMMANDS_EXT5	= "agent";					// ������� ���� ������ 
static std::string IVR_COMMANDS_EXT6	= "from-internal-xfer";		// ������� ���� ������ (������� ������)

static std::string IVR_COMMANDS_IK1 = "rec_IK_AllBusy";				// ������� ���� ������ (IVR ��� �� ������)
static std::string IVR_COMMANDS_IK2 = "rec_IK_Welcome";				// ������� ���� ������ (IVR ��� �� ������)
static std::string IVR_COMMANDS_IK3 = "rec_IK_WorkHours";			// ������� ���� ������ (IVR ��� �� ������)

static std::string IVR_REQUEST		= "asterisk -rx \"core show channels verbose\" | grep -E \"" + IVR_COMMANDS + "\" " 
																			   + " | grep -v \"" + IVR_COMMANDS_EXT1 + "\" " 
																			   + " | grep -v \"" + IVR_COMMANDS_EXT2 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT3 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT4 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT5 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_EXT6 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK1 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK2 + "\" "
																			   + " | grep -v \"" + IVR_COMMANDS_IK3 + "\" ";

class IVR;
using SP_IVR = std::shared_ptr<IVR> ;

class IVR : public IAsteriskData			
{
public:	
	enum class ECallerId
	{
		Unknown = 0,		// �����������
		Domru_220220,		// 220-220
		Domru_220000,		// 220-000
		Sts,				// STS
		Comagic,			// COMAGIC
		BeelineMih,			// MIH (����������)
	};

	struct IvrCalls
	{
		std::string phone	= "null";					// ������� ����� �������� ������� � IVR �������
		std::string waiting = "null";					// ����� � (���) ������� �� �������	
		ECallerId callerID = ECallerId::Unknown;		// ������ ������ ������		
	
		inline bool check() const noexcept 
		{
			// ���� � phone ��� waiting ���� ��������� "null" 
			// ��� callerID == Unknown � ����� false
			if (phone.find("null")		!= std::string::npos	||
				waiting.find("null")	!= std::string::npos	||
				callerID				== ECallerId::Unknown)
			{
				return false;
			}
			return true;
		}
	};
	
	
	IVR();
	~IVR() override;

	void Start() override;
	void Stop() override;
	void Parsing() override;							// ������ ����� ������	

private:
	std::vector<IvrCalls>	m_listIvr;	
	SP_SQL					m_sql;


	bool CreateCallers(const std::string&, IvrCalls&);
	bool CheckCallers(const IvrCalls &);												// �������� ������������ �������� ������
	bool IsExistListIvr();
	ECallerId StringToEnum(const std::string &_str);
	std::string EnumToString(ECallerId _caller);

	void InsertIvrCalls();																// ������� � �� ������
	void UpdateIvrCalls(int _id, const IvrCalls &_caller);								// ����������� ������ IVR �� ��
	bool IsExistIvrPhone(const IvrCalls &_caller, std::string &_errorDescription);		// ���� �� ����� ����� � ��
	int GetPhoneIDIvr(const std::string &_phone);										// id phone �� ��

};

#endif //IVR_H
