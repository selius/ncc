#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "common.h"

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
	TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN,
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


class CToken
{
public:
	CToken(ETokenType AType, const string &AText, const CPosition &APosition);
	virtual ~CToken();

	virtual CToken* Clone() const;

	ETokenType GetType() const;
	string GetStringifiedType() const;
	string GetText() const;
	CPosition GetPosition() const;

	virtual int GetIntegerValue() const;
	virtual float GetFloatValue() const;
	virtual char GetSymbolValue() const;

protected:
	ETokenType Type;
	string Text;
	CPosition Position;

	friend class CScanner;
};

class CIntegerConstToken : public CToken
{
public:
	CIntegerConstToken(const string &AText, const CPosition &APosition);

	CIntegerConstToken* Clone() const;

	int GetIntegerValue() const;

protected:
	int Value;

};

class CFloatConstToken : public CToken
{
public:
	CFloatConstToken(const string &AText, const CPosition &APosition);

	CFloatConstToken* Clone() const;

	float GetFloatValue() const;

protected:
	float Value;

};

class CSymbolConstToken : public CToken
{
public:
	CSymbolConstToken(const string &AText, const CPosition &APosition);

	CSymbolConstToken* Clone() const;

	char GetSymbolValue() const;

protected:
	char Value;

};

class CTraits
{
public:
	static bool IsWhitespace(char c);
	static bool IsDigit(char c);
	static bool IsHexDigit(char c);
	static bool IsOctDigit(char c);
	static bool IsValidNumericalConstantSymbol(char c);
	static bool IsValidIdentifierSymbol(char c, bool first = false);
	static bool IsOperationSymbol(char c);
	static bool IsKeyword(const string &s);
	static bool IsComparisonOperation(ETokenType t);
	static bool IsTrivialOperation(ETokenType t);
	static bool IsCompoundAssignment(ETokenType t);
};

class CScanner
{
public:
	CScanner(istream &AInputStream);
	~CScanner();

	const CToken* GetToken();
	const CToken* Next();

	static map<ETokenType, string> TokenTypesNames;

private:
	CToken* ScanIdentifier();
	CToken* ScanOperation();
	CToken* ScanSingleSymbol();
	CToken* ScanStringConstant();
	CToken* ScanSymbolConstant();
	CToken* ScanNumericalConstant();

	bool TryScanNumericalConstant();
	string ScanHexadecimalInteger();
	string ScanOctalInteger();
	string ScanIntegerPart();
	string ScanFractionalPart();
	string ScanExponentPart();
	string ScanFloatSuffix();
	string ScanIntegerSuffix();

	char ProcessEscapeSequence();

	bool SkipComment();
	bool SkipWhitespace();
	void SkipWhitespaceAndComments();

	char AdvanceOneSymbol();

	istream &InputStream;
	CToken *LastToken;
	CPosition CurrentPosition;
};


#endif // _SCANNER_H_
