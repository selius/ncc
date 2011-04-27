#include "codegen.h"

#include "expressions.h"
#include "parser.h"
#include "optimization.h"

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
 * CAsmOp
 ******************************************************************************/

bool CAsmOp::IsReg() const
{
	return false;
}

bool CAsmOp::IsImm() const
{
	return false;
}

bool CAsmOp::IsMem() const
{
	return false;
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

bool CAsmReg::IsReg() const
{
	return true;
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

int CAsmImm::GetValue() const
{
	return Value;
}

bool CAsmImm::IsImm() const
{
	return true;
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

bool CAsmMem::IsMem() const
{
	return true;
}

CAsmMem* mem(int ADisplacement, ERegister ABase, ERegister AOffset /*= INVALID_REGISTER*/, int AMultiplier /*= 0*/)
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

CAsmOp* CAsmCmd1::GetOp() const
{
	return Op;
}

void CAsmCmd1::SetOp(CAsmOp *AOp)
{
	Op = AOp;
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

CAsmOp* CAsmCmd2::GetOp1() const
{
	return Op1;
}

CAsmOp* CAsmCmd2::GetOp2() const
{
	return Op2;
}

void CAsmCmd2::SetOp1(CAsmOp *AOp1)
{
	Op1 = AOp1;
}

void CAsmCmd2::SetOp2(CAsmOp *AOp2)
{
	Op2 = AOp2;
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
	MnemonicsText[JA] = "ja";
	MnemonicsText[JB] = "jb";
	MnemonicsText[JAE] = "jae";
	MnemonicsText[JBE] = "jbe";
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
	MnemonicsText[SAHF] = "sahf";
	MnemonicsText[FLD] = "fld";
	MnemonicsText[FILD] = "fild";
	MnemonicsText[FSTP] = "fstp";
	MnemonicsText[FISTTP] = "fisttpl";
	MnemonicsText[FCHS] = "fchs";
	MnemonicsText[FLD1] = "fld1";
	MnemonicsText[FADD] = "fadd";
	MnemonicsText[FSUBR] = "fsubr";
	MnemonicsText[FMUL] = "fmul";
	MnemonicsText[FDIVR] = "fdivr";
	MnemonicsText[FTST] = "ftst";
	MnemonicsText[FCOMP] = "fcomp";
	MnemonicsText[FCOMPP] = "fcompp";
	MnemonicsText[FSTSW] = "fstsw";

	RegistersText[EAX] = "eax";
	RegistersText[EBX] = "ebx";
	RegistersText[ECX] = "ecx";
	RegistersText[EDX] = "edx";
	RegistersText[ESI] = "esi";
	RegistersText[EDI] = "edi";
	RegistersText[ESP] = "esp";
	RegistersText[EBP] = "ebp";
	RegistersText[AX] = "ax";
	RegistersText[CL] = "cl";
	RegistersText[ST0] = "st(0)";
}

CAsmCode::~CAsmCode()
{
	for (CodeIterator it = Code.begin(); it != Code.end(); ++it) {
		delete *it;
	}
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

CAsmCode::CodeIterator CAsmCode::Insert(CAsmCode::CodeIterator APosition, CAsmCmd *ACmd)
{
	return Code.insert(APosition, ACmd);
}

CAsmCode::CodeIterator CAsmCode::Insert(CAsmCode::CodeIterator APosition, EMnemonic ACmd, CAsmOp *AOp1, CAsmOp *AOp2)
{
	return Code.insert(APosition, new CAsmCmd2(MnemonicsText[ACmd], AOp1, AOp2));
}

CAsmCode::CodeIterator CAsmCode::Insert(CAsmCode::CodeIterator APosition, EMnemonic ACmd, int AOp1, CAsmOp *AOp2)
{
	return Code.insert(APosition, new CAsmCmd2(MnemonicsText[ACmd], new CAsmImm(AOp1), AOp2));
}

CAsmCode::CodeIterator CAsmCode::Begin()
{
	return Code.begin();
}

CAsmCode::CodeIterator CAsmCode::End()
{
	return Code.end();
}

CAsmCode::CodeIterator CAsmCode::Erase(CAsmCode::CodeIterator APosition)
{
	if (APosition != End()) {
		delete *APosition;
	}

	return Code.erase(APosition);
}

string CAsmCode::AddStringLiteral(const string &ALiteral)
{
	if (StringLiterals.count(ALiteral)) {
		return StringLiterals[ALiteral];
	}

	string NewLiteralLabel = ".SL" + ToString(StringLiterals.size() + 1);

	StringLiterals[ALiteral] = NewLiteralLabel;

	return NewLiteralLabel;
}

void CAsmCode::AddGlobalVariable(CVariableSymbol *AVariable)
{
	GlobalVariables.push_back(AVariable);
}

string CAsmCode::GenerateLabel()
{
	return ".L" + ToString(++LabelsCount);
}

void CAsmCode::Output(ostream &Stream)
{
	Stream << ".data" << endl;
	for (map<string, string>::iterator it = StringLiterals.begin(); it != StringLiterals.end(); ++it) {
		Stream << it->second << ":" << endl;
		Stream << "\t.string\t\"" << it->first << "\"" << endl;
	}

	CVariableSymbol *Var;

	for (list<CVariableSymbol *>::iterator it = GlobalVariables.begin(); it != GlobalVariables.end(); ++it) {
		Var = *it;
		if (!Var->GetType()->IsScalar()) {
			Stream << ".comm\t" << Var->GetName() + "," + ToString(Var->GetType()->GetSize());
		} else {
			Stream << Var->GetName() << ":" << endl << "\t";

			if (Var->GetType()->IsFloat()) {
				Stream << ".float";
			} else {
				Stream << ".long";
			}

			Stream << "\t" << Var->GetInitValue();
		}

		Stream << endl;
	}

	Stream << ".text" << endl;

	for (CodeIterator it = Code.begin(); it != Code.end(); ++it) {
		Stream << (*it)->GetText() << endl;
	}

	Stream << ".end" << endl;
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

void CAddressGenerationVisitor::Visit(CCharConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CStringConst &AStmt)
{
}

void CAddressGenerationVisitor::Visit(CVariable &AStmt)
{
	if (AStmt.GetSymbol()->GetGlobal()) {
		Asm.Add(PUSH, "$" + AStmt.GetName());
	} else {
		Asm.Add(LEA, mem(AStmt.GetSymbol()->GetOffset(), EBP), EAX);
		Asm.Add(PUSH, EAX);
	}
}

void CAddressGenerationVisitor::Visit(CFunction &AStmt)
{
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
	if (AStmt.GetLeft()->GetResultType()->IsPointer()) {
		if (AStmt.GetLeft()->GetResultType()->IsArray()) {
			AStmt.GetLeft()->Accept(*this);
		} else {
			AStmt.GetLeft()->Accept(Code);
		}
		AStmt.GetRight()->Accept(Code);
	} else {
		if (AStmt.GetRight()->GetResultType()->IsArray()) {
			AStmt.GetRight()->Accept(*this);
		} else {
			AStmt.GetRight()->Accept(Code);
		}
		AStmt.GetLeft()->Accept(Code);
	}
	
	Asm.Add(POP, EAX);
	Asm.Add(MOV, AStmt.GetElementSize(), EBX);
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

CCodeGenerationVisitor::CCodeGenerationVisitor(CAsmCode &AAsm, bool AOptimize) : Asm(AAsm), FuncSym(NULL), BlockNesting(0), Addr(AAsm, *this), Optimize(AOptimize)
{
	IntOperationCmd[TOKEN_TYPE_OPERATION_EQUAL] = JE;
	IntOperationCmd[TOKEN_TYPE_OPERATION_NOT_EQUAL] = JNE;
	IntOperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN] = JL;
	IntOperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN] = JG;
	IntOperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL] = JLE;
	IntOperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL] = JGE;
	IntOperationCmd[TOKEN_TYPE_OPERATION_PLUS] = ADD;
	IntOperationCmd[TOKEN_TYPE_OPERATION_MINUS] = SUB;
	IntOperationCmd[TOKEN_TYPE_OPERATION_ASTERISK] = IMUL;
	IntOperationCmd[TOKEN_TYPE_OPERATION_AMPERSAND] = AND;
	IntOperationCmd[TOKEN_TYPE_OPERATION_BITWISE_OR] = OR;
	IntOperationCmd[TOKEN_TYPE_OPERATION_BITWISE_XOR] = XOR;
	IntOperationCmd[TOKEN_TYPE_OPERATION_SHIFT_LEFT] = SAL;
	IntOperationCmd[TOKEN_TYPE_OPERATION_SHIFT_RIGHT] = SAR;
	IntOperationCmd[TOKEN_TYPE_OPERATION_INCREMENT] = INC;
	IntOperationCmd[TOKEN_TYPE_OPERATION_DECREMENT] = DEC;

	FloatOperationCmd[TOKEN_TYPE_OPERATION_EQUAL] = JE;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_NOT_EQUAL] = JNE;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN] = JB;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN] = JA;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL] = JBE;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL] = JAE;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_PLUS] = FADD;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_MINUS] = FSUBR;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_ASTERISK] = FMUL;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_SLASH] = FDIVR;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_INCREMENT] = FADD;
	FloatOperationCmd[TOKEN_TYPE_OPERATION_DECREMENT] = FSUBR;

	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_PLUS_ASSIGN] = TOKEN_TYPE_OPERATION_PLUS;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_MINUS_ASSIGN] = TOKEN_TYPE_OPERATION_MINUS;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN] = TOKEN_TYPE_OPERATION_ASTERISK;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SLASH_ASSIGN] = TOKEN_TYPE_OPERATION_SLASH;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_PERCENT_ASSIGN] = TOKEN_TYPE_OPERATION_PERCENT;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN] = TOKEN_TYPE_OPERATION_BITWISE_OR;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN] = TOKEN_TYPE_OPERATION_AMPERSAND;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN] = TOKEN_TYPE_OPERATION_BITWISE_XOR;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN] = TOKEN_TYPE_OPERATION_SHIFT_LEFT;
	CompoundAssignmentOp[TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN] = TOKEN_TYPE_OPERATION_SHIFT_RIGHT;
}

