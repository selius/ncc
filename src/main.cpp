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
