#include <fstream>
#include <iostream>

#include "scanner.h"
#include "parser.h"

using namespace std;

const int TOKEN_NAME_FIELD_WIDTH = 31;

const char ARGS_HELP[][3][255] = {
	{"-v", "--version", "Print version info and exit"},
	{"-h", "--help", "Print this help message and exit"},
	{"-o", "filename", "Write output to a file named filename"}
	};
const int ARGS_HELP_COUNT = 3;
const int ARGS_HELP_SECOND_COLUMN_LENGTH = 20;

enum EExitCode
{
	EXIT_CODE_SUCCESS,
	EXIT_CODE_TOO_FEW_ARGUMENTS,
	EXIT_CODE_INVALID_ARGUMENTS,
	EXIT_CODE_NO_INPUT_FILE,
	EXIT_CODE_TOO_MANY_INPUT_FILES,
	EXIT_CODE_SCANNER_ERROR,
	EXIT_CODE_PARSER_ERROR,
	EXIT_CODE_NOT_IMPLEMENTED,
};

enum ECompilerMode
{
	COMPILER_MODE_UNDEFINED,
	COMPILER_MODE_SCAN,
	COMPILER_MODE_PARSE,
	COMPILER_MODE_GENERATE,
	COMPILER_MODE_OPTIMIZE,
};

enum EParserOutputMode
{
	PARSER_OUTPUT_MODE_TREE,
	PARSER_OUTPUT_MODE_LINEAR,
};

void PrintVersion()
{
	cout << "Nartov C Compiler, version 0.1.0" << endl << "Copyright 2010-2011  "
		"Alexander Nartov <alexander.nartov@gmail.com> (FEFU IMCS 238 group)" << endl << endl;
}

void PrintHelp(bool Full = false)
{
	cout << "Usage: ncc [options] input-file" << endl;
	
	if (Full) {
		streamsize w = cout.width();
		for (int i = 0; i < ARGS_HELP_COUNT; i++) {
			cout << "\t" << ARGS_HELP[i][0] << "\t";
			cout.width(ARGS_HELP_SECOND_COLUMN_LENGTH);
			cout << left << ARGS_HELP[i][1] << "\t";
			cout.width(w);
			cout << ARGS_HELP[i][2] << endl;
		}
	}

	cout << endl;
}

