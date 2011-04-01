#ifndef _OPTIMIZATION_H_
#define _OPTIMIZATION_H_

#include "common.h"
#include "codegen.h"

class CLowLevelOptimization
{
public:
	CLowLevelOptimization(CAsmCode &AAsm);
	virtual ~CLowLevelOptimization();

	virtual bool Optimize() = 0;

protected:
	CAsmCode &Asm;
};

class CLowLevelOptimizer
{
public:
	CLowLevelOptimizer(CAsmCode &AAsm);
	~CLowLevelOptimizer();

	void Optimize();

private:
	typedef vector<CLowLevelOptimization *> OptimizationsContainer;
	typedef OptimizationsContainer::iterator OptimizationsIterator;

	OptimizationsContainer Optimizations;
};

class CSuperfluousInstructionsRemoval : public CLowLevelOptimization
{
public:
	CSuperfluousInstructionsRemoval(CAsmCode &AAsm);

	bool Optimize();
};

class CArithmeticInstructionsOptimization : public CLowLevelOptimization
{
public:
	CArithmeticInstructionsOptimization(CAsmCode &AAsm);

	bool Optimize();
};

class CJumpsOptimization : public CLowLevelOptimization
{
public:
	CJumpsOptimization(CAsmCode &AAsm);

	bool Optimize();
};

class CUnreachableCodeElimination : public CStatementVisitor
{
public:
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

};


#endif // _OPTIMIZATION_H_
