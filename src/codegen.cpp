#include "codegen.h"

#include "expressions.h"

/******************************************************************************
 * CAsmCmd
 ******************************************************************************/

CAsmCmd::~CAsmCmd()
{
}

string CAsmCmd::GetName() const
{
	return Name;
}

void CAsmCmd::SetName(const string &AName)
{
	Name = AName;
}

/******************************************************************************
 * CAsmReg
 ******************************************************************************/

CAsmReg::CAsmReg(const string &AName) : Name(AName)
{
}

string CAsmReg::GetName() const
{
	return Name;
}

void CAsmReg::SetName(const string &AName)
{
	Name = AName;
}

string CAsmReg::GetText() const
{
	return "%" + Name;
}

/******************************************************************************
 * CAsmImm
 ******************************************************************************/

CAsmImm::CAsmImm(int AValue) : Value(AValue)
{
}

string CAsmImm::GetText() const
{
	return "$" + ToString(Value);
}

/******************************************************************************
 * CAsmMem
 ******************************************************************************/

CAsmMem::CAsmMem(int ADisplacement, ERegister ABase) : Displacement(ADisplacement), Base(ABase)
{
}

string CAsmMem::GetText() const
{
	return ToString(Displacement) + "(" + "%" + RegistersText[Base] + ")";
}

CAsmMem* mem(int ADisplacement, ERegister ABase)
{
	return new CAsmMem(ADisplacement, ABase);
}

/******************************************************************************
 * CAsmLabelOp
 ******************************************************************************/

CAsmLabelOp::CAsmLabelOp(const string &AName)
{
	Name = AName;
}

string CAsmLabelOp::GetName() const
{
	return Name;
}

void CAsmLabelOp::SetName(const string &AName)
{
	Name = AName;
}

string CAsmLabelOp::GetText() const
{
	return Name;
}

/******************************************************************************
 * CAsmCmd0
 ******************************************************************************/

CAsmCmd0::CAsmCmd0(const string &AName)
{
	Name = AName;
}

string CAsmCmd0::GetText() const
{
	return "\t" + Name;
}

/******************************************************************************
 * CAsmCmd1
 ******************************************************************************/

CAsmCmd1::CAsmCmd1(const string &AName, CAsmOp *AOp) : Op(AOp)
{
	Name = AName;
}

CAsmCmd1::~CAsmCmd1()
{
	delete Op;
}

string CAsmCmd1::GetText() const
{
	return "\t" + Name + "\t" + Op->GetText();
}

/******************************************************************************
 * CAsmCmd2
 ******************************************************************************/

CAsmCmd2::CAsmCmd2(const string &AName, CAsmOp *AOp1, CAsmOp *AOp2) : Op1(AOp1), Op2(AOp2)
{
	Name = AName;
}

CAsmCmd2::~CAsmCmd2()
{
	delete Op1;
	delete Op2;
}

string CAsmCmd2::GetText() const
{
	return "\t" + Name + "\t" + Op1->GetText() + ", " + Op2->GetText();
}

/******************************************************************************
 * CAsmLabel
 ******************************************************************************/

CAsmLabel::CAsmLabel(const string &AName)
{
	Name = AName;
}

string CAsmLabel::GetText() const
{
	return Name + ":";
}

/******************************************************************************
 * CAsmDirective
 ******************************************************************************/

CAsmDirective::CAsmDirective(const string &AName, const string &AArgument) : Argument(AArgument)
{
	Name = AName;
}

string CAsmDirective::GetText() const
{
	return "." + Name + "\t" + Argument;
}

/******************************************************************************
 * CAsmCode
 ******************************************************************************/

