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
 * CParser::CLabelInfo
 ******************************************************************************/

CParser::CLabelInfo::CLabelInfo(CLabel *ALabel /*= NULL*/, CPosition APosition /*= CPosition()*/) : Label(ALabel), Position(APosition)
{
}

/******************************************************************************
 * CParser::CDeclarationSpecifier
 ******************************************************************************/

CParser::CDeclarationSpecifier::CDeclarationSpecifier() : Type(NULL), Typedef(false), Const(false)
{
}

/******************************************************************************
 * CParser
 ******************************************************************************/

CParser::CParser(CScanner &AScanner, EParserMode AMode /*= PARSER_MODE_NORMAL*/) : TokenStream(AScanner), CurrentFunction(NULL), AnonymousTagCounter(0), Mode(AMode)
{
	NextToken();

	CGlobalSymbolTable *GlobalSymTable = new CGlobalSymbolTable;

	SymbolTableStack.SetGlobal(GlobalSymTable);

	GlobalSymTable->AddType(new CIntegerSymbol);
	GlobalSymTable->AddType(new CFloatSymbol);
	GlobalSymTable->AddType(new CVoidSymbol);
	GlobalSymTable->AddType(new CPointerSymbol(GlobalSymTable->GetType("int")));

	AddBuiltIn("__print_int", "void", 1, "int");
	AddBuiltIn("__print_float", "void", 1, "float");

	BlockType.push(BLOCK_TYPE_DEFAULT);
	ScopeType.push(SCOPE_TYPE_GLOBAL);
}

CParser::~CParser()
{
	delete SymbolTableStack.Pop();
}

CGlobalSymbolTable* CParser::ParseTranslationUnit()
{
	CSymbol *Sym = NULL;

	while (Token->GetType() != TOKEN_TYPE_EOF) {
		CPosition DeclPos = Token->GetPosition();

		CDeclarationSpecifier DeclSpec;

		ParseDeclarationSpecifiers(DeclSpec);
		if ((Sym = ParseInitDeclaratorList(DeclSpec))->GetSymbolType() != SYMBOL_TYPE_FUNCTION) {
			if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
				throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
					+ " after declaration, got " + Token->GetStringifiedType(), Token->GetPosition());
			}

			NextToken();
			continue;
		}

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
			NextToken();
			continue;
		}

		if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
		}

		CFunctionSymbol *FuncSym = SymbolTableStack.GetGlobal()->GetFunction(Sym->GetName());

		if (FuncSym->GetType()->GetComplete()) {
			throw CParserException("function redefined: `" + FuncSym->GetName() + "`", Token->GetPosition());
		}

		ScopeType.push(SCOPE_TYPE_FUNCTION);
		SymbolTableStack.Push(FuncSym->GetArgumentsSymbolTable());

		CurrentFunction = FuncSym;

		FuncSym->SetBody(ParseBlock());

		SymbolTableStack.Pop();
		ScopeType.pop();

		for (map<string, CLabelInfo>::iterator it = LabelTable.begin(); it != LabelTable.end(); ++it) {
			if (!it->second.Label) {
				throw CParserException("label `" + it->first + "` used but not defined", it->second.Position);
			}
		}

		LabelTable.clear();

	}

	return SymbolTableStack.GetGlobal();
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

const CToken* CParser::GetToken() const
{
	return Token;
}

bool CParser::TryParseDeclaration()
{
	return (KeywordTraits::IsTypeKeyword(Token->GetText()) || SymbolTableStack.LookupType(Token->GetText()));
}

void CParser::ParseDeclaration()
{
	CDeclarationSpecifier DeclSpec;

	ParseDeclarationSpecifiers(DeclSpec);

	ParseInitDeclaratorList(DeclSpec);

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after declaration, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
}

void CParser::ParseTypedefSpecifier(CDeclarationSpecifier &DeclSpec)
{
	if (DeclSpec.Typedef) {
		throw CParserException("duplicate `typedef` specifier", Token->GetPosition());
	}

	DeclSpec.Typedef = true;

	NextToken();
}

