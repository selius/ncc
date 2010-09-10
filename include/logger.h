#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>

#include "common.h"

using namespace std;

class CLogger
{
public:
	enum ELogType
	{
		LOG_TYPE_INFO,
		LOG_TYPE_WARNING,
		LOG_TYPE_ERROR,
	};

	static CLogger* Instance();

	void Log(ELogType LogType, const CPosition &Position, const string &Message);

private:
	static CLogger LoggerInstance;


};

#endif // _LOGGER_H_
