#ifndef _EXPRESSIONS_H_
#define _EXPRESSIONS_H_

#include "scanner.h"
#include "symbols.h"

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
	CBinaryOp();
	~CBinaryOp();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetLeft() const;
	CExpression* GetRight() const;
	void SetLeft(CExpression *ALeft);
	void SetRight(CExpression *ARight);

protected:
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
	CVariable(const CToken &AToken, CSymbol *ASymbol = NULL);

	void Accept(CExpressionVisitor &AVisitor);

	bool IsLValue() const;

	string GetName() const;
	void SetName(const string &AName);

	CSymbol* GetSymbol() const;

private:
	string Name;
	CSymbol *Symbol;

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

	CFunctionCall(CSymbol *AFunction);
	~CFunctionCall();

	void Accept(CExpressionVisitor &AVisitor);

	ArgumentsIterator Begin();
	ArgumentsIterator End();

	void AddArgument(CExpression *AArgument);

private:
	CFunctionSymbol *Function;
	ArgumentsContainer Arguments;

};

class CStructAccess : public CExpression
{
public:
	CStructAccess(const CToken &AToken, CExpression *AStruct = NULL, CVariable *AField = NULL);
	~CStructAccess();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetStruct() const;
	CVariable* GetField() const;
	void SetStruct(CExpression *AStruct);
	void SetField(CVariable *AField);

private:
	CExpression *Struct;
	CVariable *Field;
};

class CIndirectAccess : public CExpression
{
public:
	CIndirectAccess(const CToken &AToken, CExpression *APointer = NULL, CVariable *AField = NULL);
	~CIndirectAccess();

	void Accept(CExpressionVisitor &AVisitor);

	CExpression* GetPointer() const;
	CVariable* GetField() const;
	void SetPointer(CExpression *APointer);
	void SetField(CVariable *AField);

private:
	CExpression *Pointer;
	CVariable *Field;
};

class CArrayAccess : public CBinaryOp
{
public:
	CArrayAccess(CExpression *ALeft = NULL, CExpression *ARight = NULL);
	~CArrayAccess();

	void Accept(CExpressionVisitor &AVisitor);
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
	virtual void Visit(CStructAccess &AExpr) = 0;
	virtual void Visit(CIndirectAccess &AExpr) = 0;
	/*virtual void Visit(CArrayAccess &AExpr) = 0;*/
};

#endif // _EXPRESSIONS_H_
