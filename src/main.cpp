#include <fstream>
#include <iostream>

#include "scanner.h"

using namespace std;

const int TOKEN_NAME_FIELD_WIDTH = 31;

void PrintVersion()
{
	cout << "Nartov C Compiler, version 0.0.1" << endl << "Copyright 2010  Alexander Nartov" << endl << endl;
}

void PrintHelp()
{
	PrintVersion();
	cout << "Usage: ncc input-file" << endl << endl;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		PrintHelp();
		return 0;
	}
	ifstream in(argv[1]);
	CScanner scanner(in);

	CToken token;

	token = scanner.Next();
	streamsize w = cout.width();
	while (!scanner.IsError()) {
		cout << token.GetPosition().Line << '\t' << token.GetPosition().Column << '\t';
		cout.width(TOKEN_NAME_FIELD_WIDTH);
		cout << left << token.GetStringifiedType();
		cout.width(w);
		cout << '\t' << token.GetValue() << endl;

		if (token.GetType() == CToken::TOKEN_TYPE_EOF) {
			break;
		}

		token = scanner.Next();
	}

	return (scanner.IsError() ? 1 : 0);
}
