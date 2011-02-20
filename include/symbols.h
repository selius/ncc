#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include "common.h"

class CSymbol
{
public:
	CSymbol();
	virtual ~CSymbol();

	string GetName() const;
	void SetName(const string &AName);

private:
	string Name;

};

class CTypeSymbol : public CSymbol
{

};

class CScalarTypeSymbol : public CTypeSymbol
{

};

class CIntegerSymbol : public CScalarTypeSymbol
{

};

class CFloatSymbol : public CScalarTypeSymbol
{

};

class CArraySymbol : public CTypeSymbol
{
public:

private:
	CTypeSymbol *ElementsType;
	unsigned int Length;

};

class CStructSymbol : public CTypeSymbol
{

};

class CPointerSymbol : public CTypeSymbol
{
public:

private:
	CTypeSymbol *RefType;

};

class CTypedefSymbol : public CTypeSymbol
{
public:

private:
	CTypeSymbol *RefType;

};

class CVariableSymbol : public CSymbol
{
public:
	CTypeSymbol* GetType() const;

private:
	CTypeSymbol *Type;

};

class CFunctionSymbol : public CSymbol
{
public:

private:
	CTypeSymbol *ReturnType;
	vector<CTypeSymbol *> ArgumentsTypes;

};

class CSymbolTable
{
public:
	typedef map<string, CSymbol *> SymbolsContainer;
	typedef SymbolsContainer::const_iterator SymbolsIterator;

	void Add(CSymbol *ASymbol);
	CSymbol* Get(const string &AName) const;

private:
	SymbolsContainer Symbols;

};

class CSymbolTableStack
{
public:
	typedef deque<CSymbolTable *> TablesContainer;
	typedef TablesContainer::const_iterator TablesIterator;

	void Push(CSymbolTable *ATable);
	CSymbolTable* Pop();
	CSymbolTable* GetTop() const;

	CSymbol* Lookup(const string &AName) const;

private:
	TablesContainer Tables;

};



#endif // _SYMBOLS_H_
