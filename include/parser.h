#ifndef _PARSER_H_
#define _PARSER_H_

#include "scanner.h"
#include "symbols.h"
#include "expressions.h"
#include "statements.h"

class CTokenStream
{
public:
	CTokenStream(CScanner &AScanner);
	~CTokenStream();

	const CToken* Next();
	const CToken* GetToken();
	const CToken* Previous();

private:
	CScanner &Scanner;
	deque<CToken *> Buffer;
	deque<CToken *>::iterator Current;
	static const size_t TOKEN_STREAM_SIZE = 5;
};

class CParser
{
public:
	CParser(CScanner &AScanner, EParserMode AMode = PARSER_MODE_NORMAL);
	~CParser();

	CSymbolTable* ParseTranslationUnit();

	CExpression* ParseExpression();

private:
	enum EBlockType
	{
		BLOCK_TYPE_DEFAULT,
		BLOCK_TYPE_LOOP,
		BLOCK_TYPE_SWITCH,
	};

	enum EScopeType
	{
		SCOPE_TYPE_GLOBAL,
		SCOPE_TYPE_FUNCTION,
		SCOPE_TYPE_PARAMETERS,
		SCOPE_TYPE_STRUCT,
	};

	struct CDeclarationSpecifier
	{
		CDeclarationSpecifier();

		CTypeSymbol *Type;
		bool Typedef;
	};

	struct CLabelInfo
	{
		CLabelInfo(CLabel *ALabel = NULL, CPosition APosition = CPosition());

		CLabel *Label;
		CPosition Position;

	};

	bool ParseDeclaration();
	CDeclarationSpecifier ParseDeclarationSpecifier();
	CSymbol* ParseDeclarator(CDeclarationSpecifier DeclSpec, bool CheckExistense = true);
	void ParseParameterList(CFunctionSymbol *Func);
	CPointerSymbol* ParsePointer(CTypeSymbol *ARefType);
	CArraySymbol* ParseArray(CTypeSymbol *AElemType);
	CStructSymbol* ParseStruct();

	CStatement* ParseStatement();

	CBlockStatement* ParseBlock();

	CStatement* ParseIf();

	CStatement* ParseFor();
	CStatement* ParseWhile();
	CStatement* ParseDo();

	CStatement* ParseCase();
	CStatement* ParseDefault();

	CStatement* ParseGoto();
	CStatement* ParseBreak();
	CStatement* ParseContinue();
	CStatement* ParseReturn();

	CStatement* ParseSwitch();

	CExpression* ParseAssignment();

	CExpression* ParseConditional();

	CExpression* ParseLogicalOr();
	CExpression* ParseLogicalAnd();

	CExpression* ParseBitwiseOr();
	CExpression* ParseBitwiseXor();
	CExpression* ParseBitwiseAnd();

	CExpression* ParseEqualityExpression();
	CExpression* ParseRelationalExpression();

	CExpression* ParseShiftExpression();
	CExpression* ParseAdditiveExpression();
	CExpression* ParseMultiplicativeExpression();

	CExpression* ParseCastExpression();
	CExpression* ParseUnaryExpression();
	CExpression* ParsePostfixExpression();

	CExpression* ParsePrimaryExpression();

	void NextToken();
	void PreviousToken();

	void AddBuiltIn(const string &AName, const string &AReturnType, int AArgumentsCount, ...);

	CTokenStream TokenStream;

	const CToken *Token;

	CSymbolTableStack SymbolTableStack;
	map<string, CLabelInfo> LabelTable;
	stack<EBlockType> BlockType;
	stack<EScopeType> ScopeType;
	stack<CSwitchStatement *> SwitchesStack;
	CFunctionSymbol *CurrentFunction;

	EParserMode Mode;
};

#endif // _PARSER_H_
