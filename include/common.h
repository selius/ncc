#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
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
#include <vector>

using namespace std;

#define COMPILER_NAME "ncc"
#define COMPILER_TITLE "Nartov C Compiler"
#define COMPILER_VERSION "0.2.4"

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

class CStatementVisitor
{
public:
	virtual ~CStatementVisitor();

	virtual void Visit(CUnaryOp &AExpr) = 0;
	virtual void Visit(CBinaryOp &AExpr) = 0;
	virtual void Visit(CConditionalOp &AExpr) = 0;
	virtual void Visit(CIntegerConst &AExpr) = 0;
	virtual void Visit(CFloatConst &AExpr) = 0;
	virtual void Visit(CSymbolConst &AExpr) = 0;
	virtual void Visit(CStringConst &AExpr) = 0;
	virtual void Visit(CVariable &AExpr) = 0;
	virtual void Visit(CPostfixOp &AExpr) = 0;
	virtual void Visit(CFunctionCall &AExpr) = 0;
	virtual void Visit(CStructAccess &AExpr) = 0;
	virtual void Visit(CIndirectAccess &AExpr) = 0;
	virtual void Visit(CArrayAccess &AExpr) = 0;
	virtual void Visit(CNullStatement &AExpr) = 0;
	virtual void Visit(CBlockStatement &AExpr) = 0;
	virtual void Visit(CIfStatement &AExpr) = 0;
	virtual void Visit(CForStatement &AExpr) = 0;
	virtual void Visit(CWhileStatement &AExpr) = 0;
	virtual void Visit(CDoStatement &AExpr) = 0;
	virtual void Visit(CLabel &AExpr) = 0;
	virtual void Visit(CCaseLabel &AExpr) = 0;
	virtual void Visit(CDefaultCaseLabel &AExpr) = 0;
	virtual void Visit(CGotoStatement &AExpr) = 0;
	virtual void Visit(CBreakStatement &AExpr) = 0;
	virtual void Visit(CContinueStatement &AExpr) = 0;
	virtual void Visit(CReturnStatement &AExpr) = 0;
};

#endif // _COMMON_H_
