#include "scanner.h"

#include "logger.h"

CToken::CToken() : Type(TOKEN_TYPE_INVALID), Position(0, 0)
{
}

CToken::CToken(ETokenType AType, const string &AValue, const CPosition &APosition) : Type(AType), Value(AValue), Position(APosition)
{
}

CToken::ETokenType CToken::GetType() const
{
	return Type;
}

string CToken::GetStringifiedType() const
{
	string result;

	switch (Type) {
	case TOKEN_TYPE_INVALID:
		result = "INVALID";
		break;
	case TOKEN_TYPE_IDENTIFIER:
		result = "IDENTIFIER";
		break;
	case TOKEN_TYPE_KEYWORD:
		result = "KEYWORD";
		break;
	case TOKEN_TYPE_BLOCK_START:
		result = "BLOCK_START";
		break;
	case TOKEN_TYPE_BLOCK_END:
		result = "BLOCK_END";
		break;
	case TOKEN_TYPE_LEFT_PARENTHESIS:
		result = "LEFT_PARENTHESIS";
		break;
	case TOKEN_TYPE_RIGHT_PARENTHESIS:
		result = "RIGHT_PARENTHESIS";
		break;
	case TOKEN_TYPE_LEFT_SQUARE_BRACKET:
		result = "LEFT_SQUARE_BRACKET";
		break;
	case TOKEN_TYPE_RIGHT_SQUARE_BRACKET:
		result = "RIGHT_SQUARE_BRACKET";
		break;
	case TOKEN_TYPE_CONSTANT_INTEGER:
		result = "CONSTANT_INTEGER";
		break;
	case TOKEN_TYPE_CONSTANT_FLOAT:
		result = "CONSTANT_FLOAT";
		break;
	case TOKEN_TYPE_CONSTANT_SYMBOL:
		result = "CONSTANT_SYMBOL";
		break;
	case TOKEN_TYPE_CONSTANT_STRING:
		result = "CONSTANT_STRING";
		break;
	case TOKEN_TYPE_OPERATION_PLUS:
		result = "OPERATION_PLUS";
		break;
	case TOKEN_TYPE_OPERATION_MINUS:
		result = "OPERATION_MINUS";
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK:
		result = "OPERATION_ASTERISK";
		break;
	case TOKEN_TYPE_OPERATION_SLASH:
		result = "OPERATION_SLASH";
		break;
	case TOKEN_TYPE_OPERATION_BACKSLASH:
		result = "OPERATION_BACKSLASH";
		break;
	case TOKEN_TYPE_OPERATION_PERCENT:
		result = "OPERATION_PERCENT";
		break;
	case TOKEN_TYPE_OPERATION_AND:
		result = "OPERATION_AND";
		break;
	case TOKEN_TYPE_OPERATION_ASSIGN:
		result = "OPERATION_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_PLUS_ASSIGN:
		result = "OPERATION_PLUS_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_MINUS_ASSIGN:
		result = "OPERATION_MINUS_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN:
		result = "OPERATION_ASTERISK_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_SLASH_ASSIGN:
		result = "OPERATION_SLASH_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_EQUAL:
		result = "OPERATION_EQUAL";
		break;
	case TOKEN_TYPE_OPERATION_NOT_EQUAL:
		result = "OPERATION_NOT_EQUAL";
		break;
	case TOKEN_TYPE_OPERATION_LESS_THAN:
		result = "OPERATION_LESS_THAN";
		break;
	case TOKEN_TYPE_OPERATION_GREATER_THAN:
		result = "OPERATION_GREATER_THAN";
		break;
	case TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL:
		result = "OPERATION_LESS_THAN_OR_EQUAL";
		break;
	case TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL:
		result = "OPERATION_GREATER_THAN_OR_EQUAL";
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_AND:
		result = "OPERATION_LOGIC_AND";
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_OR:
		result = "OPERATION_LOGIC_OR";
		break;
	case TOKEN_TYPE_OPERATION_LOGIC_NOT:
		result = "OPERATION_LOGIC_NOT";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_OR:
		result = "OPERATION_BITWISE_OR";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_NOT:
		result = "OPERATION_BITWISE_NOT";
		break;
	case TOKEN_TYPE_OPERATION_DOT:
		result = "OPERATION_DOT";
		break;
	case TOKEN_TYPE_OPERATION_INDIRECT_ACCESS:
		result = "OPERATION_INDIRECT_ACCESS";
		break;
	case TOKEN_TYPE_SEPARATOR_COMMA:
		result = "SEPARATOR_COMMA";
		break;
	case TOKEN_TYPE_SEPARATOR_SEMICOLON:
		result = "SEPARATOR_SEMICOLON";
		break;
	case TOKEN_TYPE_SEPARATOR_COLON:
		result = "SEPARATOR_COLON";
		break;
	case TOKEN_TYPE_EOF:
		result = "EOF";
		break;
	}

	return result;
}

