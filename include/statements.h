#ifndef _STATEMENTS_H_
#define _STATEMENTS_H_

#include "common.h"
#include "symbols.h"

class CExpression;

class CStatement
{
public:
	typedef map<CVariableSymbol *, int> AffectedContainer;
	typedef map<CVariableSymbol *, int> UsedContainer;

	virtual ~CStatement();

	virtual void Accept(CStatementVisitor &AVisitor) = 0;

	string GetName() const;

	virtual bool IsExpression() const;

	virtual bool CanBeHoisted() const;
	virtual void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	virtual void GetUsedVariables(UsedContainer &Used);

protected:
	void TryGetAffected(CStatement *AStmt, AffectedContainer &Affected, bool Collect = false);
	void TryGetUsed(CStatement *AStmt, UsedContainer &Used);

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
	typedef list<CStatement *> StatementsContainer;
	typedef StatementsContainer::iterator StatementsIterator;
	
	typedef vector<CBlockStatement *> NestedBlocksContainer;
	typedef NestedBlocksContainer::iterator NestedBlocksIterator;

	CBlockStatement();
	~CBlockStatement();

	void Accept(CStatementVisitor &AVisitor);

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

	StatementsIterator Begin();
	StatementsIterator End();

	NestedBlocksIterator NestedBlocksBegin();
	NestedBlocksIterator NestedBlocksEnd();

	void AddNestedBlock(CBlockStatement *ABlock);

	unsigned int GetStatementsCount() const;

	void Add(CStatement *AStatement);

	void Insert(StatementsIterator APosition, CStatement *AStatement);

	StatementsIterator Erase(StatementsIterator APosition);
	StatementsIterator Erase(StatementsIterator AFirst, StatementsIterator ALast);

	CSymbolTable* GetSymbolTable() const;
	void SetSymbolTable(CSymbolTable *ASymbolTable);

private:
	StatementsContainer Statements;
	NestedBlocksContainer NestedBlocks;
	CSymbolTable *SymbolTable;
};

class CIfStatement : public CStatement
{
public:
	CIfStatement(CExpression *ACondition = NULL, CStatement *AThenStatement = NULL, CStatement *AElseStatement = NULL);
	~CIfStatement();

	void Accept(CStatementVisitor &AVisitor);

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

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
	~CForStatement();

	void Accept(CStatementVisitor &AVisitor);

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

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
	~CSingleConditionLoopStatement();

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

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
	~CLabel();

	void Accept(CStatementVisitor &AVisitor);

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

	void SetName(const string &AName);

	CStatement* GetNext() const;
	void SetNext(CStatement *ANext);

protected:
	CStatement *Next;
};

class CCaseLabel : public CLabel
{
public:
	CCaseLabel();
	~CCaseLabel();

	void Accept(CStatementVisitor &AVisitor);

	int GetValue() const;
	void SetValue(int AValue);

private:
	int Value;
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
	~CReturnStatement();

	void Accept(CStatementVisitor &AVisitor);

	void GetAffectedVariables(AffectedContainer &Affected, bool Collect = false);
	void GetUsedVariables(UsedContainer &Used);

	CExpression* GetReturnExpression() const;
	void SetReturnExpression(CExpression *AReturnExpression);

private:
	CExpression *ReturnExpression;
};

class CSwitchStatement : public CBlockStatement
{
public:
	typedef map<int, CCaseLabel *> CasesContainer;
	typedef CasesContainer::iterator CasesIterator;

	CSwitchStatement(CExpression *ATestExpression = NULL, CStatement *ABody = NULL);
	~CSwitchStatement();

	void Accept(CStatementVisitor &AVisitor);

	CExpression* GetTestExpression() const;
	void SetTestExpression(CExpression *ATestExpression);

	CStatement* GetBody() const;
	void SetBody(CStatement *ABody);

	void AddCase(CCaseLabel *ACase);

	bool Exists(CCaseLabel *ACase);

	CasesIterator Begin();
	CasesIterator End();

	CDefaultCaseLabel* GetDefaultCase() const;
	void SetDefaultCase(CDefaultCaseLabel *ADefaultCase);

private:
	CExpression *TestExpression;
	CStatement *Body;
	CasesContainer Cases;
	CDefaultCaseLabel *DefaultCase;
};

#endif // _STATEMENTS_H_
