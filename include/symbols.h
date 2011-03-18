#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include "common.h"

class CSymbol
{
public:
	CSymbol(const string &AName = "");
	virtual ~CSymbol();

	virtual string GetName() const;
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

	CSymbolTable();
	~CSymbolTable();

	void Add(CSymbol *ASymbol);
	CSymbol* Get(const string &AName) const;
	bool Exists(const string &AName) const;

	SymbolsIterator Begin() const;
	SymbolsIterator End() const;

	unsigned int GetSize() const;

	unsigned int GetElementsSize() const;

private:
	SymbolsContainer Symbols;

	size_t CurrentOffset;

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
	CTypeSymbol(const string &AName = "");

	virtual size_t GetSize() const = 0;

	bool GetConst() const;
	void SetConst(bool AConst);

	virtual bool IsInt() const;
	virtual bool IsFloat() const;
	bool IsArithmetic() const;
	virtual bool IsVoid() const;
	virtual bool IsType(const string &AType) const;
	virtual bool IsPointer() const;
	virtual bool IsStruct() const;
	virtual bool IsArray() const;
	virtual bool IsFunction() const;
	bool IsScalar() const;

	virtual bool CompatibleWith(CTypeSymbol *ASymbol);

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

	bool IsInt() const;

private:
	

};

class CFloatSymbol : public CScalarTypeSymbol
{
public:
	CFloatSymbol();

	size_t GetSize() const;

	bool IsFloat() const;

private:

};

class CVoidSymbol : public CTypeSymbol
{
public:
	CVoidSymbol();

	size_t GetSize() const;

	bool IsVoid() const;

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

	bool IsPointer() const;
	bool IsArray() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

private:
	CTypeSymbol *ElementsType;
	unsigned int Length;

};

class CVariableSymbol;

class CStructSymbol : public CTypeSymbol
{
public:
	CStructSymbol();
	~CStructSymbol();

	size_t GetSize() const;

	void AddField(CSymbol *AField);

	CVariableSymbol* GetField(const string &AName);

	CSymbolTable* GetSymbolTable();
	void SetSymbolTable(CSymbolTable *ASymbolTable);

	unsigned int GetFieldsCount() const;

	bool IsStruct() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

private:
	CSymbolTable *Fields;

};

class CPointerSymbol : public CTypeSymbol
{
public:
	CPointerSymbol(CTypeSymbol *ARefType = NULL);

	string GetName() const;

	size_t GetSize() const;

	CTypeSymbol* GetRefType() const;
	void SetRefType(CTypeSymbol *ARefType);

	bool IsPointer() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

private:
	CTypeSymbol *RefType;

};

class CTypedefSymbol : public CTypeSymbol
{
public:
	CTypedefSymbol(const string &AName = "", CTypeSymbol *ARefType = NULL);

	size_t GetSize() const;

	CTypeSymbol* GetRefType() const;
	void SetRefType(CTypeSymbol *ARefType);

	bool IsInt() const;
	bool IsFloat() const;
	bool IsVoid() const;
	bool IsType(const string &AType) const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

private:
	CTypeSymbol *RefType;

};

class CFunctionTypeSymbol : public CTypeSymbol
{
public:
	size_t GetSize() const;

	bool IsFunction() const;
};

class CVariableSymbol : public CSymbol
{
public:
	CVariableSymbol(const string &AName = "", CTypeSymbol *AType = NULL);

	CTypeSymbol* GetType() const;
	void SetType(CTypeSymbol *AType);

	size_t GetOffset() const;
	void SetOffset(size_t AOffset);

private:
	CTypeSymbol *Type;
	size_t Offset;
};

class CFunctionSymbol : public CSymbol
{
public:
	CFunctionSymbol(const string &AName = "", CTypeSymbol *AReturnType = NULL);
	~CFunctionSymbol();

	CTypeSymbol* GetReturnType() const;
	void SetReturnType(CTypeSymbol *AReturnType);

	void AddArgument(CSymbol *AArgument);

	CSymbolTable* GetArgumentsSymbolTable();
	void SetArgumentsSymbolTable(CSymbolTable *ASymbolTable);

	CBlockStatement* GetBody() const;
	void SetBody(CBlockStatement *ABody);

	CTypeSymbol* GetType() const;

private:
	CTypeSymbol *ReturnType;

	CSymbolTable *Arguments;
	//CSymbolTable *Locals;

	CBlockStatement *Body;

	CTypeSymbol *Type;
};

#endif // _SYMBOLS_H_
