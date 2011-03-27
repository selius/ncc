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

CParser::CDeclarationSpecifier::CDeclarationSpecifier() : Type(NULL), Typedef(false)
{
}

/******************************************************************************
 * CParser
 ******************************************************************************/

CParser::CParser(CScanner &AScanner, EParserMode AMode /*= PARSER_MODE_NORMAL*/) : TokenStream(AScanner), CurrentFunction(NULL), Mode(AMode)
{
	NextToken();

	CSymbolTable *GlobalSymTable = new CSymbolTable;

	SymbolTableStack.Push(GlobalSymTable);

	GlobalSymTable->Add(new CIntegerSymbol);
	GlobalSymTable->Add(new CFloatSymbol);
	GlobalSymTable->Add(new CVoidSymbol);
	GlobalSymTable->Add(new CPointerSymbol(SymbolTableStack.Lookup<CTypeSymbol>("int")));

	AddBuiltIn("__print_int", "void", 1, "int");
	AddBuiltIn("__print_float", "void", 1, "float");

	BlockType.push(BLOCK_TYPE_DEFAULT);
	ScopeType.push(SCOPE_TYPE_GLOBAL);
}

CParser::~CParser()
{
	delete SymbolTableStack.Pop();
}

CSymbolTable* CParser::ParseTranslationUnit()
{
	CDeclarationSpecifier DeclSpec;
	CSymbol *Sym = NULL;

	while (Token->GetType() != TOKEN_TYPE_EOF) {
		CPosition DeclPos = Token->GetPosition();

		DeclSpec = ParseDeclarationSpecifier();

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON && DeclSpec.Type && DeclSpec.Type->IsStruct()) {
			NextToken();
			continue;
		}

		Sym = ParseDeclarator(DeclSpec, false);
		if (!Sym) {
			throw CParserException("expected declaration or function definition, got " + Token->GetStringifiedType(), Token->GetPosition());
		}

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_SEMICOLON) {
			if (SymbolTableStack.GetGlobal()->Exists(Sym->GetName())) {
				throw CParserException("identifier redeclared: `" + Sym->GetName() + "`", DeclPos);
			}

			SymbolTableStack.GetGlobal()->Add(Sym);

			if (CVariableSymbol *VarSym = dynamic_cast<CVariableSymbol *>(Sym)) {
				VarSym->SetGlobal(true);
			}
			NextToken();
		} else if (Token->GetType() == TOKEN_TYPE_BLOCK_START) {
			CFunctionSymbol *FuncSym = dynamic_cast<CFunctionSymbol *>(Sym);
			if (!FuncSym) {
				throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
					+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
			}

			if (SymbolTableStack.GetTop()->Exists(FuncSym->GetName())) {
				FuncSym = dynamic_cast<CFunctionSymbol *>(SymbolTableStack.GetTop()->Get(Sym->GetName()));
				if (!FuncSym) {
					throw CParserException("identifier redeclared as different kind of symbol: `" + Sym->GetName() + "`", DeclPos);
				}

				// TODO: check declaration and definition types equality

				delete Sym;
			} else {
				SymbolTableStack.GetGlobal()->Add(FuncSym);
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
		} else {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START]
				+ ", got " + Token->GetStringifiedType(), Token->GetPosition());
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

	ETokenType type = Token->GetType();
	CPosition pos;

	if (type != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		do {
			Sym = ParseDeclarator(DeclSpec);
			SymbolTableStack.GetTop()->Add(Sym);

			// TODO: add initialization support - add a pointer to the corresponding block somewhere and add assignment statements to it..

			type = Token->GetType();
			pos = Token->GetPosition();
			NextToken();
		} while (type == TOKEN_TYPE_SEPARATOR_COMMA);
	} else {
		NextToken();
	}

	if (type != TOKEN_TYPE_SEPARATOR_SEMICOLON) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON]
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
					throw CParserException("duplicate specifier: `" + text + "`", pos);
				}

				Const = true;
			} else if (text == "typedef") {
				if (Result.Typedef) {
					throw CParserException("duplicate specifier: `" + text + "`", pos);
				}

				Result.Typedef = true;
			} else if (text == "struct") {
				if (Result.Type) {
					throw CParserException("multiple data types in declaration", pos);
				}

				Result.Type = ParseStruct();
			} else {
				throw CParserException("unexpected `" + text + "` in declaration", pos);
			}
		} else if (type == TOKEN_TYPE_IDENTIFIER) {
			CTypeSymbol *IdentSym = SymbolTableStack.Lookup<CTypeSymbol>(text);
			if (IdentSym) {
				if (Result.Type) {
					throw CParserException("multiple data types in declaration", pos);
				}

				Result.Type = IdentSym;
			} else {
				if (!Result.Type) {
					throw CParserException("expected type specifier, got undeclared identifier: `" + text + "`", pos);
				}

				Result.Type->SetConst(Const);
				return Result;
			}
		} else if (type == TOKEN_TYPE_OPERATION_ASTERISK) {
			if (!Result.Type) {
				throw CParserException("expected type specifier, got " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK], pos);
			}

			Result.Type->SetConst(Const);
			return Result;
		} else {
			if (type == TOKEN_TYPE_SEPARATOR_SEMICOLON && Result.Type && Result.Type->IsStruct()) {
				return Result;
			}

			throw CParserException("unexpected " + CScanner::TokenTypesNames[type] + " in declaration", pos);
		}

		NextToken();
	}
}

