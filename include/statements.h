#ifndef _STATEMENTS_H_
#define _STATEMENTS_H_

#include "common.h"
//#include "expressions.h"

class CExpression;

class CStatement
{
public:
	virtual ~CStatement();

	virtual void Accept(CStatementVisitor &AVisitor) = 0;

	string GetName() const;

protected:
	string Name;

};

class CNullStatement : public CStatement
{
public:
	CNullStatement();

	void Accept(CStatementVisitor &AVisitor);
};

class CBlockStatement : public CStatement
{
public:
	typedef vector<CStatement *> StatementsContainer;
	typedef StatementsContainer::iterator StatementsIterator;

	CBlockStatement();

	void Accept(CStatementVisitor &AVisitor);

	StatementsIterator Begin();
	StatementsIterator End();

	void Add(CStatement *AStatement);

private:
	StatementsContainer Statements;

};

class CIfStatement : public CStatement
{
public:

private:
	CExpression *Condition;
	CStatement *IfStatement;
	CStatement *ElseStatement;
};

class CForStatement : public CStatement
{
public:

private:
	CExpression *Init;
	CExpression *Condition;
	CExpression *Update;
};

class CWhileStatement : public CStatement
{
public:

private:
	CExpression *Condition;

};

class CDoStatement : public CStatement
{
public:

private:
	CExpression *Condition;

};

class CGotoStatement : public CStatement	// ???
{

};

class CReturnStatement : public CStatement
{
public:

private:
	CExpression *ReturnExpression;

};

class CSwitchStatement : public CStatement
{
public:

private:
	CExpression *Condition;
	map<CExpression *, CStatement *> Cases;
	CStatement *DefaultCase;

};

#endif // _STATEMENTS_H_
