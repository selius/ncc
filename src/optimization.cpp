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
				Asm.Insert(it1, MOV, mem(0, ESP), cmd2->GetOp());

				cmd2->SetOp(NULL);
				Asm.Erase(it1);
				Asm.Erase(it2);

				nit1 = nit2++;
				Optimized = true;
			} else if (cmd1->GetName() == "push" && cmd2->GetName() == "pop") {
				if (!cmd1->GetOp()->IsMem() || !cmd2->GetOp()->IsMem()) {
					Asm.Insert(it1, MOV, cmd1->GetOp(), cmd2->GetOp());

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
					Asm.Insert(cur, MOV, imm, cmd2->GetOp2());

					cmd2->SetOp1(NULL);
					cmd2->SetOp2(NULL);
					Asm.Erase(cur);
					Optimized = true;
				} else if (cmd2->GetName() == "mov" && cmd2->GetOp2()->IsReg()) {
					CAsmReg *reg = dynamic_cast<CAsmReg *>(cmd2->GetOp2());

					Asm.Insert(cur, XOR, reg, new CAsmReg(*reg));

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
				Asm.Insert(cur, ADD, 1, cmd1->GetOp());

				cmd1->SetOp(NULL);
				Asm.Erase(cur);
				Optimized = true;
			} else if (cmd1->GetName() == "dec") {
				Asm.Insert(cur, SUB, 1, cmd1->GetOp());

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
	CAsmLabel *cmd2l;
	CAsmCmd1 *cmd2j;

	while (nit2 != Asm.End()) {
		it1 = nit1++;
		it2 = nit2++;

		cmd1 = dynamic_cast<CAsmCmd1 *>(*it1);
		cmd2l = dynamic_cast<CAsmLabel *>(*it2);
		cmd2j = dynamic_cast<CAsmCmd1 *>(*it2);

		if (cmd1 && cmd1->GetName() == "jmp") {
			if (cmd2l && cmd1->GetOp()->GetText() == cmd2l->GetName()) {
				Asm.Erase(it1);
				nit1 = nit2++;
				Optimized = true;
			} else if (cmd2j && cmd2j->GetName() == "jmp") {
				Asm.Erase(it2);
				nit1 = it1;
				Optimized = true;
			}
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

/******************************************************************************
 * CConstantExpressionComputer
 ******************************************************************************/

void CConstantExpressionComputer::Visit(CUnaryOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);

	ETokenType t = AStmt.GetType();

	if (t == TOKEN_TYPE_OPERATION_MINUS) {
		Result = -Result;
	} else if (t == TOKEN_TYPE_OPERATION_LOGIC_NOT) {
		Result = !Result;
	} else if (t == TOKEN_TYPE_OPERATION_BITWISE_NOT) {
		Result = ~(int)Result;
	}
}

void CConstantExpressionComputer::Visit(CBinaryOp &AStmt)
{
	AStmt.GetLeft()->Accept(*this);
	float LHS = Result;
	AStmt.GetRight()->Accept(*this);
	float RHS = Result;

	ETokenType t = AStmt.GetType();

	if (t == TOKEN_TYPE_OPERATION_PLUS) {
		Result = LHS + RHS;
	} else if (t == TOKEN_TYPE_OPERATION_MINUS) {
		Result = LHS - RHS;
	} else if (t == TOKEN_TYPE_OPERATION_ASTERISK) {
		Result = LHS * RHS;
	} else if (t == TOKEN_TYPE_OPERATION_SLASH || t == TOKEN_TYPE_OPERATION_PERCENT) {
		if (RHS == 0) {
			throw CParserException("division by zero in constant expression", AStmt.GetPosition());
		}

		if (t == TOKEN_TYPE_OPERATION_SLASH) {
			Result = LHS / RHS;
		} else {
			Result = (int) LHS % (int) RHS;
		}
	} else if (t == TOKEN_TYPE_OPERATION_SHIFT_LEFT) {
		Result = (int) LHS << (int) RHS;
	} else if (t == TOKEN_TYPE_OPERATION_SHIFT_RIGHT) {
		Result = (int) LHS >> (int) RHS;
	} else if (t == TOKEN_TYPE_OPERATION_LESS_THAN) {
		Result = LHS < RHS;
	} else if (t == TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL) {
		Result = LHS <= RHS;
	} else if (t == TOKEN_TYPE_OPERATION_GREATER_THAN) {
		Result = LHS > RHS;
	} else if (t == TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL) {
		Result = LHS >= RHS;
	} else if (t == TOKEN_TYPE_OPERATION_EQUAL) {
		Result = LHS == RHS;
	} else if (t == TOKEN_TYPE_OPERATION_NOT_EQUAL) {
		Result = LHS != RHS;
	} else if (t == TOKEN_TYPE_OPERATION_LOGIC_AND) {
		Result = LHS && RHS;
	} else if (t == TOKEN_TYPE_OPERATION_LOGIC_OR) {
		Result = LHS || RHS;
	} else if (t == TOKEN_TYPE_OPERATION_BITWISE_OR) {
		Result = (int) LHS | (int) RHS;
	} else if (t == TOKEN_TYPE_OPERATION_BITWISE_XOR) {
		Result = (int) LHS ^ (int) RHS;
	} else if (t == TOKEN_TYPE_OPERATION_AMPERSAND) {
		Result = (int) LHS & (int) RHS;
	} else if (t == TOKEN_TYPE_SEPARATOR_COMMA) {
		Result = RHS;
	}

	if (!AStmt.GetResultType()->IsFloat()) {
		Result = (int) Result;
	}
}

void CConstantExpressionComputer::Visit(CConditionalOp &AStmt)
{
	AStmt.GetCondition()->Accept(*this);
	if (Result) {
		AStmt.GetTrueExpr()->Accept(*this);
	} else {
		AStmt.GetFalseExpr()->Accept(*this);
	}
}

void CConstantExpressionComputer::Visit(CIntegerConst &AStmt)
{
	Result = AStmt.GetValue();
}

void CConstantExpressionComputer::Visit(CFloatConst &AStmt)
{
	Result = AStmt.GetValue();
}

void CConstantExpressionComputer::Visit(CCharConst &AStmt)
{
	Result = AStmt.GetValue();
}

void CConstantExpressionComputer::Visit(CStringConst &AStmt)
{
}

void CConstantExpressionComputer::Visit(CVariable &AStmt)
{
}

void CConstantExpressionComputer::Visit(CFunction &AStmt)
{
}

void CConstantExpressionComputer::Visit(CPostfixOp &AStmt)
{
}

void CConstantExpressionComputer::Visit(CFunctionCall &AStmt)
{
}

void CConstantExpressionComputer::Visit(CStructAccess &AStmt)
{
}

void CConstantExpressionComputer::Visit(CIndirectAccess &AStmt)
{
}

void CConstantExpressionComputer::Visit(CArrayAccess &AStmt)
{
}

void CConstantExpressionComputer::Visit(CNullStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CBlockStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CIfStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CForStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CWhileStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CDoStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CLabel &AStmt)
{
}

void CConstantExpressionComputer::Visit(CCaseLabel &AStmt)
{
}

void CConstantExpressionComputer::Visit(CDefaultCaseLabel &AStmt)
{
}

void CConstantExpressionComputer::Visit(CGotoStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CBreakStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CContinueStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CReturnStatement &AStmt)
{
}

void CConstantExpressionComputer::Visit(CSwitchStatement &AStmt)
{
}

int CConstantExpressionComputer::GetIntResult()
{
	return Result;
}

float CConstantExpressionComputer::GetFloatResult()
{
	return Result;
}

/******************************************************************************
 * CConstantFolding
 ******************************************************************************/

void CConstantFolding::Visit(CUnaryOp &AStmt)
{
	AStmt.SetArgument(TryFold(AStmt.GetArgument()));
}

void CConstantFolding::Visit(CBinaryOp &AStmt)
{
	AStmt.SetLeft(TryFold(AStmt.GetLeft()));
	AStmt.SetRight(TryFold(AStmt.GetRight()));
}

void CConstantFolding::Visit(CConditionalOp &AStmt)
{
	AStmt.SetCondition(TryFold(AStmt.GetCondition()));
	AStmt.SetTrueExpr(TryFold(AStmt.GetTrueExpr()));
	AStmt.SetFalseExpr(TryFold(AStmt.GetFalseExpr()));
}

void CConstantFolding::Visit(CIntegerConst &AStmt)
{
}

void CConstantFolding::Visit(CFloatConst &AStmt)
{
}

void CConstantFolding::Visit(CCharConst &AStmt)
{
}

void CConstantFolding::Visit(CStringConst &AStmt)
{
}

void CConstantFolding::Visit(CVariable &AStmt)
{
}

void CConstantFolding::Visit(CFunction &AStmt)
{
}

void CConstantFolding::Visit(CPostfixOp &AStmt)
{
	AStmt.SetArgument(TryFold(AStmt.GetArgument()));
}

void CConstantFolding::Visit(CFunctionCall &AStmt)
{
	for (CFunctionCall::ArgumentsReverseIterator it = AStmt.RBegin(); it != AStmt.REnd(); ++it) {
		*it = TryFold(*it);
	}
}

void CConstantFolding::Visit(CStructAccess &AStmt)
{
	AStmt.SetStruct(TryFold(AStmt.GetStruct()));
}

void CConstantFolding::Visit(CIndirectAccess &AStmt)
{
	AStmt.SetPointer(TryFold(AStmt.GetPointer()));
}

void CConstantFolding::Visit(CArrayAccess &AStmt)
{
	AStmt.SetLeft(TryFold(AStmt.GetLeft()));
	AStmt.SetRight(TryFold(AStmt.GetRight()));
}

void CConstantFolding::Visit(CNullStatement &AStmt)
{
}

void CConstantFolding::Visit(CBlockStatement &AStmt)
{
	for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		*it = TryFold(*it);
	}
}

void CConstantFolding::Visit(CIfStatement &AStmt)
{
	AStmt.SetCondition(TryFold(AStmt.GetCondition()));
	AStmt.SetThenStatement(TryFold(AStmt.GetThenStatement()));
	AStmt.SetElseStatement(TryFold(AStmt.GetElseStatement()));
}

void CConstantFolding::Visit(CForStatement &AStmt)
{
	AStmt.SetInit(TryFold(AStmt.GetInit()));
	AStmt.SetCondition(TryFold(AStmt.GetCondition()));
	AStmt.SetUpdate(TryFold(AStmt.GetUpdate()));
	AStmt.SetBody(TryFold(AStmt.GetBody()));
}

void CConstantFolding::Visit(CWhileStatement &AStmt)
{
	AStmt.SetCondition(TryFold(AStmt.GetCondition()));
	AStmt.SetBody(TryFold(AStmt.GetBody()));
}

void CConstantFolding::Visit(CDoStatement &AStmt)
{
	AStmt.SetCondition(TryFold(AStmt.GetCondition()));
	AStmt.SetBody(TryFold(AStmt.GetBody()));
}

void CConstantFolding::Visit(CLabel &AStmt)
{
	AStmt.SetNext(TryFold(AStmt.GetNext()));
}

void CConstantFolding::Visit(CCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CConstantFolding::Visit(CDefaultCaseLabel &AStmt)
{
	Visit(static_cast<CLabel &>(AStmt));
}

void CConstantFolding::Visit(CGotoStatement &AStmt)
{
}

void CConstantFolding::Visit(CBreakStatement &AStmt)
{
}

void CConstantFolding::Visit(CContinueStatement &AStmt)
{
}

void CConstantFolding::Visit(CReturnStatement &AStmt)
{
	AStmt.SetReturnExpression(TryFold(AStmt.GetReturnExpression()));
}

void CConstantFolding::Visit(CSwitchStatement &AStmt)
{
	AStmt.SetTestExpression(TryFold(AStmt.GetTestExpression()));
	AStmt.SetBody(TryFold(AStmt.GetBody()));
}

CExpression* CConstantFolding::TryFold(CStatement *AStmt)
{
	CExpression *Expr = static_cast<CExpression *>(AStmt);

	if (!AStmt || !AStmt->IsExpression() || !Expr->IsConst() || Expr->GetType() == TOKEN_TYPE_CONSTANT_STRING) {
		if (AStmt) {
			AStmt->Accept(*this);
		}

		return Expr;
	}

	Expr->Accept(ConstExprComp);

	CExpression *Result;

	if (Expr->GetResultType()->IsFloat()) {
		Result = new CFloatConst(CFloatConstToken(ToString(ConstExprComp.GetFloatResult()), Expr->GetPosition()), Expr->GetResultType());
	} else {
		Result = new CIntegerConst(CIntegerConstToken(ToString(ConstExprComp.GetIntResult()), Expr->GetPosition()), Expr->GetResultType());
	}
	
	delete AStmt;
	return Result;
}

/******************************************************************************
 * CLoopInvariantHoisting
 ******************************************************************************/

CLoopInvariantHoisting::CLoopInvariantHoisting() : ProcessingLoop(false)
{
}

void CLoopInvariantHoisting::Visit(CUnaryOp &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CBinaryOp &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CConditionalOp &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CIntegerConst &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CFloatConst &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CCharConst &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CStringConst &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CVariable &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CFunction &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CPostfixOp &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CFunctionCall &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CStructAccess &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CIndirectAccess &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CArrayAccess &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CNullStatement &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CBlockStatement &AStmt)
{
	if (ProcessingLoop) {
		CBlockStatement::UsedContainer Used;
		CBlockStatement::AffectedContainer Affected;
		CBlockStatement::AffectedContainer StmtAffected;

		CBlockStatement::StatementsIterator pit;

		for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End();) {
			if (!(*it)->CanBeHoisted()) {
				++it;
				continue;
			}

			Used.clear();
			Affected.clear();
			StmtAffected.clear();

			(*it)->GetUsedVariables(Used);
			(*ParentBlockIterator.top())->GetAffectedVariables(Affected);
			(*it)->GetAffectedVariables(StmtAffected);

			for (CBlockStatement::AffectedContainer::iterator ait = StmtAffected.begin(); ait != StmtAffected.end(); ++ait) {
				Affected[ait->first] -= ait->second;
				if (!Affected[ait->first]) {
					Affected.erase(ait->first);
				}
			}

			bool DoNotAct = false;

			for (CSymbolTable::VariablesIterator vit = AStmt.GetSymbolTable()->VariablesBegin(); vit != AStmt.GetSymbolTable()->VariablesEnd(); ++vit) {
				if (Used.count(vit->second)) {
					DoNotAct = true;
					break;
				}
			}

			for (CBlockStatement::AffectedContainer::iterator ait = StmtAffected.begin(); ait != StmtAffected.end(); ++ait) {
				if (Used.count(ait->first)) {
					if (Used[ait->first] > ait->second) {
						DoNotAct = true;
						break;
					}
				}
			}

			for (CBlockStatement::UsedContainer::iterator uit = Used.begin(); uit != Used.end(); ++uit) {
				if (Affected.count(uit->first)) {
					DoNotAct = true;
					break;
				}
			}

			pit = it++;

			if (!DoNotAct) {
				AStmt.Erase(pit);
				ParentBlock.top()->Insert(ParentBlockIterator.top(), *pit);
			}
		}
	} else {
		CBlockStatement::StatementsIterator pit;
		for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End();) {
			ParentBlock.push(&AStmt);
			ParentBlockIterator.push(it);

			pit = it++;
			(*pit)->Accept(*this);

			ParentBlockIterator.pop();
			ParentBlock.pop();
		}
	}
}

void CLoopInvariantHoisting::Visit(CIfStatement &AStmt)
{
	TryVisit(AStmt.GetThenStatement());
	TryVisit(AStmt.GetElseStatement());
}

void CLoopInvariantHoisting::Visit(CForStatement &AStmt)
{
	TryVisit(AStmt.GetBody());
	ProcessLoop(AStmt.GetBody());
}

void CLoopInvariantHoisting::Visit(CWhileStatement &AStmt)
{
	TryVisit(AStmt.GetBody());
	ProcessLoop(AStmt.GetBody());
}

void CLoopInvariantHoisting::Visit(CDoStatement &AStmt)
{
	TryVisit(AStmt.GetBody());
	ProcessLoop(AStmt.GetBody());
}

void CLoopInvariantHoisting::Visit(CLabel &AStmt)
{
	TryVisit(AStmt.GetNext());
}

void CLoopInvariantHoisting::Visit(CCaseLabel &AStmt)
{
	TryVisit(AStmt.GetNext());
}

void CLoopInvariantHoisting::Visit(CDefaultCaseLabel &AStmt)
{
	TryVisit(AStmt.GetNext());
}

void CLoopInvariantHoisting::Visit(CGotoStatement &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CBreakStatement &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CContinueStatement &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CReturnStatement &AStmt)
{
}

void CLoopInvariantHoisting::Visit(CSwitchStatement &AStmt)
{
	TryVisit(AStmt.GetBody());
}

void CLoopInvariantHoisting::ProcessLoop(CStatement *ALoopBody)
{
	ProcessingLoop = true;

	TryVisit(ALoopBody);

	ProcessingLoop = false;
}
