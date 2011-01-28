#include "parser.h"

/******************************************************************************
 * CExpression
 ******************************************************************************/

CExpression::CExpression(const CToken &AToken)
{
	Type = AToken.GetType();
	Name = AToken.GetText();
}

CExpression::~CExpression()
{
}

ETokenType CExpression::GetType() const
{
	return Type;
}

string CExpression::GetName() const
{
	return Name;
}

/******************************************************************************
 * CUnaryOp
 ******************************************************************************/

CUnaryOp::CUnaryOp(const CToken &AToken, CExpression *AArgument /*= NULL*/) : CExpression(AToken), Argument(AArgument)
{
}

CUnaryOp::~CUnaryOp()
{
	delete Argument;
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

/******************************************************************************
 * CBinaryOp
 ******************************************************************************/

CBinaryOp::CBinaryOp(const CToken &AToken, CExpression *ALeft /*= NULL*/, CExpression *ARight /*= NULL*/) : CExpression(AToken), Left(ALeft), Right(ARight)
{
}

CBinaryOp::~CBinaryOp()
{
	delete Left;
	delete Right;
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

/******************************************************************************
 * CConst
 ******************************************************************************/

CConst::CConst(const CToken &AToken) : CExpression(AToken)
{
}

/******************************************************************************
 * CIntegerConst
 ******************************************************************************/

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

/******************************************************************************
 * CFloatConst
 ******************************************************************************/

CFloatConst::CFloatConst(const CFloatConstToken &AToken) : CConst(AToken)
{
	Value = AToken.GetFloatValue();
}

void CFloatConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

double CFloatConst::GetValue() const
{
	return Value;
}

/******************************************************************************
 * CSymbolConst
 ******************************************************************************/

void CSymbolConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

char CSymbolConst::GetValue() const
{
	return Value;
}

/******************************************************************************
 * CStringConst
 ******************************************************************************/

void CStringConst::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

string CStringConst::GetValue() const
{
	return Value;
}

/******************************************************************************
 * CVariable
 ******************************************************************************/

CVariable::CVariable(const CToken &AToken) : CExpression(AToken)
{
}

void CVariable::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CExpressionVisitor
 ******************************************************************************/

CExpressionVisitor::~CExpressionVisitor()
{

}

/******************************************************************************
 * CExpressionLinearPrintVisitor
 ******************************************************************************/

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

/******************************************************************************
 * CParser
 ******************************************************************************/

CParser::CParser(CScanner &AScanner) : Scanner(AScanner)
{
	AdvanceOneToken();
}

CExpression* CParser::ParseSimpleExpression()
{
	CExpression *Expr = ParseTerm();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_PLUS || Token->GetType() == TOKEN_TYPE_OPERATION_MINUS) {
		Op = new CBinaryOp(*Token);

		AdvanceOneToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseTerm());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseTerm()
{
	CExpression *Expr = ParseFactor();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK || Token->GetType() == TOKEN_TYPE_OPERATION_SLASH) {
		Op = new CBinaryOp(*Token);

		AdvanceOneToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseFactor());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseFactor()
{
	if (Token->GetType() == TOKEN_TYPE_OPERATION_MINUS || Token->GetType() == TOKEN_TYPE_OPERATION_PLUS) {
		CUnaryOp *Op = new CUnaryOp(*Token);
		AdvanceOneToken();
		Op->SetArgument(ParseFactor());
		return Op;
	}

	CExpression *Expr;

	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		AdvanceOneToken();
		Expr = ParseSimpleExpression();
		if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
		}
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_INTEGER) {
		Expr = new CIntegerConst(*dynamic_cast<const CIntegerConstToken *>(Token));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_FLOAT) {
		Expr = new CFloatConst(*dynamic_cast<const CFloatConstToken *>(Token));
	} else if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		Expr = new CVariable(*Token);
	} else {
		throw CException("expected primary-expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	AdvanceOneToken();

	return Expr;
}

void CParser::AdvanceOneToken()
{
	Token = Scanner.Next();
}