void CCodeGenerationVisitor::SetFunction(CFunctionSymbol *AFuncSym)
{
	FuncSym = AFuncSym;
}

void CCodeGenerationVisitor::Visit(CUnaryOp &AStmt)
{
	ETokenType OpType = AStmt.GetType();
	CExpression *Arg = AStmt.GetArgument();

	if (OpType == TOKEN_TYPE_OPERATION_AMPERSAND) {
		Arg->Accept(Addr);
	} else if (OpType == TOKEN_TYPE_KEYWORD && AStmt.GetName() == "sizeof") {
		Asm.Add(PUSH, Arg->GetResultType()->GetSize());
	} else {
		if (Arg->GetResultType()->IsFloat()) {
			Arg->Accept(*this);

			if (OpType == TOKEN_TYPE_OPERATION_MINUS) {
				Asm.Add(FLD, mem(ESP));
				Asm.Add(FCHS);
				Asm.Add(FSTP, mem(ESP));

			} else if (OpType == TOKEN_TYPE_OPERATION_INCREMENT || OpType == TOKEN_TYPE_OPERATION_DECREMENT) {
				Arg->Accept(Addr);

				Asm.Add(POP, EBX);

				Asm.Add(FLD1);
				if (OpType == TOKEN_TYPE_OPERATION_INCREMENT) {
					Asm.Add(FADD, mem(ESP));
				} else {
					Asm.Add(FSUBR, mem(ESP));
				}
				Asm.Add(FSTP, mem(ESP));

				Asm.Add(MOV, mem(ESP), EAX);
				Asm.Add(MOV, EAX, mem(EBX));

			} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_NOT) {
				string TrueLabel = Asm.GenerateLabel();
				string EndLabel = Asm.GenerateLabel();

				Asm.Add(FLD, mem(ESP));
				Asm.Add(ADD, TypeSize::Float, ESP);

				Asm.Add(FTST);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);

				Asm.Add(JNE, TrueLabel);
				Asm.Add(MOV, 1, EAX);
				Asm.Add(JMP, EndLabel);
				Asm.Add(TrueLabel);
				Asm.Add(MOV, 0, EAX);
				Asm.Add(EndLabel);

				Asm.Add(PUSH, EAX);
			}
		} else {
			if (OpType == TOKEN_TYPE_OPERATION_INCREMENT || OpType == TOKEN_TYPE_OPERATION_DECREMENT) {
				Arg->Accept(Addr);
				Arg->Accept(*this);

				Asm.Add(POP, EAX);
				Asm.Add(POP, EBX);

				Asm.Add(IntOperationCmd[OpType], EAX);

				Asm.Add(MOV, EAX, mem(EBX));
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
	}
}