CAsmCode::CAsmCode() : LabelsCount(0)
{
	MnemonicsText[MOV] = "mov";
	MnemonicsText[PUSH] = "push";
	MnemonicsText[POP] = "pop";
	MnemonicsText[RET] = "ret";
	MnemonicsText[CALL] = "call";
	MnemonicsText[JMP] = "jmp";
	MnemonicsText[JE] = "je";
	MnemonicsText[JNE] = "jne";
	MnemonicsText[JL] = "jl";
	MnemonicsText[JG] = "jg";
	MnemonicsText[JLE] = "jle";
	MnemonicsText[JGE] = "jge";
	MnemonicsText[ADD] = "add";
	MnemonicsText[SUB] = "sub";
	MnemonicsText[MUL] = "mul";
	MnemonicsText[IMUL] = "imul";
	MnemonicsText[DIV] = "div";
	MnemonicsText[IDIV] = "idiv";
	MnemonicsText[INC] = "inc";
	MnemonicsText[DEC] = "dec";
	MnemonicsText[CMP] = "cmp";
	MnemonicsText[CDQ] = "cdq";
	MnemonicsText[AND] = "and";
	MnemonicsText[OR] = "or";
	MnemonicsText[XOR] = "xor";
	MnemonicsText[SAL] = "sal";
	MnemonicsText[SAR] = "sar";
	MnemonicsText[LEA] = "lea";

	RegistersText[EAX] = "eax";
	RegistersText[EBX] = "ebx";
	RegistersText[ECX] = "ecx";
	RegistersText[EDX] = "edx";
	RegistersText[ESI] = "esi";
	RegistersText[EDI] = "edi";
	RegistersText[ESP] = "esp";
	RegistersText[EBP] = "ebp";
}

CAsmCode::~CAsmCode()
{
	for (CodeIterator it = Code.begin(); it != Code.end(); ++it) {
		delete *it;
	}

	RegistersText.clear();
	MnemonicsText.clear();
}

void CAsmCode::Add(CAsmCmd *ACmd)
{
	Code.push_back(ACmd);
}

void CAsmCode::Add(EMnemonic ACmd)
{
	Code.push_back(new CAsmCmd0(MnemonicsText[ACmd]));
}

void CAsmCode::Add(EMnemonic ACmd, ERegister AOp)
{
	Code.push_back(new CAsmCmd1(MnemonicsText[ACmd], new CAsmReg(RegistersText[AOp])));
}

void CAsmCode::Add(EMnemonic ACmd, int AOp)
{
	Code.push_back(new CAsmCmd1(MnemonicsText[ACmd], new CAsmImm(AOp)));
}

void CAsmCode::Add(EMnemonic ACmd, ERegister AOp1, ERegister AOp2)
{
	Code.push_back(new CAsmCmd2(MnemonicsText[ACmd], new CAsmReg(RegistersText[AOp1]), new CAsmReg(RegistersText[AOp2])));
}

void CAsmCode::Add(EMnemonic ACmd, int AOp1, ERegister AOp2)
{
	Code.push_back(new CAsmCmd2(MnemonicsText[ACmd], new CAsmImm(AOp1), new CAsmReg(RegistersText[AOp2])));
}

void CAsmCode::Add(EMnemonic ACmd, const string &AOp)
{
	Code.push_back(new CAsmCmd1(MnemonicsText[ACmd], new CAsmLabelOp(AOp)));
}

void CAsmCode::Add(EMnemonic ACmd, CAsmMem *AOp)
{
	Code.push_back(new CAsmCmd1(MnemonicsText[ACmd], AOp));
}

void CAsmCode::Add(EMnemonic ACmd, ERegister AOp1, CAsmMem *AOp2)
{
	Code.push_back(new CAsmCmd2(MnemonicsText[ACmd], new CAsmReg(RegistersText[AOp1]), AOp2));
}

void CAsmCode::Add(EMnemonic ACmd, CAsmMem *AOp1, ERegister AOp2)
{
	Code.push_back(new CAsmCmd2(MnemonicsText[ACmd], AOp1, new CAsmReg(RegistersText[AOp2])));
}

void CAsmCode::Output(ostream &Stream)
{
	Stream << ".text" << endl;

	for (vector<CAsmCmd *>::iterator it = Code.begin(); it != Code.end(); ++it) {
		Stream << (*it)->GetText() << endl;
	}

	Stream << ".end" << endl;
}

string CAsmCode::GenerateLabel()
{
	string result;
	stringstream ss;
	ss << ++LabelsCount;
	ss >> result;

	return ".L" + result;
}

/******************************************************************************
 * CCodeGenerationVisitor
 ******************************************************************************/

