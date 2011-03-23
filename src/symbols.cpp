#include "symbols.h"

#include "statements.h"

/******************************************************************************
 * CSymbol
 ******************************************************************************/

CSymbol::CSymbol(const string &AName /*= ""*/) : Name(AName)
{
}

CSymbol::~CSymbol()
{
}

string CSymbol::GetName() const
{
	return Name;
}

void CSymbol::SetName(const string &AName)
{
	Name = AName;
}

/******************************************************************************
 * CSymbolTable
 ******************************************************************************/

CSymbolTable::CSymbolTable() : CurrentOffset(0)
{
}

CSymbolTable::~CSymbolTable()
{
	for (SymbolsIterator it = Begin(); it != End(); ++it) {
		delete it->second;
	}
}

void CSymbolTable::Add(CSymbol *ASymbol)
{
	if (!ASymbol) {
		return;
	}

	Symbols[ASymbol->GetName()] = ASymbol;

	CVariableSymbol *VarSym = dynamic_cast<CVariableSymbol *>(ASymbol);
	if (VarSym) {
		InitOffset(VarSym);
	}
}

CSymbol* CSymbolTable::Get(const string &AName) const
{
	SymbolsIterator it = Symbols.find(AName);
	if (it == Symbols.end()) {
		return NULL;
	}

	return it->second;
}

bool CSymbolTable::Exists(const string &AName) const
{
	return Symbols.count(AName);
}

CSymbolTable::SymbolsIterator CSymbolTable::Begin() const
{
	return Symbols.begin();
}

CSymbolTable::SymbolsIterator CSymbolTable::End() const
{
	return Symbols.end();
}

unsigned int CSymbolTable::GetSize() const
{
	return Symbols.size();
}

unsigned int CSymbolTable::GetElementsSize() const
{
	return CurrentOffset;
}

void CSymbolTable::SetCurrentOffset(size_t AOffset)
{
	CurrentOffset = AOffset;
}

void CSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	CurrentOffset += ASymbol->GetType()->GetSize();
	ASymbol->SetOffset(-CurrentOffset);
}

/******************************************************************************
 * CArgumentsSymbolTable
 ******************************************************************************/

void CArgumentsSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	CurrentOffset += ASymbol->GetType()->GetSize();
	ASymbol->SetOffset(4 + CurrentOffset);	// FIXME: magic number: 4 - old EBP register value size
}

/******************************************************************************
 * CStructSymbolTable
 ******************************************************************************/

void CStructSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	ASymbol->SetOffset(CurrentOffset);
	CurrentOffset += ASymbol->GetType()->GetSize();
}

/******************************************************************************
 * CSymbolTableStack
 ******************************************************************************/

void CSymbolTableStack::Push(CSymbolTable *ATable)
{
	Tables.push_front(ATable);
}

CSymbolTable* CSymbolTableStack::Pop()
{
	CSymbolTable *result = Tables.front();
	Tables.pop_front();
	return result;
}

CSymbolTable* CSymbolTableStack::GetTop() const
{
	return Tables.front();
}

CSymbolTable* CSymbolTableStack::GetGlobal() const
{
	return Tables.back();
}

/******************************************************************************
 * CTypeSymbol
 ******************************************************************************/

CTypeSymbol::CTypeSymbol(const string &AName /*= ""*/) : CSymbol(AName), Const(false)
{
}

bool CTypeSymbol::GetConst() const
{
	return Const;
}

void CTypeSymbol::SetConst(bool AConst)
{
	Const = AConst;
}

bool CTypeSymbol::IsInt() const
{
	return false;
}

bool CTypeSymbol::IsFloat() const
{
	return false;
}

bool CTypeSymbol::IsArithmetic() const
{
	return IsInt() || IsFloat();
}

bool CTypeSymbol::IsVoid() const
{
	return false;
}

bool CTypeSymbol::IsType(const string &AType) const
{
	return (Name == AType);
}

bool CTypeSymbol::IsPointer() const
{
	return false;
}

bool CTypeSymbol::IsStruct() const
{
	return false;
}

bool CTypeSymbol::IsArray() const
{
	return false;
}

bool CTypeSymbol::IsFunction() const
{
	return false;
}

bool CTypeSymbol::IsScalar() const
{
	return IsArithmetic() || IsPointer();
}

