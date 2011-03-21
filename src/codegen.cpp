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

CAsmMem::CAsmMem(int ADisplacement, ERegister ABase, ERegister AOffset, int AMultiplier) : Displacement(ADisplacement), Base(ABase), Offset(AOffset), Multiplier(AMultiplier)
{
}

string CAsmMem::GetText() const
{
	string result;

	result += Displacement ? ToString(Displacement) : "";
	result += "(";
	result += (Base != INVALID_REGISTER) ? "%" + RegistersText[Base] : "";
	if (Offset != INVALID_REGISTER || Multiplier) {
		result += ", ";
	}
	result += (Offset != INVALID_REGISTER) ? "%" + RegistersText[Offset] : "";
	result += Multiplier ? ", " + ToString(Multiplier) : "";
	result += ")";

	return result;
}

CAsmMem* mem(int ADisplacement, ERegister ABase, ERegister AOffset = INVALID_REGISTER, int AMultiplier = 0)
{
	return new CAsmMem(ADisplacement, ABase, AOffset, AMultiplier);
}

CAsmMem* mem(ERegister ABase)
{
	return new CAsmMem(0, ABase, INVALID_REGISTER, 0);
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
	MnemonicsText[NEG] = "neg";
	MnemonicsText[CMP] = "cmp";
	MnemonicsText[CDQ] = "cdq";
	MnemonicsText[NOT] = "not";
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

void CAsmCode::Add(const string &ALabel)
{
	Code.push_back(new CAsmLabel(ALabel));
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
 * CAddressGenerationVisitor
 ******************************************************************************/

CAddressGenerationVisitor::CAddressGenerationVisitor(CAsmCode &AAsm, CCodeGenerationVisitor &ACode) : Asm(AAsm), Code(ACode)
{
}

void CAddressGenerationVisitor::Visit(CUnaryOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);
	Asm.Add(POP, EAX);

	if (AStmt.GetType() == TOKEN_TYPE_OPERATION_ASTERISK) {
		Asm.Add(PUSH, mem(EAX));
	}
}

void CAddressGenerationVisitor::Visit(CBinaryOp &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CConditionalOp &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CIntegerConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CFloatConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CSymbolConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CStringConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CVariable &AStmt)
{
	Asm.Add(LEA, mem(AStmt.GetSymbol()->GetOffset(), EBP), EAX);
	Asm.Add(PUSH, EAX);
}

void CAddressGenerationVisitor::Visit(CPostfixOp &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CFunctionCall &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CStructAccess &AStmt)
{
	AStmt.GetStruct()->Accept(*this);
	Asm.Add(POP, EBX);

	Asm.Add(MOV, AStmt.GetField()->GetSymbol()->GetOffset(), EAX);

	Asm.Add(LEA, mem(0, EBX, EAX, 1), EAX);
	Asm.Add(PUSH, EAX);
}

void CAddressGenerationVisitor::Visit(CIndirectAccess &AStmt)
{
	AStmt.GetPointer()->Accept(Code);
	Asm.Add(POP, EBX);

	Asm.Add(MOV, AStmt.GetField()->GetSymbol()->GetOffset(), EAX);

	Asm.Add(LEA, mem(0, EBX, EAX, 1), EAX);
	Asm.Add(PUSH, EAX);
}

void CAddressGenerationVisitor::Visit(CArrayAccess &AStmt)
{
	int ElemSize;

	if (AStmt.GetLeft()->GetResultType()->IsArray()) {
		AStmt.GetLeft()->Accept(*this);
		AStmt.GetRight()->Accept(Code);
		ElemSize = static_cast<CArraySymbol *>(AStmt.GetLeft()->GetResultType())->GetElementsType()->GetSize();
	} else {
		AStmt.GetRight()->Accept(*this);
		AStmt.GetLeft()->Accept(Code);
		ElemSize = static_cast<CArraySymbol *>(AStmt.GetRight()->GetResultType())->GetElementsType()->GetSize();
	}
	
	Asm.Add(POP, EAX);
	Asm.Add(MOV, ElemSize, EBX);
	Asm.Add(IMUL, EBX, EAX);
	Asm.Add(POP, EBX);

	Asm.Add(LEA, mem(0, EBX, EAX, 1), EAX);
	Asm.Add(PUSH, EAX);
}

void CAddressGenerationVisitor::Visit(CNullStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CBlockStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CIfStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CForStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CWhileStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CDoStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CLabel &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CCaseLabel &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CDefaultCaseLabel &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CGotoStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CBreakStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CContinueStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CReturnStatement &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CSwitchStatement &AStmt)
{
}

/******************************************************************************
 * CCodeGenerationVisitor
 ******************************************************************************/

CCodeGenerationVisitor::CCodeGenerationVisitor(CAsmCode &AAsm, CFunctionSymbol *AFuncSym) : Asm(AAsm), FuncSym(AFuncSym), BlockNesting(0), Addr(AAsm, *this)
{
	OperationCmd[TOKEN_TYPE_OPERATION_EQUAL] = JE;
	OperationCmd[TOKEN_TYPE_OPERATION_NOT_EQUAL] = JNE;
	OperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN] = JL;
	OperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN] = JG;
	OperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL] = JLE;
	OperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL] = JGE;
	OperationCmd[TOKEN_TYPE_OPERATION_PLUS] = ADD;
	OperationCmd[TOKEN_TYPE_OPERATION_MINUS] = SUB;
	OperationCmd[TOKEN_TYPE_OPERATION_ASTERISK] = IMUL;
	OperationCmd[TOKEN_TYPE_OPERATION_AMPERSAND] = AND;
	OperationCmd[TOKEN_TYPE_OPERATION_BITWISE_OR] = OR;
	OperationCmd[TOKEN_TYPE_OPERATION_BITWISE_XOR] = XOR;
	OperationCmd[TOKEN_TYPE_OPERATION_SHIFT_LEFT] = SAL;
	OperationCmd[TOKEN_TYPE_OPERATION_SHIFT_RIGHT] = SAR;
	OperationCmd[TOKEN_TYPE_OPERATION_INCREMENT] = INC;
	OperationCmd[TOKEN_TYPE_OPERATION_DECREMENT] = DEC;

	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_PLUS_ASSIGN] = TOKEN_TYPE_OPERATION_PLUS;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_MINUS_ASSIGN] = TOKEN_TYPE_OPERATION_MINUS;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN] = TOKEN_TYPE_OPERATION_ASTERISK;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SLASH_ASSIGN] = TOKEN_TYPE_OPERATION_SLASH;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_PERCENT_ASSIGN] = TOKEN_TYPE_OPERATION_PERCENT;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN] = TOKEN_TYPE_OPERATION_BITWISE_NOT;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN] = TOKEN_TYPE_OPERATION_BITWISE_OR;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN] = TOKEN_TYPE_OPERATION_AMPERSAND;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN] = TOKEN_TYPE_OPERATION_BITWISE_XOR;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN] = TOKEN_TYPE_OPERATION_SHIFT_LEFT;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN] = TOKEN_TYPE_OPERATION_SHIFT_RIGHT;
}

