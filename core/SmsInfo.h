#ifndef SMSINFO_H
#define SMSINFO_H

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "../interfaces/IHTTPRequest.h"
#include "../interfaces/ISQLConnect.h"
#include "../MobileOperatorInfo.h"

static std::string HTTP_REQUEST_SMS = "https://a2p-sms-https.beeline.ru/proto/http/?gzip=none&user=%user&pass=%pass&action=status&sms_id=%sms_id";


enum class ecXMLValue 
{
    code = 0,
    close,
    send,
};

class AuthSMS 
{
private:
    std::string m_login;
    std::string m_pwd;
    bool m_init;
public:   
    AuthSMS() = default;
    AuthSMS(std::string _login, std::string _pwd);
    ~AuthSMS();

    void Init(std::string _login, std::string _pwd);
    bool IsInit() const {return m_init;}
    std::string Login() const {return m_login;}
    std::string Pass() const {return m_pwd;}
};

enum class ecSmsInfoTable // какую таблицу будем обновлять
{
    eSMS,           // sms_sending
    eHistorySMS,    // history_sms_sending
};

enum class ecSmsCode // TODO такие же ID в sms_code
{
    eQueued = 1,        // Сообщение находится в очереди отправки и еще не было передано оператору
    eAccepted,          // Сообщение уже передано оператору
    eDelivered,         // Сообщение успешно доставлено абоненту
    eRejected,          // Сообщение отклонено оператором
    eUndeliverable,     // Сообщение невозможно доставить из-за недоступности абонента
    eError,             // Ошибка отправки. Сообщение не было отправлено абоненту
    eExpired,           // Истекло время ожидания финального статуса
    eUnknown,           // Статус сообщения неизвестен
    eAborted,           // Сообщение отменено пользователем
    e20107 = 20107,	    // Неверный логин или пароль
    e20117 = 20117,	    // Некорреткный номер телефона
    e20148 = 20148,	    // Невозможно предоставить услуги для продукта
    e20154 = 20154,	    // Ошибка транспорта
    e20158 = 20158,	    // Отправка невозможна, так как номер занесён в чёрный список
    e20167 = 20167,	    // Запрещено посылать сообщение с тем же текстом тому же адресату в течение нескольких минут
    e20170 = 20170,	    // Слишком длинное сообщение
    e20171 = 20171,	    // Сообщение не прошло проверку цензуры
    e20200 = 20200,	    // Неправильный запрос
    e20202 = 20202,	    // Не найден почтовый ящик для входящих сообщений
    e20203 = 20203,	    // Нет номера телефона или идентификатора группы в запросе
    e20204 = 20204,	    // Не найдены телефоны для группы
    e20207 = 20207,	    // Неправильный формат даты
    e20208 = 20208,	    // Дата начала позже даты конца
    e20209 = 20209,	    // Параметры запроса пустые
    e20211 = 20211,	    // Превышено количество сообщений для пользователя
    e20212 = 20212,	    // Превышен интервал в выбранных датах
    e20213 = 20213,	    // Невалидные номера в списке
    e20218 = 20218,	    // Запрещено отправлять на несколько адресов
    e20230 = 20230,	    // Отправитель не одобрен на стороне оператора
    e20280 = 20280,	    // Достигнут суточный лимит на отправку SMS с платформы A2P
    e20281 = 20281,	    // Достигнут месячный лимит на отправку SMS с платформы A2P
};

struct InfoSMS
{
    int         id = 0;         // из базы
    size_t      sms_id = 0;     // из базы   
    std::string date;           // из базы
    std::string phone;          // из базы
    ecSmsCode   code = ecSmsCode::eUnknown; // из http запроса    
    bool        close;          // из http запроса
    bool        send;           // из http запроса   
};
using InfoSMSList = std::vector<InfoSMS>;


class SMSInfo : public virtual IHTTPRequest 
              , public MobileOperatorInfo
{
public:      

     SMSInfo();
     SMSInfo(ecSmsInfoTable _table);
    ~SMSInfo() override;

    bool Execute();
    virtual bool ExecuteFindMobileOpeartorInfo();     

private:
    InfoSMSList    m_listSMS;
    SP_SQL		   m_sql;
	SP_Log		   m_log; 
    AuthSMS        m_auth;
    ecSmsInfoTable m_table;    

    void InitAuthSMS();                // авторизационные данные для смс

    void CreateListSMS(); // создание списка с смс которые будем проверять
    bool GetInfoSMSList(InfoSMSList &_list, std::string &_errorDescription); // получение списка с смс
    bool IsExistList();     // есть ли список

    void FindInfoSMS();    // нахождение всех данных по отправленной смс

    std::string GetLinkHttpRequest(size_t _smsId); // создание http запроса
    bool ParsingXML(const boost::property_tree::ptree &_ptree, ecXMLValue _xmlValue, InfoSMS &_sms, std::string &_errorDescription);    // парсинг xml
        
    void UpdateToBaseInfoSMS(int _id, const InfoSMS &_sms);  // обновление записи в БД

    virtual bool Get(const std::string &_request, std::string &_responce, std::string &_errorDescription);

    // override MobileOperatorInfo
    virtual bool GetInfoMobileList(MobileInfoList &_list, std::string &_errorDescription) override;
};


#endif //SMSINFO_H