void CCodeGenerationVisitor::Visit(CBinaryOp &AStmt)
{
	ETokenType OpType = AStmt.GetType();

	if (OpType == TOKEN_TYPE_OPERATION_ASSIGN) {
		AStmt.GetLeft()->Accept(Addr);
		AStmt.GetRight()->Accept(*this);

		PerformConversion(AStmt.GetLeft()->GetResultType(), AStmt.GetRight()->GetResultType());

		Asm.Add(POP, EAX);
		Asm.Add(POP, EBX);

		Asm.Add(MOV, EAX, mem(EBX));
	} else if (OpType == TOKEN_TYPE_SEPARATOR_COMMA) {
		AStmt.GetLeft()->Accept(*this);
		AStmt.GetRight()->Accept(*this);
		Asm.Add(POP, EBX);
		Asm.Add(POP, EAX);
		Asm.Add(MOV, EBX, EAX);
	} else {
		bool CompoundAssignment = false;

		if (TokenTraits::IsCompoundAssignment(OpType)) {
			AStmt.GetLeft()->Accept(Addr);
			OpType = CompoundAssignmentOp[OpType];
			CompoundAssignment = true;
		}

		AStmt.GetLeft()->Accept(*this);
		PerformConversion(AStmt.GetCommonRealType(), AStmt.GetLeft()->GetResultType());

		AStmt.GetRight()->Accept(*this);
		PerformConversion(AStmt.GetCommonRealType(), AStmt.GetRight()->GetResultType());

		if (AStmt.GetCommonRealType()->IsFloat()) {
			if (TokenTraits::IsTrivialOperation(OpType) || OpType == TOKEN_TYPE_OPERATION_SLASH) {
				Asm.Add(FLD, mem(ESP));
				Asm.Add(ADD, TypeSize::Float, ESP);
				Asm.Add(FloatOperationCmd[OpType], mem(ESP));
				Asm.Add(FSTP, mem(ESP));
				Asm.Add(POP, EAX);

			} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_AND) {
				string FalseLabel = Asm.GenerateLabel();
				string EndCheckLabel = Asm.GenerateLabel();

				Asm.Add(FLD, mem(TypeSize::Float, ESP));

				Asm.Add(FTST);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);
				Asm.Add(JE, FalseLabel);

				Asm.Add(FLD, mem(ESP));

				Asm.Add(FTST);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);
				Asm.Add(JE, FalseLabel);

				Asm.Add(MOV, 1, EAX);
				Asm.Add(JMP, EndCheckLabel);
				Asm.Add(FalseLabel);
				Asm.Add(MOV, 0, EAX);
				Asm.Add(EndCheckLabel);

				Asm.Add(FSTP, ST0);
				Asm.Add(FSTP, ST0);
				Asm.Add(ADD, 2 * TypeSize::Float, ESP);

			} else if (OpType == TOKEN_TYPE_OPERATION_LOGIC_OR) {
				string TrueLabel = Asm.GenerateLabel();
				string EndCheckLabel = Asm.GenerateLabel();

				Asm.Add(FLD, mem(TypeSize::Float, ESP));

				Asm.Add(FTST);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);
				Asm.Add(JNE, TrueLabel);

				Asm.Add(FLD, mem(ESP));

				Asm.Add(FTST);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);
				Asm.Add(JNE, TrueLabel);

				Asm.Add(MOV, 0, EAX);
				Asm.Add(JMP, EndCheckLabel);
				Asm.Add(TrueLabel);
				Asm.Add(MOV, 1, EAX);
				Asm.Add(EndCheckLabel);

				Asm.Add(FSTP, ST0);
				Asm.Add(FSTP, ST0);
				Asm.Add(ADD, 2 * TypeSize::Float, ESP);

			} else if (TokenTraits::IsComparisonOperation(OpType)) {
				string TrueLabel = Asm.GenerateLabel();
				string EndCheckLabel = Asm.GenerateLabel();

				Asm.Add(FLD, mem(ESP));
				Asm.Add(FLD, mem(TypeSize::Float, ESP));

				Asm.Add(FCOMPP);
				Asm.Add(FSTSW, AX);
				Asm.Add(SAHF);

				Asm.Add(FloatOperationCmd[OpType], TrueLabel);
				Asm.Add(MOV, 0, EAX);
				Asm.Add(JMP, EndCheckLabel);
				Asm.Add(TrueLabel);
				Asm.Add(MOV, 1, EAX);
				Asm.Add(EndCheckLabel);

				Asm.Add(ADD, 2 * TypeSize::Float, ESP);
			}
		} else {
			Asm.Add(POP, EBX);
			Asm.Add(POP, EAX);

			if (TokenTraits::IsTrivialOperation(OpType)) {
				Asm.Add(IntOperationCmd[OpType], EBX, EAX);
			} else if (OpType == TOKEN_TYPE_OPERATION_SLASH || OpType == TOKEN_TYPE_OPERATION_PERCENT) {
				Asm.Add(CDQ);
				Asm.Add(IDIV, EBX);
				if (OpType == TOKEN_TYPE_OPERATION_PERCENT) {
					Asm.Add(MOV, EDX, EAX);
				}
			} else if (OpType == TOKEN_TYPE_OPERATION_SHIFT_LEFT || OpType == TOKEN_TYPE_OPERATION_SHIFT_RIGHT) {
				Asm.Add(MOV, EBX, ECX);
				Asm.Add(IntOperationCmd[OpType], CL, EAX);

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

			} else if (TokenTraits::IsComparisonOperation(OpType)) {
				string TrueLabel = Asm.GenerateLabel();
				string EndCheckLabel = Asm.GenerateLabel();

				Asm.Add(CMP, EBX, EAX);
				Asm.Add(IntOperationCmd[OpType], TrueLabel);
				Asm.Add(MOV, 0, EAX);
				Asm.Add(JMP, EndCheckLabel);
				Asm.Add(TrueLabel);
				Asm.Add(MOV, 1, EAX);
				Asm.Add(EndCheckLabel);
			}

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

	if (AStmt.GetCondition()->GetResultType()->IsFloat()) {
		Asm.Add(FLD, mem(ESP));
		Asm.Add(ADD, TypeSize::Float, ESP);

		Asm.Add(FTST);
		Asm.Add(FSTSW, AX);
		Asm.Add(SAHF);
	} else {
		Asm.Add(POP, EAX);
		Asm.Add(CMP, 0, EAX);
	}

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
	// this dirty hack probably will not work on other archs..

	float value = AStmt.GetValue();
	int32_t int_rep = *((int32_t *) &value);
	Asm.Add(PUSH, int_rep);
}

