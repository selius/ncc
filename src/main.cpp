#include "common.h"
#include "cli.h"
#include "scanner.h"
#include "parser.h"

const int TOKEN_NAME_FIELD_WIDTH = 31;

EExitCode Error(const string &Message, EExitCode ExitCode)
{
	cerr << COMPILER_NAME ": " << Message << endl << endl;
	return ExitCode;
}

int main(int argc, char *argv[])
{
	CCommandLineInterface CLI(argc, argv);

	CCompilerParameters Parameters;

	try {
		Parameters = CLI.ParseArguments();
	} catch (CFatalException e) {
		if (!e.GetMessage().empty()) {
			Error(e.GetMessage(), e.GetExitCode());
		}

		return e.GetExitCode();
	}

	EExitCode ExitCode = EXIT_CODE_SUCCESS;

	ifstream in(Parameters.InputFilename.c_str());

	ostream *out = &cout;
	if (!Parameters.OutputFilename.empty()) {
		out = new ofstream(Parameters.OutputFilename.c_str());
	}

	if (Parameters.CompilerMode == COMPILER_MODE_SCAN) {
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
	} else if (Parameters.CompilerMode == COMPILER_MODE_PARSE) {
		CExpressionVisitor *vis;
		if (Parameters.ParserOutputMode == PARSER_OUTPUT_MODE_TREE) {
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
