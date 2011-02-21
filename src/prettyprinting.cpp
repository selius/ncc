#include "prettyprinting.h"

/******************************************************************************
 * CStatementLinearPrintVisitor
 ******************************************************************************/

CStatementLinearPrintVisitor::CStatementLinearPrintVisitor(ostream &AStream) : Stream(AStream)
{
}

void CStatementLinearPrintVisitor::Visit(CUnaryOp &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CBinaryOp &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetLeft()) {
		AExpr.GetLeft()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetRight()) {
		AExpr.GetRight()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CConditionalOp &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetTrueExpr()) {
		AExpr.GetTrueExpr()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetFalseExpr()) {
		AExpr.GetFalseExpr()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIntegerConst &AExpr)
{
	Stream << AExpr.GetValue();
}

void CStatementLinearPrintVisitor::Visit(CFloatConst &AExpr)
{
	Stream << AExpr.GetValue();
}

void CStatementLinearPrintVisitor::Visit(CSymbolConst &AExpr)
{
	Stream << '\'' << AExpr.GetValue() << '\'';
}

void CStatementLinearPrintVisitor::Visit(CStringConst &AExpr)
{
	Stream << '"' << AExpr.GetValue() << '"';
}

void CStatementLinearPrintVisitor::Visit(CVariable &AExpr)
{
	Stream << AExpr.GetName();
}

