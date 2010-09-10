#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <istream>
#include <ostream>
#include <string>

#include "common.h"

using namespace std;

class CToken
{
public:
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
		TOKEN_TYPE_CONSTANT_SYMBOL,
		TOKEN_TYPE_CONSTANT_STRING,

		TOKEN_TYPE_OPERATION_PLUS,
		TOKEN_TYPE_OPERATION_MINUS,
		TOKEN_TYPE_OPERATION_ASTERISK,
		TOKEN_TYPE_OPERATION_SLASH,
		TOKEN_TYPE_OPERATION_BACKSLASH,
		TOKEN_TYPE_OPERATION_PERCENT,
		TOKEN_TYPE_OPERATION_AND,
		TOKEN_TYPE_OPERATION_ASSIGN,
		TOKEN_TYPE_OPERATION_PLUS_ASSIGN,
		TOKEN_TYPE_OPERATION_MINUS_ASSIGN,
		TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN,
		TOKEN_TYPE_OPERATION_SLASH_ASSIGN,
		TOKEN_TYPE_OPERATION_EQUAL,
		TOKEN_TYPE_OPERATION_NOT_EQUAL,
		TOKEN_TYPE_OPERATION_LESS_THAN,
		TOKEN_TYPE_OPERATION_GREATER_THAN,
		TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL,
		TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL,
		TOKEN_TYPE_OPERATION_LOGIC_AND,
		TOKEN_TYPE_OPERATION_LOGIC_OR,
		TOKEN_TYPE_OPERATION_LOGIC_NOT,
		TOKEN_TYPE_OPERATION_BITWISE_OR,
		TOKEN_TYPE_OPERATION_BITWISE_NOT,
		TOKEN_TYPE_OPERATION_DOT,
		TOKEN_TYPE_OPERATION_INDIRECT_ACCESS,

		TOKEN_TYPE_SEPARATOR_COMMA,
		TOKEN_TYPE_SEPARATOR_SEMICOLON,
		TOKEN_TYPE_SEPARATOR_COLON,

		TOKEN_TYPE_EOF,
	};

	CToken();
	CToken(ETokenType AType, const string &AValue, const CPosition &APosition);

	ETokenType GetType() const;
	string GetStringifiedType() const;
	string GetValue() const;
	CPosition GetPosition() const;

private:
	ETokenType Type;
	string Value;
	CPosition Position;

	friend class CScanner;
};

class CTraits
{
public:
	static bool IsWhitespace(char c);
	static bool IsValidIdentifierSymbol(char c, bool first = false);
	static bool IsOperationSymbol(char c);
	static bool IsKeyword(const string &s);
};

class CScanner
{
public:
	CScanner(istream &AInputStream);

	CToken& GetToken();
	CToken& Next();

	bool IsError() const;

private:
	CToken ScanIdentifier();
	CToken ScanOperation();
	CToken ScanSingleSymbol();
	void ScanComment();
	void SkipWhitespace();
	istream &InputStream;
	CToken LastToken;
	CPosition CurrentPosition;

	bool ErrorState;

};


#endif // _SCANNER_H_