void CCodeGenerationVisitor::Visit(CUnaryOp &AStmt)
{
	ETokenType OpType = AStmt.GetType();
	CExpression *Arg = AStmt.GetArgument();

	if (OpType == TOKEN_TYPE_OPERATION_INCREMENT || OpType == TOKEN_TYPE_OPERATION_DECREMENT) {
		Arg->Accept(Addr);
		Arg->Accept(*this);

		Asm.Add(POP, EAX);
		Asm.Add(POP, EBX);

		Asm.Add(OperationCmd[OpType], EAX);

		Asm.Add(MOV, EAX, mem(EBX));
	} else if (OpType == TOKEN_TYPE_OPERATION_AMPERSAND) {
		Arg->Accept(Addr);
		Asm.Add(POP, EAX);
	} else if (OpType == TOKEN_TYPE_KEYWORD && AStmt.GetName() == "sizeof") {
		Asm.Add(MOV, Arg->GetResultType()->GetSize(), EAX);
	} else {
		Arg->Accept(*this);
		Asm.Add(POP, EAX);

		if (OpType == TOKEN_TYPE_OPERATION_ASTERISK) {
			Asm.Add(MOV, mem(EAX), EAX);
		} else if (OpType == TOKEN_TYPE_OPERATION_MINUS) {
			Asm.Add(NEG, EAX);
		} else if (OpType == TOKEN_TYPE_OPERATION_BITWISE_NOT) {
			Asm.Add(NOT, EAX);
		} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_NOT) {
			string TrueLabel = Asm.GenerateLabel();
			string EndLabel = Asm.GenerateLabel();

			Asm.Add(CMP, 0, EAX);
			Asm.Add(JNE, TrueLabel);
			Asm.Add(MOV, 1, EAX);
			Asm.Add(JMP, EndLabel);
			Asm.Add(TrueLabel);
			Asm.Add(MOV, 0, EAX);
			Asm.Add(EndLabel);
		}
	}

	Asm.Add(PUSH, EAX);
}

