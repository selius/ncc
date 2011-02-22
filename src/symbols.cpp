#include "symbols.h"

/******************************************************************************
 * CSymbol
 ******************************************************************************/

CSymbol::CSymbol()
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

CSymbolTable::~CSymbolTable()
{
	for (SymbolsIterator it = Begin(); it != End(); ++it) {
		delete it->second;
	}

	Symbols.clear();
}

void CSymbolTable::Add(CSymbol *ASymbol)
{
	if (!ASymbol) {
		return;
	}

	Symbols[ASymbol->GetName()] = ASymbol;	
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

/******************************************************************************
 * CTypeSymbol
 ******************************************************************************/

CTypeSymbol::CTypeSymbol() : Const(false)
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

/******************************************************************************
 * CIntegerSymbol
 ******************************************************************************/

CIntegerSymbol::CIntegerSymbol()
{
	Name = "int";
}

size_t CIntegerSymbol::GetSize() const
{
	return 4; // FIXME: magic number, add header with sizes of data types
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
	return 4; // FIXME: magic number
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

CSymbolTable* CStructSymbol::GetSymbolTable()
{
	return Fields;
}

void CStructSymbol::SetSymbolTable(CSymbolTable *ASymbolTable)
{
	Fields = ASymbolTable;
}

unsigned int CStructSymbol::GetFieldsCount() const
{
	return (Fields ? Fields->GetSize() : 0);
}

/******************************************************************************
 * CPointerSymbol
 ******************************************************************************/

size_t CPointerSymbol::GetSize() const
{
	return 4; // FIXME: magic number
}

CTypeSymbol* CPointerSymbol::GetRefType() const
{
	return RefType;
}

void CPointerSymbol::SetRefType(CTypeSymbol *ARefType)
{
	RefType = ARefType;
}

/******************************************************************************
 * CTypedefSymbol
 ******************************************************************************/

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

/******************************************************************************
 * CVariableSymbol
 ******************************************************************************/

CTypeSymbol* CVariableSymbol::GetType() const
{
	return Type;
}

void CVariableSymbol::SetType(CTypeSymbol *AType)
{
	Type = AType;
}

/******************************************************************************
 * CFunctionSymbol
 ******************************************************************************/

CFunctionSymbol::CFunctionSymbol() : ReturnType(NULL), Arguments(NULL), Locals(NULL)
{
}

CFunctionSymbol::~CFunctionSymbol()
{
	delete Arguments;
	delete Locals;
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
	}
}

CSymbolTable* CFunctionSymbol::GetArgumentsSymbolTable()
{
	return Arguments;
}

void CFunctionSymbol::SetArgumentsSymbolTable(CSymbolTable *ASymbolTable)
{
	Arguments = ASymbolTable;
}

CBlockStatement* CFunctionSymbol::GetBody() const
{
	return Body;
}

void CFunctionSymbol::SetBody(CBlockStatement *ABody)
{
	Body = ABody;
}
