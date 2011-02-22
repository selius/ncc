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
 * CParser::CDeclarationSpecifier
 ******************************************************************************/

CParser::CDeclarationSpecifier::CDeclarationSpecifier() : Type(NULL), Typedef(false)
{
}

/******************************************************************************
 * CParser
 ******************************************************************************/

CParser::CParser(CScanner &AScanner) : TokenStream(AScanner)
{
	NextToken();

	CSymbolTable *GlobalSymTable = new CSymbolTable;
	GlobalSymTable->Add(new CIntegerSymbol);
	GlobalSymTable->Add(new CFloatSymbol);
	GlobalSymTable->Add(new CVoidSymbol);

	SymbolTableStack.Push(GlobalSymTable);

	

	// FIXME: remove this test code..
	/*CFunctionSymbol *main_sym_test = new CFunctionSymbol;
	main_sym_test->SetName("main");
	SymbolTableStack.GetTop()->Add(main_sym_test);*/

	BlockType.push(BLOCK_TYPE_DEFAULT);
	ScopeType.push(SCOPE_TYPE_GLOBAL);
}

CParser::~CParser()
{
	delete SymbolTableStack.Pop();
}

CSymbolTable* CParser::ParseTranslationUnit()
{
	CSymbol *Sym = NULL;

	while (Token->GetType() != TOKEN_TYPE_EOF) {
		Sym = ParseDeclarator(ParseDeclarationSpecifier(), false);
		if (!Sym) {
			throw CException("expected declaration or function definition, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
		}

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
			if (SymbolTableStack.GetTop()->Exists(Sym->GetName())) {
				throw CException("identifier redeclared: `" + Sym->GetName() + "`", Token->GetPosition()); // FIXME: position
			}

			SymbolTableStack.GetTop()->Add(Sym);
			NextToken();
		} else if (Token->GetType() == TOKEN_TYPE_BLOCK_START) {
			CFunctionSymbol *FuncSym = dynamic_cast<CFunctionSymbol *>(Sym);
			if (!FuncSym) {
				throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
					+ ", got " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START], Token->GetPosition());
			}

			if (SymbolTableStack.GetTop()->Exists(FuncSym->GetName())) {
				FuncSym = dynamic_cast<CFunctionSymbol *>(SymbolTableStack.GetTop()->Get(Sym->GetName()));
				if (!FuncSym) {
					throw CException("identifier redeclared as different kind of symbol: `" + Sym->GetName() + "`", Token->GetPosition()); // FIXME: position
				}

				delete Sym;
			} else {
				SymbolTableStack.GetTop()->Add(FuncSym);
			}

			ScopeType.push(SCOPE_TYPE_FUNCTION);
			SymbolTableStack.Push(FuncSym->GetArgumentsSymbolTable());

			FuncSym->SetBody(ParseBlock());

			SymbolTableStack.Pop();
			ScopeType.pop();
			LabelTable.clear();
		} else {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START]
				+ ", got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
		}

	}

	return SymbolTableStack.GetTop();
}

bool IsTypeKeyword(const string &s)
{
	return (s == "const"|| s == "struct" || s == "typedef");
}

bool CParser::ParseDeclaration()
{
	CDeclarationSpecifier DeclSpec = ParseDeclarationSpecifier();
	if (!DeclSpec.Type) {
		return false;
	}

	CSymbol *Sym;

	ETokenType type;
	CPosition pos;

	do {
		Sym = ParseDeclarator(DeclSpec);
		SymbolTableStack.GetTop()->Add(Sym);

		// TODO: add initialization support - add a pointer to the corresponding block somewhere and add assignment statements to it..

		type = Token->GetType();
		pos = Token->GetPosition();
		NextToken();
	} while (type == TOKEN_TYPE_SEPARATOR_COMMA);

	if (type != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after declaration, got " + CScanner::TokenTypesNames[type], pos);
	}

	return true;
}

