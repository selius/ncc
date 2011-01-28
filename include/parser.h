#ifndef _PARSER_H_
#define _PARSER_H_

#include "scanner.h"

class CExpressionVisitor;

class CExpression
{
public:
	CExpression(const CToken &AToken);
	virtual ~CExpression();

	virtual void Accept(CExpressionVisitor &AVisitor) = 0;

	ETokenType GetType() const;
	string GetName() const;

private:
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
	void Accept(CExpressionVisitor &AVisitor);

	char GetValue() const;

private:
	char Value;

};

class CStringConst : public CConst
{
public:
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

};

class CExpressionVisitor
{
public:
	virtual ~CExpressionVisitor();

	virtual void Visit(CUnaryOp &AExpr) = 0;
	virtual void Visit(CBinaryOp &AExpr) = 0;
	virtual void Visit(CIntegerConst &AExpr) = 0;
	virtual void Visit(CFloatConst &AExpr) = 0;
	virtual void Visit(CSymbolConst &AExpr) = 0;
	virtual void Visit(CStringConst &AExpr) = 0;
	virtual void Visit(CVariable &AExpr) = 0;
};

class CExpressionLinearPrintVisitor : public CExpressionVisitor
{
public:
	CExpressionLinearPrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);

private:
	ostream &Stream;

};

class CExpressionTreePrintVisitor : public CExpressionVisitor
{
public:
	CExpressionTreePrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);

private:
	void PrintTreeDecoration();

	ostream &Stream;
	int Nesting;
	map<int, bool> LastChild;

};


class CParser
{
public:
	CParser(CScanner &AScanner);

	CExpression* ParseSimpleExpression();

private:
	CExpression* ParseTerm();
	CExpression* ParseFactor();

	void AdvanceOneToken();

	CScanner &Scanner;

	const CToken *Token;

};


#endif // _PARSER_H_