void CParser::ParseStructSpecifier(CDeclarationSpecifier &DeclSpec)
{
	CheckMultipleTypeSpecifiers(DeclSpec);

	NextToken();

	string Tag;

	if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		Tag = Token->GetText();
		NextToken();
	} else {
		if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
				+ " after `struct`, got " + Token->GetStringifiedType(), Token->GetPosition());
		}

		Tag = "<anonymous_struct_" + ToString(++AnonymousTagCounter) + ">";
	}

	if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
		DeclSpec.Type = SymbolTableStack.LookupTag(Tag);
		if (!DeclSpec.Type) {
			throw CParserException("undeclared struct tag", Token->GetPosition());
		}
		return;
	}

	NextToken();

	CStructSymbol *StructSym = new CStructSymbol;
	StructSym->SetName(Tag);
	SymbolTableStack.GetTop()->AddTag(StructSym);
	SymbolTableStack.GetTop()->AddType(StructSym);

	CStructSymbolTable *StructSymTable = new CStructSymbolTable;
	StructSym->SetSymbolTable(StructSymTable);

	SymbolTableStack.Push(StructSymTable);
	ScopeType.push(SCOPE_TYPE_STRUCT);

	do {
		ParseDeclaration();
	} while (Token->GetType() != TOKEN_TYPE_BLOCK_END);

	NextToken();

	ScopeType.pop();
	SymbolTableStack.Pop();

	StructSym->SetComplete(true);

	DeclSpec.Type = StructSym;
}

void CParser::CheckMultipleTypeSpecifiers(CDeclarationSpecifier &DeclSpec)
{
	if (DeclSpec.Type) {
		throw CParserException("multiple data types in declaration specifiers", Token->GetPosition());
	}
}

void CParser::ParseDeclarationSpecifiers(CDeclarationSpecifier &DeclSpec)
{
	ETokenType type;
	string text;
	CPosition pos;
	CTypeSymbol *TypeSym = NULL;

	bool IdentFound = false;

	while (!TryParseDeclarator() && !IdentFound) {
		type = Token->GetType();
		text = Token->GetText();
		pos = Token->GetPosition();

		if (type == TOKEN_TYPE_KEYWORD) {
			if (text == "typedef") {
				ParseTypedefSpecifier(DeclSpec);
			} else if (text == "struct") {
				ParseStructSpecifier(DeclSpec);
			} else if (text == "const") {
				DeclSpec.Const = true;
				NextToken();
			} else {
				throw CParserException("unexpected `" + text + "` in declaration", pos);
			}

		} else if (type == TOKEN_TYPE_IDENTIFIER) {
			if (TypeSym = SymbolTableStack.LookupType(text)) {
				CheckMultipleTypeSpecifiers(DeclSpec);
				DeclSpec.Type = TypeSym;

				NextToken();

			} else {
				IdentFound = true;
			}
		} else {
			throw CParserException("unexpected " + Token->GetStringifiedType() + " in declaration", pos);
		}
	}

	if (!DeclSpec.Type) {	// FIXME: standard behvaiour for C compilers is to default type to int, if it's missing.. encountering ';' without a type isn't an error either, just a warning "useless declaration"..
		throw CParserException("expected declaration specifier, got " + Token->GetStringifiedType(), pos);
	} else {
		if (DeclSpec.Const) {
			CTypeSymbol *ConstType = DeclSpec.Type->ConstClone();

			if (CTypeSymbol *OldType = SymbolTableStack.LookupType(ConstType->GetQualifiedName())) {
				delete ConstType;
				ConstType = OldType;
			} else {
				SymbolTableStack.GetTop()->AddType(ConstType);
			}

			DeclSpec.Type = ConstType;
		}
	}
}

bool CParser::TryParseDeclarator()
{
	ETokenType t = Token->GetType();
	return (t == TOKEN_TYPE_OPERATION_ASTERISK || t == TOKEN_TYPE_LEFT_PARENTHESIS || t == TOKEN_TYPE_SEPARATOR_SEMICOLON);
}

