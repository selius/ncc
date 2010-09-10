#include <fstream>
#include <iostream>

#include "scanner.h"

using namespace std;

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
	while (!scanner.IsError()) {
		cout << token.GetPosition().Line << '\t' << token.GetPosition().Column << '\t' << token.GetStringifiedType() << '\t' << token.GetValue() << endl;

		if (token.GetType() == CToken::TOKEN_TYPE_EOF) {
			break;
		}

		token = scanner.Next();
	}

	return (scanner.IsError() ? 1 : 0);
}
