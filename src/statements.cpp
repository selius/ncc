/*
	ncc - Nartov C Compiler
	Copyright 2010-2011  Alexander Nartov

	ncc is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ncc is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ncc.  If not, see <http://www.gnu.org/licenses/>.
*/

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

bool CStatement::IsExpression() const
{
	return false;
}

bool CStatement::CanBeHoisted() const
{
	return false;
}

void CStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
}

void CStatement::GetUsedVariables(UsedContainer &Used)
{
}

void CStatement::TryGetAffected(CStatement *AStmt, AffectedContainer &Affected, bool Collect /*= false*/)
{
	if (AStmt) {
		AStmt->GetAffectedVariables(Affected, Collect);
	}
}

void CStatement::TryGetUsed(CStatement *AStmt, UsedContainer &Used)
{
	if (AStmt) {
		AStmt->GetUsedVariables(Used);
	}
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

void CBlockStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	for (StatementsIterator it = Begin(); it != End(); ++it) {
		(*it)->GetAffectedVariables(Affected);
	}
}

void CBlockStatement::GetUsedVariables(UsedContainer &Used)
{
	for (StatementsIterator it = Begin(); it != End(); ++it) {
		(*it)->GetUsedVariables(Used);
	}
}

CBlockStatement::StatementsIterator CBlockStatement::Begin()
{
	return Statements.begin();
}

CBlockStatement::StatementsIterator CBlockStatement::End()
{
	return Statements.end();
}

CBlockStatement::NestedBlocksIterator CBlockStatement::NestedBlocksBegin()
{
	return NestedBlocks.begin();
}

CBlockStatement::NestedBlocksIterator CBlockStatement::NestedBlocksEnd()
{
	return NestedBlocks.end();
}

void CBlockStatement::AddNestedBlock(CBlockStatement *ABlock)
{
	NestedBlocks.push_back(ABlock);
}

unsigned int CBlockStatement::GetStatementsCount() const
{
	return Statements.size();
}

void CBlockStatement::Add(CStatement *AStatement)
{
	Statements.push_back(AStatement);
}

void CBlockStatement::Insert(CBlockStatement::StatementsIterator APosition, CStatement *AStatement)
{
	Statements.insert(APosition, AStatement);
}

CBlockStatement::StatementsIterator CBlockStatement::Erase(CBlockStatement::StatementsIterator APosition)
{
	return Statements.erase(APosition);
}

CBlockStatement::StatementsIterator CBlockStatement::Erase(CBlockStatement::StatementsIterator AFirst, CBlockStatement::StatementsIterator ALast)
{
	return Statements.erase(AFirst, ALast);
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

void CIfStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Condition, Affected);
	TryGetAffected(ThenStatement, Affected);
	TryGetAffected(ElseStatement, Affected);
}

void CIfStatement::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Condition, Used);
	TryGetUsed(ThenStatement, Used);
	TryGetUsed(ElseStatement, Used);
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

void CForStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Init, Affected);
	TryGetAffected(Condition, Affected);
	TryGetAffected(Update, Affected);
	TryGetAffected(Body, Affected);
}

void CForStatement::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Init, Used);
	TryGetUsed(Condition, Used);
	TryGetUsed(Update, Used);
	TryGetUsed(Body, Used);
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

void CSingleConditionLoopStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Condition, Affected);
	TryGetAffected(Body, Affected);
}

void CSingleConditionLoopStatement::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Condition, Used);
	TryGetUsed(Body, Used);
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

void CLabel::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Next, Affected);
}

void CLabel::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Next, Used);
}

void CLabel::SetName(const string &AName)
{
	Name = AName;
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

CCaseLabel::CCaseLabel() : CLabel("case", NULL)
{
}

CCaseLabel::~CCaseLabel()
{
}

void CCaseLabel::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

int CCaseLabel::GetValue() const
{
	return Value;
}

void CCaseLabel::SetValue(int AValue)
{
	Value = AValue;
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

void CReturnStatement::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(ReturnExpression, Affected);
}

void CReturnStatement::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(ReturnExpression, Used);
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

CSwitchStatement::CSwitchStatement(CExpression *ATestExpression /*= NULL*/, CStatement *ABody /*= NULL*/) : TestExpression(ATestExpression), Body(ABody), DefaultCase(NULL)
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
		Cases[ACase->GetValue()] = ACase;
	}
}

bool CSwitchStatement::Exists(CCaseLabel *ACase)
{
	if (!ACase) {
		return false;
	}

	return Cases.count(ACase->GetValue());
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
