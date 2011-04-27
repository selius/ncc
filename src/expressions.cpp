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

	void LValue(const CBinaryOp *Op)
	{
		if (!Op->GetLeft()->IsLValue()) {
			throw CException("lvalue required as left operand of " + CScanner::TokenTypesNames[Op->GetType()], Op->GetPosition());
		}
	}

};

/******************************************************************************
 * CExpression
 ******************************************************************************/

CExpression::CExpression() : Type(TOKEN_TYPE_INVALID), ResultType(NULL)
{
}

CExpression::CExpression(const CToken &AToken)
{
	Type = AToken.GetType();
	Name = AToken.GetText();
	Position = AToken.GetPosition();
	ResultType = NULL;
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

void CExpression::SetResultType(CTypeSymbol *AResultType)
{
	ResultType = AResultType;
}

bool CExpression::IsLValue() const
{
	return false;
}

bool CExpression::IsConst() const
{
	return false;
}

bool CExpression::IsExpression() const
{
	return true;
}

void CExpression::CheckTypes() const
{
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
	if (ResultType) {
		return ResultType;
	}

	if (Type == TOKEN_TYPE_OPERATION_ASTERISK) {
		return dynamic_cast<CPointerSymbol *>(Argument->GetResultType())->GetRefType();
	}

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
		CheckTypes::Scalar(this);
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK:
		if (!Argument->GetResultType()->IsPointer()) {
			throw CException("operand of dereference operation should have pointer type", Position);
		}
		break;
	case TOKEN_TYPE_KEYWORD:
		if (Argument->GetResultType()->IsFunction()) {
			throw CException("invalid operand to sizeof operator", Position);
		}
		break;
	}
}

bool CUnaryOp::CanBeHoisted() const
{
	return (Type != TOKEN_TYPE_OPERATION_INCREMENT && Type != TOKEN_TYPE_OPERATION_DECREMENT && Argument->CanBeHoisted());
}

void CUnaryOp::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Argument, Affected, Type == TOKEN_TYPE_OPERATION_INCREMENT || Type == TOKEN_TYPE_OPERATION_DECREMENT || Collect);
}

void CUnaryOp::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Argument, Used);
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
	if (ResultType) {
		return ResultType;
	}

	if (Type == TOKEN_TYPE_SEPARATOR_COMMA) {
		return Right->GetResultType();
	}

	return GetCommonRealType();
}

CTypeSymbol* CBinaryOp::GetCommonRealType() const
{
	if (Left->GetResultType()->IsInt() && Right->GetResultType()->IsFloat()) {
		return Right->GetResultType();
	} else {
		return Left->GetResultType();
	}
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
		if (!(L->IsArithmetic() && R->IsArithmetic() || L->IsPointer() && R->IsPointer() && L->CompatibleWith(R) || L->IsPointer() && R->IsInt())) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_LESS_THAN:
	case TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL:
	case TOKEN_TYPE_OPERATION_GREATER_THAN:
	case TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL:
		if (!(L->IsArithmetic() && R->IsArithmetic() || L->IsPointer() && R->IsPointer() && L->CompatibleWith(R))) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_EQUAL:
	case TOKEN_TYPE_OPERATION_NOT_EQUAL:
		if (!(L->IsArithmetic() && R->IsArithmetic() || L->IsPointer() && R->IsPointer() && (L->CompatibleWith(R)
			|| dynamic_cast<CPointerSymbol *>(L)->GetRefType()->IsVoid() || dynamic_cast<CPointerSymbol *>(R)->GetRefType()->IsVoid()))) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK:
	case TOKEN_TYPE_OPERATION_SLASH:
		CheckTypes::Arithmetic(this);
		break;
	case TOKEN_TYPE_OPERATION_PERCENT:
	case TOKEN_TYPE_OPERATION_SHIFT_LEFT:
	case TOKEN_TYPE_OPERATION_SHIFT_RIGHT:
	case TOKEN_TYPE_OPERATION_AMPERSAND:
	case TOKEN_TYPE_OPERATION_BITWISE_XOR:
	case TOKEN_TYPE_OPERATION_BITWISE_OR:
		CheckTypes::Integer(this);
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_AND:
	case TOKEN_TYPE_OPERATION_LOGIC_OR:
		CheckTypes::Scalar(this);
		break;
	case TOKEN_TYPE_OPERATION_ASSIGN:
		CheckTypes::LValue(this);

		if (!(L->IsArithmetic() && R->IsArithmetic() ||/* L->IsStruct() && R->IsStruct() && L->CompatibleWith(R) ||*/ !L->IsArray() && L->IsPointer() && R->IsPointer() && (L->CompatibleWith(R)
			|| dynamic_cast<CPointerSymbol *>(L)->GetRefType()->IsVoid() || dynamic_cast<CPointerSymbol *>(R)->GetRefType()->IsVoid()))) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_PLUS_ASSIGN:
	case TOKEN_TYPE_OPERATION_MINUS_ASSIGN:
		CheckTypes::LValue(this);

		if (!(L->IsPointer() && R->IsInt() || L->IsArithmetic() && R->IsArithmetic())) {
			throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
		}
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN:
	case TOKEN_TYPE_OPERATION_SLASH_ASSIGN:
		CheckTypes::LValue(this);
		CheckTypes::Arithmetic(this);
		break;
	case TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN:
	case TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN:
	case TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN:
	case TOKEN_TYPE_OPERATION_PERCENT_ASSIGN:
	case TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN:
	case TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN:
		CheckTypes::LValue(this);
		CheckTypes::Integer(this);
		break;
	}
}