CCodeGenerationVisitor::CCodeGenerationVisitor(CAsmCode &AAsm, CFunctionSymbol *AFuncSym) : Asm(AAsm), FuncSym(AFuncSym), BlockNesting(0)
{
	OperationCmd["=="] = JE;
	OperationCmd["!="] = JNE;
	OperationCmd["<"] = JL;
	OperationCmd[">"] = JG;
	OperationCmd["<="] = JLE;
	OperationCmd[">="] = JGE;
	OperationCmd["+"] = ADD;
	OperationCmd["-"] = SUB;
	OperationCmd["*"] = IMUL;
	OperationCmd["&"] = AND;
	OperationCmd["|"] = OR;
	OperationCmd["^"] = XOR;
	OperationCmd["<<"] = SAL;
	OperationCmd[">>"] = SAR;
}

void CCodeGenerationVisitor::Visit(CUnaryOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);
	Asm.Add(POP, EAX);

	string OpName = AStmt.GetName();

	if (OpName == "++" || OpName == "--") {
		if (OpName == "--") {
			Asm.Add(DEC, EAX);
		} else if (OpName == "++") {
			Asm.Add(INC, EAX);
		}

		int off = dynamic_cast<CVariableSymbol *>(dynamic_cast<CVariable *>(AStmt.GetArgument())->GetSymbol())->GetOffset();
		Asm.Add(MOV, EAX, mem(-off, EBP));

	} else if (OpName == "*") {
		Asm.Add(MOV, mem(0, EAX), EAX);
	} else if (OpName == "&") {
		int off = dynamic_cast<CVariableSymbol *>(dynamic_cast<CVariable *>(AStmt.GetArgument())->GetSymbol())->GetOffset();
		Asm.Add(LEA, mem(-off, EBP), EAX);
	}

	Asm.Add(PUSH, EAX);

	// TODO: unary ops code gen..
}

void CCodeGenerationVisitor::Visit(CBinaryOp &AStmt)
{
	string OpName = AStmt.GetName();

	if (OpName == "=") {
		int off = dynamic_cast<CVariableSymbol *>(dynamic_cast<CVariable *>(AStmt.GetLeft())->GetSymbol())->GetOffset();

		AStmt.GetRight()->Accept(*this);

		Asm.Add(POP, EAX);
		Asm.Add(MOV, EAX, mem(-off, EBP));
		Asm.Add(PUSH, EAX);

		return;
	}

	AStmt.GetLeft()->Accept(*this);
	AStmt.GetRight()->Accept(*this);

	Asm.Add(POP, EBX);
	Asm.Add(POP, EAX);

	if (CTraits::IsTrivialOperation(OpName)) {
		Asm.Add(OperationCmd[OpName], EBX, EAX);
	} else if (OpName == "/" || OpName == "%") {
		Asm.Add(CDQ);
		Asm.Add(IDIV, EBX);
		if (OpName == "%") {
			Asm.Add(MOV, EDX, EAX);
		}
	} else if (OpName == "&&") {
		string FalseLabel = Asm.GenerateLabel();
		string EndCheckLabel = Asm.GenerateLabel();

		Asm.Add(CMP, 0, EAX);
		Asm.Add(JE, FalseLabel);
		Asm.Add(CMP, 0, EBX);
		Asm.Add(JE, FalseLabel);
		Asm.Add(MOV, 1, EAX);
		Asm.Add(JMP, EndCheckLabel);
		Asm.Add(new CAsmLabel(FalseLabel));
		Asm.Add(MOV, 0, EAX);
		Asm.Add(new CAsmLabel(EndCheckLabel));

	} else if (OpName == "||") {
		string TrueLabel = Asm.GenerateLabel();
		string EndCheckLabel = Asm.GenerateLabel();

		Asm.Add(CMP, 0, EAX);
		Asm.Add(JNE, TrueLabel);
		Asm.Add(CMP, 0, EBX);
		Asm.Add(JNE, TrueLabel);
		Asm.Add(MOV, 0, EAX);
		Asm.Add(JMP, EndCheckLabel);
		Asm.Add(new CAsmLabel(TrueLabel));
		Asm.Add(MOV, 1, EAX);
		Asm.Add(new CAsmLabel(EndCheckLabel));
	} else if (CTraits::IsComparisonOperation(OpName)) {
		string TrueLabel = Asm.GenerateLabel();
		string EndCheckLabel = Asm.GenerateLabel();

		Asm.Add(CMP, EBX, EAX);
		Asm.Add(OperationCmd[OpName], TrueLabel);
		Asm.Add(MOV, 0, EAX);
		Asm.Add(JMP, EndCheckLabel);
		Asm.Add(new CAsmLabel(TrueLabel));
		Asm.Add(MOV, 1, EAX);
		Asm.Add(new CAsmLabel(EndCheckLabel));
	} else if (OpName == ",") {
		Asm.Add(MOV, EBX, EAX);
	}

	Asm.Add(PUSH, EAX);
}

