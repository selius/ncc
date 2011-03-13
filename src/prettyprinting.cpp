#include "prettyprinting.h"

/******************************************************************************
 * CStatementLinearPrintVisitor
 ******************************************************************************/

CStatementLinearPrintVisitor::CStatementLinearPrintVisitor(ostream &AStream) : Stream(AStream)
{
}

void CStatementLinearPrintVisitor::Visit(CUnaryOp &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetArgument());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CBinaryOp &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetLeft());
	Stream << DELIMITER;
	TryVisit(AStmt.GetRight());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CConditionalOp &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetCondition());
	Stream << DELIMITER;
	TryVisit(AStmt.GetTrueExpr());
	Stream << DELIMITER;
	TryVisit(AStmt.GetFalseExpr());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIntegerConst &AStmt)
{
	Stream << AStmt.GetValue();
}

void CStatementLinearPrintVisitor::Visit(CFloatConst &AStmt)
{
	Stream << AStmt.GetValue();
}

void CStatementLinearPrintVisitor::Visit(CSymbolConst &AStmt)
{
	Stream << '\'' << AStmt.GetValue() << '\'';
}

void CStatementLinearPrintVisitor::Visit(CStringConst &AStmt)
{
	Stream << '"' << AStmt.GetValue() << '"';
}

void CStatementLinearPrintVisitor::Visit(CVariable &AStmt)
{
	Stream << AStmt.GetName();
}

void CStatementLinearPrintVisitor::Visit(CPostfixOp &AStmt)
{
	Stream << AStmt.GetName() << "(postfix)" << LEFT_ENCLOSING;
	TryVisit(AStmt.GetArgument());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CFunctionCall &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	for (CFunctionCall::ArgumentsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		TryVisit(*it);

		if (it != --AStmt.End()) {
			Stream << DELIMITER;
		}
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CStructAccess &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetStruct());
	Stream << DELIMITER;
	TryVisit(AStmt.GetField());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIndirectAccess &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetPointer());
	Stream << DELIMITER;
	TryVisit(AStmt.GetField());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CArrayAccess &AStmt)
{
	Visit(static_cast<CBinaryOp &>(AStmt));
}

void CStatementLinearPrintVisitor::Visit(CNullStatement &AStmt)
{
	Stream << AStmt.GetName();
}

void CStatementLinearPrintVisitor::Visit(CBlockStatement &AStmt)
{
	Stream << LEFT_ENCLOSING;
	for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		TryVisit(*it);

		if (it != --AStmt.End()) {
			Stream << DELIMITER;
		}
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIfStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetCondition());
	Stream << DELIMITER;
	TryVisit(AStmt.GetThenStatement());
	if (AStmt.GetElseStatement()) {
		Stream << DELIMITER;
		AStmt.GetElseStatement()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CForStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetInit());
	Stream << DELIMITER;
	TryVisit(AStmt.GetCondition());
	Stream << DELIMITER;
	TryVisit(AStmt.GetUpdate());
	Stream << DELIMITER;
	TryVisit(AStmt.GetBody());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CWhileStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetCondition());
	Stream << DELIMITER;
	TryVisit(AStmt.GetBody());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CDoStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetBody());
	Stream << DELIMITER;
	TryVisit(AStmt.GetCondition());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CLabel &AStmt)
{
	Stream << AStmt.GetName() << ":" << LEFT_ENCLOSING;
	TryVisit(AStmt.GetNext());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CCaseLabel &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetCaseExpression());
	Stream << DELIMITER;
	TryVisit(AStmt.GetNext());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CDefaultCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CStatementLinearPrintVisitor::Visit(CGotoStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING << AStmt.GetLabelName() << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CBreakStatement &AStmt)
{
	Stream << AStmt.GetName();
}

void CStatementLinearPrintVisitor::Visit(CContinueStatement &AStmt)
{
	Stream << AStmt.GetName();
}

void CStatementLinearPrintVisitor::Visit(CReturnStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetReturnExpression());
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CSwitchStatement &AStmt)
{
	Stream << AStmt.GetName() << LEFT_ENCLOSING;
	TryVisit(AStmt.GetTestExpression());
	Stream << DELIMITER;
	TryVisit(AStmt.GetBody());
	Stream << RIGHT_ENCLOSING;
}

