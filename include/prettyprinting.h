#ifndef _PRETTYPRINTING_H_
#define _PRETTYPRINTING_H_

#include "common.h"
#include "expressions.h"
#include "statements.h"
#include "parser.h"

class CScanPrettyPrinter
{
public:
	CScanPrettyPrinter(CScanner &AScanner);

	void Output(ostream &Stream);

private:
	CScanner &Scanner;
};

class CParsePrettyPrinter
{
public:
	CParsePrettyPrinter(CParser &AParser, const CCompilerParameters &AParameters);

	void Output(ostream &Stream);

private:
	CParser &Parser;
	const CCompilerParameters &Parameters;
};

class CSymbolsPrettyPrinter
{
public:
	CSymbolsPrettyPrinter(CGlobalSymbolTable &ASymbolTable, ostream &AStream);

	void Output();
	
	void Visit(CTypeSymbol &ASymbol);
	void Visit(CTypedefSymbol &ASymbol);
	void Visit(CFunctionSymbol &ASymbol);
	void Visit(CStructSymbol &ASymbol);
	void Visit(CVariableSymbol &ASymbol);

	void VisitBlock(CBlockStatement *AStmt);

private:
	void PrintNesting();

	CGlobalSymbolTable &SymbolTable;
	ostream &Stream;

	unsigned int Nesting;

};

class CStatementLinearPrintVisitor : public CStatementVisitor
{
public:
	CStatementLinearPrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AStmt);
	void Visit(CBinaryOp &AStmt);
	void Visit(CConditionalOp &AStmt);
	void Visit(CIntegerConst &AStmt);
	void Visit(CFloatConst &AStmt);
	void Visit(CCharConst &AStmt);
	void Visit(CStringConst &AStmt);
	void Visit(CVariable &AStmt);
	void Visit(CFunction &AStmt);
	void Visit(CPostfixOp &AStmt);
	void Visit(CFunctionCall &AStmt);
	void Visit(CStructAccess &AStmt);
	void Visit(CIndirectAccess &AStmt);
	void Visit(CArrayAccess &AStmt);
	void Visit(CNullStatement &AStmt);
	void Visit(CBlockStatement &AStmt);
	void Visit(CIfStatement &AStmt);
	void Visit(CForStatement &AStmt);
	void Visit(CWhileStatement &AStmt);
	void Visit(CDoStatement &AStmt);
	void Visit(CLabel &AStmt);
	void Visit(CCaseLabel &AStmt);
	void Visit(CDefaultCaseLabel &AStmt);
	void Visit(CGotoStatement &AStmt);
	void Visit(CBreakStatement &AStmt);
	void Visit(CContinueStatement &AStmt);
	void Visit(CReturnStatement &AStmt);
	void Visit(CSwitchStatement &AStmt);

private:
	ostream &Stream;

	static const char *LEFT_ENCLOSING;
	static const char *RIGHT_ENCLOSING;
	static const char *DELIMITER;

};

class CStatementTreePrintVisitor : public CStatementVisitor
{
public:
	CStatementTreePrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AStmt);
	void Visit(CBinaryOp &AStmt);
	void Visit(CConditionalOp &AStmt);
	void Visit(CIntegerConst &AStmt);
	void Visit(CFloatConst &AStmt);
	void Visit(CCharConst &AStmt);
	void Visit(CStringConst &AStmt);
	void Visit(CVariable &AStmt);
	void Visit(CFunction &AStmt);
	void Visit(CPostfixOp &AStmt);
	void Visit(CFunctionCall &AStmt);
	void Visit(CStructAccess &AStmt);
	void Visit(CIndirectAccess &AStmt);
	void Visit(CArrayAccess &AStmt);
	void Visit(CNullStatement &AStmt);
	void Visit(CBlockStatement &AStmt);
	void Visit(CIfStatement &AStmt);
	void Visit(CForStatement &AStmt);
	void Visit(CWhileStatement &AStmt);
	void Visit(CDoStatement &AStmt);
	void Visit(CLabel &AStmt);
	void Visit(CCaseLabel &AStmt);
	void Visit(CDefaultCaseLabel &AStmt);
	void Visit(CGotoStatement &AStmt);
	void Visit(CBreakStatement &AStmt);
	void Visit(CContinueStatement &AStmt);
	void Visit(CReturnStatement &AStmt);
	void Visit(CSwitchStatement &AStmt);

private:
	void PrintTreeDecoration();
	void PrintName(CStatement &AStmt);

	ostream &Stream;
	int Nesting;
	map<int, bool> LastChild;

};

#endif // _PRETTYPRINTING_H_
