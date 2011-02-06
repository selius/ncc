#ifndef _PARSER_H_
#define _PARSER_H_

#include "scanner.h"

class CExpressionVisitor;

class CExpression
{
public:
	CExpression();
	CExpression(const CToken &AToken);
	virtual ~CExpression();

	virtual void Accept(CExpressionVisitor &AVisitor) = 0;

	ETokenType GetType() const;
	string GetName() const;

	virtual bool IsLValue() const;

protected:
	ETokenType Type;
	string Name;

};

class CUnaryOp : public CExpression
{
public:
	CUnaryOp(const CToken &AToken, CExpression *AArgument = NULL);
	~CUnaryOp();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetArgument() const;
	void SetArgument(CExpression *AArgument);

	bool IsLValue() const;

private:
	CExpression *Argument;

};

class CBinaryOp : public CExpression
{
public:
	CBinaryOp(const CToken &AToken, CExpression *ALeft = NULL, CExpression *ARight = NULL);
	~CBinaryOp();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetLeft() const;
	CExpression* GetRight() const;
	void SetLeft(CExpression *ALeft);
	void SetRight(CExpression *ARight);

private:
	CExpression *Left;
	CExpression *Right;
};

class CConditionalOp : public CExpression
{
public:
	CConditionalOp(const CToken &AToken, CExpression *ACondition = NULL, CExpression *ATrueExpr = NULL, CExpression *AFalseExpr = NULL);
	~CConditionalOp();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetCondition() const;
	CExpression* GetTrueExpr() const;
	CExpression* GetFalseExpr() const;
	void SetCondition(CExpression *ACondition);
	void SetTrueExpr(CExpression *ATrueExpr);
	void SetFalseExpr(CExpression *AFalseExpr);

	bool IsLValue() const;

private:
	CExpression *Condition;
	CExpression *TrueExpr;
	CExpression *FalseExpr;

};

class CConst : public CExpression
{
public:
	CConst(const CToken &AToken);

private:
	

};

class CIntegerConst : public CConst
{
public:
	CIntegerConst(const CIntegerConstToken &AToken);

	void Accept(CExpressionVisitor &AVisitor);

	int GetValue() const;

private:
	int Value;

};

class CFloatConst : public CConst
{
public:
	CFloatConst(const CFloatConstToken &AToken);

	void Accept(CExpressionVisitor &AVisitor);

	double GetValue() const;

private:
	double Value;

};

class CSymbolConst : public CConst
{
public:
	CSymbolConst(const CSymbolConstToken &AToken);

	void Accept(CExpressionVisitor &AVisitor);

	char GetValue() const;

private:
	char Value;

};

class CStringConst : public CConst
{
public:
	CStringConst(const CToken &AToken);

	void Accept(CExpressionVisitor &AVisitor);

	string GetValue() const;

private:
	string Value;

};

class CVariable : public CExpression
{
public:
	CVariable(const CToken &AToken);

	void Accept(CExpressionVisitor &AVisitor);

	bool IsLValue() const;

};

class CPostfixOp : public CUnaryOp
{
public:
	CPostfixOp(const CToken &AToken, CExpression *AArgument = NULL);

	void Accept(CExpressionVisitor &AVisitor);
};

class CFunctionCall : public CExpression
{
public:
	typedef vector<CExpression *> ArgumentsContainer;
	typedef ArgumentsContainer::iterator ArgumentsIterator;

	CFunctionCall(CExpression *AFunction);
	~CFunctionCall();

	void Accept(CExpressionVisitor &AVisitor);

	ArgumentsIterator Begin();
	ArgumentsIterator End();

	void AddArgument(CExpression *AArgument);

private:
	CExpression *Function;
	ArgumentsContainer Arguments;

};

class CExpressionVisitor
{
public:
	virtual ~CExpressionVisitor();

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
};

class CExpressionLinearPrintVisitor : public CExpressionVisitor
{
public:
	CExpressionLinearPrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CConditionalOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);
	void Visit(CPostfixOp &AExpr);
	void Visit(CFunctionCall &AExpr);

private:
	ostream &Stream;

	static const char *LEFT_ENCLOSING;
	static const char *RIGHT_ENCLOSING;
	static const char *DELIMITER;

};

class CExpressionTreePrintVisitor : public CExpressionVisitor
{
public:
	CExpressionTreePrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CConditionalOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);
	void Visit(CPostfixOp &AExpr);
	void Visit(CFunctionCall &AExpr);

private:
	void PrintTreeDecoration();

	ostream &Stream;
	int Nesting;
	map<int, bool> LastChild;

};

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

};


#endif // _PARSER_H_
