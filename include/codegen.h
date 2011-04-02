#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "common.h"
#include "symbols.h"
#include "parser.h"
#include "prettyprinting.h"

enum ERegister
{
	EAX,
	EBX,
	ECX,
	EDX,
	ESI,
	EDI,
	ESP,
	EBP,
	AX,
	CL,
	ST0,
	INVALID_REGISTER,
	// possibly add FPU registers
};

enum EMnemonic
{
	MOV,
	PUSH,
	POP,
	RET,
	CALL,
	JMP,
	JE,
	JNE,
	JL,
	JG,
	JLE,
	JGE,
	JA,
	JB,
	JAE,
	JBE,
	ADD,
	SUB,
	MUL,
	IMUL,
	DIV,
	IDIV,
	INC,
	DEC,
	NEG,
	CMP,
	CDQ,
	NOT,
	AND,
	OR,
	XOR,
	SAL,
	SAR,
	LEA,
	SAHF,
	FLD,
	FILD,
	FSTP,
	FISTTP,
	FCHS,
	FLD1,
	FADD,
	FSUBR,
	FMUL,
	FDIVR,
	FTST,
	FCOMP,
	FCOMPP,
	FSTSW,
	// to be continued
};

static map<EMnemonic, string> MnemonicsText;
static map<ERegister, string> RegistersText;

class CAsmCmd
{
public:
	virtual ~CAsmCmd();

	string GetName() const;
	void SetName(const string &AName);

	virtual string GetText() const = 0;

protected:
	string Name;

};

class CAsmOp
{
public:
	virtual string GetText() const = 0;

	virtual bool IsReg() const;
	virtual bool IsImm() const;
	virtual bool IsMem() const;

};

class CAsmReg : public CAsmOp
{
public:
	CAsmReg(const string &AName);

	string GetName() const;
	void SetName(const string &AName);

	string GetText() const;

	bool IsReg() const;

private:
	string Name;

};

class CAsmImm : public CAsmOp
{
public:
	CAsmImm(int AValue);

	string GetText() const;

	int GetValue() const;

	bool IsImm() const;

private:
	int Value;
};

class CAsmMem : public CAsmOp
{
public:
	CAsmMem(int ADisplacement, ERegister ABase, ERegister AOffset, int AMultiplier);

	string GetText() const;

	bool IsMem() const;

private:
	int Displacement;
	ERegister Base;
	ERegister Offset;
	int Multiplier;
};

CAsmMem* mem(int ADisplacement, ERegister ABase, ERegister AOffset = INVALID_REGISTER, int AMultiplier = 0);
CAsmMem* mem(ERegister ABase);

class CAsmLabelOp : public CAsmOp
{
public:
	CAsmLabelOp(const string &AName);

	string GetName() const;
	void SetName(const string &AName);

	string GetText() const;

private:
	string Name;
};

class CAsmCmd0 : public CAsmCmd
{
public:
	CAsmCmd0(const string &AName);

	string GetText() const;

};

class CAsmCmd1 : public CAsmCmd
{
public:
	CAsmCmd1(const string &AName, CAsmOp *AOp);
	~CAsmCmd1();

	string GetText() const;

	CAsmOp* GetOp() const;
	void SetOp(CAsmOp *AOp);

private:
	CAsmOp *Op;

};

class CAsmCmd2 : public CAsmCmd
{
public:
	CAsmCmd2(const string &AName, CAsmOp *AOp1, CAsmOp *AOp2);
	~CAsmCmd2();

	string GetText() const;

	CAsmOp* GetOp1() const;
	CAsmOp* GetOp2() const;

	void SetOp1(CAsmOp *AOp1);
	void SetOp2(CAsmOp *AOp2);

private:
	CAsmOp *Op1;
	CAsmOp *Op2;
};

class CAsmLabel : public CAsmCmd
{
public:
	CAsmLabel(const string &AName);

	string GetText() const;

};

class CAsmDirective : public CAsmCmd
{
public:
	CAsmDirective(const string &AName, const string &AArgument);

	string GetText() const;

private:
	string Argument;

};

class CAsmCode
{
public:
	typedef list<CAsmCmd *> CodeContainer;
	typedef CodeContainer::iterator CodeIterator;

	CAsmCode();
	~CAsmCode();

	void Add(CAsmCmd *ACmd);

	void Add(EMnemonic ACmd);
	void Add(EMnemonic ACmd, ERegister AOp);
	void Add(EMnemonic ACmd, int AOp);
	void Add(EMnemonic ACmd, ERegister AOp1, ERegister AOp2);
	void Add(EMnemonic ACmd, int AOp1, ERegister AOp2);
	void Add(EMnemonic ACmd, const string &AOp);
	void Add(EMnemonic ACmd, CAsmMem *AOp);
	void Add(EMnemonic ACmd, ERegister AOp1, CAsmMem *AOp2);
	void Add(EMnemonic ACmd, CAsmMem *AOp1, ERegister AOp2);
	void Add(const string &ALabel);

	string AddStringLiteral(const string &ALiteral);
	void AddGlobalVariable(CVariableSymbol *AVariable);

	void Output(ostream &Stream);

	CodeIterator Insert(CodeIterator APosition, CAsmCmd *ACmd);

	CodeIterator Begin();
	CodeIterator End();

	CodeIterator Erase(CodeIterator APosition);

	string GenerateLabel();

private:
	CodeContainer Code;

	map<string, string> StringLiterals;

	unsigned int LabelsCount;

};

class CCodeGenerationVisitor;

class CAddressGenerationVisitor : public CStatementVisitor
{
public:
	CAddressGenerationVisitor(CAsmCode &AAsm, CCodeGenerationVisitor &ACode);

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
	CAsmCode &Asm;
	CCodeGenerationVisitor &Code;

};

class CCodeGenerationVisitor : public CStatementVisitor
{
public:
	CCodeGenerationVisitor(CAsmCode &AAsm, bool AOptimize);

	void SetFunction(CFunctionSymbol *AFuncSym);

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
	void ConvertFloatToInt();
	void ConvertIntToFloat();
	void PerformConversion(CTypeSymbol *LHS, CTypeSymbol *RHS);
	//void CopyStruct(CVariableSymbol *ADest, CVariableSymbol *ASrc);

	CAsmCode &Asm;
	CFunctionSymbol *FuncSym;
	int BlockNesting;

	stack<CBlockStatement *> Blocks;
	stack<string> BreakLabels;
	stack<string> ContinueLabels;

	map<ETokenType, EMnemonic> IntOperationCmd;
	map<ETokenType, EMnemonic> FloatOperationCmd;
	map<ETokenType, ETokenType> CompoundAssignmentOp;

	CAddressGenerationVisitor Addr;

	bool Optimize;
};

class CCodeGenerator
{
public:
	CCodeGenerator(CParser &AParser, const CCompilerParameters &AParameters);

	void Output(ostream &Stream);

private:
	CParser &Parser;
	const CCompilerParameters &Parameters;

	CAsmCode Code;
	CCodeGenerationVisitor Visitor;
};

#endif // _CODEGEN_H_
