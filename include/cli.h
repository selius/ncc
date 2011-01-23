#ifndef _CLI_H_
#define _CLI_H_

#include "common.h"

struct CCompilerParameters
{
	CCompilerParameters();

	string InputFilename;
	string OutputFilename;
	ECompilerMode CompilerMode;
	EParserOutputMode ParserOutputMode;

};

class CCommandLineInterface
{
public:

	CCommandLineInterface(int argc, char *argv[]);

	CCompilerParameters ParseArguments();

	void PrintVersion();
	void PrintHelp(bool Full = false);

private:
	class CHelpContainer
	{
	public:
		struct CHelpEntry
		{
			CHelpEntry(const string &AShort, const string &ALong, const string &ADescription);

			string Short;
			string Long;
			string Description;
		};

		CHelpContainer();

		void Add(const string &AShort, const string &ALong, const string &ADescription);
		void Output(ostream &Stream);

	private:
		vector<CHelpEntry> Entries;
		size_t ShortWidth;
		size_t LongWidth;
	};

	void PopulateHelp();

	vector<string> Args;
	CCompilerParameters Parameters;
	CHelpContainer Help;
};


#endif // _CLI_H_