void CCodeGenerationVisitor::Visit(CCharConst &AStmt)
{
	Asm.Add(PUSH, AStmt.GetValue());
}

void CCodeGenerationVisitor::Visit(CStringConst &AStmt)
{
	Asm.Add(PUSH, "$" + Asm.AddStringLiteral(AStmt.GetValue()));
}

void CCodeGenerationVisitor::Visit(CVariable &AStmt)
{
	if (AStmt.GetSymbol()->GetType()->IsArray()) {
		AStmt.Accept(Addr);
	} else {
		if (AStmt.GetSymbol()->GetGlobal()) {
			Asm.Add(PUSH, AStmt.GetSymbol()->GetName());
		} else {
			Asm.Add(PUSH, mem(AStmt.GetSymbol()->GetOffset(), EBP));
		}
	}
}

void CCodeGenerationVisitor::Visit(CFunction &AStmt)
{
}

void CCodeGenerationVisitor::Visit(CPostfixOp &AStmt)
{
	AStmt.GetArgument()->Accept(*this);
	AStmt.GetArgument()->Accept(Addr);

	Asm.Add(POP, EBX);
	Asm.Add(MOV, mem(ESP), EAX);

	if (AStmt.GetArgument()->GetResultType()->IsFloat()) {
		Asm.Add(FLD1);

		Asm.Add(FloatOperationCmd[AStmt.GetType()], mem(ESP));

		Asm.Add(FSTP, mem(-TypeSize::Float, ESP));
		Asm.Add(MOV, mem(-TypeSize::Float, ESP), EAX);
	} else {
		Asm.Add(IntOperationCmd[AStmt.GetType()], EAX);
	}

	Asm.Add(MOV, EAX, mem(EBX));
}

