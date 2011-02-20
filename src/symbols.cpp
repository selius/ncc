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

CSymbol* CSymbolTableStack::Lookup(const string &AName) const
{
	// FIXME: delete this test code
	CSymbol *r = new CFunctionSymbol;
	r->SetName(AName);
	return r;
	//

	CSymbol *result = NULL;

	for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
		result = (*it)->Get(AName); 
		if (result) {
			return result;
		}
	}

	return NULL;
}