CParser::CDeclarationSpecifier CParser::ParseDeclarationSpecifier()
{
	CDeclarationSpecifier Result;

	ETokenType type = Token->GetType();
	string text = Token->GetText();
	CPosition pos = Token->GetPosition();


	if (type == TOKEN_TYPE_KEYWORD) {
		if (!IsTypeKeyword(text)) {
			return Result;
		}
	} else {
		if (!SymbolTableStack.Lookup<CTypeSymbol>(text)) {
			return Result;
		}
	}

	bool Const = false;

	while (true) {
		type = Token->GetType();
		text = Token->GetText();
		pos = Token->GetPosition();

		if (type == TOKEN_TYPE_KEYWORD) {
			if (text == "const") {
				if (Const) {
					throw CException("duplicate specifier: `" + text + "`", pos);
				}

				Const = true;
			} else if (text == "typedef") {
				if (Result.Typedef) {
					throw CException("duplicate specifier: `" + text + "`", pos);
				}

				Result.Typedef = true;
			} else if (text == "struct") {
				if (Result.Type) {
					throw CException("multiple data types in declaration", pos);
				}

				Result.Type = ParseStruct();
			} else {
				throw CException("unexpected `" + text + "` in declaration", pos);
			}
		} else if (type == TOKEN_TYPE_IDENTIFIER) {
			CTypeSymbol *IdentSym = SymbolTableStack.Lookup<CTypeSymbol>(text);
			if (IdentSym) {
				if (Result.Type) {
					throw CException("multiple data types in declaration", pos);
				}

				Result.Type = IdentSym;
			} else {
				if (!Result.Type) {
					throw CException("expected type specifier, got undeclared identifier: `" + text + "`", pos);
				}

				Result.Type->SetConst(Const);
				return Result;
			}
		} else if (type == TOKEN_TYPE_OPERATION_ASTERISK) {
			if (!Result.Type) {
				throw CException("expected type specifier, got " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK], pos);
			}

			Result.Type->SetConst(Const);
			return Result;
		} else {
			throw CException("unexpected " + CScanner::TokenTypesNames[type] + " in declaration", pos);
		}

		NextToken();
	}
}

CSymbol* CParser::ParseDeclarator(CParser::CDeclarationSpecifier DeclSpec, bool CheckExistense /*= true*/)
{
	if (!DeclSpec.Type) {
		return NULL;
	}

	CSymbol *Result = NULL;
	bool IdentifierFound = false;
	string text;

	while (!IdentifierFound) {
		text = Token->GetText();

		if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
			if (CheckExistense && SymbolTableStack.GetTop()->Exists(text)) {
				throw CException("identifier redeclared: `" + text + "`", Token->GetPosition());
			}

			IdentifierFound = true;

		} else if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
			DeclSpec.Type = ParsePointer(DeclSpec.Type);
		}

		NextToken();
	}

	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		if (ScopeType.top() != SCOPE_TYPE_GLOBAL) {
			throw CException("function declaration is allowed only in global scope", Token->GetPosition());
		}

		CFunctionSymbol *FuncSym = new CFunctionSymbol;
		FuncSym->SetName(text);
		FuncSym->SetReturnType(DeclSpec.Type);

		ParseParameterList(FuncSym);

		Result = FuncSym;
	} else {
		if (Token->GetType() == TOKEN_TYPE_LEFT_SQUARE_BRACKET) {
			if (ScopeType.top() == SCOPE_TYPE_PARAMETERS || ScopeType.top() == SCOPE_TYPE_STRUCT) {
				throw CException("array declaration is not allowed here", Token->GetPosition());
			}

			DeclSpec.Type = ParseArray(DeclSpec.Type);
		}

		if (DeclSpec.Typedef) {
			if (ScopeType.top() == SCOPE_TYPE_PARAMETERS || ScopeType.top() == SCOPE_TYPE_STRUCT) {
				throw CException("typedef is not allowed here", Token->GetPosition());
			}

			CTypedefSymbol *TypedefSym = new CTypedefSymbol;
			TypedefSym->SetName(text);
			TypedefSym->SetRefType(DeclSpec.Type);
			Result = TypedefSym;
		} else {
			CVariableSymbol *VarSym = new CVariableSymbol;
			VarSym->SetName(text);
			VarSym->SetType(DeclSpec.Type);
			Result = VarSym;
		}
	}

	return Result;
}

