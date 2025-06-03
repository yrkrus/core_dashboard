// Базовый класс для файла
#include <string>
#include <set>

#ifndef IFILE_H
#define	IFILE_H


using std::string;

class IFile 
{
private:			
		string m_directory;				// директория с 		
		string	m_fileName;				// название файла
		uint m_counter;					// счетчик

		std::set<string> m_list;		// список с созданными файлами		

public:
	IFile(const string &_fileName);
	~IFile();

	//std::set<string> *GetList() const;			// получение списканазвание файла
	bool CreateFile(string &_response, string &_errorDescription);



};



#endif // IFILE_H