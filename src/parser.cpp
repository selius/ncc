#include "parser.h"

CExpression::CExpression(const CToken &AToken)
{
	Type = AToken.GetType();
	Name = AToken.GetText();
}

ETokenType CExpression::GetType() const
{
	return Type;
}

string CExpression::GetName() const
{
	return Name;
}

CUnaryOp::CUnaryOp(const CToken &AToken) : CExpression(AToken)
{
}

void CUnaryOp::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CUnaryOp::GetArgument() const
{
	return Argument;
}

void CUnaryOp::SetArgument(CExpression *AArgument)
{
	Argument = AArgument;
}

CBinaryOp::CBinaryOp(const CToken &AToken) : CExpression(AToken)
{
}

void CBinaryOp::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CBinaryOp::GetLeft() const
{
	return Left;
}

CExpression* CBinaryOp::GetRight() const
{
	return Right;
}

void CBinaryOp::SetLeft(CExpression *ALeft)
{
	Left = ALeft;
}

void CBinaryOp::SetRight(CExpression *ARight)
{
	Right = ARight;
}

CConst::CConst(const CToken &AToken) : CExpression(AToken)
{
}

CIntegerConst::CIntegerConst(const CIntegerConstToken &AToken) : CConst(AToken)
{
	Value = AToken.GetIntegerValue();
}

void CIntegerConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

int CIntegerConst::GetValue() const
{
	return Value;
}

void CFloatConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

double CFloatConst::GetValue() const
{
	return Value;
}

void CSymbolConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

char CSymbolConst::GetValue() const
{
	return Value;
}

void CStringConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

string CStringConst::GetValue() const
{
	return Value;
}

CVariable::CVariable(const CToken &AToken) : CExpression(AToken)
{
}

void CVariable::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpressionLinearPrintVisitor::CExpressionLinearPrintVisitor(ostream &AStream) : Stream(AStream)
{
}

void CExpressionLinearPrintVisitor::Visit(CUnaryOp &AExpr)
{
	Stream << AExpr.GetName() << "<";
	if (AExpr.GetArgument()) {
		AExpr.GetArgument()->Accept(*this);
	}
	Stream << ">";
}

void CExpressionLinearPrintVisitor::Visit(CBinaryOp &AExpr)
{
	Stream << AExpr.GetName() << "<";
	if (AExpr.GetLeft()) {
		AExpr.GetLeft()->Accept(*this);
	}
	Stream << ", ";
	if (AExpr.GetRight()) {
		AExpr.GetRight()->Accept(*this);
	}
	Stream << ">";
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
	Stream << AExpr.GetValue();
}

void CExpressionLinearPrintVisitor::Visit(CStringConst &AExpr)
{
	Stream << AExpr.GetValue();
}

void CExpressionLinearPrintVisitor::Visit(CVariable &AExpr)
{
	Stream << AExpr.GetName();
}

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
	Stream << AExpr.GetValue() << endl;
}

void CExpressionTreePrintVisitor::Visit(CStringConst &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetValue() << endl;
}

void CExpressionTreePrintVisitor::Visit(CVariable &AExpr)
{
	PrintTreeDecoration();
	Stream << AExpr.GetName() << endl;
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
