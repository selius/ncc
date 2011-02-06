#include "parser.h"

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

string CExpression::GetName() const
{
	return Name;
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

void CConditionalOp::Accept(CExpressionVisitor &AVisitor)
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

CSymbolConst::CSymbolConst(const CSymbolConstToken &AToken) : CConst(AToken)
{
	Value = AToken.GetSymbolValue();
}

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

CStringConst::CStringConst(const CToken &AToken) : CConst(AToken)
{
	Value = AToken.GetText();
}

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

bool CVariable::IsLValue() const
{
	// TODO: add check for constness using info from symbol object
	return true;
}

/******************************************************************************
 * CPostfixOp
 ******************************************************************************/

CPostfixOp::CPostfixOp(const CToken &AToken, CExpression *AArgument /*= NULL*/) : CUnaryOp(AToken, AArgument)
{
}

void CPostfixOp::Accept(CExpressionVisitor &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CFunctionCall
 ******************************************************************************/

CFunctionCall::CFunctionCall(CExpression *AFunction)
{
	//Type = TOKEN_TYPE_IDENTIFIER;
	Function = AFunction;
	if (Function) {
		Name = Function->GetName() + "()";
	}
}

CFunctionCall::~CFunctionCall()
{
	delete Function;

	while (!Arguments.empty()) {
		delete Arguments.back();
		Arguments.pop_back();
	}
}

void CFunctionCall::Accept(CExpressionVisitor &AVisitor)
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
 * CTokenStream
 ******************************************************************************/

CTokenStream::CTokenStream(CScanner &AScanner) : Scanner(AScanner)
{
	Current = --Buffer.end();
}

CTokenStream::~CTokenStream()
{
	while (!Buffer.empty()) {
		delete Buffer.back();
		Buffer.pop_back();
	}
}

const CToken* CTokenStream::Next()
{
	++Current;

	if (Current == Buffer.end()) {
		Buffer.push_back(Scanner.Next()->Clone());
		if (Buffer.size() > TOKEN_STREAM_SIZE) {
			delete Buffer.front();
			Buffer.pop_front();
		}
		Current = --Buffer.end();
	}

	return GetToken();
}

const CToken* CTokenStream::GetToken()
{
	return *Current;
}

const CToken* CTokenStream::Previous()
{
	if (Current == Buffer.begin()) {
		return NULL;
	}

	--Current;

	return GetToken();
}

/******************************************************************************
 * CParser
 ******************************************************************************/

CParser::CParser(CScanner &AScanner) : TokenStream(AScanner)
{
	NextToken();
}

CExpression* CParser::ParseExpression()
{
	CExpression *Expr = ParseAssignment();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_SEPARATOR_COMMA) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseAssignment());

		Expr = Op;
	}

	return Expr;
}

bool IsAssignment(const CToken &Token)
{
	static const ETokenType Assignments[] = {
		TOKEN_TYPE_OPERATION_ASSIGN,
		TOKEN_TYPE_OPERATION_PLUS_ASSIGN,
		TOKEN_TYPE_OPERATION_MINUS_ASSIGN,
		TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN,
		TOKEN_TYPE_OPERATION_SLASH_ASSIGN,
		TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN,
		TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN,
		TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN,
		TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN,
		TOKEN_TYPE_OPERATION_PERCENT_ASSIGN,
		TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN,
		TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN,
		TOKEN_TYPE_INVALID
		};

	ETokenType t = Token.GetType();

	for (int i = 0; Assignments[i] != TOKEN_TYPE_INVALID; i++) {
		if (t == Assignments[i]) {
			return true;
		}
	}

	return false;
}

bool IsRelational(const CToken &Token)
{
	static const ETokenType RelationalOps[] = {
		TOKEN_TYPE_OPERATION_LESS_THAN,
		TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL,
		TOKEN_TYPE_OPERATION_GREATER_THAN,
		TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL,
		TOKEN_TYPE_INVALID
		};

	ETokenType t = Token.GetType();

	for (int i = 0; RelationalOps[i] != TOKEN_TYPE_INVALID; i++) {
		if (t == RelationalOps[i]) {
			return true;
		}
	}

	return false;
}

bool IsUnaryOp(const CToken &Token)
{
	static const ETokenType UnaryOps[] = {
		TOKEN_TYPE_OPERATION_AMPERSAND,
		TOKEN_TYPE_OPERATION_ASTERISK,
		TOKEN_TYPE_OPERATION_PLUS,
		TOKEN_TYPE_OPERATION_MINUS,
		TOKEN_TYPE_OPERATION_BITWISE_NOT,
		TOKEN_TYPE_OPERATION_LOGIC_NOT,
		TOKEN_TYPE_INVALID
		};

	ETokenType t = Token.GetType();

	for (int i = 0; UnaryOps[i] != TOKEN_TYPE_INVALID; i++) {
		if (t == UnaryOps[i]) {
			return true;
		}
	}

	return false;
}