EExitCode Error(const string &Message, EExitCode ExitCode)
{
	cerr << "ncc: " << Message << endl << endl;
	return ExitCode;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		PrintVersion();
		PrintHelp();
		return EXIT_CODE_TOO_FEW_ARGUMENTS;
	}

	string InputFilename;
	string OutputFilename;
	ECompilerMode CompilerMode = COMPILER_MODE_UNDEFINED;
	EParserOutputMode ParserOutputMode = PARSER_OUTPUT_MODE_TREE;

	string CurArg;
	string CurOpt;
	bool OptionsEnd = false;
	for (int i = 1; i < argc; i++)
	{
		CurArg = argv[i];
		if (!OptionsEnd && CurArg[0] == '-') {
			if (CurArg == "-v" || CurArg == "--version") {
				PrintVersion();
				return EXIT_CODE_SUCCESS;
			} else if (CurArg == "-h" || CurArg == "--help") {
				PrintHelp(true);
				return EXIT_CODE_SUCCESS;
			} else if (CurArg == "-o") {
				if (i == (argc - 1)) {
					return Error("-o option requires an argument", EXIT_CODE_INVALID_ARGUMENTS);
				}

				if (!OutputFilename.empty()) {
					return Error("only one output file could be specified", EXIT_CODE_INVALID_ARGUMENTS);
				}

				OutputFilename = argv[++i];
			} else if (CurArg == "-S" || CurArg == "-P" || CurArg == "-G" || CurArg == "-O") {
				if (CompilerMode != COMPILER_MODE_UNDEFINED) {
					return Error("only one mode could be specified", EXIT_CODE_INVALID_ARGUMENTS);
				}

				if (CurArg == "-S") {
					CompilerMode = COMPILER_MODE_SCAN;
				} else if (CurArg == "-P") {
					CompilerMode = COMPILER_MODE_PARSE;
				} else if (CurArg == "-G") {
					CompilerMode = COMPILER_MODE_GENERATE;
				} else if (CurArg == "-O") {
					CompilerMode = COMPILER_MODE_OPTIMIZE;
				}
			} else if (CurArg == "--parser-output-mode") {
				if (i == (argc - 1)) {
					return Error("--parser-output-mode option requires an argument", EXIT_CODE_INVALID_ARGUMENTS);
				}

				string OptValue = argv[++i];
				if (OptValue == "linear") {
					ParserOutputMode = PARSER_OUTPUT_MODE_LINEAR;
				} else if (OptValue == "tree") {
					ParserOutputMode = PARSER_OUTPUT_MODE_TREE;
				} else {
					return Error("invalid value for --parser-output-mode option", EXIT_CODE_INVALID_ARGUMENTS);
				}
			} else if (CurArg == "--") {
				OptionsEnd = true;
			} else {
				return Error(string("invalid option: ") + CurArg, EXIT_CODE_INVALID_ARGUMENTS);
			}
		} else {
			if (!InputFilename.empty()) {
				return Error("only one input file per run is supported", EXIT_CODE_TOO_MANY_INPUT_FILES);
			}

			InputFilename = argv[i];
		}
	}
	if (InputFilename.empty()) {
		return Error("no input file", EXIT_CODE_NO_INPUT_FILE);
	}

	if (CompilerMode == COMPILER_MODE_UNDEFINED) {
		CompilerMode = COMPILER_MODE_SCAN;	// set to the latest implemented mode..
	}

	EExitCode ExitCode = EXIT_CODE_SUCCESS;

	ifstream in(InputFilename.c_str());

	ostream *out = &cout;
	if (!OutputFilename.empty()) {
		out = new ofstream(OutputFilename.c_str());
	}

	if (CompilerMode == COMPILER_MODE_SCAN) {
		try {
			CScanner scanner(in);
			const CToken *token = NULL;

			streamsize w = cout.width();

			do {
				token = scanner.Next();
				*out << token->GetPosition().Line << '\t' << token->GetPosition().Column << '\t';
				out->width(TOKEN_NAME_FIELD_WIDTH);
				*out << left << token->GetStringifiedType();
				out->width(w);
				*out << '\t' << token->GetText() << endl;

			} while (token->GetType() != TOKEN_TYPE_EOF);

		} catch (CException e) {
			cerr << e.GetPosition().Line << ", " << e.GetPosition().Column
				<< ": error: " << e.GetMessage() << endl;
			ExitCode = EXIT_CODE_SCANNER_ERROR;
		}
	} else if (CompilerMode == COMPILER_MODE_PARSE) {
		CExpressionVisitor *vis;
		if (ParserOutputMode == PARSER_OUTPUT_MODE_TREE) {
			vis = new CExpressionTreePrintVisitor(*out);
		} else {
			vis = new CExpressionLinearPrintVisitor(*out);
		}
		CBinaryOp *expr = new CBinaryOp(CToken(TOKEN_TYPE_OPERATION_PLUS, "+", CPosition()));
		expr->SetLeft(new CIntegerConst(CIntegerConstToken("5", CPosition())));

		CBinaryOp *subexpr = new CBinaryOp(CToken(TOKEN_TYPE_OPERATION_PLUS, "+", CPosition()));
		subexpr->SetLeft(new CVariable(CToken(TOKEN_TYPE_IDENTIFIER, "a", CPosition())));
		
		CBinaryOp *subsubexpr = new CBinaryOp(CToken(TOKEN_TYPE_OPERATION_ASTERISK, "*", CPosition()));
		subsubexpr->SetLeft(new CIntegerConst(CIntegerConstToken("2", CPosition())));
		subsubexpr->SetRight(new CVariable(CToken(TOKEN_TYPE_IDENTIFIER, "b", CPosition())));

		subexpr->SetRight(subsubexpr);
		expr->SetRight(subexpr);

		expr->Accept(*vis);

		delete vis;
	} else {
		ExitCode = Error("mode is not implemented yet", EXIT_CODE_NOT_IMPLEMENTED);
	}

	if (out != &cout) {
		delete out;
	}


	return ExitCode;
}