void CCodeGenerationVisitor::Visit(CBinaryOp &AStmt)
{
	ETokenType OpType = AStmt.GetType();

	if (OpType == TOKEN_TYPE_OPERATION_ASSIGN) {
		AStmt.GetLeft()->Accept(Addr);
		AStmt.GetRight()->Accept(*this);

		Asm.Add(POP, EAX);
		Asm.Add(POP, EBX);

		Asm.Add(MOV, EAX, mem(EBX));
	} else {
		bool CompoundAssignment = false;

		if (CTraits::IsCompoundAssignment(OpType)) {
			AStmt.GetLeft()->Accept(Addr);
			OpType = CompoundAssignmentOp[OpType];
			CompoundAssignment = true;
		}

		AStmt.GetLeft()->Accept(*this);
		AStmt.GetRight()->Accept(*this);

		Asm.Add(POP, EBX);
		Asm.Add(POP, EAX);

		if (CTraits::IsTrivialOperation(OpType)) {
			Asm.Add(OperationCmd[OpType], EBX, EAX);
		} else if (OpType == TOKEN_TYPE_OPERATION_SLASH || OpType == TOKEN_TYPE_OPERATION_PERCENT) {
			Asm.Add(CDQ);
			Asm.Add(IDIV, EBX);
			if (OpType == TOKEN_TYPE_OPERATION_PERCENT) {
				Asm.Add(MOV, EDX, EAX);
			}
		} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_AND) {
			string FalseLabel = Asm.GenerateLabel();
			string EndCheckLabel = Asm.GenerateLabel();

			Asm.Add(CMP, 0, EAX);
			Asm.Add(JE, FalseLabel);
			Asm.Add(CMP, 0, EBX);
			Asm.Add(JE, FalseLabel);
			Asm.Add(MOV, 1, EAX);
			Asm.Add(JMP, EndCheckLabel);
			Asm.Add(FalseLabel);
			Asm.Add(MOV, 0, EAX);
			Asm.Add(EndCheckLabel);

		} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_OR) {
			string TrueLabel = Asm.GenerateLabel();
			string EndCheckLabel = Asm.GenerateLabel();

			Asm.Add(CMP, 0, EAX);
			Asm.Add(JNE, TrueLabel);
			Asm.Add(CMP, 0, EBX);
			Asm.Add(JNE, TrueLabel);
			Asm.Add(MOV, 0, EAX);
			Asm.Add(JMP, EndCheckLabel);
			Asm.Add(TrueLabel);
			Asm.Add(MOV, 1, EAX);
			Asm.Add(EndCheckLabel);

		} else if (CTraits::IsComparisonOperation(OpType)) {
			string TrueLabel = Asm.GenerateLabel();
			string EndCheckLabel = Asm.GenerateLabel();

			Asm.Add(CMP, EBX, EAX);
			Asm.Add(OperationCmd[OpType], TrueLabel);
			Asm.Add(MOV, 0, EAX);
			Asm.Add(JMP, EndCheckLabel);
			Asm.Add(TrueLabel);
			Asm.Add(MOV, 1, EAX);
			Asm.Add(EndCheckLabel);

		} else if (OpType == TOKEN_TYPE_SEPARATOR_COMMA) {
			Asm.Add(MOV, EBX, EAX);
		}

		if (CompoundAssignment) {
			Asm.Add(POP, EBX);
			Asm.Add(MOV, EAX, mem(EBX));
		}
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
	Asm.Add(ElseLabel);

	AStmt.GetFalseExpr()->Accept(*this);

	Asm.Add(ConditionalEndLabel);
}