void CCodeGenerationVisitor::Visit(CFunctionCall &AStmt)
{
	CFunctionSymbol *Func = AStmt.GetFunction();
	CFunctionSymbol::ArgumentsOrderContainer *FormalArgs = Func->GetArgumentsOrderedList();

	CFunctionCall::ArgumentsReverseIterator ait;
	CFunctionSymbol::ArgumentsReverseOrderIterator fit;

	for (ait = AStmt.RBegin(), fit = FormalArgs->rbegin(); ait != AStmt.REnd() && fit != FormalArgs->rend(); ++ait, ++fit) {
		(*ait)->Accept(*this);
		PerformConversion(static_cast<CVariableSymbol *>(*fit)->GetType(), (*ait)->GetResultType());
	}

	Asm.Add(CALL, Func->GetName());
	Asm.Add(ADD, Func->GetArgumentsSymbolTable()->GetElementsSize(), ESP);

	if (!Func->GetReturnType()->IsVoid()) {
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
	if (AStmt.GetLeft()->GetResultType()->IsPointer()) {
		if (AStmt.GetLeft()->GetResultType()->IsArray()) {
			AStmt.GetLeft()->Accept(Addr);
		} else {
			AStmt.GetLeft()->Accept(*this);
		}

		AStmt.GetRight()->Accept(*this);
	} else {
		if (AStmt.GetRight()->GetResultType()->IsArray()) {
			AStmt.GetRight()->Accept(Addr);
		} else {
			AStmt.GetRight()->Accept(*this);
		}
		AStmt.GetLeft()->Accept(*this);
	}

	Asm.Add(POP, EAX);
	Asm.Add(MOV, AStmt.GetElementSize(), EBX);
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
		Asm.Add(new CAsmDirective("globl", FuncSym->GetName()));
		Asm.Add(FuncSym->GetName());

		if (!Optimize || AStmt.GetSymbolTable()->GetElementsSize() != 0 || FuncSym->GetArgumentsSymbolTable()->GetElementsSize() != 0) {
			Asm.Add(PUSH, EBP);
			Asm.Add(MOV, ESP, EBP);
		}
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

	if (!BlockNesting) {
		Asm.Add(".RL" + FuncSym->GetName());
	}

	Asm.Add(ADD, AStmt.GetSymbolTable()->GetElementsSize(), ESP);

	if (!BlockNesting) {
		if (!Optimize || AStmt.GetSymbolTable()->GetElementsSize() != 0 || FuncSym->GetArgumentsSymbolTable()->GetElementsSize() != 0) {
			Asm.Add(MOV, EBP, ESP);
			Asm.Add(POP, EBP);
		}

		Asm.Add(RET);
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
	ContinueLabels.push(LoopContinue);

	AStmt.GetBody()->Accept(*this);

	BreakLabels.pop();
	ContinueLabels.pop();

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

		PerformConversion(FuncSym->GetReturnType(), AStmt.GetReturnExpression()->GetResultType());

		Asm.Add(POP, EAX);
	}

	Asm.Add(JMP, ".RL" + FuncSym->GetName());
}

void CCodeGenerationVisitor::Visit(CSwitchStatement &AStmt)
{
	AStmt.GetTestExpression()->Accept(*this);

	Asm.Add(POP, EDX);

	string CaseLabelName;

	for (CSwitchStatement::CasesIterator it = AStmt.Begin(); it != AStmt.End(); ++it) {
		Asm.Add(MOV, it->second->GetValue(), EAX);

		CaseLabelName = Asm.GenerateLabel();
		it->second->SetName(CaseLabelName);

		Asm.Add(CMP, EAX, EDX);
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

void CCodeGenerationVisitor::ConvertFloatToInt()
{
	Asm.Add(FLD, mem(ESP));
	Asm.Add(FISTTP, mem(ESP));
}

void CCodeGenerationVisitor::ConvertIntToFloat()
{
	Asm.Add(FILD, mem(ESP));
	Asm.Add(FSTP, mem(ESP));
}

void CCodeGenerationVisitor::PerformConversion(CTypeSymbol *LHS, CTypeSymbol *RHS)
{
	if (LHS->IsInt() && RHS->IsFloat()) {
		ConvertFloatToInt();
	} else if (LHS->IsFloat() && RHS->IsInt()) {
		ConvertIntToFloat();
	}
}

/******************************************************************************
 * CCodeGenerator
 ******************************************************************************/

CCodeGenerator::CCodeGenerator(CParser &AParser, const CCompilerParameters &AParameters) : Parser(AParser), Parameters(AParameters), Visitor(Code, Parameters.Optimize)
{
}

void CCodeGenerator::Output(ostream &Stream)
{

	CGlobalSymbolTable *SymTable = Parser.ParseTranslationUnit();

	for (CGlobalSymbolTable::VariablesIterator it = SymTable->VariablesBegin(); it != SymTable->VariablesEnd(); ++it) {
		Code.AddGlobalVariable(it->second);
	}

	ofstream *TreeStream = NULL;
	if (!Parameters.TreeFilename.empty()) {
		TreeStream = new ofstream(Parameters.TreeFilename.c_str());
	}

	CFunctionSymbol *FuncSym = NULL;

	for (CGlobalSymbolTable::FunctionsIterator it = SymTable->FunctionsBegin(); it != SymTable->FunctionsEnd(); ++it) {
		FuncSym = it->second;

		if (FuncSym->GetBody()) {
			if (Parameters.Optimize) {
				CConstantFolding cf;
				FuncSym->GetBody()->Accept(cf);

				CUnreachableCodeElimination uce;
				FuncSym->GetBody()->Accept(uce);

				CLoopInvariantHoisting lih;
				FuncSym->GetBody()->Accept(lih);
			}

			if (TreeStream) {
				CStatementTreePrintVisitor stpv(*TreeStream);
				*TreeStream << FuncSym->GetName() << ":" << endl;
				FuncSym->GetBody()->Accept(stpv);
			}

			Visitor.SetFunction(FuncSym);
			FuncSym->GetBody()->Accept(Visitor);
		}
	}

	delete TreeStream;

	if (Parameters.Optimize) {
		CLowLevelOptimizer optimizer(Code);
		optimizer.Optimize();
	}

	Code.Output(Stream);
}
