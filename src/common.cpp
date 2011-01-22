#include "common.h"

CPosition::CPosition(int ALine /*= 0*/, int AColumn /*= 0*/) : Line(ALine), Column(AColumn)
{
}

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
