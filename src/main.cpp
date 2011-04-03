#include "common.h"
#include "cli.h"
#include "scanner.h"
#include "parser.h"
#include "codegen.h"
#include "prettyprinting.h"

int main(int argc, char *argv[])
{
	CCommandLineInterface CLI(argc, argv);

	CCompilerParameters Parameters;

	try {
		Parameters = CLI.ParseArguments();
	} catch (CFatalException &e) {
		if (!e.GetMessage().empty()) {
			CLI.Error(e.GetMessage(), e.GetExitCode());
		}

		return e.GetExitCode();
	}

	EExitCode ExitCode = EXIT_CODE_SUCCESS;

	istream *in = &cin;
	if (Parameters.InputFilename != "-") {
		in = new ifstream(Parameters.InputFilename.c_str());
	}

	ostream *out = &cout;
	if (!Parameters.OutputFilename.empty() && Parameters.OutputFilename != "-") {
		out = new ofstream(Parameters.OutputFilename.c_str());
	}

	try {
		CScanner Scanner(*in);

		if (Parameters.CompilerMode == COMPILER_MODE_SCAN) {
			CScanPrettyPrinter Printer(Scanner);
			Printer.Output(*out);

		} else if (Parameters.CompilerMode == COMPILER_MODE_PARSE) {
			CParser Parser(Scanner, Parameters.ParserMode);
			CParsePrettyPrinter Printer(Parser, Parameters);
			Printer.Output(*out);

		} else if (Parameters.CompilerMode == COMPILER_MODE_GENERATE) {
			CParser Parser(Scanner, Parameters.ParserMode);
			CCodeGenerator Generator(Parser, Parameters);
			Generator.Output(*out);
		}

	} catch (CException &e) {
		e.Output(cerr);
		ExitCode = e.GetExitCode();
	}

	if (in != &cin) {
		delete in;
	}
	if (out != &cout) {
		delete out;
	}

	return ExitCode;
}