CSymbol* CParser::ParseInitDeclaratorList(CDeclarationSpecifier &DeclSpec)
{
	if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		return DeclSpec.Type; // FIXME: we can return NULL... or add separate function for parsing this shit.. or something else..
	}

	CSymbol *LastDeclared;

	ETokenType type;

	do {
		LastDeclared = ParseInitDeclarator(DeclSpec);
		type = Token->GetType();
		NextToken();
	} while (type == TOKEN_TYPE_SEPARATOR_COMMA);

	PreviousToken();

	return LastDeclared;
}

CSymbol* CParser::ParseInitDeclarator(CDeclarationSpecifier &DeclSpec)
{
	CSymbol *Result = ParseDeclarator(DeclSpec);

	if (Token->GetType() == TOKEN_TYPE_OPERATION_ASSIGN) {
		if (Result->GetSymbolType() != SYMBOL_TYPE_VARIABLE) {
			throw CParserException("can't initialize this kind of symbol", Token->GetPosition());
		}

		ParseInitializer(static_cast<CVariableSymbol *>(Result));
	}

	return Result;
}

CVariableSymbol* CParser::AddVariable(const string &Ident, CTypeSymbol *Type)
{
	if (SymbolTableStack.GetTop()->Exists(Ident)) {
		throw CParserException("identifier redeclared: `" + Ident + "`", Token->GetPosition());
	}

	CVariableSymbol *VarSym = new CVariableSymbol(Ident, Type);

	if (ScopeType.top() == SCOPE_TYPE_GLOBAL) {
		VarSym->SetGlobal(true);
	}

	SymbolTableStack.GetTop()->AddVariable(VarSym);

	return VarSym;
}

CFunctionSymbol* CParser::AddFunction(const string &Ident, CTypeSymbol *RetType)
{
	if (ScopeType.top() != SCOPE_TYPE_GLOBAL) {
		throw CParserException("function declaration is allowed only in global scope", Token->GetPosition());
	}
	if (RetType->IsArray()) {
		throw CParserException("can't declare array of functions", Token->GetPosition());
	}

	CSymbol *OldDecl = SymbolTableStack.GetGlobal()->Get(Ident);

	if (OldDecl) {
		if (OldDecl->GetSymbolType() != SYMBOL_TYPE_FUNCTION) {
			throw CParserException("identifier redeclared as different kind of symbol: `" + Ident + "`", Token->GetPosition());
		}

		// TODO: reparse parameter list and check type compatibility

		return static_cast<CFunctionSymbol *>(OldDecl);
	} else {
		CFunctionSymbol *FuncSym = new CFunctionSymbol(Ident, RetType);
		ParseParameterList(FuncSym);
		SymbolTableStack.GetGlobal()->AddFunction(FuncSym);

		return FuncSym;
	}
}

CTypedefSymbol* CParser::AddTypedef(const string &Ident, CTypeSymbol *RefType)
{
	if (ScopeType.top() != SCOPE_TYPE_GLOBAL && ScopeType.top() != SCOPE_TYPE_FUNCTION) {
		throw CParserException("typedef is not allowed here", Token->GetPosition());
	}

	if (SymbolTableStack.GetTop()->Exists(Ident)) {
		throw CParserException("identifier redeclared: `" + Ident + "`", Token->GetPosition());
	}

	CTypedefSymbol *TypedefSym = new CTypedefSymbol(Ident, RefType);
	SymbolTableStack.GetTop()->AddType(TypedefSym);

	return TypedefSym;
}

CSymbol* CParser::ParseDeclarator(CDeclarationSpecifier &DeclSpec)
{
	CTypeSymbol *DeclType = DeclSpec.Type;

	if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
		DeclType = ParsePointer(DeclType);
	}

	if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
			+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	string Ident = Token->GetText();
	NextToken();

	if (Token->GetType() == TOKEN_TYPE_LEFT_SQUARE_BRACKET) {
		DeclType = ParseArray(DeclType);
	}

	CSymbol *Result;

	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		if (DeclSpec.Typedef) {
			throw CParserException("typedef of functions is not supported", Token->GetPosition());
		}

		Result = AddFunction(Ident, DeclType);
	} else {
		if (DeclSpec.Typedef) {
			Result = AddTypedef(Ident, DeclType);
		} else {
			Result = AddVariable(Ident, DeclType);
		}
	}

	return Result;
}

