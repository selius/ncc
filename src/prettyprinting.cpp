#include "prettyprinting.h"

/******************************************************************************
 * CExpressionLinearPrintVisitor
 ******************************************************************************/

CExpressionLinearPrintVisitor::CExpressionLinearPrintVisitor(ostream &AStream) : Stream(AStream)
{
}

void CExpressionLinearPrintVisitor::Visit(CUnaryOp &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CExpressionLinearPrintVisitor::Visit(CBinaryOp &AExpr)
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

void CExpressionLinearPrintVisitor::Visit(CConditionalOp &AExpr)
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

void CExpressionLinearPrintVisitor::Visit(CIntegerConst &AExpr)
{
	Stream << AExpr.GetValue();
}

void CExpressionLinearPrintVisitor::Visit(CFloatConst &AExpr)
{
	Stream << AExpr.GetValue();
}

void CExpressionLinearPrintVisitor::Visit(CSymbolConst &AExpr)
{
	Stream << '\'' << AExpr.GetValue() << '\'';
}

void CExpressionLinearPrintVisitor::Visit(CStringConst &AExpr)
{
	Stream << '"' << AExpr.GetValue() << '"';
}

void CExpressionLinearPrintVisitor::Visit(CVariable &AExpr)
{
	Stream << AExpr.GetName();
}

void CExpressionLinearPrintVisitor::Visit(CPostfixOp &AExpr)
{
	Stream << AExpr.GetName() << "(postfix)" << LEFT_ENCLOSING;
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Stream << RIGHT_ENCLOSING;
}

void CExpressionLinearPrintVisitor::Visit(CFunctionCall &AExpr)
{
	Stream << AExpr.GetName() << LEFT_ENCLOSING;
	for (CFunctionCall::ArgumentsIterator it = AExpr.Begin(); it != AExpr.End(); ++it) {
		if (*it) {
			(*it)->Accept(*this); 
		}

		if (it != --AExpr.End()) {
			Stream << ", ";
		}
	}
	Stream << RIGHT_ENCLOSING;
}

void CExpressionLinearPrintVisitor::Visit(CStructAccess &AExpr)
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

void CExpressionLinearPrintVisitor::Visit(CIndirectAccess &AExpr)
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

/*void CExpressionLinearPrintVisitor::Visit(CArrayAccess &AExpr)
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
}*/

const char *CExpressionLinearPrintVisitor::LEFT_ENCLOSING = "{";
const char *CExpressionLinearPrintVisitor::RIGHT_ENCLOSING = "}";
const char *CExpressionLinearPrintVisitor::DELIMITER = ", ";

/******************************************************************************
 * CExpressionTreePrintVisitor
 ******************************************************************************/

CExpressionTreePrintVisitor::CExpressionTreePrintVisitor(ostream &AStream) : Stream(AStream), Nesting(0)
{
	LastChild[0] = true;
}

void CExpressionTreePrintVisitor::Visit(CUnaryOp &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CBinaryOp &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CConditionalOp &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CIntegerConst &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetValue() << endl;
}

void CExpressionTreePrintVisitor::Visit(CFloatConst &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetValue() << endl;
}

void CExpressionTreePrintVisitor::Visit(CSymbolConst &AExpr)
{
	PrintTreeDecoration();
	Stream << '\'' << AExpr.GetValue() << '\'' << endl;
}

void CExpressionTreePrintVisitor::Visit(CStringConst &AExpr)
{
	PrintTreeDecoration();
	Stream << '"' << AExpr.GetValue() << '"' << endl;
}

void CExpressionTreePrintVisitor::Visit(CVariable &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
}

void CExpressionTreePrintVisitor::Visit(CPostfixOp &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CFunctionCall &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CStructAccess &AExpr)
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

void CExpressionTreePrintVisitor::Visit(CIndirectAccess &AExpr)
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

void CExpressionTreePrintVisitor::PrintTreeDecoration()
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

