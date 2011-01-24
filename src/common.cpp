#include "common.h"

/******************************************************************************
 * CPosition
 ******************************************************************************/

CPosition::CPosition(int ALine /*= 0*/, int AColumn /*= 0*/) : Line(ALine), Column(AColumn)
{
}

/******************************************************************************
 * CException
 ******************************************************************************/

CException::CException(const string &AMessage, const CPosition &APosition) : Message(AMessage), Position(APosition)
{
}

string CException::GetMessage() const
{
	return Message;
}

CPosition CException::GetPosition() const
{
	return Position;
}

void CException::Output(ostream &Stream) const
{
	Stream << Position.Line << ", " << Position.Column << ": error: " << Message << endl;
}

/******************************************************************************
 * CFatalException
 ******************************************************************************/

CFatalException::CFatalException(EExitCode AExitCode, const string &AMessage /*= ""*/) : ExitCode(AExitCode), Message(AMessage)
{
}

EExitCode CFatalException::GetExitCode() const
{
	return ExitCode;
}

string CFatalException::GetMessage() const
{
	return Message;
}