CTypeSymbol* CParser::ParsePointer(CTypeSymbol *ARefType)
{
	NextToken();

	CTypeSymbol *PointerSym = new CPointerSymbol(ARefType);

	while (Token->GetText() == "const") {
		PointerSym->SetConst(true);
		NextToken();
	}

	PointerSym = FilterDuplicates(PointerSym);

	if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
		return ParsePointer(PointerSym);
	}

	return PointerSym;
}

CTypeSymbol* CParser::ParseArray(CTypeSymbol *AElemType)
{
	NextToken();

	CArraySymbol *Sym = new CArraySymbol;

	if (Token->GetType() != TOKEN_TYPE_CONSTANT_INTEGER) { // FIXME: const expression, probably, eh?..
		throw CParserException("expected array length integer constant, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	Sym->SetLength(Token->GetIntegerValue());

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_RIGHT_SQUARE_BRACKET) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET]
			+ "after array length, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	Sym->SetElementsType(AElemType);

	if (Token->GetType() == TOKEN_TYPE_LEFT_SQUARE_BRACKET) {
		Sym->SetElementsType(ParseArray(AElemType));
	}

	return FilterDuplicates(Sym);
}

void CParser::ParseParameterList(CFunctionSymbol *Func)
{
	NextToken();

	CArgumentsSymbolTable *FuncArgs = new CArgumentsSymbolTable;
	Func->SetArgumentsSymbolTable(FuncArgs);

	SymbolTableStack.Push(FuncArgs);
	ScopeType.push(SCOPE_TYPE_PARAMETERS);

	while (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		CDeclarationSpecifier DeclSpec;
		ParseDeclarationSpecifiers(DeclSpec);
		Func->AddArgument(SymbolTableStack.GetTop()->GetVariable(ParseDeclarator(DeclSpec)->GetName()));

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_COMMA) {
			NextToken();
		}
	}

	ScopeType.pop();
	SymbolTableStack.Pop();

	NextToken();
}

CTypeSymbol* CParser::FilterDuplicates(CTypeSymbol *AType)
{
	CTypeSymbol *OldSym = NULL;
	if (OldSym = SymbolTableStack.GetGlobal()->GetType(AType->GetQualifiedName())) {
		delete AType;
		return OldSym;
	}

	SymbolTableStack.GetGlobal()->AddType(AType);
	return AType;
}

void CParser::ParseInitializer(CVariableSymbol *ASymbol)
{
	if (!ASymbol->GetType()->IsScalar()) {
		throw CParserException("can't initialize this kind of symbol", Token->GetPosition());
	}

	if (Blocks.empty()) {
		NextToken();

		// TODO: add support for initialization of global variables with const expressions, like:
		// 	x:
		// 		.long	24
		// 	y:
		// 		.float	56.42
		// or something like this
		throw CParserException("initialization of global variables is not implemented yet", Token->GetPosition());
	}

	CBinaryOp *Op = new CBinaryOp(CToken(TOKEN_TYPE_OPERATION_ASSIGN, "=", Token->GetPosition()));

	NextToken();

	Op->SetLeft(new CVariable(CToken(TOKEN_TYPE_IDENTIFIER, ASymbol->GetName(), CPosition()), static_cast<CVariableSymbol *>(ASymbol)));
	Op->SetRight(ParseConditional());

	Op->CheckTypes();

	Blocks.top()->Add(Op);
}

