void CCodeGenerationVisitor::Visit(CIntegerConst &AStmt)
{
	Asm.Add(PUSH, AStmt.GetValue());
}

void CCodeGenerationVisitor::Visit(CFloatConst &AStmt)
{
	// DIRTY HACK: probably will not work on other archs..

	float value = AStmt.GetValue();
	int32_t int_rep = *((int32_t *) &value);
	Asm.Add(PUSH, int_rep);
}

void CCodeGenerationVisitor::Visit(CSymbolConst &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CStringConst &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CVariable &AStmt)
{
	Asm.Add(PUSH, mem(AStmt.GetSymbol()->GetOffset(), EBP));
}

void CCodeGenerationVisitor::Visit(CPostfixOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);
	AStmt.GetArgument()->Accept(Addr);

	Asm.Add(POP, EBX);
	Asm.Add(MOV, mem(ESP), EAX);

	Asm.Add(OperationCmd[AStmt.GetType()], EAX);

	Asm.Add(MOV, EAX, mem(EBX));
}

void CCodeGenerationVisitor::Visit(CFunctionCall &AStmt)
{
	for (CFunctionCall::ArgumentsReverseIterator it = AStmt.RBegin(); it != AStmt.REnd(); ++it) {
		(*it)->Accept(*this);
	}

	Asm.Add(CALL, AStmt.GetFunction()->GetName());
	Asm.Add(ADD, AStmt.GetFunction()->GetArgumentsSymbolTable()->GetElementsSize(), ESP);

	if (!AStmt.GetFunction()->GetReturnType()->IsVoid()) {
		Asm.Add(PUSH, EAX);
	}
}

void CCodeGenerationVisitor::Visit(CStructAccess &AStmt)
{
	AStmt.GetStruct()->Accept(Addr);
	Asm.Add(POP, EBX);

	Asm.Add(MOV, AStmt.GetField()->GetSymbol()->GetOffset(), EAX);

	Asm.Add(PUSH, mem(0, EBX, EAX, 1));
}

void CCodeGenerationVisitor::Visit(CIndirectAccess &AStmt)
{
	AStmt.GetPointer()->Accept(*this);
	Asm.Add(POP, EBX);

	Asm.Add(MOV, AStmt.GetField()->GetSymbol()->GetOffset(), EAX);

	Asm.Add(PUSH, mem(0, EBX, EAX, 1));
}