bool CBinaryOp::CanBeHoisted() const
{
	return !TokenTraits::IsCompoundAssignment(Type) && Left->CanBeHoisted() && Right->CanBeHoisted();
}

void CBinaryOp::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Left, Affected, TokenTraits::IsAssignment(Type) || Collect);
	TryGetAffected(Right, Affected, Collect);
}

void CBinaryOp::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Left, Used);
	TryGetUsed(Right, Used);
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
	return TrueExpr->GetResultType();
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

	CTypeSymbol *TE = TrueExpr->GetResultType();
	CTypeSymbol *FE = FalseExpr->GetResultType();

	if (!(TE->IsArithmetic() && FE->IsArithmetic() ||
		TE->IsStruct() && FE->IsStruct() && TE->CompatibleWith(FE) ||
		TE->IsVoid() && FE->IsVoid() || TE->IsPointer() && FE->IsPointer() && (TE->CompatibleWith(FE)
		|| dynamic_cast<CPointerSymbol *>(TE)->GetRefType()->IsVoid() || dynamic_cast<CPointerSymbol *>(FE)->GetRefType()->IsVoid()))) {
		throw CException("invalid operands to " + CScanner::TokenTypesNames[Type], Position);
	}
}

bool CConditionalOp::CanBeHoisted() const
{
	return Condition->CanBeHoisted() && TrueExpr->CanBeHoisted() && FalseExpr->CanBeHoisted();
}

void CConditionalOp::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	TryGetAffected(Condition, Affected, Collect);
	TryGetAffected(TrueExpr, Affected, Collect);
	TryGetAffected(FalseExpr, Affected, Collect);
}

void CConditionalOp::GetUsedVariables(UsedContainer &Used)
{
	TryGetUsed(Condition, Used);
	TryGetUsed(TrueExpr, Used);
	TryGetUsed(FalseExpr, Used);
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

bool CConst::CanBeHoisted() const
{
	return true;
}

/******************************************************************************
 * CIntegerConst
 ******************************************************************************/

CIntegerConst::CIntegerConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
{
	Value = static_cast<const CIntegerConstToken &>(AToken).GetIntegerValue();
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
	Value = static_cast<const CFloatConstToken &>(AToken).GetFloatValue();
}

void CFloatConst::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

float CFloatConst::GetValue() const
{
	return Value;
}

/******************************************************************************
 * CCharConst
 ******************************************************************************/

CCharConst::CCharConst(const CToken &AToken, CTypeSymbol *AType) : CConst(AToken, AType)
{
	Value = static_cast<const CCharConstToken &>(AToken).GetCharValue();
}

void CCharConst::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

char CCharConst::GetValue() const
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

CVariable::CVariable(const CToken &AToken, CVariableSymbol *ASymbol /*= NULL*/) : CExpression(AToken), Symbol(ASymbol)
{
	Name = AToken.GetText();
}

void CVariable::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CVariableSymbol* CVariable::GetSymbol() const
{
	return Symbol;
}

void CVariable::SetSymbol(CVariableSymbol *ASymbol)
{
	Symbol = ASymbol;
}

CTypeSymbol* CVariable::GetResultType() const
{
	if (!Symbol) {
		return NULL;
	}

	return Symbol->GetType();
}

bool CVariable::IsLValue() const
{
	if (!Symbol) {
		return false;
	}

	return !Symbol->GetType()->GetConst();
}

bool CVariable::CanBeHoisted() const
{
	if (!Symbol) {
		return false;
	}

	return !Symbol->GetGlobal() && !Symbol->GetType()->IsPointer();
}

void CVariable::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	if (!Collect) {
		return;
	}

	if (!Affected.count(Symbol)) {
		Affected[Symbol] = 0;
	}

	++Affected[Symbol];
}

