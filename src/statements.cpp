#include "statements.h"

#include "expressions.h"

/******************************************************************************
 * CStatement
 ******************************************************************************/

CStatement::~CStatement()
{
}

string CStatement::GetName() const
{
	return Name;
}

/******************************************************************************
 * CNullStatement
 ******************************************************************************/

CNullStatement::CNullStatement()
{
	Name = "(null statement)";
}

void CNullStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CBlockStatement
 ******************************************************************************/

CBlockStatement::CBlockStatement()
{
	Name = "{ }";
}

CBlockStatement::~CBlockStatement()
{
	while (!Statements.empty()) {
		delete Statements.back();
		Statements.pop_back();
	}

	delete SymbolTable;
}

void CBlockStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CBlockStatement::StatementsIterator CBlockStatement::Begin()
{
	return Statements.begin();
}

CBlockStatement::StatementsIterator CBlockStatement::End()
{
	return Statements.end();
}

unsigned int CBlockStatement::GetStatementsCount() const
{
	return Statements.size();
}

void CBlockStatement::Add(CStatement *AStatement)
{
	Statements.push_back(AStatement);
}

CSymbolTable* CBlockStatement::GetSymbolTable() const
{
	return SymbolTable;
}

void CBlockStatement::SetSymbolTable(CSymbolTable *ASymbolTable)
{
	SymbolTable = ASymbolTable;
}

/******************************************************************************
 * CIfStatement
 ******************************************************************************/

CIfStatement::CIfStatement(CExpression *ACondition /*= NULL*/, CStatement *AThenStatement /*= NULL*/, CStatement *AElseStatement /*= NULL*/)
	: Condition(ACondition), ThenStatement(AThenStatement), ElseStatement(AElseStatement)
{
	Name = "if";
}

CIfStatement::~CIfStatement()
{
	delete Condition;
	delete ThenStatement;
	delete ElseStatement;
}

void CIfStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CIfStatement::GetCondition() const
{
	return Condition;
}

void CIfStatement::SetCondition(CExpression *ACondition)
{
	Condition = ACondition;
}

CStatement* CIfStatement::GetThenStatement() const
{
	return ThenStatement;
}

void CIfStatement::SetThenStatement(CStatement *AThenStatement)
{
	ThenStatement = AThenStatement;
}

CStatement* CIfStatement::GetElseStatement() const
{
	return ElseStatement;
}

void CIfStatement::SetElseStatement(CStatement *AElseStatement)
{
	ElseStatement = AElseStatement;
}

/******************************************************************************
 * CForStatement
 ******************************************************************************/

CForStatement::CForStatement(CExpression *AInit /*= NULL*/,  CExpression *ACondition /*= NULL*/, CExpression *AUpdate /*= NULL*/, CStatement *ABody /*= NULL*/)
	: Init(AInit), Condition(ACondition), Update(AUpdate), Body(ABody)
{
	Name = "for";
}

CForStatement::~CForStatement()
{
	delete Init;
	delete Condition;
	delete Update;
	delete Body;
}

void CForStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CForStatement::GetInit() const
{
	return Init;
}

void CForStatement::SetInit(CExpression *AInit)
{
	Init = AInit;
}

CExpression* CForStatement::GetCondition() const
{
	return Condition;
}

void CForStatement::SetCondition(CExpression *ACondition)
{
	Condition = ACondition;
}

CExpression* CForStatement::GetUpdate() const
{
	return Update;
}

void CForStatement::SetUpdate(CExpression *AUpdate)
{
	Update = AUpdate;
}

CStatement* CForStatement::GetBody() const
{
	return Body;
}

void CForStatement::SetBody(CStatement *ABody)
{
	Body = ABody;
}

/******************************************************************************
 * CSingleConditionLoopStatement
 ******************************************************************************/

CSingleConditionLoopStatement::CSingleConditionLoopStatement(CExpression *ACondition, CStatement *ABody) : Condition(ACondition), Body(ABody)
{
}

CSingleConditionLoopStatement::~CSingleConditionLoopStatement()
{
	delete Condition;
	delete Body;
}

CExpression* CSingleConditionLoopStatement::GetCondition() const
{
	return Condition;
}

void CSingleConditionLoopStatement::SetCondition(CExpression *ACondition)
{
	Condition = ACondition;
}

CStatement* CSingleConditionLoopStatement::GetBody() const
{
	return Body;
}

void CSingleConditionLoopStatement::SetBody(CStatement *ABody)
{
	Body = ABody;
}

/******************************************************************************
 * CWhileStatement
 ******************************************************************************/

