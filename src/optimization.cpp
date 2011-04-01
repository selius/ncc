#include "optimization.h"

#include "statements.h"

/******************************************************************************
 * CLowLevelOptimization
 ******************************************************************************/

CLowLevelOptimization::CLowLevelOptimization(CAsmCode &AAsm) : Asm(AAsm)
{
}

CLowLevelOptimization::~CLowLevelOptimization()
{
}

/******************************************************************************
 * CLowLevelOptimizer
 ******************************************************************************/

CLowLevelOptimizer::CLowLevelOptimizer(CAsmCode &AAsm)
{
	Optimizations.push_back(new CSuperfluousInstructionsRemoval(AAsm));
	Optimizations.push_back(new CArithmeticInstructionsOptimization(AAsm));
	Optimizations.push_back(new CJumpsOptimization(AAsm));
}

CLowLevelOptimizer::~CLowLevelOptimizer()
{
	for (OptimizationsIterator it = Optimizations.begin(); it != Optimizations.end(); ++it) {
		delete *it;
	}
}

void CLowLevelOptimizer::Optimize()
{
	bool Optimized;
	do {
		Optimized = false;
		for (OptimizationsIterator it = Optimizations.begin(); it != Optimizations.end(); ++it) {
			Optimized = Optimized || (*it)->Optimize();
		}
	} while (Optimized);
}

/******************************************************************************
 * CSuperfluousInstructionsRemoval
 ******************************************************************************/

CSuperfluousInstructionsRemoval::CSuperfluousInstructionsRemoval(CAsmCode &AAsm) : CLowLevelOptimization(AAsm)
{
}

bool CSuperfluousInstructionsRemoval::Optimize()
{
	bool Optimized = false;

	CAsmCode::CodeIterator nit1 = Asm.Begin();
	CAsmCode::CodeIterator nit2 = ++Asm.Begin();

	CAsmCode::CodeIterator it1;
	CAsmCode::CodeIterator it2;

	bool beginning = false;

	CAsmCmd1 *cmd1;
	CAsmCmd1 *cmd2;

	while (nit2 != Asm.End()) {
		it1 = nit1++;
		it2 = nit2++;

		cmd1 = dynamic_cast<CAsmCmd1 *>(*it1);
		cmd2 = dynamic_cast<CAsmCmd1 *>(*it2);

		if (cmd1 && cmd2) {
			if (cmd1->GetName() == "push" && cmd2->GetName() == "pop" && cmd1->GetOp()->GetText() == cmd2->GetOp()->GetText()) {
				beginning = (it1 == Asm.Begin());

				Asm.Erase(it1);
				Asm.Erase(it2);

				if (!beginning) {
					nit1 = nit2;
					--nit1;
				} else {
					nit1 = nit2++;
				}
				Optimized = true;

			} else if (cmd1->GetName() == "pop" && cmd2->GetName() == "push" && cmd1->GetOp()->IsReg() && cmd1->GetOp()->GetText() == cmd2->GetOp()->GetText()) {
				Asm.Insert(it1, new CAsmCmd2("mov", mem(0, ESP), cmd2->GetOp()));

				cmd2->SetOp(NULL);
				Asm.Erase(it1);
				Asm.Erase(it2);

				nit1 = nit2++;
				Optimized = true;
			} else if (cmd1->GetName() == "push" && cmd2->GetName() == "pop") {
				if (!cmd1->GetOp()->IsMem() || !cmd2->GetOp()->IsMem()) {
					Asm.Insert(it1, new CAsmCmd2("mov", cmd1->GetOp(), cmd2->GetOp()));

					cmd1->SetOp(NULL);
					cmd2->SetOp(NULL);
					Asm.Erase(it1);
					Asm.Erase(it2);

					nit1 = nit2++;
					Optimized = true;
				}
			}
		}
	}

	return Optimized;
}

/******************************************************************************
 * CArithmeticInstructionsOptimization
 ******************************************************************************/

CArithmeticInstructionsOptimization::CArithmeticInstructionsOptimization(CAsmCode &AAsm) : CLowLevelOptimization(AAsm)
{
}