void CCodeGenerationVisitor::Visit(CArrayAccess &AStmt)
{
	int ElemSize;

	if (AStmt.GetLeft()->GetResultType()->IsArray()) {
		AStmt.GetLeft()->Accept(Addr);
		AStmt.GetRight()->Accept(*this);
		ElemSize = static_cast<CArraySymbol *>(AStmt.GetLeft()->GetResultType())->GetElementsType()->GetSize();
	} else {
		AStmt.GetRight()->Accept(Addr);
		AStmt.GetLeft()->Accept(*this);
		ElemSize = static_cast<CArraySymbol *>(AStmt.GetRight()->GetResultType())->GetElementsType()->GetSize();
	}

	Asm.Add(POP, EAX);
	Asm.Add(MOV, ElemSize, EBX);
	Asm.Add(IMUL, EBX, EAX);
	Asm.Add(POP, EBX);

	Asm.Add(PUSH, mem(0, EBX, EAX, 1));
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

		if ((*it)->IsExpression()) {
			Asm.Add(POP, EAX);
		}
	}

	BlockNesting--;

	if (BlockNesting == 0) {
		/*if (!ReturnEncountered && !FuncSym->GetReturnType()->IsVoid()) {
			throw CException("no `return` statement in function 
		}*/
		Asm.Add(".RL" + FuncSym->GetName());
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
	Asm.Add(ElseLabel);

	TryVisit(AStmt.GetElseStatement());

	Asm.Add(IfEndLabel);
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

	Asm.Add(LoopStart);

	if (AStmt.GetCondition()) {
		AStmt.GetCondition()->Accept(*this);
		Asm.Add(POP, EAX);
		Asm.Add(CMP, 0, EAX);
		Asm.Add(JE, LoopEnd);
	}

	BreakLabels.push(LoopEnd);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();

	Asm.Add(LoopContinue);

	if (AStmt.GetUpdate()) {
		AStmt.GetUpdate()->Accept(*this);
		Asm.Add(POP, EAX);
	}
	Asm.Add(JMP, LoopStart);
	Asm.Add(LoopEnd);
}

void CCodeGenerationVisitor::Visit(CWhileStatement &AStmt)
{
	string LoopStart = Asm.GenerateLabel();
	string LoopEnd = Asm.GenerateLabel();

	Asm.Add(LoopStart);

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
	Asm.Add(LoopEnd);
}

void CCodeGenerationVisitor::Visit(CDoStatement &AStmt)
{
	string LoopStart = Asm.GenerateLabel();
	string LoopEnd = Asm.GenerateLabel();
	string LoopContinue = Asm.GenerateLabel();

	Asm.Add(LoopStart);

	BreakLabels.push(LoopEnd);
	ContinueLabels.push(LoopContinue);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();
	ContinueLabels.pop();

	Asm.Add(LoopContinue);

	AStmt.GetCondition()->Accept(*this);
	Asm.Add(POP, EAX);
	Asm.Add(CMP, 0, EAX);
	Asm.Add(JNE, LoopStart);

	Asm.Add(LoopEnd);
}

void CCodeGenerationVisitor::Visit(CLabel &AStmt)
{
	Asm.Add(".CL" + FuncSym->GetName() + "_" + AStmt.GetName());

	TryVisit(AStmt.GetNext());
}

void CCodeGenerationVisitor::Visit(CCaseLabel &AStmt)
{
	Asm.Add(AStmt.GetName());
	TryVisit(AStmt.GetNext());
}

void CCodeGenerationVisitor::Visit(CDefaultCaseLabel &AStmt)
{
	Asm.Add(AStmt.GetName());
	TryVisit(AStmt.GetNext());
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
	AStmt.GetTestExpression()->Accept(*this);

	Asm.Add(POP, ECX);

	string CaseLabelName;

	for (CSwitchStatement::CasesIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		it->second->GetCaseExpression()->Accept(*this);

		CaseLabelName = Asm.GenerateLabel();
		it->second->SetName(CaseLabelName);

		Asm.Add(POP, EAX);
		Asm.Add(CMP, EAX, ECX);
		Asm.Add(JE, CaseLabelName);
	}

	if (AStmt.GetDefaultCase()) {
		CaseLabelName = Asm.GenerateLabel();
		AStmt.GetDefaultCase()->SetName(CaseLabelName);
		Asm.Add(JMP, CaseLabelName);
	}

	CaseLabelName = Asm.GenerateLabel();
	Asm.Add(JMP, CaseLabelName);

	BreakLabels.push(CaseLabelName);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();

	Asm.Add(CaseLabelName);
}
