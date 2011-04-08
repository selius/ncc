#include "symbols.h"

#include "statements.h"
#include "prettyprinting.h"

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

CSymbolTable::CSymbolTable() : CurrentOffset(0), ElementsSize(0)
{
}

CSymbolTable::~CSymbolTable()
{
	for (VariablesIterator it = Variables.begin(); it != Variables.end(); ++it) {
		delete it->second;
	}

	for (TypesIterator it = Types.begin(); it != Types.end(); ++it) {
		delete it->second;
	}
}

void CSymbolTable::AddVariable(CVariableSymbol *ASymbol)
{
	assert(ASymbol != NULL);

	Variables[ASymbol->GetName()] = ASymbol;
	InitOffset(ASymbol);
}

void CSymbolTable::AddType(CTypeSymbol *ASymbol)
{
	assert(ASymbol != NULL);
	
	if (ASymbol->IsStruct()) {
		Types["struct " + ASymbol->GetName()] = ASymbol;
	} else {
		Types[ASymbol->GetQualifiedName()] = ASymbol;
	}
}

void CSymbolTable::AddTag(CStructSymbol *ASymbol)
{
	assert(ASymbol != NULL);

	Tags[ASymbol->GetName()] = ASymbol;
}

CVariableSymbol* CSymbolTable::GetVariable(const string &AName) const
{
	VariablesIterator it = Variables.find(AName);
	if (it == Variables.end()) {
		return NULL;
	}

	return it->second;
}

CTypeSymbol* CSymbolTable::GetType(const string &AName) const
{
	TypesIterator it = Types.find(AName);
	if (it == Types.end()) {
		return NULL;
	}

	return it->second;
}

CFunctionSymbol* CSymbolTable::GetFunction(const string &AName) const
{
	return NULL;
}

CStructSymbol* CSymbolTable::GetTag(const string &AName) const
{
	TagsIterator it = Tags.find(AName);
	if (it == Tags.end()) {
		return NULL;
	}

	return it->second;
}

CSymbol* CSymbolTable::Get(const string &AName) const
{
	CSymbol *Result = NULL;

	if (Result = GetVariable(AName)) {
		return Result;
	}
	if (Result = GetType(AName)) {
		return Result;
	}

	return Result;
}

bool CSymbolTable::Exists(const string &AName) const
{
	return Variables.count(AName) || Types.count(AName);
}

CSymbolTable::VariablesIterator CSymbolTable::VariablesBegin() const
{
	return Variables.begin();
}

CSymbolTable::VariablesIterator CSymbolTable::VariablesEnd() const
{
	return Variables.end();
}

CSymbolTable::TypesIterator CSymbolTable::TypesBegin() const
{
	return Types.begin();
}

CSymbolTable::TypesIterator CSymbolTable::TypesEnd() const
{
	return Types.end();
}

unsigned int CSymbolTable::GetSize() const
{
	return Variables.size() + Types.size() + Tags.size();
}

unsigned int CSymbolTable::VariablesSize() const
{
	return Variables.size();
}

unsigned int CSymbolTable::GetElementsSize() const
{
	return ElementsSize;
}

size_t CSymbolTable::GetCurrentOffset() const
{
	return CurrentOffset;
}

void CSymbolTable::SetCurrentOffset(size_t AOffset)
{
	CurrentOffset = AOffset;
}

void CSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	ElementsSize += ASymbol->GetType()->GetSize();
	CurrentOffset += ASymbol->GetType()->GetSize();
	ASymbol->SetOffset(-CurrentOffset);
}

/******************************************************************************
 * CGlobalSymbolTable
 ******************************************************************************/

CGlobalSymbolTable::~CGlobalSymbolTable()
{
	for (FunctionsIterator it = Functions.begin(); it != Functions.end(); ++it) {
		delete it->second;
	}
}

void CGlobalSymbolTable::AddFunction(CFunctionSymbol *ASymbol)
{
	assert(ASymbol != NULL);

	Functions[ASymbol->GetName()] = ASymbol;
}

CFunctionSymbol* CGlobalSymbolTable::GetFunction(const string &AName) const
{
	FunctionsIterator it = Functions.find(AName);
	if (it == Functions.end()) {
		return NULL;
	}

	return it->second;
}

CSymbol* CGlobalSymbolTable::Get(const string &AName) const
{
	CSymbol *Result = CSymbolTable::Get(AName);
	if (Result) {
		return Result;
	}
	
	return GetFunction(AName);
}

bool CGlobalSymbolTable::Exists(const string &AName) const
{
	return CSymbolTable::Exists(AName) || Functions.count(AName);
}

CGlobalSymbolTable::FunctionsIterator CGlobalSymbolTable::FunctionsBegin() const
{
	return Functions.begin();
}

CGlobalSymbolTable::FunctionsIterator CGlobalSymbolTable::FunctionsEnd() const
{
	return Functions.end();
}

/******************************************************************************
 * CArgumentsSymbolTable
 ******************************************************************************/

void CArgumentsSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	ElementsSize += ASymbol->GetType()->GetSize();
	CurrentOffset += ASymbol->GetType()->GetSize();
	ASymbol->SetOffset(TypeSize::Pointer + CurrentOffset);
}

/******************************************************************************
 * CStructSymbolTable
 ******************************************************************************/

void CStructSymbolTable::InitOffset(CVariableSymbol *ASymbol)
{
	ElementsSize += ASymbol->GetType()->GetSize();
	ASymbol->SetOffset(CurrentOffset);
	CurrentOffset += ASymbol->GetType()->GetSize();
}

/******************************************************************************
 * CSymbolTableStack
 ******************************************************************************/

CSymbolTableStack::CSymbolTableStack() : Global(NULL)
{
}

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

CSymbolTable* CSymbolTableStack::GetPreviousTop() const
{
	return *(++Tables.begin());
}

CGlobalSymbolTable* CSymbolTableStack::GetGlobal() const
{
	return Global;
}

void CSymbolTableStack::SetGlobal(CGlobalSymbolTable *ASymbolTable)
{
	if (Global) {
		return;
	}

	Global = ASymbolTable;
	Tables.push_back(ASymbolTable);
}

CVariableSymbol* CSymbolTableStack::LookupVariable(const string &AName) const
{
	CVariableSymbol *Result = NULL;

	for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
		Result = (*it)->GetVariable(AName); 
		if (Result) {
			return Result;
		}
	}

	return NULL;
}

CTypeSymbol* CSymbolTableStack::LookupType(const string &AName) const
{
	CTypeSymbol *Result = NULL;

	for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
		Result = (*it)->GetType(AName); 
		if (Result) {
			return Result;
		}
	}

	return NULL;
}

CStructSymbol* CSymbolTableStack::LookupTag(const string &AName) const
{
	CStructSymbol *Result = NULL;

	for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
		Result = (*it)->GetTag(AName); 
		if (Result) {
			return Result;
		}
	}

	return NULL;
}

CSymbol* CSymbolTableStack::LookupAll(const string &AName) const
{
	CSymbol *Result = NULL;

	for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
		if (Result = (*it)->GetVariable(AName)) {
			return Result;
		}
		if (Result = (*it)->GetType(AName)) {
			return Result;
		}
		if (Result = (*it)->GetFunction(AName)) {
			return Result;
		}
		if (Result = (*it)->GetTag(AName)) {
			return Result;
		}
	}

	return NULL;

}

/******************************************************************************
 * CTypeSymbol
 ******************************************************************************/

CTypeSymbol::CTypeSymbol(const string &AName /*= ""*/) : CSymbol(AName), Const(false), Complete(true)
{
}

string CTypeSymbol::GetQualifiedName() const
{
	return (Const ? "const " : "") + Name;
}

ESymbolType CTypeSymbol::GetSymbolType() const
{
	return SYMBOL_TYPE_TYPE;
}

bool CTypeSymbol::GetConst() const
{
	return Const;
}

void CTypeSymbol::SetConst(bool AConst)
{
	Const = AConst;
}

bool CTypeSymbol::GetComplete() const
{
	return Complete;
}

void CTypeSymbol::SetComplete(bool AComplete)
{
	Complete = AComplete;
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
	return IsArithmetic() || (IsPointer() && !IsArray());
}

bool CTypeSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (ASymbol->IsArray() || ASymbol->IsStruct() || ASymbol->IsPointer() || ASymbol->IsFunction()) {
		return false;
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		// it seems that any plain type is compatible with any other plain type..
		return true;
	}
}

void CTypeSymbol::Accept(CSymbolsPrettyPrinter &AVisitor)
{
	AVisitor.Visit(*this);
}