CStatement* CParser::ParseStatement()
{
	if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		string LabelName = Token->GetText();

		NextToken();
		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_COLON) {
			NextToken();
			CLabel *Label = new CLabel(LabelName, ParseStatement());
			LabelTable[LabelName] = CLabelInfo(Label);
			return Label;
		}
		PreviousToken();
	}

	ETokenType type = Token->GetType();
	if (type == TOKEN_TYPE_KEYWORD && Token->GetText() != "sizeof") {
		// possibly remove this temp 'result' var..
		CStatement *result = NULL;
		string text = Token->GetText();
		if (text == "if") {
			result = ParseIf();
		} else if (text == "for") {
			result = ParseFor();
		} else if (text == "while") {
			result = ParseWhile();
		} else if (text == "do") {
			result = ParseDo();
		} else if (text == "case") {
			result = ParseCase();
		} else if (text == "default") {
			result = ParseDefault();
		} else if (text == "goto") {
			result = ParseGoto();
		} else if (text == "break") {
			result = ParseBreak();
		} else if (text == "continue") {
			result = ParseContinue();
		} else if (text == "return") {
			result = ParseReturn();
		} else if (text == "switch") {
			result = ParseSwitch();
		}
		return result;
	} else if (type == TOKEN_TYPE_BLOCK_START) {
		return ParseBlock();
	} else if (type == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		NextToken();
		return new CNullStatement;
	} else {
		CStatement *Expr = ParseExpression();
		if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
				+ " after expression, got " + Token->GetStringifiedType(), Token->GetPosition());
		}
		NextToken();
		return Expr;
	}
}

CBlockStatement* CParser::ParseBlock()
{
	NextToken();

	CBlockStatement *Stmt = new CBlockStatement;

	CSymbolTable *BlockSymTable = new CSymbolTable;
	Stmt->SetSymbolTable(BlockSymTable);

	if (typeid(*SymbolTableStack.GetTop()) != typeid(CArgumentsSymbolTable)) {
		BlockSymTable->SetCurrentOffset(SymbolTableStack.GetTop()->GetCurrentOffset());
	}

	SymbolTableStack.Push(BlockSymTable);
	Blocks.push(Stmt);

	while (TryParseDeclaration())
	{
		ParseDeclaration();
	}

	while (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		if (Token->GetType() == TOKEN_TYPE_EOF) {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
				+ ", got " + CScanner::TokenTypesNames[TOKEN_TYPE_EOF], Token->GetPosition());
		}

		Stmt->Add(ParseStatement());
	}

	Blocks.pop();
	SymbolTableStack.Pop();

	if (!Blocks.empty()) {
		Blocks.top()->AddNestedBlock(Stmt);
	}

	NextToken();

	return Stmt;
}

CStatement* CParser::ParseIf()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `if`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	CIfStatement *Stmt = new CIfStatement;

	NextToken();
	CPosition CondPos = Token->GetPosition();
	Stmt->SetCondition(ParseExpression());

	if (!Stmt->GetCondition()->GetResultType()->IsScalar()) {
		throw CParserException("if-condition expression should have scalar type", CondPos);
	}

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after if-condition expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	Stmt->SetThenStatement(ParseStatement());

	if (Token->GetType() == TOKEN_TYPE_KEYWORD && Token->GetText() == "else") {
		NextToken();
		Stmt->SetElseStatement(ParseStatement());
	}

	return Stmt;
}

CStatement* CParser::ParseFor()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `for`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	CForStatement *Stmt = new CForStatement;

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		Stmt->SetInit(ParseExpression());
	}

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after for-init expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		CPosition CondPos = Token->GetPosition();
		Stmt->SetCondition(ParseExpression());

		if (!Stmt->GetCondition()->GetResultType()->IsScalar()) {
			throw CParserException("for-condition expression should have scalar type", CondPos);
		}
	}

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after for-condition expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		Stmt->SetUpdate(ParseExpression());
	}

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after for-update expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();

	BlockType.push(BLOCK_TYPE_LOOP);
	Stmt->SetBody(ParseStatement());
	BlockType.pop();

	return Stmt;
}

CStatement* CParser::ParseWhile()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `while`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	CWhileStatement *Stmt = new CWhileStatement;

	NextToken();
	CPosition CondPos = Token->GetPosition();
	Stmt->SetCondition(ParseExpression());

	if (!Stmt->GetCondition()->GetResultType()->IsScalar()) {
		throw CParserException("while-condition expression should have scalar type", CondPos);
	}

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after while-condition expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();

	BlockType.push(BLOCK_TYPE_LOOP);
	Stmt->SetBody(ParseStatement());
	BlockType.pop();

	return Stmt;
}

