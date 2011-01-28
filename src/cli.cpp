#include "cli.h"

/******************************************************************************
 * CCompilerParameters
 ******************************************************************************/

CCompilerParameters::CCompilerParameters() : CompilerMode(COMPILER_MODE_UNDEFINED), ParserOutputMode(PARSER_OUTPUT_MODE_TREE)
{
}

/******************************************************************************
 * CCommandLineInterface
 ******************************************************************************/

CCommandLineInterface::CCommandLineInterface(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		Args.push_back(argv[i]);
	}

	PopulateHelp();
}

CCompilerParameters CCommandLineInterface::ParseArguments()
{
	if (Args.size() < 2) {
		PrintVersion();
		PrintHelp();
		throw CFatalException(EXIT_CODE_TOO_FEW_ARGUMENTS);
	}

	string CurArg;
	string CurOpt;
	bool OptionsEnd = false;

	for (vector<string>::iterator it = ++Args.begin(); it != Args.end(); ++it) {
		CurArg = *it;
		if (!OptionsEnd && CurArg[0] == '-') {
			if (CurArg == "-v" || CurArg == "--version") {
				PrintVersion();
				throw CFatalException(EXIT_CODE_SUCCESS);
			} else if (CurArg == "-h" || CurArg == "--help") {
				PrintHelp(true);
				throw CFatalException(EXIT_CODE_SUCCESS);
			} else if (CurArg == "-o") {
				if (it == --Args.end()) {
					throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, CurArg + " option requires an argument");
				}

				if (!Parameters.OutputFilename.empty()) {
					throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, "only one output file could be specified");
				}

				Parameters.OutputFilename = *(++it);
			} else if (CurArg == "-S" || CurArg == "-P" || CurArg == "-G" || CurArg == "-O") {
				if (Parameters.CompilerMode != COMPILER_MODE_UNDEFINED) {
					throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, "only one mode could be specified");
				}

				if (CurArg == "-S") {
					Parameters.CompilerMode = COMPILER_MODE_SCAN;
				} else if (CurArg == "-P") {
					Parameters.CompilerMode = COMPILER_MODE_PARSE;
				} else if (CurArg == "-G") {
					Parameters.CompilerMode = COMPILER_MODE_GENERATE;
				} else if (CurArg == "-O") {
					Parameters.CompilerMode = COMPILER_MODE_OPTIMIZE;
				}
			} else if (CurArg == "--parser-output-mode") {
				if (it == --Args.end()) {
					throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, CurArg + " option requires an argument");
				}

				string OptValue = *(++it);
				if (OptValue == "linear") {
					Parameters.ParserOutputMode = PARSER_OUTPUT_MODE_LINEAR;
				} else if (OptValue == "tree") {
					Parameters.ParserOutputMode = PARSER_OUTPUT_MODE_TREE;
				} else {
					throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, "invalid value for " + CurArg + " option");
				}
			} else if (CurArg == "--") {
				OptionsEnd = true;
			} else {
				throw CFatalException(EXIT_CODE_INVALID_ARGUMENTS, "invalid option: " + CurArg);
			}
		} else {
			if (!Parameters.InputFilename.empty()) {
				throw CFatalException(EXIT_CODE_TOO_MANY_INPUT_FILES, "only one input file per run is supported");
			}

			Parameters.InputFilename = CurArg;
		}
	}
	if (Parameters.InputFilename.empty()) {
		throw CFatalException(EXIT_CODE_NO_INPUT_FILE, "no input file");
	}

	if (Parameters.CompilerMode == COMPILER_MODE_UNDEFINED) {
		Parameters.CompilerMode = COMPILER_MODE_PARSE;	// default mode
	}

	return Parameters;
}

void CCommandLineInterface::PrintVersion()
{
	cout << COMPILER_TITLE ", version " COMPILER_VERSION << endl << "Copyright 2010-2011  "
		"Alexander Nartov <alexander.nartov@gmail.com> (FEFU IMCS 238 group)" << endl << endl;
}

void CCommandLineInterface::PrintHelp(bool Full /*= false*/)
{
	cout << "Usage: " COMPILER_NAME " [options] input-file" << endl;
	
	if (Full) {
		Help.Output(cout);
	}

	cout << endl;
}

EExitCode CCommandLineInterface::Error(const string &Message, EExitCode ExitCode)
{
	cerr << COMPILER_NAME ": " << Message << endl << endl;
	return ExitCode;
}

CCommandLineInterface::CHelpContainer::CHelpEntry::CHelpEntry(const string &AShort, const string &ALong, const string &ADescription) : Short(AShort), Long(ALong), Description(ADescription)
{
}

CCommandLineInterface::CHelpContainer::CHelpContainer() : ShortWidth(0), LongWidth(0)
{
}

void CCommandLineInterface::CHelpContainer::Add(const string &AShort, const string &ALong, const string &ADescription)
{
	Entries.push_back(CHelpEntry(AShort, ALong, ADescription));
	ShortWidth = max(ShortWidth, AShort.length());
	LongWidth = max(LongWidth, ALong.length());
}

void CCommandLineInterface::CHelpContainer::Output(ostream &Stream)
{
	streamsize w = Stream.width();
	for (vector<CHelpEntry>::iterator it = Entries.begin(); it != Entries.end(); ++it) {
		Stream << "\t";
		Stream.width(ShortWidth);
		Stream << left << it->Short << "\t";
		Stream.width(LongWidth);
		Stream << left << it->Long << "\t";
		Stream.width(w);
		Stream << it->Description << "\n"; 
	}
}

void CCommandLineInterface::PopulateHelp()
{
	Help.Add("-v", "--version", "Print version info and exit");
	Help.Add("-h", "--help", "Print this help message and exit");
	Help.Add("-o", "filename", "Write output to a file named filename");

	Help.Add("-S", "--scan", "Run scanner");
	Help.Add("-P", "--parser", "Run parser");
	Help.Add("-G", "--generate", "Run code generator");
	Help.Add("-O", "--optimize", "Run optimizer");

	Help.Add("", "--parser-output-mode linear|tree", "Set parser output mode to linear or tree-like");
}