void CVariable::GetUsedVariables(UsedContainer &Used)
{
	if (!Used.count(Symbol)) {
		Used[Symbol] = 0;
	}

	++Used[Symbol];
}

/******************************************************************************
 * CFunction
 ******************************************************************************/

CFunction::CFunction(const CToken &AToken, CFunctionSymbol *ASymbol /*= NULL*/) : CExpression(AToken), Symbol(ASymbol)
{
}

void CFunction::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

CFunctionSymbol* CFunction::GetSymbol() const
{
	return Symbol;
}

void CFunction::SetSymbol(CFunctionSymbol *ASymbol)
{
	Symbol = ASymbol;
}

CTypeSymbol* CFunction::GetResultType() const
{
	if (!Symbol) {
		return NULL;
	}

	return Symbol->GetType();
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

		if (!Argument->GetResultType()->IsScalar()) {
			throw CException("operand of postfix " + CScanner::TokenTypesNames[Type] + " should has scalar type", Position);
		}
	}
}

/******************************************************************************
 * CFunctionCall
 ******************************************************************************/

CFunctionCall::CFunctionCall(const CToken &AToken, CFunctionSymbol *AFunction) : CExpression(AToken)
{
	Type = TOKEN_TYPE_RIGHT_PARENTHESIS;	// well, that's not really clear, but looks good in error messages at least..
	Name = AFunction->GetName() + "()";

	Function = AFunction;
}

