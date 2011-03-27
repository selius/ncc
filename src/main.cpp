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

	try {
		if (Parameters.CompilerMode == COMPILER_MODE_SCAN) {
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
		} else if (Parameters.CompilerMode == COMPILER_MODE_PARSE) {
			CScanner scanner(in);
			CParser parser(scanner, Parameters.ParserMode);

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
					if (FuncSym && !FuncSym->GetBuiltIn()) {
						*out << FuncSym->GetName() << ":" << endl;
						if (!FuncSym->GetBody()) {
							*out << "\t(declared, but not defined)" << endl;
						}

						CSymbolTable *FSST = FuncSym->GetArgumentsSymbolTable();
						for (CSymbolTable::SymbolsIterator it = FSST->Begin(); it != FSST->End(); ++it) {
							*out << "\t" << it->second->GetName() <<": " << dynamic_cast<CVariableSymbol *>(it->second)->GetType()->GetName() << endl;
						}

						if (FuncSym->GetBody()) {
							FuncSym->GetBody()->Accept(*vis);
						}
					}
				}
			}

			delete vis;
		} else if (Parameters.CompilerMode == COMPILER_MODE_GENERATE) {
			CScanner scanner(in);
			CParser parser(scanner);
			CAsmCode code;
			CCodeGenerationVisitor vis(code);

			CSymbolTable *SymTable = parser.ParseTranslationUnit();

			CFunctionSymbol *FuncSym = NULL;

			for (CSymbolTable::SymbolsIterator it = SymTable->Begin(); it != SymTable->End(); ++it) {
				FuncSym = dynamic_cast<CFunctionSymbol *>(it->second);
				if (FuncSym && FuncSym->GetBody()) {
					vis.SetFunction(FuncSym);

					code.Add(new CAsmDirective("globl", FuncSym->GetName()));
					code.Add(FuncSym->GetName());
					FuncSym->GetBody()->Accept(vis);
					code.Add(RET);
				}
			}

			code.Output(*out);
		}
	} catch (CException &e) {
		e.Output(cerr);
		ExitCode = e.GetExitCode();
	}

	if (out != &cout) {
		delete out;
	}

	return ExitCode;
}