void CParser::ParseParameterList(CFunctionSymbol *Func)
{
	NextToken();

	SymbolTableStack.Push(new CSymbolTable);
	Func->SetArgumentsSymbolTable(SymbolTableStack.GetTop());

	ScopeType.push(SCOPE_TYPE_PARAMETERS);

	while (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		SymbolTableStack.GetTop()->Add(ParseDeclarator(ParseDeclarationSpecifier()));
	}

	ScopeType.pop();

	SymbolTableStack.Pop();

	NextToken();
}

CPointerSymbol* CParser::ParsePointer(CTypeSymbol *ARefType)
{
	NextToken();

	CPointerSymbol *Sym = new CPointerSymbol;

	if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
		Sym->SetRefType(ParsePointer(ARefType));
	} else {
		Sym->SetRefType(ARefType);
		PreviousToken();
	}

	return Sym;

	// TODO: possibly add support of const pointers..
	/*if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		Sym->SetRefType(ARefType);
		PreviousToken();
		return Sym;
	} else if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
		CPointerSymbol *PointerSym = ParsePointer(ARefType);
		PointerSym->SetRefType(Sym);
		return PointerSym;
	}

	throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
		+ ", got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());*/
}

CArraySymbol* CParser::ParseArray(CTypeSymbol *AElemType)
{
	NextToken();

	CArraySymbol *Sym = new CArraySymbol;

	if (Token->GetType() != TOKEN_TYPE_CONSTANT_INTEGER) {
		throw CException("expected array length integer constant, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	Sym->SetLength(Token->GetIntegerValue());

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_RIGHT_SQUARE_BRACKET) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET]
			+ "after array length, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	Sym->SetElementsType(AElemType);

	if (Token->GetType() == TOKEN_TYPE_LEFT_SQUARE_BRACKET) {
		Sym->SetElementsType(ParseArray(AElemType));
	}

	return Sym;
}

CStructSymbol* CParser::ParseStruct()
{
	NextToken();

	CStructSymbol *Sym = NULL;

	string StructName;

	if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		StructName = Token->GetText();
		NextToken();

		if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
			Sym = SymbolTableStack.Lookup<CStructSymbol>(StructName);
			if (!Sym) {
				throw CException("undeclared identifier `" + StructName + "`", Token->GetPosition());
			}

			return Sym;
		}

		if (SymbolTableStack.GetTop()->Get(StructName)) {
			throw CException("identifier redeclared: `" + StructName + "`", Token->GetPosition());
		}

	} else  if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
			+ " after `struct`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	Sym = new CStructSymbol;

	if (!StructName.empty()) {
		Sym->SetName(StructName);
		SymbolTableStack.GetTop()->Add(Sym);
	}

	NextToken();

	SymbolTableStack.Push(new CSymbolTable);

	ScopeType.push(SCOPE_TYPE_STRUCT);

	while (ParseDeclaration());

	ScopeType.pop();

	Sym->SetSymbolTable(SymbolTableStack.Pop());

	if (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
			+ " after struct-declaration-list, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	//NextToken();

	return Sym;
}

CStatement* CParser::ParseStatement()
{
	if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		string LabelName = Token->GetText();

		NextToken();
		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_COLON) {
			NextToken();
			CLabel *Label = new CLabel(LabelName);
			LabelTable[LabelName] = Label;
			Label->SetNext(ParseStatement());
			return Label;
		}
		PreviousToken();
	}

	ETokenType type = Token->GetType();
	if (type == TOKEN_TYPE_KEYWORD) {
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
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
				+ " after expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
		}
		NextToken();
		return Expr;
	}
}

CBlockStatement* CParser::ParseBlock()
{
	NextToken();

	CBlockStatement *Stmt = new CBlockStatement;

	SymbolTableStack.Push(new CSymbolTable);

	while (ParseDeclaration());

	while (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		if (Token->GetType() == TOKEN_TYPE_EOF) {
			throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
				+ ", got " + CScanner::TokenTypesNames[TOKEN_TYPE_EOF], Token->GetPosition());
		}

		Stmt->Add(ParseStatement());
	}

	Stmt->SetSymbolTable(SymbolTableStack.Pop());

	NextToken();

	return Stmt;
}