CWhileStatement::CWhileStatement(CExpression *ACondition /*= NULL*/, CStatement *ABody /*= NULL*/) : CSingleConditionLoopStatement(ACondition, ABody)
{
	Name = "while";
}

void CWhileStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CDoStatement
 ******************************************************************************/

CDoStatement::CDoStatement(CExpression *ACondition /*= NULL*/, CStatement *ABody /*= NULL*/) : CSingleConditionLoopStatement(ACondition, ABody)
{
	Name = "do";
}

void CDoStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CLabel
 ******************************************************************************/

CLabel::CLabel(const string &AName, CStatement *ANext /*= NULL*/) : Next(ANext)
{
	Name = AName;
}

CLabel::~CLabel()
{
	delete Next;
}

void CLabel::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CStatement* CLabel::GetNext() const
{
	return Next;
}

void CLabel::SetNext(CStatement *ANext)
{
	Next = ANext;
}

/******************************************************************************
 * CCaseLabel
 ******************************************************************************/

CCaseLabel::CCaseLabel(CExpression *ACaseExpression /*= NULL*/, CStatement *ANext /*= NULL*/) : CLabel("case", ANext), CaseExpression(ACaseExpression)
{
}

CCaseLabel::~CCaseLabel()
{
	delete CaseExpression;
}

void CCaseLabel::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CCaseLabel::GetCaseExpression() const
{
	return CaseExpression;
}

void CCaseLabel::SetCaseExpression(CExpression *ACaseExpression)
{
	CaseExpression = ACaseExpression;
}

/******************************************************************************
 * CDefaultCaseLabel
 ******************************************************************************/

CDefaultCaseLabel::CDefaultCaseLabel(CStatement *ANext /*= NULL*/) : CLabel("default", ANext)
{
}

void CDefaultCaseLabel::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CGotoStatement
 ******************************************************************************/

CGotoStatement::CGotoStatement(const string &ALabelName) : LabelName(ALabelName)
{
	Name = "goto";
}

void CGotoStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

string CGotoStatement::GetLabelName() const
{
	return LabelName;
}

void CGotoStatement::SetLabelName(const string &ALabelName)
{
	LabelName = ALabelName;
}

/******************************************************************************
 * CBreakStatement
 ******************************************************************************/

CBreakStatement::CBreakStatement()
{
	Name = "break";
}

void CBreakStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CContinueStatement
 ******************************************************************************/

CContinueStatement::CContinueStatement()
{
	Name = "continue";
}

void CContinueStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CReturnStatement
 ******************************************************************************/

CReturnStatement::CReturnStatement(CExpression *AReturnExpression /*= NULL*/) : ReturnExpression(AReturnExpression)
{
	Name = "return";
}

CReturnStatement::~CReturnStatement()
{
	delete ReturnExpression;
}

void CReturnStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CReturnStatement::GetReturnExpression() const
{
	return ReturnExpression;
}

void CReturnStatement::SetReturnExpression(CExpression *AReturnExpression)
{
	ReturnExpression = AReturnExpression;
}

/******************************************************************************
 * CSwitchStatement
 ******************************************************************************/

CSwitchStatement::CSwitchStatement(CExpression *ATestExpression /*= NULL*/, CStatement *ABody /*= NULL*/) : TestExpression(ATestExpression), Body(ABody)
{
	Name = "switch";
}

CSwitchStatement::~CSwitchStatement()
{
	delete TestExpression;
	delete Body;
}

void CSwitchStatement::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CSwitchStatement::GetTestExpression() const
{
	return TestExpression;
}

void CSwitchStatement::SetTestExpression(CExpression *ATestExpression)
{
	TestExpression = ATestExpression;
}

CStatement* CSwitchStatement::GetBody() const
{
	return Body;
}

void CSwitchStatement::SetBody(CStatement *ABody)
{
	Body = ABody;
}

void CSwitchStatement::AddCase(CCaseLabel *ACase)
{
	if (ACase) {
		Cases[ACase->GetCaseExpression()] = ACase;
	}
}

bool CSwitchStatement::Exists(CCaseLabel *ACase)
{
	if (!ACase) {
		return false;
	}

	return Cases.count(ACase->GetCaseExpression());
}

CSwitchStatement::CasesIterator CSwitchStatement::Begin()
{
	return Cases.begin();
}

CSwitchStatement::CasesIterator CSwitchStatement::End()
{
	return Cases.end();
}

CDefaultCaseLabel* CSwitchStatement::GetDefaultCase() const
{
	return DefaultCase;
}

void CSwitchStatement::SetDefaultCase(CDefaultCaseLabel *ADefaultCase)
{
	DefaultCase = ADefaultCase;
}
