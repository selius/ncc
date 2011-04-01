#include "common.h"

#include "statements.h"
#include "scanner.h"

/******************************************************************************
 * CCompilerParameters
 ******************************************************************************/

CCompilerParameters::CCompilerParameters() : CompilerMode(COMPILER_MODE_UNDEFINED), ParserOutputMode(PARSER_OUTPUT_MODE_TREE), ParserMode(PARSER_MODE_NORMAL), Optimize(false)
{
}

/******************************************************************************
 * CPosition
 ******************************************************************************/

CPosition::CPosition(int ALine /*= 0*/, int AColumn /*= 0*/) : Line(ALine), Column(AColumn)
{
}

/******************************************************************************
 * CException
 ******************************************************************************/

CException::CException(const string &AMessage, const CPosition &APosition) : Message(AMessage), Position(APosition)
{
}

string CException::GetMessage() const
{
	return Message;
}

CPosition CException::GetPosition() const
{
	return Position;
}

EExitCode CException::GetExitCode() const
{
	return EXIT_CODE_UNKNOWN_ERROR;
}

void CException::Output(ostream &Stream) const
{
	Stream << Position.Line << ", " << Position.Column << ": error: " << Message << endl;
}

/******************************************************************************
 * CScannerException
 ******************************************************************************/

CScannerException::CScannerException(const string &AMessage, const CPosition &APosition) : CException(AMessage, APosition)
{
}

EExitCode CScannerException::GetExitCode() const
{
	return EXIT_CODE_SCANNER_ERROR;
}

/******************************************************************************
 * CParserException
 ******************************************************************************/

CParserException::CParserException(const string &AMessage, const CPosition &APosition) : CException(AMessage, APosition)
{
}

EExitCode CParserException::GetExitCode() const
{
	return EXIT_CODE_PARSER_ERROR;
}

/******************************************************************************
 * CFatalException
 ******************************************************************************/

CFatalException::CFatalException(EExitCode AExitCode, const string &AMessage /*= ""*/) : ExitCode(AExitCode), Message(AMessage)
{
}

EExitCode CFatalException::GetExitCode() const
{
	return ExitCode;
}

string CFatalException::GetMessage() const
{
	return Message;
}

/******************************************************************************
 * CStatementVisitor
 ******************************************************************************/

CStatementVisitor::~CStatementVisitor()
{
}

void CStatementVisitor::TryVisit(CStatement *AStmt)
{
	if (AStmt) {
		AStmt->Accept(*this);
	}
}

/******************************************************************************
 * CharTraits
 ******************************************************************************/

namespace CharTraits
{
	bool IsWhitespace(char c)
	{
		return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
	}

	bool IsDigit(char c)
	{
		return (c >= '0' && c <= '9');
	}

	bool IsHexDigit(char c)
	{
		return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
	}

	bool IsOctDigit(char c)
	{
		return (c >= '0' && c <= '7');
	}

	bool IsValidIdentifierChar(char c, bool first /*= false*/)
	{
		return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
			(!first && ((c >= '0' && c <= '9'))));
	}

	bool IsOperationChar(char c)
	{
		return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '<' || c == '>' ||
			c == '!' || c == '^' || c == '~' || c == '&' || c == '|' || c == '?' || c == '.');
	}
};

/******************************************************************************
 * TokenTraits
 ******************************************************************************/

