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
	CIfStatement(CExpression *ACondition = NULL, CStatement *AThenStatement = NULL, CStatement *AElseStatement = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetCondition() const;
	void SetCondition(CExpression *ACondition);

	CStatement* GetThenStatement() const;
	void SetThenStatement(CStatement *AThenStatement);

	CStatement* GetElseStatement() const;
	void SetElseStatement(CStatement *AElseStatement);

private:
	CExpression *Condition;
	CStatement *ThenStatement;
	CStatement *ElseStatement;
};

class CForStatement : public CStatement
{
public:
	CForStatement(CExpression *AInit = NULL,  CExpression *ACondition = NULL, CExpression *AUpdate = NULL, CStatement *ABody = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetInit() const;
	void SetInit(CExpression *AInit);

	CExpression* GetCondition() const;
	void SetCondition(CExpression *ACondition);

	CExpression* GetUpdate() const;
	void SetUpdate(CExpression *AUpdate);

	CStatement* GetBody() const;
	void SetBody(CStatement *ABody);

private:
	CExpression *Init;
	CExpression *Condition;
	CExpression *Update;
	CStatement *Body;
};

class CSingleConditionLoopStatement : public CStatement
{
public:
	CSingleConditionLoopStatement(CExpression *ACondition, CStatement *ABody);

	CExpression* GetCondition() const;
	void SetCondition(CExpression *ACondition);

	CStatement* GetBody() const;
	void SetBody(CStatement *ABody);

private:
	CExpression *Condition;
	CStatement *Body;
};

class CWhileStatement : public CSingleConditionLoopStatement
{
public:
	CWhileStatement(CExpression *ACondition = NULL, CStatement *ABody = NULL);
	void Accept(CStatementVisitor &AVisitor);
};

class CDoStatement : public CSingleConditionLoopStatement
{
public:
	CDoStatement(CExpression *ACondition = NULL, CStatement *ABody = NULL);
	void Accept(CStatementVisitor &AVisitor);
};

class CLabel : public CStatement
{
public:
	CLabel(const string &AName, CStatement *ANext = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CStatement* GetNext() const;
	void SetNext(CStatement *ANext);

protected:
	CStatement *Next;
};

class CCaseLabel : public CLabel
{
public:
	CCaseLabel(CExpression *ACaseExpression = NULL, CStatement *ANext = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetCaseExpression() const;
	void SetCaseExpression(CExpression *ACaseExpression);

private:
	CExpression *CaseExpression;
};

class CDefaultCaseLabel : public CLabel
{
public:
	CDefaultCaseLabel(CStatement *ANext = NULL);
	void Accept(CStatementVisitor &AVisitor);
};

class CGotoStatement : public CStatement
{
public:
	CGotoStatement(const string &ALabelName);

	void Accept(CStatementVisitor &AVisitor);

	string GetLabelName() const;
	void SetLabelName(const string &ALabelName);

private:
	string LabelName;
};

class CBreakStatement : public CStatement
{
public:
	CBreakStatement();
	void Accept(CStatementVisitor &AVisitor);
};

class CContinueStatement : public CStatement
{
public:
	CContinueStatement();
	void Accept(CStatementVisitor &AVisitor);
};

class CReturnStatement : public CStatement
{
public:
	CReturnStatement(CExpression *AReturnExpression = NULL);

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetReturnExpression() const;
	void SetReturnExpression(CExpression *AReturnExpression);

private:
	CExpression *ReturnExpression;
};

class CSwitchStatement : public CBlockStatement
{
public:
	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetTestExpression() const;
	void SetTestExpression(CExpression *ATestExpression);

	CStatement *GetDefaultCase() const;
	void SetDefaultCase(CStatement *ADefaultCase);

private:
	CExpression *TestExpression;
	map<CExpression *, CStatement *> Cases;	// how to address them and jump to them?..
	CStatement *DefaultCase;
};

#endif // _STATEMENTS_H_