CTypeSymbol* CTypeSymbol::ConstClone() const
{
	throw logic_error("can't const-clone this kind of symbol");
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

CIntegerSymbol* CIntegerSymbol::ConstClone() const
{
	CIntegerSymbol *result = new CIntegerSymbol;
	*result = *this;
	result->SetConst(true);
	return result;
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

CFloatSymbol* CFloatSymbol::ConstClone() const
{
	CFloatSymbol *result = new CFloatSymbol;
	*result = *this;
	result->SetConst(true);
	return result;
}

/******************************************************************************
 * CVoidSymbol
 ******************************************************************************/

CVoidSymbol::CVoidSymbol()
{
	Name = "void";
	Complete = false;
}

size_t CVoidSymbol::GetSize() const
{
	return 0;
}

bool CVoidSymbol::IsVoid() const
{
	return true;
}

CVoidSymbol* CVoidSymbol::ConstClone() const
{
	CVoidSymbol *result = new CVoidSymbol;
	*result = *this;
	result->SetConst(true);
	return result;
}

/******************************************************************************
 * CArraySymbol
 ******************************************************************************/

CArraySymbol::CArraySymbol(CTypeSymbol *AElementsType /*= NULL*/, unsigned int ALength /*= 0*/) : ElementsType(AElementsType), Length(ALength)
{
	UpdateName();
}

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
	UpdateName();
}

unsigned int CArraySymbol::GetLength() const
{
	return Length;
}

void CArraySymbol::SetLength(unsigned int ALength)
{
	Length = ALength;
	UpdateName();
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

void CArraySymbol::UpdateName()
{
	if (!ElementsType) {
		return;
	}

	Name = ElementsType->GetQualifiedName() + "[" + ToString(Length) + "]";
}

CArraySymbol* CArraySymbol::ConstClone() const
{
	CArraySymbol *result = new CArraySymbol;
	*result = *this;
	result->SetConst(true);
	return result;
}

/******************************************************************************
 * CStructSymbol
 ******************************************************************************/

CStructSymbol::CStructSymbol() : Fields(NULL)
{
	Complete = false;
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

	for (CStructSymbolTable::VariablesIterator it = Fields->VariablesBegin(); it != Fields->VariablesEnd(); ++it) {
		result += it->second->GetType()->GetSize();
	}

	return result;
}

void CStructSymbol::AddField(CVariableSymbol *AField)
{
	if (Fields) {
		Fields->AddVariable(AField);
	}
}

CVariableSymbol* CStructSymbol::GetField(const string &AName)
{
	if (!Fields) {
		return NULL;
	}

	return Fields->GetVariable(AName);
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
	return (Fields ? Fields->VariablesSize() : 0);
}

bool CStructSymbol::IsStruct() const
{
	return true;
}

bool CStructSymbol::CompatibleWith(CTypeSymbol *ASymbol)
{
	if (ASymbol->IsStruct()) {
		return (Name == ASymbol->GetName());
	} else if (CTypedefSymbol *TypedefSym = dynamic_cast<CTypedefSymbol *>(ASymbol)) {
		return CompatibleWith(TypedefSym->GetRefType());
	} else {
		return false;
	}
}

void CStructSymbol::Accept(CSymbolsPrettyPrinter &AVisitor)
{
	AVisitor.Visit(*this);
}

CStructSymbol* CStructSymbol::ConstClone() const
{
	CStructSymbol *result = new CStructSymbol;
	*result = *this;
	result->SetConst(true);
	return result;
}

/******************************************************************************
 * CPointerSymbol
 ******************************************************************************/

CPointerSymbol::CPointerSymbol(CTypeSymbol *ARefType /*= NULL*/)
{
	SetRefType(ARefType);
}

string CPointerSymbol::GetQualifiedName() const
{
	return Name + (Const ? " const" : "");
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

	if (RefType) {
		Name = RefType->GetQualifiedName() + "*";

	}
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

CPointerSymbol* CPointerSymbol::ConstClone() const
{
	CPointerSymbol *result = new CPointerSymbol;
	*result = *this;
	result->SetConst(true);
	return result;
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

void CTypedefSymbol::Accept(CSymbolsPrettyPrinter &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CFunctionTypeSymbol
 ******************************************************************************/

CFunctionTypeSymbol::CFunctionTypeSymbol()
{
	Complete = false;
}

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

CVariableSymbol::CVariableSymbol(const string &AName /*= ""*/, CTypeSymbol *AType /*= NULL*/) : CSymbol(AName), Type(AType), Offset(0), Global(false), InitValue(0.0f)
{
}

ESymbolType CVariableSymbol::GetSymbolType() const
{
	return SYMBOL_TYPE_VARIABLE;
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

bool CVariableSymbol::GetGlobal() const
{
	return Global;
}

void CVariableSymbol::SetGlobal(bool AGlobal)
{
	Global = AGlobal;
}

float CVariableSymbol::GetInitValue() const
{
	return InitValue;
}

void CVariableSymbol::SetInitValue(float AInitValue)
{
	InitValue = AInitValue;
}

void CVariableSymbol::Accept(CSymbolsPrettyPrinter &AVisitor)
{
	AVisitor.Visit(*this);
}

/******************************************************************************
 * CFunctionSymbol
 ******************************************************************************/

CFunctionSymbol::CFunctionSymbol(const string &AName /*= ""*/, CTypeSymbol *AReturnType /*= NULL*/) : CSymbol(AName), ReturnType(AReturnType), Arguments(NULL), Body(NULL), Type(new CFunctionTypeSymbol), BuiltIn(false)
{
}

CFunctionSymbol::~CFunctionSymbol()
{
	delete Arguments;
	delete Body;
	delete Type;
}

ESymbolType CFunctionSymbol::GetSymbolType() const
{
	return SYMBOL_TYPE_FUNCTION;
}

CTypeSymbol* CFunctionSymbol::GetReturnType() const
{
	return ReturnType;
}

void CFunctionSymbol::SetReturnType(CTypeSymbol *AReturnType)
{
	ReturnType = AReturnType;
}

void CFunctionSymbol::AddArgument(CVariableSymbol *AArgument)
{
	ArgumentsOrder.push_back(AArgument);
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

bool CFunctionSymbol::GetBuiltIn() const
{
	return BuiltIn;
}

void CFunctionSymbol::SetBuiltIn(bool ABuiltIn)
{
	BuiltIn = ABuiltIn;
}

void CFunctionSymbol::Accept(CSymbolsPrettyPrinter &AVisitor)
{
	AVisitor.Visit(*this);
}
