#include "common.h"
#include "cli.h"
#include "scanner.h"
#include "parser.h"
#include "codegen.h"
#include "prettyprinting.h"
#include "statements.h"

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

			CStatementVisitor *vis;

			if (Parameters.ParserOutputMode == PARSER_OUTPUT_MODE_TREE) {
				vis = new CStatementTreePrintVisitor(*out);
			} else {
				vis = new CStatementLinearPrintVisitor(*out);
			}

			if (Parameters.ParserMode == PARSER_MODE_EXPRESSION) {
				CExpression *expr = parser.ParseExpression();

				if (scanner.GetToken()->GetType() != TOKEN_TYPE_EOF) {
					throw CException("trailing characters", scanner.GetToken()->GetPosition());
				}

				expr->Accept(*vis);
			} else {
				CSymbolTable *SymTable = parser.ParseTranslationUnit();

				CFunctionSymbol *FuncSym = NULL;

				for (CSymbolTable::SymbolsIterator it = SymTable->Begin(); it != SymTable->End(); ++it) {
					FuncSym = dynamic_cast<CFunctionSymbol *>(it->second);
					if (FuncSym) {
						*out << FuncSym->GetName() << ":" << endl;
						if (FuncSym->GetBody()) {
							FuncSym->GetBody()->Accept(*vis);
						} else {
							*out << "\t(declared, but not defined)" << endl;
						}
					}
				}
			}

			delete vis;

		} catch (CException e) {
			e.Output(cerr);
			ExitCode = EXIT_CODE_PARSER_ERROR;
		}
	} else if (Parameters.CompilerMode == COMPILER_MODE_GENERATE) {
		try {
			CScanner scanner(in);
			CParser parser(scanner);
			CAsmCode code;
			//CCodeGenerationVisitor vis(code);

			CSymbolTable *SymTable = parser.ParseTranslationUnit();

			CFunctionSymbol *FuncSym = NULL;

			for (CSymbolTable::SymbolsIterator it = SymTable->Begin(); it != SymTable->End(); ++it) {
				FuncSym = dynamic_cast<CFunctionSymbol *>(it->second);
				if (FuncSym && FuncSym->GetBody()) {
					CCodeGenerationVisitor vis(code, FuncSym);

					code.Add(new CAsmDirective("globl", FuncSym->GetName()));
					code.Add(new CAsmLabel(FuncSym->GetName()));
					FuncSym->GetBody()->Accept(vis);
					code.Add(RET);
				}
			}

			code.Output(*out);
		} catch (CException e) {
			e.Output(cerr);
			//ExitCode = EXIT_CODE_GENERATOR_ERROR;
		}
	} else {
		ExitCode = CLI.Error("mode is not implemented yet", EXIT_CODE_NOT_IMPLEMENTED);
	}

	if (out != &cout) {
		delete out;
	}

	return ExitCode;
}
