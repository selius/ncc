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

protected:
	string Name;

};

class CSymbolTable
{
public:
	typedef map<string, CSymbol *> SymbolsContainer;
	typedef SymbolsContainer::const_iterator SymbolsIterator;

	// TODO: make symbol table the owner of contained symbols, i. e. destroy 'em in destructor

	~CSymbolTable();

	void Add(CSymbol *ASymbol);
	CSymbol* Get(const string &AName) const;
	bool Exists(const string &AName) const;

	SymbolsIterator Begin() const;
	SymbolsIterator End() const;

	unsigned int GetSize() const;

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

	template<typename T>
	T* Lookup(const string &AName) const
	{
		CSymbol *result = NULL;

		for (TablesIterator it = Tables.begin(); it != Tables.end(); ++it) {
			result = (*it)->Get(AName); 
			if (result) {
				return dynamic_cast<T *>(result);
			}
		}

		return NULL;
	}

private:
	TablesContainer Tables;

};

class CTypeSymbol : public CSymbol
{
public:
	CTypeSymbol();

	virtual size_t GetSize() const = 0;

	bool GetConst() const;
	void SetConst(bool AConst);

private:
	bool Const;
};

class CScalarTypeSymbol : public CTypeSymbol
{

};

class CIntegerSymbol : public CScalarTypeSymbol
{
public:
	CIntegerSymbol();

	size_t GetSize() const;

private:
	

};

class CFloatSymbol : public CScalarTypeSymbol
{
public:
	CFloatSymbol();

	size_t GetSize() const;

private:

};

class CVoidSymbol : public CTypeSymbol
{
public:
	CVoidSymbol();

	size_t GetSize() const;

private:

};

class CArraySymbol : public CTypeSymbol
{
public:
	size_t GetSize() const;

	CTypeSymbol* GetElementsType() const;
	void SetElementsType(CTypeSymbol *AElementsType);

	unsigned int GetLength() const;
	void SetLength(unsigned int ALength);

private:
	CTypeSymbol *ElementsType;
	unsigned int Length;

};

class CStructSymbol : public CTypeSymbol
{
public:
	CStructSymbol();
	~CStructSymbol();

	size_t GetSize() const;

	void AddField(CSymbol *AField);

	CSymbolTable* GetSymbolTable();
	void SetSymbolTable(CSymbolTable *ASymbolTable);

	unsigned int GetFieldsCount() const;

private:
	CSymbolTable *Fields;

};

class CPointerSymbol : public CTypeSymbol
{
public:
	size_t GetSize() const;

	CTypeSymbol* GetRefType() const;
	void SetRefType(CTypeSymbol *ARefType);

private:
	CTypeSymbol *RefType;

};

class CTypedefSymbol : public CTypeSymbol
{
public:
	size_t GetSize() const;

	CTypeSymbol* GetRefType() const;
	void SetRefType(CTypeSymbol *ARefType);

private:
	CTypeSymbol *RefType;

};

class CVariableSymbol : public CSymbol
{
public:
	CTypeSymbol* GetType() const;
	void SetType(CTypeSymbol *AType);

private:
	CTypeSymbol *Type;
};

class CFunctionSymbol : public CSymbol
{
public:
	CFunctionSymbol();
	~CFunctionSymbol();

	CTypeSymbol* GetReturnType() const;
	void SetReturnType(CTypeSymbol *AReturnType);

	void AddArgument(CSymbol *AArgument);

	CSymbolTable* GetArgumentsSymbolTable();
	void SetArgumentsSymbolTable(CSymbolTable *ASymbolTable);

	CBlockStatement* GetBody() const;
	void SetBody(CBlockStatement *ABody);

private:
	CTypeSymbol *ReturnType;

	CSymbolTable *Arguments;
	CSymbolTable *Locals;

	CBlockStatement *Body;
};

#endif // _SYMBOLS_H_
