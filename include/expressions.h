#ifndef _EXPRESSIONS_H_
#define _EXPRESSIONS_H_

#include "common.h"
#include "scanner.h"
#include "symbols.h"
#include "statements.h"

class CExpression : public CStatement
{
public:
	CExpression();
	CExpression(const CToken &AToken);
	virtual ~CExpression();

	//virtual void Accept(CStatementVisitor &AVisitor) = 0;

	ETokenType GetType() const;
	CPosition GetPosition() const;

	virtual CTypeSymbol* GetResultType() const = 0;
	void SetResultType(CTypeSymbol *AResultType);

	virtual bool IsLValue() const;
	virtual bool IsConst() const;
	bool IsExpression() const;

	virtual void CheckTypes() const;

protected:
	ETokenType Type;
	CPosition Position;

	CTypeSymbol *ResultType;
};

class CUnaryOp : public CExpression
{
public:
	CUnaryOp(const CToken &AToken, CExpression *AArgument = NULL);
	~CUnaryOp();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetArgument() const;
	virtual void SetArgument(CExpression *AArgument);

	CTypeSymbol* GetResultType() const;

	bool IsLValue() const;
	bool IsConst() const;

	void CheckTypes() const;

protected:
	CExpression *Argument;

};

class CBinaryOp : public CExpression
{
public:
	CBinaryOp(const CToken &AToken, CExpression *ALeft = NULL, CExpression *ARight = NULL);
	CBinaryOp();
	~CBinaryOp();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetLeft() const;
	CExpression* GetRight() const;
	void SetLeft(CExpression *ALeft);
	void SetRight(CExpression *ARight);

	CTypeSymbol* GetResultType() const;
	CTypeSymbol* GetCommonRealType() const;

	bool IsConst() const;

	void CheckTypes() const;

protected:
	CExpression *Left;
	CExpression *Right;
};

class CConditionalOp : public CExpression
{
public:
	CConditionalOp(const CToken &AToken, CExpression *ACondition = NULL, CExpression *ATrueExpr = NULL, CExpression *AFalseExpr = NULL);
	~CConditionalOp();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetCondition() const;
	CExpression* GetTrueExpr() const;
	CExpression* GetFalseExpr() const;
	void SetCondition(CExpression *ACondition);
	void SetTrueExpr(CExpression *ATrueExpr);
	void SetFalseExpr(CExpression *AFalseExpr);

	CTypeSymbol* GetResultType() const;

	bool IsConst() const;

	void CheckTypes() const;

private:
	CExpression *Condition;
	CExpression *TrueExpr;
	CExpression *FalseExpr;

};

class CConst : public CExpression
{
public:
	CConst(const CToken &AToken, CTypeSymbol *AType);

	CTypeSymbol* GetResultType() const;

	bool IsConst() const;

private:
	CTypeSymbol *Type;
	

};

class CIntegerConst : public CConst
{
public:
	CIntegerConst(const CToken &AToken, CTypeSymbol *AType);

	void Accept(CStatementVisitor &AVisitor);

	int GetValue() const;

private:
	int Value;

};

class CFloatConst : public CConst
{
public:
	CFloatConst(const CToken &AToken, CTypeSymbol *AType);

	void Accept(CStatementVisitor &AVisitor);

	float GetValue() const;

private:
	float Value;

};

class CSymbolConst : public CConst
{
public:
	CSymbolConst(const CToken &AToken, CTypeSymbol *AType);

	void Accept(CStatementVisitor &AVisitor);

	char GetValue() const;

private:
	char Value;

};

class CStringConst : public CConst
{
public:
	CStringConst(const CToken &AToken, CTypeSymbol *AType);

	void Accept(CStatementVisitor &AVisitor);

	string GetValue() const;

private:
	string Value;

};

class CVariable : public CExpression
{
public:
	CVariable(const CToken &AToken, CSymbol *ASymbol = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CVariableSymbol* GetSymbol() const;
	void SetSymbol(CVariableSymbol *ASymbol);

	CTypeSymbol* GetResultType() const;

	bool IsLValue() const;

private:
	CSymbol *Symbol;

};

class CPostfixOp : public CUnaryOp
{
public:
	CPostfixOp(const CToken &AToken, CExpression *AArgument = NULL);

	void Accept(CStatementVisitor &AVisitor);

	void CheckTypes() const;

};

class CFunctionCall : public CExpression
{
public:
	typedef vector<CExpression *> ArgumentsContainer;
	typedef ArgumentsContainer::const_iterator ArgumentsIterator;
	typedef ArgumentsContainer::reverse_iterator ArgumentsReverseIterator;

	CFunctionCall(const CToken &AToken, CSymbol *AFunction);
	~CFunctionCall();

	void Accept(CStatementVisitor &AVisitor);

	CFunctionSymbol* GetFunction() const;
	void SetFunction(CFunctionSymbol *AFunction);

	ArgumentsIterator Begin() const;
	ArgumentsIterator End() const;
	ArgumentsReverseIterator RBegin();
	ArgumentsReverseIterator REnd();

	unsigned int GetArgumentsCount() const;

	void AddArgument(CExpression *AArgument);

	CTypeSymbol* GetResultType() const;

	bool IsExpression() const;

	void CheckTypes() const;

private:
	CFunctionSymbol *Function;
	ArgumentsContainer Arguments;

};

class CStructAccess : public CExpression
{
public:
	CStructAccess(const CToken &AToken, CExpression *AStruct = NULL, CVariable *AField = NULL);
	~CStructAccess();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetStruct() const;
	CVariable* GetField() const;
	void SetStruct(CExpression *AStruct);
	void SetField(CVariable *AField);

	CTypeSymbol* GetResultType() const;

	bool IsLValue() const;

	void CheckTypes() const;

private:
	CExpression *Struct;
	CVariable *Field;
	CStructSymbol *StructSymbol;
};

class CIndirectAccess : public CExpression
{
public:
	CIndirectAccess(const CToken &AToken, CExpression *APointer = NULL, CVariable *AField = NULL);
	~CIndirectAccess();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetPointer() const;
	CVariable* GetField() const;
	void SetPointer(CExpression *APointer);
	void SetField(CVariable *AField);

	CTypeSymbol* GetResultType() const;

	bool IsLValue() const;

	void CheckTypes() const;

private:
	CExpression *Pointer;
	CVariable *Field;
	CStructSymbol *StructSymbol;
};

class CArrayAccess : public CBinaryOp
{
public:
	CArrayAccess(const CToken &AToken, CExpression *ALeft = NULL, CExpression *ARight = NULL);
	~CArrayAccess();

	void Accept(CStatementVisitor &AVisitor);

	size_t GetElementSize() const;

	CTypeSymbol* GetResultType() const;

	bool IsLValue() const;

	void CheckTypes() const;
};

class CAddressOfOp : public CUnaryOp
{
public:
	CAddressOfOp(const CToken &AToken, CExpression *AArgument = NULL);
	~CAddressOfOp();

	void SetArgument(CExpression *AArgument);

	CTypeSymbol* GetResultType() const;

	void CheckTypes() const;

private:
	CTypeSymbol *ResultType;
};

#endif // _EXPRESSIONS_H_
