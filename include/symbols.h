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

class CVariableSymbol;

class CSymbolTable
{
public:
	typedef map<string, CSymbol *> SymbolsContainer;
	typedef SymbolsContainer::const_iterator SymbolsIterator;

	CSymbolTable();
	virtual ~CSymbolTable();

	void Add(CSymbol *ASymbol);
	CSymbol* Get(const string &AName) const;
	bool Exists(const string &AName) const;

	SymbolsIterator Begin() const;
	SymbolsIterator End() const;

	unsigned int GetSize() const;

	size_t GetElementsSize() const;

	size_t GetCurrentOffset() const;
	void SetCurrentOffset(size_t AOffset);

protected:
	virtual void InitOffset(CVariableSymbol *ASymbol);
	SymbolsContainer Symbols;

	size_t CurrentOffset;
	size_t ElementsSize;

};

class CArgumentsSymbolTable : public CSymbolTable
{
protected:
	void InitOffset(CVariableSymbol *ASymbol);
};

class CStructSymbolTable : public CSymbolTable
{
	// TODO: make this class more useful, get rid of dynamic_casts, etc.
	// 	structs can only have CVariableSymbols
protected:
	void InitOffset(CVariableSymbol *ASymbol);
};

class CTagsSymbolTable : public CSymbolTable
{
public:
	// TODO: add interface and implementaton for tags symbol table;
	// 	tags have scope that begins just after their appearance, and ends at the block end.

};

class CSymbolTableStack
{
public:
	typedef deque<CSymbolTable *> TablesContainer;
	typedef TablesContainer::const_iterator TablesIterator;

	void Push(CSymbolTable *ATable);
	CSymbolTable* Pop();

	CSymbolTable* GetTop() const;
	CSymbolTable* GetPreviousTop() const;
	CSymbolTable* GetGlobal() const;

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

// TODO: add support for incomplete types..
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

	// TODO: make void symbol incomplete
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

	CStructSymbolTable* GetSymbolTable();
	void SetSymbolTable(CStructSymbolTable *ASymbolTable);

	unsigned int GetFieldsCount() const;

	bool IsStruct() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

private:
	CStructSymbolTable *Fields;
	// TODO: struct can also have internal tags..

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

	int GetOffset() const;
	void SetOffset(int AOffset);

	bool GetGlobal() const;
	void SetGlobal(bool AGlobal);

private:
	CTypeSymbol *Type;
	int Offset;
	bool Global;
};

class CFunctionSymbol : public CSymbol
{
public:
	typedef vector<CSymbol *> ArgumentsOrderContainer;
	typedef ArgumentsOrderContainer::iterator ArgumentsOrderIterator;
	typedef ArgumentsOrderContainer::reverse_iterator ArgumentsReverseOrderIterator;

	CFunctionSymbol(const string &AName = "", CTypeSymbol *AReturnType = NULL);
	~CFunctionSymbol();

	CTypeSymbol* GetReturnType() const;
	void SetReturnType(CTypeSymbol *AReturnType);

	void AddArgument(CSymbol *AArgument);

	CArgumentsSymbolTable* GetArgumentsSymbolTable();
	void SetArgumentsSymbolTable(CArgumentsSymbolTable *ASymbolTable);

	ArgumentsOrderContainer* GetArgumentsOrderedList();

	CBlockStatement* GetBody() const;
	void SetBody(CBlockStatement *ABody);

	CTypeSymbol* GetType() const;

	bool GetBuiltIn() const;
	void SetBuiltIn(bool ABuiltIn);

	// TODO: add possibility to check equality of function types
	// 	a function is characterized by return type and number and type of arguments

private:
	CTypeSymbol *ReturnType;

	CArgumentsSymbolTable *Arguments;
	ArgumentsOrderContainer ArgumentsOrder;

	CBlockStatement *Body;

	CTypeSymbol *Type;

	bool BuiltIn;
};

#endif // _SYMBOLS_H_