void CCodeGenerationVisitor::Visit(CConditionalOp &AStmt)
{
	AStmt.GetCondition()->Accept(*this);

	string ElseLabel = Asm.GenerateLabel();
	string ConditionalEndLabel = Asm.GenerateLabel();

	Asm.Add(POP, EAX);
	Asm.Add(CMP, 0, EAX);
	Asm.Add(JE, ElseLabel);

	AStmt.GetTrueExpr()->Accept(*this);

	Asm.Add(JMP, ConditionalEndLabel);
	Asm.Add(new CAsmLabel(ElseLabel));

	AStmt.GetFalseExpr()->Accept(*this);

	Asm.Add(new CAsmLabel(ConditionalEndLabel));
}

void CCodeGenerationVisitor::Visit(CIntegerConst &AStmt)
{
	Asm.Add(PUSH, AStmt.GetValue());
}

void CCodeGenerationVisitor::Visit(CFloatConst &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CSymbolConst &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CStringConst &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CVariable &AStmt)
{
	CVariableSymbol *VarSym = dynamic_cast<CVariableSymbol *>(AStmt.GetSymbol());
	int off = VarSym->GetOffset();
	Asm.Add(PUSH, mem(-off, EBP));
	//Asm.Add(PUSH, 
}

void CCodeGenerationVisitor::Visit(CPostfixOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);
	Asm.Add(POP, EAX);
	Asm.Add(MOV, EAX, EBX);

	string OpName = AStmt.GetName();

	if (OpName == "--") {
		Asm.Add(DEC, EAX);
	} else if (OpName == "++") {
		Asm.Add(INC, EAX);
	}

	int off = dynamic_cast<CVariableSymbol *>(dynamic_cast<CVariable *>(AStmt.GetArgument())->GetSymbol())->GetOffset();
	Asm.Add(MOV, EAX, mem(-off, EBP));

	Asm.Add(PUSH, EBX);

	// TODO: postfix ops code gen..
}

void CCodeGenerationVisitor::Visit(CFunctionCall &AStmt)
{
	for (CFunctionCall::ArgumentsReverseIterator it = AStmt.RBegin(); it != AStmt.REnd(); ++it) {
		(*it)->Accept(*this);
	}

	Asm.Add(CALL, AStmt.GetFunction()->GetName());
	Asm.Add(ADD, AStmt.GetFunction()->GetArgumentsSymbolTable()->GetElementsSize(), ESP);
	Asm.Add(PUSH, EAX);
}

void CCodeGenerationVisitor::Visit(CStructAccess &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CIndirectAccess &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CArrayAccess &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CNullStatement &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CBlockStatement &AStmt)
{
	Blocks.push(&AStmt);

	if (BlockNesting && AStmt.GetStatementsCount() == 0) {
		return;
	}

	if (!BlockNesting) {
		Asm.Add(PUSH, EBP);
		Asm.Add(MOV, ESP, EBP);
	}

	Asm.Add(SUB, AStmt.GetSymbolTable()->GetElementsSize(), ESP);

	BlockNesting++;

	for (CBlockStatement::StatementsIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		(*it)->Accept(*this);
		// TODO: if an expression yields a value, we should pop it from the stack..
	}

	BlockNesting--;

	if (BlockNesting == 0) {
		/*if (!ReturnEncountered && !FuncSym->GetReturnType()->IsVoid()) {
			throw CException("no `return` statement in function 
		}*/
		Asm.Add(new CAsmLabel(".RL" + FuncSym->GetName()));
	}

	Asm.Add(ADD, AStmt.GetSymbolTable()->GetElementsSize(), ESP);

	if (!BlockNesting) {
		Asm.Add(MOV, EBP, ESP);
		Asm.Add(POP, EBP);
	}

	Blocks.pop();
}