bool CTypeSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (ASymbol->IsArray() || ASymbol->IsStruct() || ASymbol->IsPointer() || ASymbol->IsFunction()) {
		return false;
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		// FIXME: well, i don't know.. it seems that any plain type are compatible with any other plain type..
		return true;
		//return (Name == ASymbol->GetName());
	}
}

/******************************************************************************
 * CIntegerSymbol
 ******************************************************************************/

CIntegerSymbol::CIntegerSymbol()
{
	Name = "int";
}

size_t CIntegerSymbol::GetSize() const
{
	return TypeSize::Integer;
}

bool CIntegerSymbol::IsInt() const
{
	return true;
}

/******************************************************************************
 * CFloatSymbol
 ******************************************************************************/

CFloatSymbol::CFloatSymbol()
{
	Name = "float";
}

size_t CFloatSymbol::GetSize() const
{
	return TypeSize::Float;
}

bool CFloatSymbol::IsFloat() const
{
	return true;
}

/******************************************************************************
 * CVoidSymbol
 ******************************************************************************/

CVoidSymbol::CVoidSymbol()
{
	Name = "void";
}

size_t CVoidSymbol::GetSize() const
{
	return 0;
}

bool CVoidSymbol::IsVoid() const
{
	return true;
}

/******************************************************************************
 * CArraySymbol
 ******************************************************************************/

size_t CArraySymbol::GetSize() const
{
	return Length * (ElementsType ? ElementsType->GetSize() : 0);
}

CTypeSymbol* CArraySymbol::GetElementsType() const
{
	return ElementsType;
}

void CArraySymbol::SetElementsType(CTypeSymbol *AElementsType)
{
	ElementsType = AElementsType;
}

unsigned int CArraySymbol::GetLength() const
{
	return Length;
}

void CArraySymbol::SetLength(unsigned int ALength)
{
	Length = ALength;
}

bool CArraySymbol::IsPointer() const
{
	return true;
}

bool CArraySymbol::IsArray() const
{
	return true;
}

bool CArraySymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (CArraySymbol *ArraySym = dynamic_cast<CArraySymbol *>(ASymbol)) {
		return ElementsType->CompatibleWith(ArraySym->GetElementsType()) && (Length == ArraySym->GetLength());
	} else if (CPointerSymbol *PointerSym = dynamic_cast<CPointerSymbol *>(ASymbol)) {
		return ElementsType->CompatibleWith(PointerSym->GetRefType());
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		return false;
	}
}

/******************************************************************************
 * CStructSymbol
 ******************************************************************************/

CStructSymbol::CStructSymbol() : Fields(NULL)
{

}

CStructSymbol::~CStructSymbol()
{
	delete Fields;
}

size_t CStructSymbol::GetSize() const
{
	if (!Fields) {
		return 0;
	}

	size_t result = 0;

	for (CSymbolTable::SymbolsIterator it = Fields->Begin(); it != Fields->End(); ++it) {
		result += static_cast<CVariableSymbol *>(it->second)->GetType()->GetSize();
	}

	return result;
}

void CStructSymbol::AddField(CSymbol *AField)
{
	if (Fields) {
		Fields->Add(AField);
	}
}

CVariableSymbol* CStructSymbol::GetField(const string &AName)
{
	if (!Fields) {
		return NULL;
	}

	return dynamic_cast<CVariableSymbol *>(Fields->Get(AName));
}

CStructSymbolTable* CStructSymbol::GetSymbolTable()
{
	return Fields;
}

void CStructSymbol::SetSymbolTable(CStructSymbolTable *ASymbolTable)
{
	Fields = ASymbolTable;
}

unsigned int CStructSymbol::GetFieldsCount() const
{
	return (Fields ? Fields->GetSize() : 0);
}

bool CStructSymbol::IsStruct() const
{
	return true;
}

bool CStructSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (ASymbol->IsStruct()) {
		return (Name == ASymbol->GetName()); // TODO: verify that it's true..
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		return false;
	}
}

/******************************************************************************
 * CPointerSymbol
 ******************************************************************************/

CPointerSymbol::CPointerSymbol(CTypeSymbol *ARefType /*= NULL*/) : RefType(ARefType)
{
}

string CPointerSymbol::GetName() const
{
	return RefType ? RefType->GetName() + "*" : "";
}

size_t CPointerSymbol::GetSize() const
{
	return TypeSize::Pointer;
}

