#include "statements.h"

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

void CBlockStatement::Add(CStatement *AStatement)
{
	Statements.push_back(AStatement);
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