bool CArithmeticInstructionsOptimization::Optimize()
{
	bool Optimized = false;

	CAsmCode::CodeIterator next = Asm.Begin();
	CAsmCode::CodeIterator cur;

	CAsmCmd1 *cmd1;
	CAsmCmd2 *cmd2;

	while (next != Asm.End()) {
		cur = next++;

		cmd1 = dynamic_cast<CAsmCmd1 *>(*cur);
		cmd2 = dynamic_cast<CAsmCmd2 *>(*cur);

		if (cmd2 && cmd2->GetOp1()->IsImm()) {
			CAsmImm *imm = dynamic_cast<CAsmImm *>(cmd2->GetOp1());

			if (imm->GetValue() == 0) {
				if (cmd2->GetName() == "add" || cmd2->GetName() == "sub") {
					Asm.Erase(cur);
					Optimized = true;
				} else if (cmd2->GetName() == "imul") {
					Asm.Insert(cur, new CAsmCmd2("mov", imm, cmd2->GetOp2()));

					cmd2->SetOp1(NULL);
					cmd2->SetOp2(NULL);
					Asm.Erase(cur);
					Optimized = true;
				} else if (cmd2->GetName() == "mov" && cmd2->GetOp2()->IsReg()) {
					CAsmReg *reg = dynamic_cast<CAsmReg *>(cmd2->GetOp2());

					Asm.Insert(cur, new CAsmCmd2("xor", reg, new CAsmReg(*reg)));

					cmd2->SetOp2(NULL);
					Asm.Erase(cur);
					Optimized = true;
				}
			} else if (imm->GetValue() == 1) {
				if (cmd2->GetName() == "imul" || cmd2->GetName() == "idiv") {
					Asm.Erase(cur);
					Optimized = true;
				}
			}
		} else if (cmd1 && cmd1->GetOp()->IsReg()) {
			if (cmd1->GetName() == "inc") {
				Asm.Insert(cur, new CAsmCmd2("add", new CAsmImm(1), cmd1->GetOp()));

				cmd1->SetOp(NULL);
				Asm.Erase(cur);
				Optimized = true;
			} else if (cmd1->GetName() == "dec") {
				Asm.Insert(cur, new CAsmCmd2("sub", new CAsmImm(1), cmd1->GetOp()));

				cmd1->SetOp(NULL);
				Asm.Erase(cur);
				Optimized = true;
			}
		}
	}

	return Optimized;
}

/******************************************************************************
 * CJumpsOptimization
 ******************************************************************************/

CJumpsOptimization::CJumpsOptimization(CAsmCode &AAsm) : CLowLevelOptimization(AAsm)
{
}

bool CJumpsOptimization::Optimize()
{
	bool Optimized = false;

	CAsmCode::CodeIterator nit1 = Asm.Begin();
	CAsmCode::CodeIterator nit2 = ++Asm.Begin();

	CAsmCode::CodeIterator it1;
	CAsmCode::CodeIterator it2;

	CAsmCmd1 *cmd1;
	CAsmLabel *cmd2;

	while (nit2 != Asm.End()) {
		it1 = nit1++;
		it2 = nit2++;

		cmd1 = dynamic_cast<CAsmCmd1 *>(*it1);
		cmd2 = dynamic_cast<CAsmLabel *>(*it2);

		if (cmd1 && cmd2 && cmd1->GetName() == "jmp" && cmd1->GetOp()->GetText() == cmd2->GetName()) {
			Asm.Erase(it1);
			nit1 = nit2++;
			Optimized = true;
		}
	}

	return Optimized;
}

/******************************************************************************
 * CUnreachableCodeElimination
 ******************************************************************************/

void CUnreachableCodeElimination::Visit(CUnaryOp &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CBinaryOp &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CConditionalOp &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CIntegerConst &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CFloatConst &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CCharConst &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CStringConst &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CVariable &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CFunction &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CPostfixOp &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CFunctionCall &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CStructAccess &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CIndirectAccess &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CArrayAccess &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CNullStatement &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CBlockStatement &AStmt)
{
	bool DoNotAct = false;

	for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		if (dynamic_cast<CGotoStatement *>(*it) || dynamic_cast<CLabel *>(*it)) {
			DoNotAct = true;
		}

		(*it)->Accept(*this);

		if (!DoNotAct) {
			if (dynamic_cast<CReturnStatement *>(*it) || dynamic_cast<CContinueStatement *>(*it) || dynamic_cast<CBreakStatement *>(*it)) {
				for (CBlockStatement::StatementsIterator dit = ++it; dit != AStmt.End(); ++dit) {
					delete *dit;
				}

				AStmt.Erase(it, AStmt.End());
				break;
			}
		}
	}
}

void CUnreachableCodeElimination::Visit(CIfStatement &AStmt)
{
	AStmt.GetThenStatement()->Accept(*this);
	TryVisit(AStmt.GetElseStatement());
}

void CUnreachableCodeElimination::Visit(CForStatement &AStmt)
{
	AStmt.GetBody()->Accept(*this);
}

void CUnreachableCodeElimination::Visit(CWhileStatement &AStmt)
{
	AStmt.GetBody()->Accept(*this);
}

void CUnreachableCodeElimination::Visit(CDoStatement &AStmt)
{
	AStmt.GetBody()->Accept(*this);
}

void CUnreachableCodeElimination::Visit(CLabel &AStmt)
{
	AStmt.GetNext()->Accept(*this);
}

void CUnreachableCodeElimination::Visit(CCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CUnreachableCodeElimination::Visit(CDefaultCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CUnreachableCodeElimination::Visit(CGotoStatement &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CBreakStatement &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CContinueStatement &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CReturnStatement &AStmt)
{
}

void CUnreachableCodeElimination::Visit(CSwitchStatement &AStmt)
{
	Visit(static_cast<CBlockStatement &>(AStmt));
}
