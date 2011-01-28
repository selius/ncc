#include "common.h"
#include "cli.h"
#include "scanner.h"
#include "parser.h"

int main(int argc, char *argv[])
{
	CCommandLineInterface CLI(argc, argv);

	CCompilerParameters Parameters;

	try {
		Parameters = CLI.ParseArguments();
	} catch (CFatalException e) {
		if (!e.GetMessage().empty()) {
			CLI.Error(e.GetMessage(), e.GetExitCode());
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

			const streamsize TOKEN_NAME_FIELD_WIDTH = 31;
			streamsize w = out->width();

			do {
				token = scanner.Next();
				*out << token->GetPosition().Line << '\t' << token->GetPosition().Column << '\t';
				out->width(TOKEN_NAME_FIELD_WIDTH);
				*out << left << token->GetStringifiedType();
				out->width(w);
				*out << '\t' << token->GetText() << endl;

			} while (token->GetType() != TOKEN_TYPE_EOF);

		} catch (CException e) {
			e.Output(cerr);
			ExitCode = EXIT_CODE_SCANNER_ERROR;
		}
	} else if (Parameters.CompilerMode == COMPILER_MODE_PARSE) {
		try {
			CScanner scanner(in);
			CParser parser(scanner);

			CExpressionVisitor *vis;

			CExpression *expr = parser.ParseSimpleExpression();

			if (scanner.GetToken()->GetType() != TOKEN_TYPE_EOF) {
				throw CException("trailing characters", scanner.GetToken()->GetPosition());
			}

			if (Parameters.ParserOutputMode == PARSER_OUTPUT_MODE_TREE) {
				vis = new CExpressionTreePrintVisitor(*out);
			} else {
				vis = new CExpressionLinearPrintVisitor(*out);
			}

			expr->Accept(*vis);

			delete vis;
			delete expr;

		} catch (CException e) {
			e.Output(cerr);
			ExitCode = EXIT_CODE_PARSER_ERROR;
		}
	} else {
		ExitCode = CLI.Error("mode is not implemented yet", EXIT_CODE_NOT_IMPLEMENTED);
	}

	if (out != &cout) {
		delete out;
	}

	return ExitCode;
}