CStatement* CParser::ParseDo()
{
	NextToken();
	CDoStatement *Stmt = new CDoStatement;

	BlockType.push(BLOCK_TYPE_LOOP);
	Stmt->SetBody(ParseStatement());
	BlockType.pop();

	if (Token->GetType() != TOKEN_TYPE_KEYWORD || Token->GetText() != "while") {
		throw CParserException("expected `while` after do loop body, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `while`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	CPosition CondPos = Token->GetPosition();
	Stmt->SetCondition(ParseExpression());

	if (!Stmt->GetCondition()->GetResultType()->IsScalar()) {
		throw CParserException("do-while-condition expression should have scalar type", CondPos);
	}

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after do-while-condition expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS] + ", got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	return Stmt;
}

CStatement* CParser::ParseCase()
{
	if (BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CParserException("unexpected `case` encountered outside of switch", Token->GetPosition());
	}

	NextToken();
	CPosition ExprPos = Token->GetPosition();

	CCaseLabel *CaseLabel = new CCaseLabel(ParseExpression());

	if (!(CaseLabel->GetCaseExpression()->GetResultType()->IsInt() && CaseLabel->GetCaseExpression()->IsConst())) {
		throw CParserException("expected constant integer expression after `case`", ExprPos);
	}

	// FIXME: check _values_, not pointers to expressions... OMG OMG, compute expressions at compile-time?!.. CCompileTimeComputerVisitor comes to imagination, LOL..
	if (SwitchesStack.top()->Exists(CaseLabel)) {
		throw CParserException("duplicate case-expression", ExprPos);
	}

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_COLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON]
			+ " after case-expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	CaseLabel->SetNext(ParseStatement());

	SwitchesStack.top()->AddCase(CaseLabel);
	
	return CaseLabel;
}

CStatement* CParser::ParseDefault()
{
	if (BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CParserException("unexpected `default` encountered outside of switch", Token->GetPosition());
	}

	if (SwitchesStack.top()->GetDefaultCase()) {
		throw CParserException("multiple `default` labels in one switch", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_COLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON]
			+ " after `default`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	CDefaultCaseLabel *DefCaseLabel = new CDefaultCaseLabel(ParseStatement());

	SwitchesStack.top()->SetDefaultCase(DefCaseLabel);

	return DefCaseLabel;
}

CStatement* CParser::ParseGoto()
{
	NextToken();

	if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
			+ " as goto-label, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	string LabelName = Token->GetText();

	if (!LabelTable.count(LabelName)) {
		LabelTable[LabelName] = CLabelInfo(NULL, Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after goto-label, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	return new CGotoStatement(LabelName);
}

CStatement* CParser::ParseBreak()
{
	if (BlockType.top() != BLOCK_TYPE_LOOP && BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CParserException("unexpected `break` encountered outside of loop or switch", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after `break`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	return new CBreakStatement;
}

CStatement* CParser::ParseContinue()
{
	if (BlockType.top() != BLOCK_TYPE_LOOP) {
		throw CParserException("unexpected `continue` encountered outside of loop", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after `break`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	return new CContinueStatement;
}

CStatement* CParser::ParseReturn()
{
	if (ScopeType.top() != SCOPE_TYPE_FUNCTION) {
		throw CParserException("`return` is only allowed in functions", Token->GetPosition());
	}

	NextToken();
	CReturnStatement *Stmt = new CReturnStatement;

	CPosition ExprPos = Token->GetPosition();

	if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		if (!CurrentFunction->GetReturnType()->IsVoid()) {
			throw CParserException("return with no expression in function returning non-void", ExprPos);
		}

		NextToken();
		return Stmt;
	}

	Stmt->SetReturnExpression(ParseExpression());

	if (!CurrentFunction->GetReturnType()->CompatibleWith(Stmt->GetReturnExpression()->GetResultType())) {
		throw CParserException("return expression type is incompatible with function return type", ExprPos);
	}

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after return expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	return Stmt;
}

CStatement* CParser::ParseSwitch()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `switch`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	CPosition TestExprPos = Token->GetPosition();

	CSwitchStatement *Stmt = new CSwitchStatement(ParseExpression());

	if (!Stmt->GetTestExpression()->GetResultType()->IsInt()) {
		throw CParserException("switch-test-expression should have integer type", TestExprPos);
	}

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after switch-test-expression, got " + Token->GetStringifiedType(), Token->GetPosition());
	}
	NextToken();

	SwitchesStack.push(Stmt);
	BlockType.push(BLOCK_TYPE_SWITCH);
	Stmt->SetBody(ParseStatement());
	BlockType.pop();
	SwitchesStack.pop();

	return Stmt;
}

CExpression* CParser::ParseAssignment()
{
	CExpression *Expr = ParseConditional();

	if (TokenTraits::IsAssignment(Token->GetType())) {
		CBinaryOp *Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseAssignment());

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
		}

		NextToken();

		Op->SetFalseExpr(ParseConditional());

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

		Op->SetResultType(SymbolTableStack.GetGlobal()->GetType("int"));

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

		Op->SetResultType(SymbolTableStack.GetGlobal()->GetType("int"));

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

		Op->SetResultType(SymbolTableStack.GetGlobal()->GetType("int"));

		Expr = Op;
	}

	return Expr;
}

CExpression* CParser::ParseRelationalExpression()
{
	CExpression *Expr = ParseShiftExpression();

	CBinaryOp *Op;

	while (TokenTraits::IsRelational(Token->GetType())) {
		Op = new CBinaryOp(*Token);

		NextToken();

		Op->SetLeft(Expr);
		Op->SetRight(ParseShiftExpression());

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

		Op->SetResultType(SymbolTableStack.GetGlobal()->GetType("int"));

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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
	
		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

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
					throw CParserException("expected RIGHT_PARENTHESIS, got " + Token->GetStringifiedType(), Token->GetPosition());
				}*/
			}
		}
		PreviousToken();
	}

	return ParseUnaryExpression();
}