CFunctionCall::~CFunctionCall()
{
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

CFunctionCall::ArgumentsIterator CFunctionCall::Begin() const
{
	return Arguments.begin();
}

CFunctionCall::ArgumentsIterator CFunctionCall::End() const
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

bool CFunctionCall::IsExpression() const
{
	if (!Function || !Function->GetReturnType()) {
		return false;
	}

	return !Function->GetReturnType()->IsVoid();
}

void CFunctionCall::CheckTypes() const
{
	assert(Function != NULL);

	CFunctionSymbol::ArgumentsOrderContainer *FormalArgs = Function->GetArgumentsOrderedList();

	ArgumentsIterator ait;
	CFunctionSymbol::ArgumentsOrderIterator fit;

	if (Arguments.size() != Function->GetArgumentsSymbolTable()->VariablesSize()) {
		throw CException("number of actual and formal parameters don't match", Position);
	}

	for (ait = Begin(), fit = FormalArgs->begin(); ait != End() && fit != FormalArgs->end(); ++ait, ++fit) {
		if (!(*ait)->GetResultType()->CompatibleWith(dynamic_cast<CVariableSymbol *>(*fit)->GetType())) {
			throw CException("function actual parameters don't match its formal parameters", Position);
		}
	}
}

void CFunctionCall::GetAffectedVariables(AffectedContainer &Affected, bool Collect /*= false*/)
{
	for (ArgumentsIterator it = Begin(); it != End(); ++it) {
		TryGetAffected(*it, Affected, true);
	}
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
	delete Field;
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

	if (Field && StructSymbol) {
		Field->SetSymbol(StructSymbol->GetField(Field->GetName()));
	}
}

void CStructAccess::SetField(CVariable *AField)
{
	Field = AField;

	if (Field && StructSymbol) {
		Field->SetSymbol(StructSymbol->GetField(Field->GetName()));
	}
}

CTypeSymbol* CStructAccess::GetResultType() const
{
	if (!StructSymbol || !Field) {
		return NULL;
	}

	return Field->GetSymbol()->GetType();
}

bool CStructAccess::IsLValue() const
{
	return !StructSymbol->GetConst() && !Field->GetSymbol()->GetType()->GetConst();
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
	delete Field;
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
	if (Pointer && (PointerSym = dynamic_cast<CPointerSymbol *>(Pointer->GetResultType()))) {
		StructSymbol = dynamic_cast<CStructSymbol *>(PointerSym->GetRefType());
	}

	if (Field && StructSymbol) {
		Field->SetSymbol(StructSymbol->GetField(Field->GetName()));
	}
}

void CIndirectAccess::SetField(CVariable *AField)
{
	Field = AField;

	if (Field && StructSymbol) {
		Field->SetSymbol(StructSymbol->GetField(Field->GetName()));
	}
}

CTypeSymbol* CIndirectAccess::GetResultType() const
{
	if (!StructSymbol || !Field) {
		return NULL;
	}

	return Field->GetSymbol()->GetType();
}

bool CIndirectAccess::IsLValue() const
{
	return !StructSymbol->GetConst() && !Field->GetSymbol()->GetType()->GetConst();
}

void CIndirectAccess::CheckTypes() const
{
	assert(Pointer != NULL && Field != NULL);

	if (!StructSymbol) {
		throw CException("first operand of " + CScanner::TokenTypesNames[Type] + " should have pointer to struct type", Position);
	}

	if (!StructSymbol->GetField(Field->GetName())) {
		throw CException("second operand of " + CScanner::TokenTypesNames[Type] + " should be identifier of member of struct", Position);
	}
}

/******************************************************************************
 * CArrayAccess
 ******************************************************************************/

CArrayAccess::CArrayAccess(const CToken &AToken, CExpression *ALeft /*= NULL*/, CExpression *ARight /*= NULL*/) : CBinaryOp(AToken, ALeft, ARight)
{
	Name = "[]";
}

CArrayAccess::~CArrayAccess()
{
}

void CArrayAccess::Accept(CStatementVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

size_t CArrayAccess::GetElementSize() const
{
	return GetResultType()->GetSize();
}

CTypeSymbol* CArrayAccess::GetResultType() const
{
	CTypeSymbol *result;

	if (Left->GetResultType()->IsPointer()) {
		if (Left->GetResultType()->IsArray()) {
			result = dynamic_cast<CArraySymbol *>(Left->GetResultType())->GetElementsType();
		} else {
			result = dynamic_cast<CPointerSymbol *>(Left->GetResultType())->GetRefType();
		}
	} else {
		if (Right->GetResultType()->IsArray()) {
			result = dynamic_cast<CArraySymbol *>(Right->GetResultType())->GetElementsType();
		} else {
			result = dynamic_cast<CPointerSymbol *>(Right->GetResultType())->GetRefType();
		}
	}

	return result;
}

bool CArrayAccess::IsLValue() const
{
	return !GetResultType()->GetConst();
}

void CArrayAccess::CheckTypes() const
{
	assert(Left != NULL && Right != NULL);

	CTypeSymbol *L = Left->GetResultType();
	CTypeSymbol *R = Right->GetResultType();

	if (!(L->IsPointer() && R->IsInt() || L->IsInt() && R->IsPointer())) {
		throw CException("invalid operands to array-access operation", Position);
	}
}

/******************************************************************************
 * CAddressOfOp
 ******************************************************************************/

CAddressOfOp::CAddressOfOp(const CToken &AToken, CExpression *AArgument /*= NULL*/) : CUnaryOp(AToken, AArgument), ResultType(NULL)
{
	SetArgument(AArgument);
}

CAddressOfOp::~CAddressOfOp()
{
	delete ResultType;
}

void CAddressOfOp::SetArgument(CExpression *AArgument)
{
	delete ResultType;
	CUnaryOp::SetArgument(AArgument);
	ResultType = Argument ? new CPointerSymbol(Argument->GetResultType()) : NULL;
}

CTypeSymbol* CAddressOfOp::GetResultType() const
{
	return ResultType;
}

void CAddressOfOp::CheckTypes() const
{
	if (!Argument->IsLValue() && Argument->GetType() != TOKEN_TYPE_LEFT_SQUARE_BRACKET && (typeid(*Argument) != typeid(CUnaryOp) || Argument->GetName() != "*")) {
		throw CException("operand of address-of operation should be lvalue or result of dereferencing or array-accessing", Position);
	}
}