CSymbol* CParser::ParseDeclarator(CParser::CDeclarationSpecifier DeclSpec, bool CheckExistense /*= true*/)
{
	if (!DeclSpec.Type) {
		return NULL;
	}

	/*if (CTypeSymbol *ExistingSym = SymbolTableStack.Lookup<CTypeSymbol>(DeclSpec.Type->GetName())) {
		delete DeclSpec.Type;
		DeclSpec.Type = ExistingSym;
	} else {
		SymbolTableStack.GetTop()->Add(DeclSpec.Type);
	}*/

	CSymbol *Result = NULL;
	bool IdentifierFound = false;
	string text;

	while (!IdentifierFound) {
		text = Token->GetText();

		if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
			if (CheckExistense && SymbolTableStack.GetTop()->Exists(text)) {
				throw CParserException("identifier redeclared: `" + text + "`", Token->GetPosition());
			}

			IdentifierFound = true;

		} else if (Token->GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
			DeclSpec.Type = ParsePointer(DeclSpec.Type);
			// TODO: add this pointer type to global symbol table with name like "type***"...
		}

		NextToken();
	}

	if (Token->GetType() == TOKEN_TYPE_LEFT_PARENTHESIS) {
		if (ScopeType.top() != SCOPE_TYPE_GLOBAL) {
			throw CParserException("function declaration is allowed only in global scope", Token->GetPosition());
		}

		CFunctionSymbol *FuncSym = new CFunctionSymbol(text, DeclSpec.Type);
		ParseParameterList(FuncSym);
		Result = FuncSym;
	} else {
		if (Token->GetType() == TOKEN_TYPE_LEFT_SQUARE_BRACKET) {
			if (ScopeType.top() == SCOPE_TYPE_PARAMETERS || ScopeType.top() == SCOPE_TYPE_STRUCT) {
				throw CParserException("array declaration is not allowed here", Token->GetPosition());
			}

			DeclSpec.Type = ParseArray(DeclSpec.Type);
		}

		if (DeclSpec.Typedef) {
			if (ScopeType.top() == SCOPE_TYPE_PARAMETERS || ScopeType.top() == SCOPE_TYPE_STRUCT) {
				throw CParserException("typedef is not allowed here", Token->GetPosition());
			}

			Result = new CTypedefSymbol(text, DeclSpec.Type);
		} else {
			Result = new CVariableSymbol(text, DeclSpec.Type);
		}
	}

	return Result;
}

void CParser::ParseParameterList(CFunctionSymbol *Func)
{
	NextToken();

	CArgumentsSymbolTable *FuncArgs = new CArgumentsSymbolTable;
	SymbolTableStack.Push(FuncArgs);
	Func->SetArgumentsSymbolTable(FuncArgs);

	ScopeType.push(SCOPE_TYPE_PARAMETERS);

	while (Token->GetType() != TOKEN_TYPE_RIGHT_PARENTHESIS) {
		Func->AddArgument(ParseDeclarator(ParseDeclarationSpecifier()));

		if (Token->GetType() == TOKEN_TYPE_SEPARATOR_COMMA) {
			NextToken();
		}
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

	SymbolTableStack.GetGlobal()->Add(Sym);

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

	throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
		+ ", got " + Token->GetStringifiedType(), Token->GetPosition());*/
}

