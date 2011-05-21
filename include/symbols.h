/*
	ncc - Nartov C Compiler
	Copyright 2010-2011  Alexander Nartov

	ncc is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ncc is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ncc.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include "common.h"

enum ESymbolType
{
	SYMBOL_TYPE_VARIABLE,
	SYMBOL_TYPE_FUNCTION,
	SYMBOL_TYPE_TYPE,
};

class CSymbolsPrettyPrinter;

class CSymbol
{
public:
	CSymbol(const string &AName = "");
	virtual ~CSymbol();

	string GetName() const;
	void SetName(const string &AName);

	virtual ESymbolType GetSymbolType() const = 0;

	virtual void Accept(CSymbolsPrettyPrinter &AVisitor) = 0;

protected:
	string Name;

};

class CVariableSymbol;
class CFunctionSymbol;
class CTypeSymbol;
class CStructSymbol;

class CSymbolTable
{
public:
	typedef map<string, CVariableSymbol *> VariablesContainer;
	typedef VariablesContainer::const_iterator VariablesIterator;

	typedef map<string, CTypeSymbol *> TypesContainer;
	typedef TypesContainer::const_iterator TypesIterator;

	typedef map<string, CStructSymbol *> TagsContainer;
	typedef TagsContainer::const_iterator TagsIterator;

	CSymbolTable();
	virtual ~CSymbolTable();

	void AddVariable(CVariableSymbol *ASymbol);
	void AddType(CTypeSymbol *ASymbol);
	void AddTag(CStructSymbol *ASymbol);

	CVariableSymbol* GetVariable(const string &AName) const;
	CTypeSymbol* GetType(const string &AName) const;
	virtual CFunctionSymbol* GetFunction(const string &AName) const;
	CStructSymbol* GetTag(const string &AName) const;

	virtual CSymbol* Get(const string &AName) const;

	virtual bool Exists(const string &AName) const;

	VariablesIterator VariablesBegin() const;
	VariablesIterator VariablesEnd() const;

	TypesIterator TypesBegin() const;
	TypesIterator TypesEnd() const;

	unsigned int GetSize() const;

	unsigned int VariablesSize() const;

	size_t GetElementsSize() const;

	size_t GetCurrentOffset() const;
	void SetCurrentOffset(size_t AOffset);

protected:
	virtual void InitOffset(CVariableSymbol *ASymbol);

	VariablesContainer Variables;
	TypesContainer Types;
	TagsContainer Tags;

	size_t CurrentOffset;
	size_t ElementsSize;
};

class CGlobalSymbolTable : public CSymbolTable
{
public:
	typedef map<string, CFunctionSymbol *> FunctionsContainer;
	typedef FunctionsContainer::const_iterator FunctionsIterator;

	~CGlobalSymbolTable();

	void AddFunction(CFunctionSymbol *ASymbol);

	CFunctionSymbol* GetFunction(const string &AName) const;

	CSymbol* Get(const string &AName) const;

	bool Exists(const string &AName) const;

	FunctionsIterator FunctionsBegin() const;
	FunctionsIterator FunctionsEnd() const;

private:
	FunctionsContainer Functions;

};

class CArgumentsSymbolTable : public CSymbolTable
{
protected:
	void InitOffset(CVariableSymbol *ASymbol);
};

class CStructSymbolTable : public CSymbolTable
{
protected:
	void InitOffset(CVariableSymbol *ASymbol);
};

class CSymbolTableStack
{
public:
	typedef deque<CSymbolTable *> TablesContainer;
	typedef TablesContainer::const_iterator TablesIterator;

	CSymbolTableStack();

	void Push(CSymbolTable *ATable);
	CSymbolTable* Pop();

	CSymbolTable* GetTop() const;
	CSymbolTable* GetPreviousTop() const;
	CGlobalSymbolTable* GetGlobal() const;

	void SetGlobal(CGlobalSymbolTable *ASymbolTable);

	CVariableSymbol* LookupVariable(const string &AName) const;
	CTypeSymbol* LookupType(const string &AName) const;
	CStructSymbol* LookupTag(const string &AName) const;
	CSymbol* LookupAll(const string &AName) const;

private:
	TablesContainer Tables;
	CGlobalSymbolTable *Global;

};

class CTypeSymbol : public CSymbol
{
public:
	CTypeSymbol(const string &AName = "");

	virtual string GetQualifiedName() const;

	ESymbolType GetSymbolType() const;

	virtual size_t GetSize() const = 0;

	bool GetConst() const;
	void SetConst(bool AConst);

	bool GetComplete() const;
	void SetComplete(bool AComplete);

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

	void Accept(CSymbolsPrettyPrinter &AVisitor);

	virtual CTypeSymbol* ConstClone() const;

protected:
	bool Const;
	bool Complete;
};

class CIntegerSymbol : public CTypeSymbol
{
public:
	CIntegerSymbol();

	size_t GetSize() const;
	bool IsInt() const;

	CIntegerSymbol* ConstClone() const;
};

class CFloatSymbol : public CTypeSymbol
{
public:
	CFloatSymbol();

	size_t GetSize() const;
	bool IsFloat() const;

	CFloatSymbol* ConstClone() const;
};

class CVoidSymbol : public CTypeSymbol
{
public:
	CVoidSymbol();

	size_t GetSize() const;
	bool IsVoid() const;

	CVoidSymbol* ConstClone() const;
};

class CArraySymbol : public CTypeSymbol
{
public:
	CArraySymbol(CTypeSymbol *AElementsType = NULL, unsigned int ALength = 0);

	size_t GetSize() const;

	CTypeSymbol* GetElementsType() const;
	void SetElementsType(CTypeSymbol *AElementsType);

	unsigned int GetLength() const;
	void SetLength(unsigned int ALength);

	bool IsPointer() const;
	bool IsArray() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

	CArraySymbol* ConstClone() const;

private:
	void UpdateName();

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

	void AddField(CVariableSymbol *AField);

	CVariableSymbol* GetField(const string &AName);

	CStructSymbolTable* GetSymbolTable();
	void SetSymbolTable(CStructSymbolTable *ASymbolTable);

	unsigned int GetFieldsCount() const;

	bool IsStruct() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

	void Accept(CSymbolsPrettyPrinter &AVisitor);

	CStructSymbol* ConstClone() const;

private:
	CStructSymbolTable *Fields;
};

class CPointerSymbol : public CTypeSymbol
{
public:
	CPointerSymbol(CTypeSymbol *ARefType = NULL);

	string GetQualifiedName() const;

	size_t GetSize() const;

	CTypeSymbol* GetRefType() const;
	void SetRefType(CTypeSymbol *ARefType);

	bool IsPointer() const;

	bool CompatibleWith(CTypeSymbol *ASymbol);

	CPointerSymbol* ConstClone() const;

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

	void Accept(CSymbolsPrettyPrinter &AVisitor);

private:
	CTypeSymbol *RefType;

};

class CFunctionTypeSymbol : public CTypeSymbol
{
public:
	CFunctionTypeSymbol();

	size_t GetSize() const;

	bool IsFunction() const;
};

class CVariableSymbol : public CSymbol
{
public:
	CVariableSymbol(const string &AName = "", CTypeSymbol *AType = NULL);

	ESymbolType GetSymbolType() const;

	CTypeSymbol* GetType() const;
	void SetType(CTypeSymbol *AType);

	int GetOffset() const;
	void SetOffset(int AOffset);

	bool GetGlobal() const;
	void SetGlobal(bool AGlobal);

	float GetInitValue() const;
	void SetInitValue(float AInitValue);

	void Accept(CSymbolsPrettyPrinter &AVisitor);

private:
	CTypeSymbol *Type;
	int Offset;
	bool Global;
	float InitValue;
};

class CFunctionSymbol : public CSymbol
{
public:
	typedef vector<CVariableSymbol *> ArgumentsOrderContainer;
	typedef ArgumentsOrderContainer::iterator ArgumentsOrderIterator;
	typedef ArgumentsOrderContainer::reverse_iterator ArgumentsReverseOrderIterator;

	CFunctionSymbol(const string &AName = "", CTypeSymbol *AReturnType = NULL);
	~CFunctionSymbol();

	ESymbolType GetSymbolType() const;

	CTypeSymbol* GetReturnType() const;
	void SetReturnType(CTypeSymbol *AReturnType);

	void AddArgument(CVariableSymbol *AArgument);

	CArgumentsSymbolTable* GetArgumentsSymbolTable();
	void SetArgumentsSymbolTable(CArgumentsSymbolTable *ASymbolTable);

	ArgumentsOrderContainer* GetArgumentsOrderedList();

	CBlockStatement* GetBody() const;
	void SetBody(CBlockStatement *ABody);

	CTypeSymbol* GetType() const;

	bool GetBuiltIn() const;
	void SetBuiltIn(bool ABuiltIn);

	void Accept(CSymbolsPrettyPrinter &AVisitor);

private:
	CTypeSymbol *ReturnType;

	CArgumentsSymbolTable *Arguments;
	ArgumentsOrderContainer ArgumentsOrder;

	CBlockStatement *Body;

	CTypeSymbol *Type;

	bool BuiltIn;
};

#endif // _SYMBOLS_H_
