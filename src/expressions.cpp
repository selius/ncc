#include "expressions.h"

/******************************************************************************
 * CExpression
 ******************************************************************************/

CExpression::CExpression() : Type(TOKEN_TYPE_INVALID)
{
}

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

bool CExpression::IsLValue() const
{
	return false;
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

void CUnaryOp::Accept(CStatementVisitor &AVisitor)
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

bool CUnaryOp::IsLValue() const
{
	return (Type == TOKEN_TYPE_OPERATION_ASTERISK);
}

/******************************************************************************
 * CBinaryOp
 ******************************************************************************/

CBinaryOp::CBinaryOp(const CToken &AToken, CExpression *ALeft /*= NULL*/, CExpression *ARight /*= NULL*/) : CExpression(AToken), Left(ALeft), Right(ARight)
{
}

CBinaryOp::CBinaryOp() : Left(NULL), Right(NULL)
{
}

CBinaryOp::~CBinaryOp()
{
	delete Left;
	delete Right;
}

void CBinaryOp::Accept(CStatementVisitor &AVisitor)
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
 * CConditionalOp
 ******************************************************************************/

CConditionalOp::CConditionalOp(const CToken &AToken, CExpression *ACondition /*= NULL*/, CExpression *ATrueExpr /*= NULL*/, CExpression *AFalseExpr /*= NULL*/)
	: CExpression(AToken), Condition(ACondition), TrueExpr(ATrueExpr), FalseExpr(AFalseExpr)
{
}

CConditionalOp::~CConditionalOp()
{
	delete Condition;
	delete TrueExpr;
	delete FalseExpr;
}

void CConditionalOp::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CConditionalOp::GetCondition() const
{
	return Condition;
}

CExpression* CConditionalOp::GetTrueExpr() const
{
	return TrueExpr;
}

CExpression* CConditionalOp::GetFalseExpr() const
{
	return FalseExpr;
}

void CConditionalOp::SetCondition(CExpression *ACondition)
{
	Condition = ACondition;
}

void CConditionalOp::SetTrueExpr(CExpression *ATrueExpr)
{
	TrueExpr = ATrueExpr;
}

void CConditionalOp::SetFalseExpr(CExpression *AFalseExpr)
{
	FalseExpr = AFalseExpr;
}

bool CConditionalOp::IsLValue() const
{
	return true;
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

void CIntegerConst::Accept(CStatementVisitor &AVisitor)
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

void CFloatConst::Accept(CStatementVisitor &AVisitor)
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

CSymbolConst::CSymbolConst(const CSymbolConstToken &AToken) : CConst(AToken)
{
	Value = AToken.GetSymbolValue();
}

void CSymbolConst::Accept(CStatementVisitor &AVisitor)
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

CStringConst::CStringConst(const CToken &AToken) : CConst(AToken)
{
	Value = AToken.GetText();
}

void CStringConst::Accept(CStatementVisitor &AVisitor)
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

CVariable::CVariable(const CToken &AToken, CSymbol *ASymbol /*= NULL*/) : CExpression(AToken), Symbol(ASymbol)
{
	Name = AToken.GetText();
}

void CVariable::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

bool CVariable::IsLValue() const
{
	// TODO: add check for constness using info from symbol object
	return true;
}

CSymbol* CVariable::GetSymbol() const
{
	return Symbol;
}

/******************************************************************************
 * CPostfixOp
 ******************************************************************************/

CPostfixOp::CPostfixOp(const CToken &AToken, CExpression *AArgument /*= NULL*/) : CUnaryOp(AToken, AArgument)
{
}

void CPostfixOp::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CFunctionCall
 ******************************************************************************/

CFunctionCall::CFunctionCall(CSymbol *AFunction)
{
	Type = TOKEN_TYPE_RIGHT_PARENTHESIS;	// well, that's not really clear, but looks good in error messages at least..

	Function = dynamic_cast<CFunctionSymbol *>(AFunction);
	if (Function) {
		Name = Function->GetName() + "()";
	}
}

CFunctionCall::~CFunctionCall()
{
	//delete Function;

	while (!Arguments.empty()) {
		delete Arguments.back();
		Arguments.pop_back();
	}
}

void CFunctionCall::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CFunctionCall::ArgumentsIterator CFunctionCall::Begin()
{
	return Arguments.begin();
}

CFunctionCall::ArgumentsIterator CFunctionCall::End()
{
	return Arguments.end();
}

void CFunctionCall::AddArgument(CExpression *AArgument)
{
	Arguments.push_back(AArgument);
}

/******************************************************************************
 * CStructAccess
 ******************************************************************************/

CStructAccess::CStructAccess(const CToken &AToken, CExpression *AStruct /*= NULL*/, CVariable *AField /*= NULL*/) : CExpression(AToken), Struct(AStruct), Field(AField)
{
}

CStructAccess::~CStructAccess()
{
	delete Struct;
}

void CStructAccess::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CStructAccess::GetStruct() const
{
	return Struct;
}

CVariable* CStructAccess::GetField() const
{
	return Field;
}

void CStructAccess::SetStruct(CExpression *AStruct)
{
	Struct = AStruct;
}

void CStructAccess::SetField(CVariable *AField)
{
	Field = AField;
}

/******************************************************************************
 * CIndirectAccess
 ******************************************************************************/

CIndirectAccess::CIndirectAccess(const CToken &AToken, CExpression *APointer /*= NULL*/, CVariable *AField /*= NULL*/) : CExpression(AToken), Pointer(APointer), Field(AField)
{
}

CIndirectAccess::~CIndirectAccess()
{
	delete Pointer;
}

void CIndirectAccess::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CExpression* CIndirectAccess::GetPointer() const
{
	return Pointer;
}

CVariable* CIndirectAccess::GetField() const
{
	return Field;
}

void CIndirectAccess::SetPointer(CExpression *APointer)
{
	Pointer = APointer;
}

void CIndirectAccess::SetField(CVariable *AField)
{
	Field = AField;
}

/******************************************************************************
 * CArrayAccess
 ******************************************************************************/

CArrayAccess::CArrayAccess(CExpression *ALeft /*= NULL*/, CExpression *ARight /*= NULL*/)
{
	Type = TOKEN_TYPE_LEFT_SQUARE_BRACKET;
	Name = "[]";
	Left = ALeft;
	Right = ARight;
}

CArrayAccess::~CArrayAccess()
{
}

void CArrayAccess::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

