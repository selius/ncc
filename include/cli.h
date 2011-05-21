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

#ifndef _CLI_H_
#define _CLI_H_

#include "common.h"

class CCommandLineInterface
{
public:

	CCommandLineInterface(int argc, char *argv[]);

	CCompilerParameters ParseArguments();

	void PrintVersion();
	void PrintHelp(bool Full = false);

	EExitCode Error(const string &Message, EExitCode ExitCode);

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
		void AddSeparator();
		void Output(ostream &Stream);

	private:
		vector<CHelpEntry> Entries;
		size_t ShortWidth;
		size_t LongWidth;
	};

	typedef vector<string> ArgumentsContainer;
	typedef ArgumentsContainer::iterator ArgumentsIterator;

	void PopulateHelp();

	void RequireArgument(ArgumentsIterator &AOption);

	ArgumentsContainer Args;
	CCompilerParameters Parameters;
	CHelpContainer Help;
};


#endif // _CLI_H_