bool IsPostfix(const CToken &Token)
{
	static const ETokenType PostfixOps[] = {
		TOKEN_TYPE_LEFT_SQUARE_BRACKET,
		TOKEN_TYPE_LEFT_PARENTHESIS,
		TOKEN_TYPE_OPERATION_DOT,
		TOKEN_TYPE_OPERATION_INDIRECT_ACCESS,
		TOKEN_TYPE_OPERATION_INCREMENT,
		TOKEN_TYPE_OPERATION_DECREMENT,
		TOKEN_TYPE_INVALID
		};

	ETokenType t = Token.GetType();

	for (int i = 0; PostfixOps[i] != TOKEN_TYPE_INVALID; i++) {
		if (t == PostfixOps[i]) {
			return true;
		}
	}

	return false;
}

CExpression* CParser::ParseAssignment()
{
	CExpression *Expr = ParseConditional();

	if (IsAssignment(*Token)) {
		if (!Expr->IsLValue()) {
			throw CException("lvalue required as left operand of assignment", Token->GetPosition());
		}

		CBinaryOp *Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseAssignment());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseConditional()
{
	CExpression *Expr = ParseLogicalOr();

	if (Token->GetType() == TOKEN_TYPE_OPERATION_CONDITIONAL) {

		CConditionalOp *Op = new CConditionalOp(*Token);

		NextToken();

		Op->SetCondition(Expr);
		Op->SetTrueExpr(ParseExpression());

		if (Token->GetType() != TOKEN_TYPE_SEPARATOR_COLON) {
			throw CException("expected SEPARATOR_COLON, got " + Token->GetStringifiedType(), Token->GetPosition());
		}

		NextToken();

		Op->SetFalseExpr(ParseConditional());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseLogicalOr()
{
	CExpression *Expr = ParseLogicalAnd();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_LOGIC_OR) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseLogicalAnd());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseLogicalAnd()
{
	CExpression *Expr = ParseBitwiseOr();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_LOGIC_AND) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseBitwiseOr());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseBitwiseOr()
{
	CExpression *Expr = ParseBitwiseXor();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_BITWISE_OR) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseBitwiseXor());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseBitwiseXor()
{
	CExpression *Expr = ParseBitwiseAnd();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_BITWISE_XOR) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseBitwiseAnd());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseBitwiseAnd()
{
	CExpression *Expr = ParseEqualityExpression();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_AMPERSAND) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseEqualityExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseEqualityExpression()
{
	CExpression *Expr = ParseRelationalExpression();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_EQUAL || Token->GetType() == TOKEN_TYPE_OPERATION_NOT_EQUAL) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseRelationalExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseRelationalExpression()
{
	CExpression *Expr = ParseShiftExpression();

	CBinaryOp *Op;

	while (IsRelational(*Token)) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseShiftExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseShiftExpression()
{
	CExpression *Expr = ParseAdditiveExpression();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_SHIFT_LEFT || Token->GetType() == TOKEN_TYPE_OPERATION_SHIFT_RIGHT) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseAdditiveExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseAdditiveExpression()
{
	CExpression *Expr = ParseMultiplicativeExpression();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_PLUS || Token->GetType() == TOKEN_TYPE_OPERATION_MINUS) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseMultiplicativeExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseMultiplicativeExpression()
{
	CExpression *Expr = ParseCastExpression();

	CBinaryOp *Op;

	while (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK || Token->GetType() == TOKEN_TYPE_OPERATION_SLASH || Token->GetType() == TOKEN_TYPE_OPERATION_PERCENT) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseCastExpression());

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseCastExpression()
{
	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		NextToken();
		if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
			// parse type..
			//CSymbol *CastType = NULL;
			CExpression *CastType = NULL;
			//CastType = ParseType();
			if (CastType) {
				/*if (Token->GetType() == TOKEN_TYPE_RIGHT_PARENTHESIS) {
					CCastOperator *Op = new CCastOperator(CastType);
					Op->SetArgument(ParseCastExpression());
					return Op;
				} else {
					throw CException("expected RIGHT_PARENTHESIS, got " + Token->GetStringifiedType(), Token->GetPosition());
				}*/
			}
		}
		PreviousToken();
	}

	return ParseUnaryExpression();
}

