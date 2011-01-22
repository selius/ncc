#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>

using namespace std;

struct CPosition
{
	CPosition(int ALine = 0, int AColumn = 0);
	int Line;
	int Column;
};

class CException
{
public:
	CException(const string &AMessage, const CPosition &APosition);

	string GetMessage() const;
	CPosition GetPosition() const;

private:
	string Message;
	CPosition Position;
};


#endif // _COMMON_H_
