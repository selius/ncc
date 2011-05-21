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

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "common.h"

class CToken
{
public:
	CToken(ETokenType AType, const string &AText, const CPosition &APosition);
	virtual ~CToken();

	virtual CToken* Clone() const;

	ETokenType GetType() const;
	string GetStringifiedType() const;
	string GetText() const;
	CPosition GetPosition() const;

	virtual int GetIntegerValue() const;
	virtual float GetFloatValue() const;
	virtual char GetCharValue() const;

protected:
	ETokenType Type;
	string Text;
	CPosition Position;

	friend class CScanner;
};

class CIntegerConstToken : public CToken
{
public:
	CIntegerConstToken(const string &AText, const CPosition &APosition);

	CIntegerConstToken* Clone() const;

	int GetIntegerValue() const;

protected:
	int Value;

};

class CFloatConstToken : public CToken
{
public:
	CFloatConstToken(const string &AText, const CPosition &APosition);

	CFloatConstToken* Clone() const;

	float GetFloatValue() const;

protected:
	float Value;

};

class CCharConstToken : public CToken
{
public:
	CCharConstToken(const string &AText, const CPosition &APosition);

	CCharConstToken* Clone() const;

	char GetCharValue() const;

protected:
	char Value;

};

class CScanner
{
public:
	CScanner(istream &AInputStream);
	~CScanner();

	const CToken* GetToken();
	const CToken* Next();

	static map<ETokenType, string> TokenTypesNames;

private:
	CToken* ScanIdentifier();
	CToken* ScanOperation();
	CToken* ScanSingleChar();
	CToken* ScanStringConstant();
	CToken* ScanCharConstant();
	CToken* ScanNumericalConstant();

	bool TryScanNumericalConstant();
	string ScanHexadecimalInteger();
	string ScanOctalInteger();
	string ScanIntegerPart();
	string ScanFractionalPart();
	string ScanExponentPart();
	string ScanFloatSuffix();
	string ScanIntegerSuffix();

	char ProcessEscapeSequence();

	bool SkipComment();
	bool SkipWhitespace();
	void SkipWhitespaceAndComments();

	char NextChar();

	istream &InputStream;
	CToken *LastToken;
	CPosition CurrentPosition;
};


#endif // _SCANNER_H_