string CToken::GetValue() const
{
	return Value;
}

CPosition CToken::GetPosition() const
{
	return Position;
}

bool CTraits::IsWhitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

bool CTraits::IsValidIdentifierSymbol(char c, bool first /*= false*/)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
		(!first && ((c >= '0' && c <= '9'))));
}

bool CTraits::IsOperationSymbol(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '<' || c == '>' ||
		c == '!' || c == '^' || c == '~' || c == '\\' || c == '&' || c == '|');
}

bool CTraits::IsKeyword(const string &s)
{
	// we need something like set<string> Keywords, but there is no suitable way to initialize it now..
	return (s == "return" || s == "if" || s == "else" || s == "for" || s == "while" || s == "do" || s == "switch" || s == "case");
}

CScanner::CScanner(istream &AInputStream) : InputStream(AInputStream), CurrentPosition(1, 1), ErrorState(false)
{
	if (!InputStream.good()) {
		CLogger::Instance()->Log(CLogger::LOG_TYPE_ERROR, CurrentPosition, "can't read from input stream");
		ErrorState = true;
	}

	InputStream >> noskipws;
}

CToken& CScanner::GetToken()
{
	return LastToken;
}

CToken& CScanner::Next()
{
	char Symbol;
	CToken NewToken;

	SkipWhitespace();

	NewToken.Position = CurrentPosition;

	if (!InputStream.good()) {
		NewToken.Type = CToken::TOKEN_TYPE_EOF;
		return (LastToken = NewToken);
	}

	Symbol = InputStream.peek();

	if (CTraits::IsValidIdentifierSymbol(Symbol, true)) {
		NewToken = ScanIdentifier();
	} else if (CTraits::IsOperationSymbol(Symbol)) {
		NewToken = ScanOperation();
	} else {
		NewToken = ScanSingleSymbol();
	}

	LastToken = NewToken;

	return LastToken;
}

bool CScanner::IsError() const
{
	return ErrorState;
}

CToken CScanner::ScanIdentifier()
{
	char Symbol;
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	while (InputStream.good() && CTraits::IsValidIdentifierSymbol((Symbol = InputStream.peek()))) {
		NewToken.Value += Symbol;
		InputStream.ignore();
		CurrentPosition.Column++;
	}

	NewToken.Type = (CTraits::IsKeyword(NewToken.Value) ? CToken::TOKEN_TYPE_KEYWORD : CToken::TOKEN_TYPE_IDENTIFIER);

	return NewToken;
}

CToken CScanner::ScanOperation()
{
	CToken NewToken(CToken::TOKEN_TYPE_OPERATION_PLUS, "some operation", CurrentPosition);
	InputStream.ignore();
	CurrentPosition.Column++;
	return NewToken;
}

CToken CScanner::ScanSingleSymbol()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	char Symbol = InputStream.peek();

	switch (Symbol) {
	case '{':
		NewToken.Type = CToken::TOKEN_TYPE_BLOCK_START;
		break;
	case '}':
		NewToken.Type = CToken::TOKEN_TYPE_BLOCK_END;
		break;
	case '(':
		NewToken.Type = CToken::TOKEN_TYPE_LEFT_PARENTHESIS;
		break;
	case ')':
		NewToken.Type = CToken::TOKEN_TYPE_RIGHT_PARENTHESIS;
		break;
	case '[':
		NewToken.Type = CToken::TOKEN_TYPE_LEFT_SQUARE_BRACKET;
		break;
	case ']':
		NewToken.Type = CToken::TOKEN_TYPE_RIGHT_SQUARE_BRACKET;
		break;
	case ';':
		NewToken.Type = CToken::TOKEN_TYPE_SEPARATOR_SEMICOLON;
		break;
	case ',':
		NewToken.Type = CToken::TOKEN_TYPE_SEPARATOR_COMMA;
		break;
	case ':':
		NewToken.Type = CToken::TOKEN_TYPE_SEPARATOR_COLON;
		break;
	default:
		// log it and return invalid token without value..
		NewToken.Type = CToken::TOKEN_TYPE_INVALID;
	}

	NewToken.Value = Symbol;

	InputStream.ignore();
	CurrentPosition.Column++;

	return NewToken;
}

void CScanner::ScanComment()
{

}

void CScanner::SkipWhitespace()
{
	char Symbol;
	while (InputStream.good() && CTraits::IsWhitespace((Symbol = InputStream.peek()))) {
		if (Symbol == '\n') {
			CurrentPosition.Line++;
			CurrentPosition.Column = 1;
		} else {
			CurrentPosition.Column++;
		}

		InputStream.ignore();
	}
}