CExpression* CParser::ParseUnaryExpression()
{
	if (IsUnaryOp(*Token)) {
		CUnaryOp *Op = new CUnaryOp(*Token);
		NextToken();
		Op->SetArgument(ParseCastExpression());
		return Op;
	}

	if (Token->GetType() == TOKEN_TYPE_OPERATION_INCREMENT || Token->GetType() == TOKEN_TYPE_OPERATION_DECREMENT) {
		CUnaryOp *Op = new CUnaryOp(*Token);
		NextToken();
		CPosition ArgPos = Token->GetPosition();
		Op->SetArgument(ParseUnaryExpression());
		if (!Op->GetArgument()->IsLValue()) {
			throw CException("lvalue required as operand of prefix increment or decrement", ArgPos);
		}
		return Op;
	}

	if (Token->GetType() == TOKEN_TYPE_KEYWORD && Token->GetText() == "sizeof") {
		// handle sizeof somehow..
	}

	return ParsePostfixExpression();
}

CExpression* CParser::ParsePostfixExpression()
{
	CExpression *Expr = ParsePrimaryExpression();

	while (IsPostfix(*Token)) {
		//CExpression *Op;

		switch (Token->GetType()) {
		case TOKEN_TYPE_OPERATION_DOT:
			/*CStructAccess *Op = new CStructAccess(*Token);
			Op->SetLeft(Expr);

			NextToken();

			if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
					+ " as right operand of " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_DOT]
					+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
			}
			Op->SetRight(new CVariable(*Token));*/


			///////// NOT DONE YET! //////////
			break;
		case TOKEN_TYPE_OPERATION_INDIRECT_ACCESS:
			/*CIndirectAccess *Op = new CIndirectAccess(*Token);
			Op->SetLeft(Expr);

			NextToken();

			if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
					+ " as right operand of " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_INDIRECT_ACCESS]
					+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
			}
			Op->SetRight(new CVariable(*Token));*/

			///////// NOT DONE YET! //////////
			break;
		case TOKEN_TYPE_LEFT_SQUARE_BRACKET:
			/*CArrayAccess *Op = new CArrayAccess(*Token);
			Op->SetLeft(Expr);

			NextToken();

			Op->SetRight(ParseExpression());
			if (Token->GetType() != TOKEN_TYPE_RIGHT_SQUARE_BRACKET) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET]
					+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
			}
			Expr = Op;*/

			///////// NOT DONE YET! //////////
			break;
		case TOKEN_TYPE_OPERATION_INCREMENT:
		case TOKEN_TYPE_OPERATION_DECREMENT:
			{
				if (!Expr->IsLValue()) {
					throw CException("lvalue required as operand of postfix increment or decrement", Token->GetPosition());
				}

				CPostfixOp *Op = new CPostfixOp(*Token);
				Op->SetArgument(Expr);
				NextToken();
				Expr = Op;
			}
			break;
		case TOKEN_TYPE_LEFT_PARENTHESIS:
			if (Expr->GetType() != TOKEN_TYPE_IDENTIFIER) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
					+ " before function call operator, got " + CScanner::TokenTypesNames[Expr->GetType()], Token->GetPosition());
			}
			CFunctionCall *FuncCall = new CFunctionCall(Expr);
			NextToken();
			if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
				PreviousToken();
				do {
					NextToken();
					FuncCall->AddArgument(ParseAssignment());
				} while (Token->GetType() == TOKEN_TYPE_SEPARATOR_COMMA);
			}
			
			if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
					+ " after function arguments list, got " + Token->GetStringifiedType(), Token->GetPosition());
			}
			NextToken();

			Expr = FuncCall;
			break;
		}

		//Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParsePrimaryExpression()
{
	CExpression *Expr;

	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		NextToken();
		Expr = ParseExpression();
		if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
		}
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_INTEGER) {
		Expr = new CIntegerConst(*dynamic_cast<const CIntegerConstToken *>(Token));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_FLOAT) {
		Expr = new CFloatConst(*dynamic_cast<const CFloatConstToken *>(Token));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_SYMBOL) {
		Expr = new CSymbolConst(*dynamic_cast<const CSymbolConstToken *>(Token));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_STRING) {
		Expr = new CStringConst(*Token);
	} else if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		Expr = new CVariable(*Token);
	} else {
		throw CException("expected primary-expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();

	return Expr;
}

void CParser::NextToken()
{
	Token = TokenStream.Next();
}

void CParser::PreviousToken()
{
	Token = TokenStream.Previous();
}