void CCodeGenerationVisitor::Visit(CIfStatement &AStmt)
{
	AStmt.GetCondition()->Accept(*this);

	string ElseLabel = Asm.GenerateLabel();
	string IfEndLabel = Asm.GenerateLabel();

	Asm.Add(POP, EAX);
	Asm.Add(CMP, 0, EAX);
	Asm.Add(JE, ElseLabel);

	TryVisit(AStmt.GetThenStatement());

	Asm.Add(JMP, IfEndLabel);
	Asm.Add(new CAsmLabel(ElseLabel));

	TryVisit(AStmt.GetElseStatement());

	Asm.Add(new CAsmLabel(IfEndLabel));
}

void CCodeGenerationVisitor::Visit(CForStatement &AStmt)
{
	string LoopStart = Asm.GenerateLabel();
	string LoopEnd = Asm.GenerateLabel();
	string LoopContinue = Asm.GenerateLabel();

	if (AStmt.GetInit()) {
		AStmt.GetInit()->Accept(*this);
		Asm.Add(POP, EAX);
	}

	Asm.Add(new CAsmLabel(LoopStart));

	if (AStmt.GetCondition()) {
		AStmt.GetCondition()->Accept(*this);
		Asm.Add(POP, EAX);
		Asm.Add(CMP, 0, EAX);
		Asm.Add(JE, LoopEnd);
	}

	BreakLabels.push(LoopEnd);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();

	Asm.Add(new CAsmLabel(LoopContinue));

	if (AStmt.GetUpdate()) {
		AStmt.GetUpdate()->Accept(*this);
		Asm.Add(POP, EAX);
	}
	Asm.Add(JMP, LoopStart);
	Asm.Add(new CAsmLabel(LoopEnd));
}

void CCodeGenerationVisitor::Visit(CWhileStatement &AStmt)
{
	string LoopStart = Asm.GenerateLabel();
	string LoopEnd = Asm.GenerateLabel();

	Asm.Add(new CAsmLabel(LoopStart));

	AStmt.GetCondition()->Accept(*this);
	Asm.Add(POP, EAX);
	Asm.Add(CMP, 0, EAX);
	Asm.Add(JE, LoopEnd);

	BreakLabels.push(LoopEnd);
	ContinueLabels.push(LoopStart);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();
	ContinueLabels.pop();

	Asm.Add(JMP, LoopStart);
	Asm.Add(new CAsmLabel(LoopEnd));
}

void CCodeGenerationVisitor::Visit(CDoStatement &AStmt)
{
	string LoopStart = Asm.GenerateLabel();
	string LoopEnd = Asm.GenerateLabel();
	string LoopContinue = Asm.GenerateLabel();

	Asm.Add(new CAsmLabel(LoopStart));

	BreakLabels.push(LoopEnd);
	ContinueLabels.push(LoopContinue);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();
	ContinueLabels.pop();

	Asm.Add(new CAsmLabel(LoopContinue));

	AStmt.GetCondition()->Accept(*this);
	Asm.Add(POP, EAX);
	Asm.Add(CMP, 0, EAX);
	Asm.Add(JNE, LoopStart);

	Asm.Add(new CAsmLabel(LoopEnd));
}

void CCodeGenerationVisitor::Visit(CLabel &AStmt)
{
	Asm.Add(new CAsmLabel(".CL" + FuncSym->GetName() + "_" + AStmt.GetName()));

	TryVisit(AStmt.GetNext());
}

void CCodeGenerationVisitor::Visit(CCaseLabel &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CDefaultCaseLabel &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CGotoStatement &AStmt)
{
	Asm.Add(JMP, ".CL" + FuncSym->GetName() + "_" + AStmt.GetLabelName());
}

void CCodeGenerationVisitor::Visit(CBreakStatement &AStmt)
{
	Asm.Add(JMP, BreakLabels.top());
}

void CCodeGenerationVisitor::Visit(CContinueStatement &AStmt)
{
	Asm.Add(JMP, ContinueLabels.top());
}

void CCodeGenerationVisitor::Visit(CReturnStatement &AStmt)
{
	if (AStmt.GetReturnExpression()) {
		AStmt.GetReturnExpression()->Accept(*this);
		Asm.Add(POP, EAX);
	}

	//Asm.Add(ADD, Blocks.top()->GetSymbolTable()->GetElementsSize(), ESP);
	Asm.Add(JMP, ".RL" + FuncSym->GetName());
}

void CCodeGenerationVisitor::Visit(CSwitchStatement &AStmt)
{
}

CAddressGenerationVisitor::CAddressGenerationVisitor(CAsmCode &AAsm) : Asm(AAsm)
{
}