void CStatementLinearPrintVisitor::Visit(CPostfixOp &AExpr)
{
	Stream << AExpr.GetName() << "(postfix)" << LEFT_ENCLOSING;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CFunctionCall &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	for (CFunctionCall::ArgumentsIterator it = AExpr.Begin(); it != AExpr.End(); ++it) {
		if (*it) {
			(*it)->Accept(*this); 
		}

		if (it != --AExpr.End()) {
			Stream << DELIMITER;
		}
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CStructAccess &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetStruct()) {
		AExpr.GetStruct()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetField()) {
		AExpr.GetField()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIndirectAccess &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetPointer()) {
		AExpr.GetPointer()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetField()) {
		AExpr.GetField()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CArrayAccess &AExpr)
{
	Visit(static_cast<CBinaryOp &>(AExpr));
	/*Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetLeft()) {
		AExpr.GetLeft()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetRight()) {
		AExpr.GetRight()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;*/
}

void CStatementLinearPrintVisitor::Visit(CNullStatement &AExpr)
{
	Stream << AExpr.GetName();
}

void CStatementLinearPrintVisitor::Visit(CBlockStatement &AExpr)
{
	Stream << LEFT_ENCLOSING;
	for (CBlockStatement::StatementsIterator it = AExpr.Begin(); it != AExpr.End(); ++it) {
		if (*it) {
			(*it)->Accept(*this); 
		}

		if (it != --AExpr.End()) {
			Stream << DELIMITER;
		}
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CIfStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetThenStatement()) {
		AExpr.GetThenStatement()->Accept(*this);
	}
	if (AExpr.GetElseStatement()) {
		Stream << DELIMITER;
		AExpr.GetElseStatement()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CForStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetInit()) {
		AExpr.GetInit()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetUpdate()) {
		AExpr.GetUpdate()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CWhileStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CDoStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CLabel &AExpr)
{
	Stream << AExpr.GetName() << ":" << LEFT_ENCLOSING;
	if (AExpr.GetNext()) {
		AExpr.GetNext()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CCaseLabel &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetCaseExpression()) {
		AExpr.GetCaseExpression()->Accept(*this);
	}
	Stream << DELIMITER;
	if (AExpr.GetNext()) {
		AExpr.GetNext()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CDefaultCaseLabel &AExpr)
{
	Visit(static_cast<CLabel &>(AExpr));
}

void CStatementLinearPrintVisitor::Visit(CGotoStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING << AExpr.GetLabelName() << RIGHT_ENCLOSING;
}

void CStatementLinearPrintVisitor::Visit(CBreakStatement &AExpr)
{
	Stream << AExpr.GetName();
}

void CStatementLinearPrintVisitor::Visit(CContinueStatement &AExpr)
{
	Stream << AExpr.GetName();
}

void CStatementLinearPrintVisitor::Visit(CReturnStatement &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetReturnExpression()) {
		AExpr.GetReturnExpression()->Accept(*this);
	}
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

void CStatementTreePrintVisitor::Visit(CUnaryOp &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	LastChild[Nesting] = true;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CBinaryOp &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetLeft()) {
		AExpr.GetLeft()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetRight()) {
		AExpr.GetRight()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CConditionalOp &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	if (AExpr.GetTrueExpr()) {
		AExpr.GetTrueExpr()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetFalseExpr()) {
		AExpr.GetFalseExpr()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIntegerConst &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetValue() << endl;
}

void CStatementTreePrintVisitor::Visit(CFloatConst &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetValue() << endl;
}

void CStatementTreePrintVisitor::Visit(CSymbolConst &AExpr)
{
	PrintTreeDecoration();
	Stream << '\'' << AExpr.GetValue() << '\'' << endl;
}

void CStatementTreePrintVisitor::Visit(CStringConst &AExpr)
{
	PrintTreeDecoration();
	Stream << '"' << AExpr.GetValue() << '"' << endl;
}

void CStatementTreePrintVisitor::Visit(CVariable &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
}

void CStatementTreePrintVisitor::Visit(CPostfixOp &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << "(postfix)" << endl;

	Nesting++;
	LastChild[Nesting] = true;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CFunctionCall &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	for (CFunctionCall::ArgumentsIterator it = AExpr.Begin(); it != AExpr.End(); ++it) {
		if (it == --AExpr.End()) {
			LastChild[Nesting] = true;
		}
		if (*it) {
			(*it)->Accept(*this);
		}
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CStructAccess &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetStruct()) {
		AExpr.GetStruct()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetField()) {
		AExpr.GetField()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIndirectAccess &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetPointer()) {
		AExpr.GetPointer()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetField()) {
		AExpr.GetField()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CArrayAccess &AExpr)
{
	Visit(static_cast<CBinaryOp &>(AExpr));
}

void CStatementTreePrintVisitor::Visit(CNullStatement &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
}

void CStatementTreePrintVisitor::Visit(CBlockStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	for (CBlockStatement::StatementsIterator it = AExpr.Begin(); it != AExpr.End(); ++it) {
		if (it == --AExpr.End()) {
			LastChild[Nesting] = true;
		}
		if (*it) {
			(*it)->Accept(*this);
		}
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CIfStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}

	LastChild[Nesting] = !AExpr.GetElseStatement();
	if (AExpr.GetThenStatement()) {
		AExpr.GetThenStatement()->Accept(*this);
	}

	LastChild[Nesting] = true;
	if (AExpr.GetElseStatement()) {
		AExpr.GetElseStatement()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CForStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetInit()) {
		AExpr.GetInit()->Accept(*this);
	}
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	if (AExpr.GetUpdate()) {
		AExpr.GetUpdate()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CWhileStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CDoStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetBody()) {
		AExpr.GetBody()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetCondition()) {
		AExpr.GetCondition()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CLabel &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << ":" << endl;

	Nesting++;
	LastChild[Nesting] = true;
	if (AExpr.GetNext()) {
		AExpr.GetNext()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CCaseLabel &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	if (AExpr.GetCaseExpression()) {
		AExpr.GetCaseExpression()->Accept(*this);
	}
	LastChild[Nesting] = true;
	if (AExpr.GetNext()) {
		AExpr.GetNext()->Accept(*this);
	}
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CDefaultCaseLabel &AExpr)
{
	Visit(static_cast<CLabel &>(AExpr));
}

void CStatementTreePrintVisitor::Visit(CGotoStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	LastChild[Nesting] = true;
	PrintTreeDecoration(); 
	Stream << AExpr.GetLabelName() << endl;
	Nesting--;
}

void CStatementTreePrintVisitor::Visit(CBreakStatement &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
}

void CStatementTreePrintVisitor::Visit(CContinueStatement &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
}

void CStatementTreePrintVisitor::Visit(CReturnStatement &AExpr)
{
	PrintTreeDecoration();

	Stream << AExpr.GetName() << endl;

	Nesting++;
	LastChild[Nesting] = true;
	if (AExpr.GetReturnExpression()) {
		AExpr.GetReturnExpression()->Accept(*this);
	}
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