const char *CStatementLinearPrintVisitor::LEFT_ENCLOSING = "{";
const char *CStatementLinearPrintVisitor::RIGHT_ENCLOSING = "}";
const char *CStatementLinearPrintVisitor::DELIMITER = ", ";

/******************************************************************************
 * CStatementTreePrintVisitor
 ******************************************************************************/

CStatementTreePrintVisitor::CStatementTreePrintVisitor(ostream &AStream) : Stream(AStream), Nesting(0)
{
	LastChild[0] = true;
}

void CStatementTreePrintVisitor::Visit(CUnaryOp &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetArgument());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CBinaryOp &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetLeft());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetRight());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CConditionalOp &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetCondition());
	TryVisit(AStmt.GetTrueExpr());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetFalseExpr());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIntegerConst &AStmt)
{
	PrintTreeDecoration();
	Stream << AStmt.GetValue() << endl;
}

void CStatementTreePrintVisitor::Visit(CFloatConst &AStmt)
{
	PrintTreeDecoration();
	Stream << AStmt.GetValue() << endl;
}

void CStatementTreePrintVisitor::Visit(CSymbolConst &AStmt)
{
	PrintTreeDecoration();
	Stream << '\'' << AStmt.GetValue() << '\'' << endl;
}

void CStatementTreePrintVisitor::Visit(CStringConst &AStmt)
{
	PrintTreeDecoration();
	Stream << '"' << AStmt.GetValue() << '"' << endl;
}

void CStatementTreePrintVisitor::Visit(CVariable &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);
}

void CStatementTreePrintVisitor::Visit(CPostfixOp &AStmt)
{
	PrintTreeDecoration();

	Stream << AStmt.GetName() << "(postfix)" << endl;

	Nesting++;
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetArgument());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CFunctionCall &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	for (CFunctionCall::ArgumentsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		if (it == --AStmt.End()) {
			LastChild[Nesting] = true;
		}

		TryVisit(*it);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CStructAccess &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetStruct());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetField());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIndirectAccess &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetPointer());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetField());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CArrayAccess &AStmt)
{
	Visit(static_cast<CBinaryOp &>(AStmt));
}

void CStatementTreePrintVisitor::Visit(CNullStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);
}

void CStatementTreePrintVisitor::Visit(CBlockStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		if (it == --AStmt.End()) {
			LastChild[Nesting] = true;
		}

		TryVisit(*it);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIfStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetCondition());

	LastChild[Nesting] = !AStmt.GetElseStatement();
	TryVisit(AStmt.GetThenStatement());

	LastChild[Nesting] = true;
	TryVisit(AStmt.GetElseStatement());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CForStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetInit());
	TryVisit(AStmt.GetCondition());
	TryVisit(AStmt.GetUpdate());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetBody());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CWhileStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetCondition());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetBody());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CDoStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetBody());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetCondition());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CLabel &AStmt)
{
	PrintTreeDecoration();
	Stream << AStmt.GetName() << ":" << endl;

	Nesting++;
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetNext());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CCaseLabel &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetCaseExpression());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetNext());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CDefaultCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CStatementTreePrintVisitor::Visit(CGotoStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	LastChild[Nesting] = true;
	PrintTreeDecoration(); 
	Stream << AStmt.GetLabelName() << endl;
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CBreakStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);
}

void CStatementTreePrintVisitor::Visit(CContinueStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);
}

void CStatementTreePrintVisitor::Visit(CReturnStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetReturnExpression());
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CSwitchStatement &AStmt)
{
	PrintTreeDecoration();
	PrintName(AStmt);

	Nesting++;
	TryVisit(AStmt.GetTestExpression());
	LastChild[Nesting] = true;
	TryVisit(AStmt.GetBody());
	Nesting--;
}

void CStatementTreePrintVisitor::PrintTreeDecoration()
{
	for (int i = 1; i < Nesting; i++) {
		if (LastChild[i]) {
			Stream << "   ";
		} else {
			Stream << "|  ";
		}
	}

	LastChild[Nesting + 1] = false;

	if (Nesting > 0) {
		if (LastChild[Nesting]) {
			Stream << "`- ";
		} else {
			Stream << "|- ";
		}
	}
}

void CStatementTreePrintVisitor::PrintName(CStatement &AStmt)
{
	Stream << AStmt.GetName() << endl;
}