CArraySymbol* CParser::ParseArray(CTypeSymbol *AElemType)
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

	//SymbolTableStack.GetGlobal()->Add(Sym);

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
				throw CParserException("undeclared identifier `" + StructName + "`", Token->GetPosition());
			}

			PreviousToken();

			return Sym;
		}

		if (SymbolTableStack.GetTop()->Get(StructName)) {
			throw CParserException("identifier redeclared: `" + StructName + "`", Token->GetPosition());
		}

	} else  if (Token->GetType() != TOKEN_TYPE_BLOCK_START) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_START] + " or " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
			+ " after `struct`, got " + Token->GetStringifiedType(), Token->GetPosition());
	}

	Sym = new CStructSymbol;

	if (!StructName.empty()) {
		Sym->SetName(StructName);
		SymbolTableStack.GetTop()->Add(Sym);
	}

	NextToken();

	CStructSymbolTable *StructSymTable = new CStructSymbolTable;

	SymbolTableStack.Push(StructSymTable);

	ScopeType.push(SCOPE_TYPE_STRUCT);

	while (ParseDeclaration());

	ScopeType.pop();

	SymbolTableStack.Pop();

	Sym->SetSymbolTable(StructSymTable);

	if (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
			+ " after struct-declaration-list, got " + Token->GetStringifiedType(), Token->GetPosition());
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

	if (typeid(*SymbolTableStack.GetTop()) != typeid(CArgumentsSymbolTable)) {
		BlockSymTable->SetCurrentOffset(SymbolTableStack.GetTop()->GetCurrentOffset());
	}

	SymbolTableStack.Push(BlockSymTable);

	while (ParseDeclaration());

	while (Token->GetType() != TOKEN_TYPE_BLOCK_END) {
		if (Token->GetType() == TOKEN_TYPE_EOF) {
			throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_BLOCK_END]
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
		TOKEN_TYPE_OPERATION_PLUS,
		TOKEN_TYPE_OPERATION_MINUS,
		TOKEN_TYPE_OPERATION_BITWISE_NOT,
		TOKEN_TYPE_OPERATION_LOGIC_NOT,
		TOKEN_TYPE_OPERATION_AMPERSAND,
		TOKEN_TYPE_OPERATION_ASTERISK,
		TOKEN_TYPE_OPERATION_INCREMENT,
		TOKEN_TYPE_OPERATION_DECREMENT,
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

		Op->SetResultType(SymbolTableStack.Lookup<CTypeSymbol>("int"));

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

		Op->SetResultType(SymbolTableStack.Lookup<CTypeSymbol>("int"));

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

		Op->SetResultType(SymbolTableStack.Lookup<CTypeSymbol>("int"));

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

		if (Mode != PARSER_MODE_EXPRESSION) {
			Op->CheckTypes();
		}

		Op->SetResultType(SymbolTableStack.Lookup<CTypeSymbol>("int"));

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
	if (!IsUnaryOp(*Token) && Token->GetText() != "sizeof") {
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
		Op->SetResultType(SymbolTableStack.Lookup<CTypeSymbol>("int"));
	}

	return Op;
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
				CVariable *VarExpr = dynamic_cast<CVariable *>(Expr);
				if (!VarExpr) {
					throw CParserException("expected " + CScanner::TokenTypesNames[TOKEN_TYPE_IDENTIFIER]
						+ " before function call operator", Token->GetPosition());
				}

				CFunctionCall *FuncCall = new CFunctionCall(*Token, VarExpr->GetSymbol());
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

				delete VarExpr;
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
		Expr = new CIntegerConst(*Token, SymbolTableStack.Lookup<CTypeSymbol>("int"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_FLOAT) {
		Expr = new CFloatConst(*Token, SymbolTableStack.Lookup<CTypeSymbol>("float"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_SYMBOL) {
		Expr = new CSymbolConst(*Token, SymbolTableStack.Lookup<CTypeSymbol>("int"));
	} else if (Token->GetType() == TOKEN_TYPE_CONSTANT_STRING) {
		Expr = new CStringConst(*Token, SymbolTableStack.Lookup<CTypeSymbol>("int*"));
	} else if (Token->GetType() == TOKEN_TYPE_IDENTIFIER) {
		CSymbol *Sym = SymbolTableStack.Lookup<CSymbol>(Token->GetText());

		if (!Sym) {
			if (Mode == PARSER_MODE_EXPRESSION) {
				Sym = new CVariableSymbol(Token->GetText(), SymbolTableStack.Lookup<CTypeSymbol>("int"));
				SymbolTableStack.GetTop()->Add(Sym);
			} else {
				throw CParserException("undeclared identifier `" + Token->GetText() + "`", Token->GetPosition());
			}
		}

		if (dynamic_cast<CTypeSymbol *>(Sym)) {
			throw CParserException("declaration is not allowed here", Token->GetPosition());
		}

		Expr = new CVariable(*Token, Sym);
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
	CFunctionSymbol *fs = new CFunctionSymbol(AName, SymbolTableStack.Lookup<CTypeSymbol>(AReturnType));
	fs->SetArgumentsSymbolTable(new CArgumentsSymbolTable);
	fs->SetBuiltIn(true);
	va_list vl;
	va_start(vl, AArgumentsCount);
	for (int i = 0; i < AArgumentsCount; i++) {
		fs->AddArgument(new CVariableSymbol("", SymbolTableStack.Lookup<CTypeSymbol>(va_arg(vl, const char *))));
	}
	va_end(vl);
	SymbolTableStack.GetGlobal()->Add(fs);
}
