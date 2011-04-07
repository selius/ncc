#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <deque>
#include <fstream>
#include <istream>
#include <iostream>
#include <list>
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
	EXIT_CODE_UNKNOWN_ERROR,
	EXIT_CODE_NOT_IMPLEMENTED,
};

enum ECompilerMode
{
	COMPILER_MODE_UNDEFINED,
	COMPILER_MODE_SCAN,
	COMPILER_MODE_PARSE,
	COMPILER_MODE_GENERATE,
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

enum ETokenType
{
	TOKEN_TYPE_INVALID,

	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_BLOCK_START,
	TOKEN_TYPE_BLOCK_END,

	TOKEN_TYPE_LEFT_PARENTHESIS,
	TOKEN_TYPE_RIGHT_PARENTHESIS,

	TOKEN_TYPE_LEFT_SQUARE_BRACKET,
	TOKEN_TYPE_RIGHT_SQUARE_BRACKET,

	TOKEN_TYPE_CONSTANT_INTEGER,
	TOKEN_TYPE_CONSTANT_FLOAT,
	TOKEN_TYPE_CONSTANT_CHAR,
	TOKEN_TYPE_CONSTANT_STRING,

	TOKEN_TYPE_OPERATION_PLUS,
	TOKEN_TYPE_OPERATION_MINUS,
	TOKEN_TYPE_OPERATION_ASTERISK,
	TOKEN_TYPE_OPERATION_SLASH,
	TOKEN_TYPE_OPERATION_PERCENT,

	TOKEN_TYPE_OPERATION_ASSIGN,
	TOKEN_TYPE_OPERATION_PLUS_ASSIGN,
	TOKEN_TYPE_OPERATION_MINUS_ASSIGN,
	TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN,
	TOKEN_TYPE_OPERATION_SLASH_ASSIGN,
	TOKEN_TYPE_OPERATION_PERCENT_ASSIGN,

	TOKEN_TYPE_OPERATION_EQUAL,
	TOKEN_TYPE_OPERATION_NOT_EQUAL,
	TOKEN_TYPE_OPERATION_LESS_THAN,
	TOKEN_TYPE_OPERATION_GREATER_THAN,
	TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL,
	TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL,

	TOKEN_TYPE_OPERATION_LOGIC_AND,
	TOKEN_TYPE_OPERATION_LOGIC_OR,
	TOKEN_TYPE_OPERATION_LOGIC_NOT,

	TOKEN_TYPE_OPERATION_AMPERSAND,

	TOKEN_TYPE_OPERATION_BITWISE_OR,
	TOKEN_TYPE_OPERATION_BITWISE_NOT,
	TOKEN_TYPE_OPERATION_BITWISE_XOR,

	TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN,
	TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN,
	TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN,

	TOKEN_TYPE_OPERATION_SHIFT_LEFT,
	TOKEN_TYPE_OPERATION_SHIFT_RIGHT,

	TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN,
	TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN,

	TOKEN_TYPE_OPERATION_DOT,
	TOKEN_TYPE_OPERATION_INDIRECT_ACCESS,

	TOKEN_TYPE_OPERATION_INCREMENT,
	TOKEN_TYPE_OPERATION_DECREMENT,

	TOKEN_TYPE_OPERATION_CONDITIONAL,

	TOKEN_TYPE_SEPARATOR_COMMA,
	TOKEN_TYPE_SEPARATOR_SEMICOLON,
	TOKEN_TYPE_SEPARATOR_COLON,

	TOKEN_TYPE_EOF,
};

struct CCompilerParameters
{
	CCompilerParameters();

	string InputFilename;
	string OutputFilename;
	ECompilerMode CompilerMode;
	EParserOutputMode ParserOutputMode;
	EParserMode ParserMode;
	bool SymbolTables;
	bool Optimize;
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
	virtual EExitCode GetExitCode() const;

	void Output(ostream &Stream) const;

private:
	string Message;
	CPosition Position;
};

class CScannerException : public CException
{
public:
	CScannerException(const string &AMessage, const CPosition &APosition);

	EExitCode GetExitCode() const;
};

class CParserException : public CException
{
public:
	CParserException(const string &AMessage, const CPosition &APosition);

	EExitCode GetExitCode() const;
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
class CCharConst;
class CStringConst;
class CVariable;
class CFunction;
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
	virtual void Visit(CCharConst &AStmt) = 0;
	virtual void Visit(CStringConst &AStmt) = 0;
	virtual void Visit(CVariable &AStmt) = 0;
	virtual void Visit(CFunction &AStmt) = 0;
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

namespace CharTraits
{
	bool IsWhitespace(char c);
	bool IsDigit(char c);
	bool IsHexDigit(char c);
	bool IsOctDigit(char c);
	bool IsValidIdentifierChar(char c, bool first = false);
	bool IsOperationChar(char c);
};

namespace TokenTraits
{
	bool IsCompoundAssignment(ETokenType t);
	bool IsAssignment(ETokenType t);
	bool IsRelational(ETokenType t);
	bool IsUnaryOp(ETokenType t);
	bool IsPostfix(ETokenType t);
	bool IsComparisonOperation(ETokenType t);
	bool IsTrivialOperation(ETokenType t);
};

namespace KeywordTraits
{
	bool IsKeyword(const string &s);
	bool IsTypeKeyword(const string &s);
	bool IsInternalType(const string &s);
};

#endif // _COMMON_H_
