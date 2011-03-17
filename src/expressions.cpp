#include "expressions.h"

/******************************************************************************
 * CheckTypes
 ******************************************************************************/

namespace CheckTypes
{
	void Integer(const CBinaryOp *Op)
	{
		if (!(Op->GetLeft()->GetResultType()->IsInt() && Op->GetRight()->GetResultType()->IsInt())) {
			throw CException("operands of " + CScanner::TokenTypesNames[Op->GetType()] + " should have integer types", Op->GetPosition());
		}
	}

	void Integer(const CUnaryOp *Op)
	{
		if (!Op->GetArgument()->GetResultType()->IsInt()) {
			throw CException("operand of unary " + CScanner::TokenTypesNames[Op->GetType()] + " should has integer type", Op->GetPosition());
		}
	}

	void Scalar(const CBinaryOp *Op)
	{
		if (!(Op->GetLeft()->GetResultType()->IsScalar() && Op->GetRight()->GetResultType()->IsScalar())) {
			throw CException("operands of " + CScanner::TokenTypesNames[Op->GetType()] + " should have scalar types", Op->GetPosition());
		}
	}

	void Scalar(const CUnaryOp *Op)
	{
		if (!Op->GetArgument()->GetResultType()->IsScalar()) {
			throw CException("operand of unary " + CScanner::TokenTypesNames[Op->GetType()] + " should has scalar type", Op->GetPosition());
		}
	}

	void Arithmetic(const CBinaryOp *Op)
	{
		if (!(Op->GetLeft()->GetResultType()->IsArithmetic() && Op->GetRight()->GetResultType()->IsArithmetic())) {
			throw CException("operands of " + CScanner::TokenTypesNames[Op->GetType()] + " should have arithmetic types", Op->GetPosition());
		}
	}

	void Arithmetic(const CUnaryOp *Op)
	{
		if (!Op->GetArgument()->GetResultType()->IsArithmetic()) {
			throw CException("operand of unary " + CScanner::TokenTypesNames[Op->GetType()] + " should has arithmetic type", Op->GetPosition());
		}
	}

};

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
	Position = AToken.GetPosition();
}

CExpression::~CExpression()
{
}

ETokenType CExpression::GetType() const
{
	return Type;
}

CPosition CExpression::GetPosition() const
{
	return Position;
}

bool CExpression::IsLValue() const
{
	return false;
}

bool CExpression::IsConst() const
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

CTypeSymbol* CUnaryOp::GetResultType() const
{
	// TODO: what about address-getting?.. -- generate pointer to argument type, obviously.. like this:
	/*if (Type == TOKEN_TYPE_OPERATION_AMPERSAND) {
		return GeneratePointer(Argument->GetResultType());
	}*/
	return Argument->GetResultType();
}

bool CUnaryOp::IsLValue() const
{
	return (Type == TOKEN_TYPE_OPERATION_ASTERISK);
}

bool CUnaryOp::IsConst() const
{
	return Argument->IsConst();
}

