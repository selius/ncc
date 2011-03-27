#include "common.h"

#include "statements.h"

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

EExitCode CException::GetExitCode() const
{
	return EXIT_CODE_UNKNOWN_ERROR;
}

void CException::Output(ostream &Stream) const
{
	Stream << Position.Line << ", " << Position.Column << ": error: " << Message << endl;
}

/******************************************************************************
 * CScannerException
 ******************************************************************************/

CScannerException::CScannerException(const string &AMessage, const CPosition &APosition) : CException(AMessage, APosition)
{
}

EExitCode CScannerException::GetExitCode() const
{
	return EXIT_CODE_SCANNER_ERROR;
}

/******************************************************************************
 * CParserException
 ******************************************************************************/

CParserException::CParserException(const string &AMessage, const CPosition &APosition) : CException(AMessage, APosition)
{
}

EExitCode CParserException::GetExitCode() const
{
	return EXIT_CODE_PARSER_ERROR;
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

/******************************************************************************
 * CStatementVisitor
 ******************************************************************************/

CStatementVisitor::~CStatementVisitor()
{
}

void CStatementVisitor::TryVisit(CStatement *AStmt)
{
	if (AStmt) {
		AStmt->Accept(*this);
	}
}