CExpression* CParser::ParseUnaryExpression()
{
	if (!TokenTraits::IsUnaryOp(Token->GetType()) && Token->GetText() != "sizeof") {
		return ParsePostfixExpression();
	}

	ETokenType type = Token->GetType();

	CUnaryOp *Op;

	if (type == TOKEN_TYPE_OPERATION_AMPERSAND) {
		Op = new CAddressOfOp(*Token);
	} else {
		Op = new CUnaryOp(*Token);
	}

	NextToken();

	CPosition ArgPos = Token->GetPosition();

	if (type == TOKEN_TYPE_OPERATION_INCREMENT || type == TOKEN_TYPE_OPERATION_DECREMENT) {
		Op->SetArgument(ParseUnaryExpression());
	} else  if (type == TOKEN_TYPE_KEYWORD) {
		// TODO: handle sizeof somehow..
		/*if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
			//ParseTypeName();
		}*/
		Op->SetArgument(ParseUnaryExpression());
	} else {
		Op->SetArgument(ParseCastExpression());
	}

	if (Mode != PARSER_MODE_EXPRESSION) {
		Op->CheckTypes();
	}

	if (type == TOKEN_TYPE_OPERATION_LOGIC_NOT) {
		Op->SetResultType(SymbolTableStack.GetGlobal()->GetType("int"));
	}

	return Op;
}

