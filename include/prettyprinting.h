#ifndef _PRETTYPRINTING_H_
#define _PRETTYPRINTING_H_

//#include "common.h"
#include "expressions.h"
#include "statements.h"


class CStatementLinearPrintVisitor : public CStatementVisitor
{
public:
	CStatementLinearPrintVisitor(ostream &AStream);

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CConditionalOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);
	void Visit(CPostfixOp &AExpr);
	void Visit(CFunctionCall &AExpr);
	void Visit(CStructAccess &AExpr);
	void Visit(CIndirectAccess &AExpr);
	/*void Visit(CArrayAccess &AExpr);*/
	void Visit(CNullStatement &AExpr);
	void Visit(CBlockStatement &AExpr);

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

	void Visit(CUnaryOp &AExpr);
	void Visit(CBinaryOp &AExpr);
	void Visit(CConditionalOp &AExpr);
	void Visit(CIntegerConst &AExpr);
	void Visit(CFloatConst &AExpr);
	void Visit(CSymbolConst &AExpr);
	void Visit(CStringConst &AExpr);
	void Visit(CVariable &AExpr);
	void Visit(CPostfixOp &AExpr);
	void Visit(CFunctionCall &AExpr);
	void Visit(CStructAccess &AExpr);
	void Visit(CIndirectAccess &AExpr);
	/*void Visit(CArrayAccess &AExpr);*/
	void Visit(CNullStatement &AExpr);
	void Visit(CBlockStatement &AExpr);

private:
	void PrintTreeDecoration();

	ostream &Stream;
	int Nesting;
	map<int, bool> LastChild;

};

#endif // _PRETTYPRINTING_H_