CTypeSymbol* CPointerSymbol::GetRefType() const
{
	return RefType;
}

void CPointerSymbol::SetRefType(CTypeSymbol *ARefType)
{
	RefType = ARefType;
}

bool CPointerSymbol::IsPointer() const
{
	return true;
}

bool CPointerSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (CPointerSymbol *PointerSym = dynamic_cast<CPointerSymbol *>(ASymbol)) {
		return RefType->CompatibleWith(PointerSym->GetRefType());
	} else if (CArraySymbol *ArraySym = dynamic_cast<CArraySymbol *>(ASymbol)) {
		return RefType->CompatibleWith(ArraySym->GetElementsType());
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		return false;
	}
}

/******************************************************************************
 * CTypedefSymbol
 ******************************************************************************/

CTypedefSymbol::CTypedefSymbol(const string &AName /*= ""*/, CTypeSymbol *ARefType /*= NULL*/) : CTypeSymbol(AName), RefType(ARefType)
{
}

size_t CTypedefSymbol::GetSize() const
{
	return (RefType ? RefType->GetSize() : 0);
}

CTypeSymbol* CTypedefSymbol::GetRefType() const
{
	return RefType;
}

void CTypedefSymbol::SetRefType(CTypeSymbol *ARefType)
{
	RefType = ARefType;
}

bool CTypedefSymbol::IsInt() const
{
	return RefType->IsInt();
}

bool CTypedefSymbol::IsFloat() const
{
	return RefType->IsFloat();
}

bool CTypedefSymbol::IsVoid() const
{
	return RefType->IsVoid();
}

bool CTypedefSymbol::IsType(const string &AType) const
{
	return RefType->IsType(AType);
}

bool CTypedefSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return RefType->CompatibleWith(TypedefSym->GetRefType());
	} else {
		return RefType->CompatibleWith(ASymbol);
	}
}

/******************************************************************************
 * CFunctionTypeSymbol
 ******************************************************************************/

size_t CFunctionTypeSymbol::GetSize() const
{
	return 0;
}

bool CFunctionTypeSymbol::IsFunction() const
{
	return true;
}

/******************************************************************************
 * CVariableSymbol
 ******************************************************************************/

CVariableSymbol::CVariableSymbol(const string &AName /*= ""*/, CTypeSymbol *AType /*= NULL*/) : CSymbol(AName), Type(AType), Offset(0)
{
}

CTypeSymbol* CVariableSymbol::GetType() const
{
	return Type;
}

void CVariableSymbol::SetType(CTypeSymbol *AType)
{
	Type = AType;
}

int CVariableSymbol::GetOffset() const
{
	return Offset;
}

void CVariableSymbol::SetOffset(int AOffset)
{
	Offset = AOffset;
}

/******************************************************************************
 * CFunctionSymbol
 ******************************************************************************/

CFunctionSymbol::CFunctionSymbol(const string &AName /*= ""*/, CTypeSymbol *AReturnType /*= NULL*/) : CSymbol(AName), ReturnType(AReturnType), Arguments(NULL), Body(NULL), Type(new CFunctionTypeSymbol)
{
}

CFunctionSymbol::~CFunctionSymbol()
{
	delete Arguments;
	delete Body;
	delete Type;
}

CTypeSymbol* CFunctionSymbol::GetReturnType() const
{
	return ReturnType;
}

void CFunctionSymbol::SetReturnType(CTypeSymbol *AReturnType)
{
	ReturnType = AReturnType;
}

void CFunctionSymbol::AddArgument(CSymbol *AArgument)
{
	if (Arguments) {
		Arguments->Add(AArgument);
		ArgumentsOrder.push_back(AArgument);
	}
}

CArgumentsSymbolTable* CFunctionSymbol::GetArgumentsSymbolTable()
{
	return Arguments;
}

void CFunctionSymbol::SetArgumentsSymbolTable(CArgumentsSymbolTable *ASymbolTable)
{
	Arguments = ASymbolTable;
}

CFunctionSymbol::ArgumentsOrderContainer* CFunctionSymbol::GetArgumentsOrderedList()
{
	return &ArgumentsOrder;
}

CBlockStatement* CFunctionSymbol::GetBody() const
{
	return Body;
}

void CFunctionSymbol::SetBody(CBlockStatement *ABody)
{
	Body = ABody;
}

CTypeSymbol* CFunctionSymbol::GetType() const
{
	return Type;
}