void CUnaryOp::CheckTypes() const
{
	assert(Argument != NULL);

	switch (Type) {
	case TOKEN_TYPE_OPERATION_PLUS:
	case TOKEN_TYPE_OPERATION_MINUS:
		CheckTypes::Arithmetic(this);
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_NOT:
		CheckTypes::Integer(this);
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_NOT:
		CheckTypes::Scalar(this);
		break;
	case TOKEN_TYPE_OPERATION_INCREMENT:
	case TOKEN_TYPE_OPERATION_DECREMENT:
		if (!Argument->IsLValue()) {
			throw CException("lvalue required as operand of prefix "  + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_AMPERSAND:
		if (!Argument->IsLValue() && Argument->GetType() != TOKEN_TYPE_LEFT_SQUARE_BRACKET && (typeid(*Argument) != typeid(CUnaryOp) || Argument->GetName() != "*")) { // FIXME
			throw CException("operand of address-of operation should be lvalue or result of dereferencing or array-accessing", Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK:
		if (!Argument->GetResultType()->IsPointer()) {
			throw CException("operand of dereference operation should have pointer type", Position);
		}
		break;
	}
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

CTypeSymbol* CBinaryOp::GetResultType() const
{
	if (Type == TOKEN_TYPE_OPERATION_PERCENT || Type == TOKEN_TYPE_OPERATION_SHIFT_LEFT || Type == TOKEN_TYPE_OPERATION_SHIFT_RIGHT) {
		return Left->GetResultType();
	}

	// TODO: deal with this..
	return Left->GetResultType();
}

bool CBinaryOp::IsConst() const
{
	return Left->IsConst() && Right->IsConst();
}

void CBinaryOp::CheckTypes() const
{
	assert(Left != NULL && Right != NULL);

	CTypeSymbol *L = Left->GetResultType();
	CTypeSymbol *R = Right->GetResultType();

	switch (Type) {
	case TOKEN_TYPE_OPERATION_PLUS:
		if (!(L->IsArithmetic() && R->IsArithmetic() || L->IsPointer() && R->IsInt() || L->IsInt() && R->IsPointer())) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_MINUS:
		if (!(L->IsArithmetic() && R->IsArithmetic() || L->IsPointer() && R->IsPointer() || L->IsPointer() && R->IsInt())) {	// TODO: add check for type compatibility
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK:
	case TOKEN_TYPE_OPERATION_SLASH:
		CheckTypes::Arithmetic(this);
		break;
	case TOKEN_TYPE_OPERATION_PERCENT:
		CheckTypes::Integer(this);
		break;
	case TOKEN_TYPE_OPERATION_SHIFT_LEFT:
	case TOKEN_TYPE_OPERATION_SHIFT_RIGHT:
		CheckTypes::Integer(this);
		break;
	case TOKEN_TYPE_OPERATION_LESS_THAN:
	case TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL:
	case TOKEN_TYPE_OPERATION_GREATER_THAN:
	case TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL:
		// TODO: both real (arithmetic maybe?..), or pointers to compatible types
		break;
	case TOKEN_TYPE_OPERATION_EQUAL:
	case TOKEN_TYPE_OPERATION_NOT_EQUAL:
		// TODO: both arithmetic, or pointers to compatible types, or one operand is pointer and the other is pointer to void, or one operand is pointer and the oher is null pointer constant
		break;
	case TOKEN_TYPE_OPERATION_AMPERSAND:
	case TOKEN_TYPE_OPERATION_BITWISE_XOR:
	case TOKEN_TYPE_OPERATION_BITWISE_OR:
		CheckTypes::Integer(this);
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_AND:
	case TOKEN_TYPE_OPERATION_LOGIC_OR:
		CheckTypes::Scalar(this);
		break;
	}

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

CTypeSymbol* CConditionalOp::GetResultType() const
{
	return TrueExpr->GetResultType();	// types of TrueExpr and FalseExpr must match..
}

bool CConditionalOp::IsConst() const
{
	return Condition->IsConst() && TrueExpr->IsConst() && FalseExpr->IsConst();
}

void CConditionalOp::CheckTypes() const
{
	assert(Condition != NULL && TrueExpr != NULL && FalseExpr != NULL);

	if (!Condition->GetResultType()->IsScalar()) {
		throw CException("first operand of conditional operator should have scalar type", Position);
	}

	// TODO: add second and third operands type check
}

/******************************************************************************
 * CConst
 ******************************************************************************/

CConst::CConst(const CToken &AToken, CTypeSymbol *AType) : CExpression(AToken), Type(AType)
{
}

CTypeSymbol* CConst::GetResultType() const
{
	return Type;
}

bool CConst::IsConst() const
{
	return true;
}

/******************************************************************************
 * CIntegerConst
 ******************************************************************************/

CIntegerConst::CIntegerConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
{
	Value = dynamic_cast<const CIntegerConstToken &>(AToken).GetIntegerValue();
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

CFloatConst::CFloatConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
{
	Value = dynamic_cast<const CFloatConstToken &>(AToken).GetFloatValue();
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

CSymbolConst::CSymbolConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
{
	Value = dynamic_cast<const CSymbolConstToken &>(AToken).GetSymbolValue();
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

CStringConst::CStringConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
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

CSymbol* CVariable::GetSymbol() const
{
	return Symbol;
}

CTypeSymbol* CVariable::GetResultType() const
{
	CVariableSymbol *VarSym = dynamic_cast<CVariableSymbol *>(Symbol);
	if (!VarSym) {
		return NULL;
	}

	return VarSym->GetType();
}

bool CVariable::IsLValue() const
{
	// this is check for modifiable lvalue, not regular lvalue..
	CVariableSymbol *VarSym = dynamic_cast<CVariableSymbol *>(Symbol);
	if (!VarSym) {
		return false;
	}

	return !VarSym->GetType()->GetConst();
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

void CPostfixOp::CheckTypes() const
{
	assert(Argument != NULL);

	if (Type == TOKEN_TYPE_OPERATION_INCREMENT || Type == TOKEN_TYPE_OPERATION_DECREMENT) {
		if (!Argument->IsLValue()) {
			throw CException("lvalue required as operand of postfix " + CScanner::TokenTypesNames[Type], Position);
		}
	}
}

/******************************************************************************
 * CFunctionCall
 ******************************************************************************/

CFunctionCall::CFunctionCall(CSymbol *AFunction)
{
	Type = TOKEN_TYPE_RIGHT_PARENTHESIS;	// well, that's not really clear, but looks good in error messages at least..
	Name = AFunction->GetName() + "()";

	Function = dynamic_cast<CFunctionSymbol *>(AFunction);
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

CFunctionSymbol* CFunctionCall::GetFunction() const
{
	return Function;
}

void CFunctionCall::SetFunction(CFunctionSymbol *AFunction)
{
	Function = AFunction;
}

CFunctionCall::ArgumentsIterator CFunctionCall::Begin()
{
	return Arguments.begin();
}

CFunctionCall::ArgumentsIterator CFunctionCall::End()
{
	return Arguments.end();
}

CFunctionCall::ArgumentsReverseIterator CFunctionCall::RBegin()
{
	return Arguments.rbegin();
}

CFunctionCall::ArgumentsReverseIterator CFunctionCall::REnd()
{
	return Arguments.rend();
}

unsigned int CFunctionCall::GetArgumentsCount() const
{
	return Arguments.size();
}

void CFunctionCall::AddArgument(CExpression *AArgument)
{
	Arguments.push_back(AArgument);
}

CTypeSymbol* CFunctionCall::GetResultType() const
{
	return (Function ? Function->GetReturnType() : NULL);
}

/******************************************************************************
 * CStructAccess
 ******************************************************************************/

CStructAccess::CStructAccess(const CToken &AToken, CExpression *AStruct /*= NULL*/, CVariable *AField /*= NULL*/) : CExpression(AToken), Field(AField)
{
	SetStruct(AStruct);
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
	StructSymbol = Struct ? dynamic_cast<CStructSymbol *>(Struct->GetResultType()) : NULL;
}

void CStructAccess::SetField(CVariable *AField)
{
	Field = AField;
}

CTypeSymbol* CStructAccess::GetResultType() const
{
	if (!StructSymbol || !Field) {
		return NULL;
	}

	return StructSymbol->GetField(Field->GetName())->GetType();
}

void CStructAccess::CheckTypes() const
{
	assert(Struct != NULL && Field != NULL);

	if (!StructSymbol) {
		throw CException("first operand of " + CScanner::TokenTypesNames[Type] + " should have struct type", Position);
	}

	if (!StructSymbol->GetField(Field->GetName())) {
		throw CException("second operand of " + CScanner::TokenTypesNames[Type] + " should be identifier of member of struct", Position);
	}
}

/******************************************************************************
 * CIndirectAccess
 ******************************************************************************/

CIndirectAccess::CIndirectAccess(const CToken &AToken, CExpression *APointer /*= NULL*/, CVariable *AField /*= NULL*/) : CExpression(AToken), Field(AField)
{
	SetPointer(APointer);
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
	StructSymbol = NULL;

	CPointerSymbol *PointerSym = NULL;
	if (Pointer && (PointerSym = dynamic_cast<CStructSymbol *>(Pointer->GetResultType()))) {
		StructSymbol = dynamic_cast<CStructSymbol *>(PointerSym->GetRefType());
	}
}

void CIndirectAccess::SetField(CVariable *AField)
{
	Field = AField;
}

CTypeSymbol* CIndirectAccess::GetResultType() const
{
	if (!StructSymbol || !Field) {
		return NULL;
	}

	return StructSymbol->GetField(Field->GetName())->GetType();
}

void CIndirectAccess::CheckTypes() const
{
	assert(Pointer != NULL && Field != NULL);

	if (!StructSymbol) {
		throw CException("first operand of " + CScanner::TokenTypesNames[Type] + " should have struct type", Position);
	}

	if (!StructSymbol->GetField(Field->GetName())) {
		throw CException("second operand of " + CScanner::TokenTypesNames[Type] + " should be identifier of member of struct", Position);
	}
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

CTypeSymbol* CArrayAccess::GetResultType() const
{
	if (Left->GetResultType()->IsInt()) {
		return Right->GetResultType();
	} else {
		return Left->GetResultType();
	}
}

void CArrayAccess::CheckTypes() const
{
	assert(Left != NULL && Right != NULL);

	CTypeSymbol *L = Left->GetResultType();
	CTypeSymbol *R = Right->GetResultType();

	if (!(L->IsPointer() && R->IsInt() || L->IsInt() && R->IsPointer())) {
		throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
	}
}
