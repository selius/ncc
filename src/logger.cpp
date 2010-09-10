#include "logger.h"

#include <iostream>

CLogger* CLogger::Instance()
{
	return &LoggerInstance;
}

void CLogger::Log(ELogType LogType, const CPosition &Position, const string &Message)
{
	cout << Position.Line << ", " << Position.Column << ": ";
	switch (LogType) {
	case LOG_TYPE_ERROR:
		cout << "error";
		break;
	case LOG_TYPE_WARNING:
		cout << "warning";
		break;
	case LOG_TYPE_INFO:
		cout << "info";
		break;
	}

	cout << ": " << Message << endl;
}

CLogger CLogger::LoggerInstance;

