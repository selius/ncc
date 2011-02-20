#ifndef _PARSER_H_
#define _PARSER_H_

#include "scanner.h"
#include "symbols.h"
#include "expressions.h"

class CTokenStream
{
public:
	CTokenStream(CScanner &AScanner);
	~CTokenStream();

	const CToken* Next();
	const CToken* GetToken();
	const CToken* Previous();

private:
	CScanner &Scanner;
	deque<CToken *> Buffer;
	deque<CToken *>::iterator Current;
	static const size_t TOKEN_STREAM_SIZE = 5;
};

class CParser
{
public:
	CParser(CScanner &AScanner);

	CStatement* ParseStatement();

	CExpression* ParseExpression();

private:
	CExpression* ParseAssignment();

	CExpression* ParseConditional();

	CExpression* ParseLogicalOr();
	CExpression* ParseLogicalAnd();

	CExpression* ParseBitwiseOr();
	CExpression* ParseBitwiseXor();
	CExpression* ParseBitwiseAnd();

	CExpression* ParseEqualityExpression();
	CExpression* ParseRelationalExpression();

	CExpression* ParseShiftExpression();
	CExpression* ParseAdditiveExpression();
	CExpression* ParseMultiplicativeExpression();

	CExpression* ParseCastExpression();
	CExpression* ParseUnaryExpression();
	CExpression* ParsePostfixExpression();

	CExpression* ParsePrimaryExpression();

	void NextToken();
	void PreviousToken();

	CTokenStream TokenStream;

	const CToken *Token;

	CSymbolTableStack SymbolTableStack;

};


#endif // _PARSER_H_
