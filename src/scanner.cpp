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

	if (!InputStream.good()) {
		NewToken.Position = CurrentPosition;
		NewToken.Type = CToken::TOKEN_TYPE_EOF;
	}

	while (InputStream.good()) {
		NewToken.Position = CurrentPosition;
		InputStream >> Symbol;

		CurrentPosition.Column++;

		if (Symbol == '=') {
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_ASSIGN;
			NewToken.Value = "=";
			break;
		} else if (Symbol == '{') {
			NewToken.Type = CToken::TOKEN_TYPE_BLOCK_START;
			NewToken.Value = "{";
			break;
		} else if (Symbol == '}') {
			NewToken.Type = CToken::TOKEN_TYPE_BLOCK_END;
			NewToken.Value = "}";
			break;
		} else if (Symbol == '\n') {
			CurrentPosition.Line++;
			CurrentPosition.Column = 1;
		} else {
			NewToken.Type = CToken::TOKEN_TYPE_INVALID;
			break;
		}
	}

	LastToken = NewToken;

	return LastToken;
}

bool CScanner::IsError() const
{
	return ErrorState;
}
