#include "parser.h"

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
	SymbolTableStack.Push(new CSymbolTable);
	CFunctionSymbol *main_sym_test = new CFunctionSymbol;
	main_sym_test->SetName("main");
	SymbolTableStack.GetTop()->Add(main_sym_test);
}

CStatement* CParser::ParseStatement()
{
	ETokenType type = Token->GetType();
	if (type == TOKEN_TYPE_KEYWORD) {
		CStatement *result = NULL;
		string text = Token->GetText();
		/*if (text == "if") {
			result = ParseIf();
		} else if (text == "for") {
			result = ParseFor();
		} else if (text == "while") {
			result = ParseWhile();
		} else if (text == "do") {
			result = ParseDo();
		} else if (text == "return") {
			result = ParseReturn;
		} else if (text == "switch") {
			result = ParseSwitch();
		}*/
		return result;
	} else if (type == TOKEN_TYPE_BLOCK_START) {
		return ParseBlock();
	} else if (type == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		NextToken();
		return new CNullStatement;
	} else {
		CStatement *Expr = ParseExpression();
		if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
				+ " after expression, got " + CScanner::TokenTypesNames[type], Token->GetPosition());
		}
		NextToken();
		return Expr;
	}
}

CStatement* CParser::ParseBlock()
{
	NextToken();

	CBlockStatement *Stmt = new CBlockStatement;

	while (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		if (Token->GetType() == TOKEN_TYPE_EOF) {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
				+ ", got " + CScanner::TokenTypesNames[TOKEN_TYPE_EOF], Token->GetPosition());
		}

		Stmt->Add(ParseStatement());
	}

	NextToken();

	return Stmt;
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
	CExpression *Op;

	while (IsPostfix(*Token)) {
		switch (Token->GetType()) {
		case TOKEN_TYPE_OPERATION_DOT:
			{
				CStructAccess *StructAccess = new CStructAccess(*Token, Expr);
				Op = StructAccess;

				NextToken();

				if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
					throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
						+ " as right operand of " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_DOT]
						+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
				}

				StructAccess->SetField(new CVariable(*Token));
			}
			break;
		case TOKEN_TYPE_OPERATION_INDIRECT_ACCESS:
			{
				CIndirectAccess *IndirectAccess = new CIndirectAccess(*Token, Expr);
				Op = IndirectAccess;

				NextToken();

				if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
					throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
						+ " as right operand of " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_INDIRECT_ACCESS]
						+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
				}

				IndirectAccess->SetField(new CVariable(*Token));
			}
			break;
		case TOKEN_TYPE_LEFT_SQUARE_BRACKET:
			{
				CArrayAccess *ArrayAccess = new CArrayAccess;
				Op = ArrayAccess;

				ArrayAccess->SetLeft(Expr);

				NextToken();

				ArrayAccess->SetRight(ParseExpression());

				// TODO: check types..

				if (Token->GetType() != TOKEN_TYPE_RIGHT_SQUARE_BRACKET) {
					throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET]
						+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
				}
			}
			break;
		case TOKEN_TYPE_OPERATION_INCREMENT:
		case TOKEN_TYPE_OPERATION_DECREMENT:
			{
				if (!Expr->IsLValue()) {
					throw CException("lvalue required as operand of postfix increment or decrement", Token->GetPosition());
				}

				CPostfixOp *PostfixOp = new CPostfixOp(*Token);
				Op = PostfixOp; 
				PostfixOp->SetArgument(Expr);
			}
			break;
		case TOKEN_TYPE_LEFT_PARENTHESIS:
			{
				CVariable *VarExpr = dynamic_cast<CVariable *>(Expr);
				if (!VarExpr) {
					throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
						+ " before function call operator, got " + CScanner::TokenTypesNames[Expr->GetType()], Token->GetPosition());
				}

				CFunctionCall *FuncCall = new CFunctionCall(VarExpr->GetSymbol());
				Op = FuncCall;

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
			}
			break;
		}

		NextToken();
		Expr = Op;
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
		CSymbol *Sym = SymbolTableStack.Lookup(Token->GetText());
		/*if (!Sym) {
			throw CException("undeclared identifier `" + Token->GetText() + "`", Token->GetPosition());
		}*/

		Expr = new CVariable(*Token, Sym);
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