CStatement* CParser::ParseIf()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `if`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	CIfStatement *Stmt = new CIfStatement;

	NextToken();
	Stmt->SetCondition(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after if-condition expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
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
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `for`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	CForStatement *Stmt = new CForStatement;

	NextToken();
	Stmt->SetInit(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after for-init expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	NextToken();
	Stmt->SetCondition(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after for-condition expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	NextToken();
	Stmt->SetUpdate(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after for-update expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
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
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `while`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	CWhileStatement *Stmt = new CWhileStatement;

	NextToken();
	Stmt->SetCondition(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after while-condition expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
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
		throw CException("expected `while` after do loop body, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `while`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	NextToken();
	Stmt->SetCondition(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after while-condition expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS] + ", got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	return Stmt;
}

CStatement* CParser::ParseCase()
{
	if (BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CException("unexpected `case` encountered outside of switch", Token->GetPosition());
	}

	NextToken();

	CCaseLabel *CaseLabel = new CCaseLabel;

	CaseLabel->SetCaseExpression(ParseExpression());	// TODO: check type: integer constant

	// FIXME: check _values_, not pointers to expressions...
	if (SwitchesStack.top()->Exists(CaseLabel)) {
		throw CException("duplicate case-expression", Token->GetPosition());	// FIXME: possibly fix this position, make it point to the beginning of the expression
	}

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_COLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON]
			+ " after case-expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	CaseLabel->SetNext(ParseStatement());

	SwitchesStack.top()->AddCase(CaseLabel);
	
	return CaseLabel;
}

CStatement* CParser::ParseDefault()
{
	if (BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CException("unexpected `default` encountered outside of switch", Token->GetPosition());
	}

	if (SwitchesStack.top()->GetDefaultCase()) {
		throw CException("multiple `default` labels in one switch", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_COLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON]
			+ " after `default`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
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
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
			+ " as goto-label, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}

	string LabelName = Token->GetText();

	if (!LabelTable.count(LabelName)) {
		throw CException("label `" + LabelName + "` used but not defined", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after goto-label, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	return new CGotoStatement(LabelName);
}

CStatement* CParser::ParseBreak()
{
	if (BlockType.top() != BLOCK_TYPE_LOOP && BlockType.top() != BLOCK_TYPE_SWITCH) {
		throw CException("unexpected `break` encountered outside of loop or switch", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after `break`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	return new CBreakStatement;
}

CStatement* CParser::ParseContinue()
{
	if (BlockType.top() != BLOCK_TYPE_LOOP) {
		throw CException("unexpected `continue` encountered outside of loop", Token->GetPosition());
	}

	NextToken();
	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after `break`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	return new CContinueStatement;
}

CStatement* CParser::ParseReturn()
{
	if (ScopeType.top() != SCOPE_TYPE_FUNCTION) {
		throw CException("`return` is only allowed in functions", Token->GetPosition());
	}

	NextToken();
	CReturnStatement *Stmt = new CReturnStatement;

	if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		NextToken();
		return Stmt;
	}

	Stmt->SetReturnExpression(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
			+ " after return expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	return Stmt;
}

CStatement* CParser::ParseSwitch()
{
	NextToken();
	if (Token->GetType() != TOKEN_TYPE_LEFT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS]
			+ " after `switch`, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	CSwitchStatement *Stmt = new CSwitchStatement;
	Stmt->SetTestExpression(ParseExpression());

	if (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		throw CException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS]
			+ " after switch-test-expression, got " + CScanner::TokenTypesNames[Token->GetType()], Token->GetPosition());
	}
	NextToken();

	SwitchesStack.push(Stmt);
	BlockType.push(BLOCK_TYPE_SWITCH);
	Stmt->SetBody(ParseStatement());
	BlockType.pop();
	SwitchesStack.pop();

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
		// TODO: add check for symbol type.. output "declaration is not allowed here" if it's type symbol
		CSymbol *Sym = SymbolTableStack.Lookup<CSymbol>(Token->GetText());
		if (!Sym) {
			throw CException("undeclared identifier `" + Token->GetText() + "`", Token->GetPosition());
		}

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