namespace TokenTraits
{
	bool IsCompoundAssignment(ETokenType t)
	{
		static const ETokenType CompoundAssignments[] = {
			TOKEN_TYPE_OPERATION_PLUS_ASSIGN,
			TOKEN_TYPE_OPERATION_MINUS_ASSIGN,
			TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN,
			TOKEN_TYPE_OPERATION_SLASH_ASSIGN,
			TOKEN_TYPE_OPERATION_PERCENT_ASSIGN,
			TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN,
			TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN,
			TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN,
			TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN,
			TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; CompoundAssignments[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == CompoundAssignments[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsAssignment(ETokenType t)
	{
		static const ETokenType Assignments[] = {
			TOKEN_TYPE_OPERATION_ASSIGN,
			TOKEN_TYPE_OPERATION_PLUS_ASSIGN,
			TOKEN_TYPE_OPERATION_MINUS_ASSIGN,
			TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN,
			TOKEN_TYPE_OPERATION_SLASH_ASSIGN,
			TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN,
			TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN,
			TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN,
			TOKEN_TYPE_OPERATION_PERCENT_ASSIGN,
			TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN,
			TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; Assignments[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == Assignments[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsRelational(ETokenType t)
	{
		static const ETokenType RelationalOps[] = {
			TOKEN_TYPE_OPERATION_LESS_THAN,
			TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL,
			TOKEN_TYPE_OPERATION_GREATER_THAN,
			TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; RelationalOps[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == RelationalOps[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsUnaryOp(ETokenType t)
	{
		static const ETokenType UnaryOps[] = {
			TOKEN_TYPE_OPERATION_PLUS,
			TOKEN_TYPE_OPERATION_MINUS,
			TOKEN_TYPE_OPERATION_BITWISE_NOT,
			TOKEN_TYPE_OPERATION_LOGIC_NOT,
			TOKEN_TYPE_OPERATION_AMPERSAND,
			TOKEN_TYPE_OPERATION_ASTERISK,
			TOKEN_TYPE_OPERATION_INCREMENT,
			TOKEN_TYPE_OPERATION_DECREMENT,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; UnaryOps[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == UnaryOps[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsPostfix(ETokenType t)
	{
		static const ETokenType PostfixOps[] = {
			TOKEN_TYPE_LEFT_SQUARE_BRACKET,
			TOKEN_TYPE_LEFT_PARENTHESIS,
			TOKEN_TYPE_OPERATION_DOT,
			TOKEN_TYPE_OPERATION_INDIRECT_ACCESS,
			TOKEN_TYPE_OPERATION_INCREMENT,
			TOKEN_TYPE_OPERATION_DECREMENT,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; PostfixOps[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == PostfixOps[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsComparisonOperation(ETokenType t)
	{
		static const ETokenType ComparisonOps[] = {
			TOKEN_TYPE_OPERATION_EQUAL,
			TOKEN_TYPE_OPERATION_NOT_EQUAL,
			TOKEN_TYPE_OPERATION_LESS_THAN,
			TOKEN_TYPE_OPERATION_GREATER_THAN,
			TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL,
			TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; ComparisonOps[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == ComparisonOps[i]) {
				return true;
			}
		}

		return false;
	}

	bool IsTrivialOperation(ETokenType t)
	{
		static const ETokenType TrivialOps[] = {
			TOKEN_TYPE_OPERATION_PLUS,
			TOKEN_TYPE_OPERATION_MINUS,
			TOKEN_TYPE_OPERATION_ASTERISK,
			TOKEN_TYPE_OPERATION_AMPERSAND,
			TOKEN_TYPE_OPERATION_BITWISE_OR,
			TOKEN_TYPE_OPERATION_BITWISE_XOR,
			TOKEN_TYPE_INVALID
			};

		for (int i = 0; TrivialOps[i] != TOKEN_TYPE_INVALID; i++) {
			if (t == TrivialOps[i]) {
				return true;
			}
		}

		return false;
	}
};

/******************************************************************************
 * KeywordTraits
 ******************************************************************************/

namespace KeywordTraits
{
	bool IsKeyword(const string &s)
	{
		return (s == "break" || s == "case" || s == "const" || s == "continue" || s == "default" || s == "do" ||
			s == "else" || s == "for" || s == "goto" || s == "if" || s == "return" || s == "sizeof" ||
			s == "struct" || s == "switch" || s == "typedef" || s == "while");
	}

	bool IsTypeKeyword(const string &s)
	{
		return (s == "const"|| s == "struct" || s == "typedef");
	}
};