CExpression* CParser::ParsePostfixExpression()
{
	CExpression *Expr = ParsePrimaryExpression();
	CExpression *Op;

	while (TokenTraits::IsPostfix(Token->GetType())) {
		switch (Token->GetType()) {
		case TOKEN_TYPE_OPERATION_DOT:
			{
				CStructAccess *StructAccess = new CStructAccess(*Token, Expr);
				Op = StructAccess;

				NextToken();

				if (Token->GetType() != TOKEN_TYPE_IDENTIFIER) {
					throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
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
					throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
						+ " as right operand of " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_INDIRECT_ACCESS]
						+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
				}

				IndirectAccess->SetField(new CVariable(*Token));
			}
			break;
		case TOKEN_TYPE_LEFT_SQUARE_BRACKET:
			{
				CArrayAccess *ArrayAccess = new CArrayAccess(*Token);
				Op = ArrayAccess;

				ArrayAccess->SetLeft(Expr);

				NextToken();

				ArrayAccess->SetRight(ParseExpression());

				if (Token->GetType() != TOKEN_TYPE_RIGHT_SQUARE_BRACKET) {
					throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET]
						+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
				}
			}
			break;
		case TOKEN_TYPE_OPERATION_INCREMENT:
		case TOKEN_TYPE_OPERATION_DECREMENT:
			{
				Op = new CPostfixOp(*Token, Expr);
			}
			break;
		case TOKEN_TYPE_LEFT_PARENTHESIS:
			{
				CFunctionSymbol *FuncSym;

				if (CFunction *FuncExpr = dynamic_cast<CFunction *>(Expr)) {
					FuncSym = FuncExpr->GetSymbol();
				} else {
					if (Mode == PARSER_MODE_EXPRESSION && Expr->GetType() == TOKEN_TYPE_IDENTIFIER) {
						if (!(FuncSym = SymbolTableStack.GetGlobal()->GetFunction(Expr->GetName()))) {
							FuncSym = new CFunctionSymbol(Expr->GetName(), SymbolTableStack.GetGlobal()->GetType("int"));
							SymbolTableStack.GetGlobal()->AddFunction(FuncSym);
						}
					} else {
						throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
							+ " before function call operator", Token->GetPosition());
					}
				}

				CFunctionCall *FuncCall = new CFunctionCall(*Token, FuncSym);

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
					throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
						+ " after function arguments list, got " + Token->GetStringifiedType(), Token->GetPosition());
				}

				delete Expr;
			}
			break;
		}

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
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
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
		}
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_INTEGER) {
		Expr = new CIntegerConst(*Token, SymbolTableStack.GetGlobal()->GetType("int"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_FLOAT) {
		Expr = new CFloatConst(*Token, SymbolTableStack.GetGlobal()->GetType("float"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_CHAR) {
		Expr = new CCharConst(*Token, SymbolTableStack.GetGlobal()->GetType("int"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_STRING) {
		Expr = new CStringConst(*Token, SymbolTableStack.GetGlobal()->GetType("int*"));
	} else if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		CSymbol *Sym = SymbolTableStack.LookupAll(Token->GetText());

		if (!Sym) {
			if (Mode == PARSER_MODE_EXPRESSION) {
				// FIXME FFFFUUUUU~~~... well, this is not SO bad.. leaks are fixed now, and no crashes happen.. so may be i'll left it as is.
				CVariableSymbol *VarSym = new CVariableSymbol(Token->GetText(), SymbolTableStack.GetGlobal()->GetType("int"));
				SymbolTableStack.GetTop()->AddVariable(VarSym);
				Sym = VarSym;
			} else {
				throw CParserException("undeclared identifier `" + Token->GetText() + "`", Token->GetPosition());
			}
		}

		if (Sym->GetSymbolType() == SYMBOL_TYPE_TYPE) {
			throw CParserException("declaration is not allowed here", Token->GetPosition());
		} else if (Sym->GetSymbolType() == SYMBOL_TYPE_VARIABLE) {
			Expr = new CVariable(*Token, static_cast<CVariableSymbol *>(Sym));
		} else if (Sym->GetSymbolType() == SYMBOL_TYPE_FUNCTION) {
			Expr = new CFunction(*Token, static_cast<CFunctionSymbol *>(Sym));
		}
	} else {
		throw CParserException("expected primary-expression, got " + Token->GetStringifiedType(), Token->GetPosition());
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

void CParser::AddBuiltIn(const string &AName, const string &AReturnType, int AArgumentsCount, ...)
{
	CFunctionSymbol *FuncSym = new CFunctionSymbol(AName, SymbolTableStack.LookupType(AReturnType));
	FuncSym->SetArgumentsSymbolTable(new CArgumentsSymbolTable);
	FuncSym->SetBuiltIn(true);

	va_list vl;
	va_start(vl, AArgumentsCount);

	CVariableSymbol *Arg;

	for (int i = 0; i < AArgumentsCount; i++) {
		Arg = new CVariableSymbol("", SymbolTableStack.LookupType(va_arg(vl, const char *)));
		FuncSym->GetArgumentsSymbolTable()->AddVariable(Arg);
		FuncSym->AddArgument(Arg);
	}

	va_end(vl);

	SymbolTableStack.GetGlobal()->AddFunction(FuncSym);
}
