#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <istream>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

using namespace std;

#define COMPILER_NAME "ncc"
#define COMPILER_TITLE "Nartov C Compiler"
#define COMPILER_VERSION "0.4.0"

enum EExitCode
{
	EXIT_CODE_SUCCESS,
	EXIT_CODE_TOO_FEW_ARGUMENTS,
	EXIT_CODE_INVALID_ARGUMENTS,
	EXIT_CODE_NO_INPUT_FILE,
	EXIT_CODE_TOO_MANY_INPUT_FILES,
	EXIT_CODE_SCANNER_ERROR,
	EXIT_CODE_PARSER_ERROR,
	EXIT_CODE_NOT_IMPLEMENTED,
};

enum ECompilerMode
{
	COMPILER_MODE_UNDEFINED,
	COMPILER_MODE_SCAN,
	COMPILER_MODE_PARSE,
	COMPILER_MODE_GENERATE,
	COMPILER_MODE_OPTIMIZE,
};

enum EParserOutputMode
{
	PARSER_OUTPUT_MODE_TREE,
	PARSER_OUTPUT_MODE_LINEAR,
};

enum EParserMode
{
	PARSER_MODE_NORMAL,
	PARSER_MODE_EXPRESSION,
};

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

	void Output(ostream &Stream) const;

private:
	string Message;
	CPosition Position;
};

class CFatalException
{
public:
	CFatalException(EExitCode AExitCode, const string &AMessage = "");

	EExitCode GetExitCode() const;
	string GetMessage() const;

private:
	EExitCode ExitCode;
	string Message;

};

template<typename T>
string ToString(const T &t)
{
	stringstream ss;
	ss << t;
	return ss.str();
}

class CStatement;
class CUnaryOp;
class CBinaryOp;
class CConditionalOp;
class CIntegerConst;
class CFloatConst;
class CSymbolConst;
class CStringConst;
class CVariable;
class CPostfixOp;
class CFunctionCall;
class CStructAccess;
class CIndirectAccess;
class CArrayAccess;
class CNullStatement;
class CBlockStatement;
class CIfStatement;
class CForStatement;
class CWhileStatement;
class CDoStatement;
class CLabel;
class CCaseLabel;
class CDefaultCaseLabel;
class CGotoStatement;
class CBreakStatement;
class CContinueStatement;
class CReturnStatement;
class CSwitchStatement;

class CStatementVisitor
{
public:
	virtual ~CStatementVisitor();

	virtual void Visit(CUnaryOp &AStmt) = 0;
	virtual void Visit(CBinaryOp &AStmt) = 0;
	virtual void Visit(CConditionalOp &AStmt) = 0;
	virtual void Visit(CIntegerConst &AStmt) = 0;
	virtual void Visit(CFloatConst &AStmt) = 0;
	virtual void Visit(CSymbolConst &AStmt) = 0;
	virtual void Visit(CStringConst &AStmt) = 0;
	virtual void Visit(CVariable &AStmt) = 0;
	virtual void Visit(CPostfixOp &AStmt) = 0;
	virtual void Visit(CFunctionCall &AStmt) = 0;
	virtual void Visit(CStructAccess &AStmt) = 0;
	virtual void Visit(CIndirectAccess &AStmt) = 0;
	virtual void Visit(CArrayAccess &AStmt) = 0;
	virtual void Visit(CNullStatement &AStmt) = 0;
	virtual void Visit(CBlockStatement &AStmt) = 0;
	virtual void Visit(CIfStatement &AStmt) = 0;
	virtual void Visit(CForStatement &AStmt) = 0;
	virtual void Visit(CWhileStatement &AStmt) = 0;
	virtual void Visit(CDoStatement &AStmt) = 0;
	virtual void Visit(CLabel &AStmt) = 0;
	virtual void Visit(CCaseLabel &AStmt) = 0;
	virtual void Visit(CDefaultCaseLabel &AStmt) = 0;
	virtual void Visit(CGotoStatement &AStmt) = 0;
	virtual void Visit(CBreakStatement &AStmt) = 0;
	virtual void Visit(CContinueStatement &AStmt) = 0;
	virtual void Visit(CReturnStatement &AStmt) = 0;
	virtual void Visit(CSwitchStatement &AStmt) = 0;

protected:
	void TryVisit(CStatement *AStmt);
};

namespace TypeSize
{
	const size_t Integer = 4;
	const size_t Float = 4;
	const size_t Pointer = 4;
};

#endif // _COMMON_H_
