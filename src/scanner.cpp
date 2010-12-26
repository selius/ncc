#include "scanner.h"

#include <sstream>

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
	case TOKEN_TYPE_OPERATION_PERCENT:
		result = "OPERATION_PERCENT";
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
	case TOKEN_TYPE_OPERATION_PERCENT_ASSIGN:
		result = "OPERATION_PERCENT_ASSIGN";
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

	case TOKEN_TYPE_OPERATION_AMPERSAND:
		result = "OPERATION_AMPERSAND";
		break;

	case TOKEN_TYPE_OPERATION_BITWISE_OR:
		result = "OPERATION_BITWISE_OR";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_NOT:
		result = "OPERATION_BITWISE_NOT";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_XOR:
		result = "OPERATION_BITWISE_XOR";
		break;

	case TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN:
		result = "OPERATION_AMPERSAND_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN:
		result = "OPERATION_BITWISE_OR_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN:
		result = "OPERATION_BITWISE_NOT_ASSIGN";
		break;
	case TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN:
		result = "OPERATION_BITWISE_XOR_ASSIGN";
		break;

	case TOKEN_TYPE_OPERATION_SHIFT_LEFT:
		result = "OPERATION_SHIFT_LEFT";
		break;
	case TOKEN_TYPE_OPERATION_SHIFT_RIGHT:
		result = "OPERATION_SHIFT_RIGHT";
		break;

	case TOKEN_TYPE_OPERATION_DOT:
		result = "OPERATION_DOT";
		break;
	case TOKEN_TYPE_OPERATION_INDIRECT_ACCESS:
		result = "OPERATION_INDIRECT_ACCESS";
		break;

	case TOKEN_TYPE_OPERATION_INCREMENT:
		result = "OPERATION_INCREMENT";
		break;
	case TOKEN_TYPE_OPERATION_DECREMENT:
		result = "OPERATION_DECREMENT";
		break;

	case TOKEN_TYPE_OPERATION_CONDITIONAL:
		result = "OPERATION_CONDITIONAL";
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

bool CTraits::IsDigit(char c)
{
	return (c >= '0' && c <= '9');
}

bool CTraits::IsHexDigit(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

bool CTraits::IsOctDigit(char c)
{
	return (c >= '0' && c <= '7');
}

bool CTraits::IsValidIdentifierSymbol(char c, bool first /*= false*/)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
		(!first && ((c >= '0' && c <= '9'))));
}

bool CTraits::IsOperationSymbol(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '<' || c == '>' ||
		c == '!' || c == '^' || c == '~' || c == '&' || c == '|' || c == '?' || c == '.');
}

bool CTraits::IsKeyword(const string &s)
{
	// we need something like set<string> Keywords, but there is no suitable way to initialize it now..
	return (s == "auto" || s == "break" || s == "case" || s == "const" || s == "continue" || s == "default" ||
		s == "do" || s == "else" || s == "enum" || s == "extern" || s == "for" || s == "goto" || s == "if" ||
		s == "register" || s == "return" || s == "static" || s == "struct" || s == "switch" || s == "typedef" ||
		s == "union" || s == "volatile" || s == "while");

	// do internal data types count as keywords? or is it just built-in identifiers?..
	/*
	auto 	 break 	 case 	char 	const 	continue 	default 	do
	double 	else 	enum 	extern 	float 	for 	goto 	if
	int 	long 	register 	return 	short 	signed 	sizeof 	static
	struct 	switch 	typedef 	union 	unsigned 	void 	volatile 	while
	*/
}

CScanner::CScanner(istream &AInputStream) : InputStream(AInputStream), CurrentPosition(1, 1), ErrorState(false)
{
	if (!InputStream.good()) {
		Error(CurrentPosition, "can't read from input stream");
	}

	InputStream >> noskipws;
}

CToken& CScanner::GetToken()
{
	return LastToken;
}

CToken& CScanner::Next()
{
	CToken NewToken;

	SkipWhitespaceAndComments();

	if (!InputStream.good()) {
		NewToken.Position = CurrentPosition;
		NewToken.Type = CToken::TOKEN_TYPE_EOF;
		return (LastToken = NewToken);
	}

	char symbol = InputStream.peek();

	if (CTraits::IsValidIdentifierSymbol(symbol, true)) {
		NewToken = ScanIdentifier();
	} else if (TryScanNumericalConstant()) {
		NewToken = ScanNumericalConstant();
	} else if (CTraits::IsOperationSymbol(symbol)) {
		NewToken = ScanOperation();
	} else if (symbol == '"') {
		NewToken = ScanStringConstant();
	} else if (symbol == '\'') {
		NewToken = ScanSymbolConstant();
	} else {
		NewToken = ScanSingleSymbol();
	}

	return (LastToken = NewToken);
}

bool CScanner::IsError() const
{
	return ErrorState;
}

CToken CScanner::ScanIdentifier()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	while (InputStream.good() && CTraits::IsValidIdentifierSymbol(InputStream.peek())) {
		NewToken.Value += AdvanceOneSymbol();
	}

	NewToken.Type = (CTraits::IsKeyword(NewToken.Value) ? CToken::TOKEN_TYPE_KEYWORD : CToken::TOKEN_TYPE_IDENTIFIER);

	return NewToken;
}

CToken CScanner::ScanOperation()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	char fs = AdvanceOneSymbol();
	char ss = InputStream.peek();

	bool TwoSymbolOperation = true;

	switch (fs) {
	case '+':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_PLUS_ASSIGN;
			break;
		case '+':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_INCREMENT;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_PLUS;
			TwoSymbolOperation = false;
		}
		break;
	case '-':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_MINUS_ASSIGN;
			break;
		case '-':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_DECREMENT;
			break;
		case '>':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_INDIRECT_ACCESS;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_MINUS;
			TwoSymbolOperation = false;
		}
		break;
	case '*':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_ASTERISK;
			TwoSymbolOperation = false;
		}
		break;
	case '/':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_SLASH_ASSIGN;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_SLASH;
			TwoSymbolOperation = false;
		}
		break;
	case '%':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_PERCENT_ASSIGN;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_PERCENT;
			TwoSymbolOperation = false;
		}
		break;
	case '~':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_NOT;
			TwoSymbolOperation = false;
		}
		break;
	case '^':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_XOR;
			TwoSymbolOperation = false;
		}
		break;
	case '&':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN;
			break;
		case '&':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_LOGIC_AND;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_AMPERSAND;
			TwoSymbolOperation = false;
		}
		break;
	case '|':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN;
			break;
		case '|':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_LOGIC_OR;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_BITWISE_OR;
			TwoSymbolOperation = false;
		}
		break;
	case '!':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_NOT_EQUAL;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_LOGIC_NOT;
			TwoSymbolOperation = false;
		}
		break;
	case '=':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_EQUAL;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_ASSIGN;
			TwoSymbolOperation = false;
		}
		break;
	case '<':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL;
			break;
		case '<':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_SHIFT_LEFT;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_LESS_THAN;
			TwoSymbolOperation = false;
		}
		break;
	case '>':
		switch (ss) {
		case '=':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL;
			break;
		case '>':
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_SHIFT_RIGHT;
			break;
		default:
			NewToken.Type = CToken::TOKEN_TYPE_OPERATION_GREATER_THAN;
			TwoSymbolOperation = false;
		}
		break;
	case '.':
		NewToken.Type = CToken::TOKEN_TYPE_OPERATION_DOT;
		TwoSymbolOperation = false;
		break;
	case '?':
		NewToken.Type = CToken::TOKEN_TYPE_OPERATION_CONDITIONAL;
		TwoSymbolOperation = false;
		break;
	}

	NewToken.Value += fs;

	if (TwoSymbolOperation) {
		AdvanceOneSymbol();
		NewToken.Value += ss;
	}

	return NewToken;
}

CToken CScanner::ScanSingleSymbol()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	char symbol = InputStream.peek();

	switch (symbol) {
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
		return Error(CurrentPosition, string("invalid symbol '") + symbol + "' encountered");
	}

	NewToken.Value = symbol;

	AdvanceOneSymbol();

	return NewToken;
}

CToken CScanner::ScanStringConstant()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;
	NewToken.Type = CToken::TOKEN_TYPE_CONSTANT_STRING;

	AdvanceOneSymbol();

	char symbol;

	while (InputStream.good() && ((symbol = AdvanceOneSymbol()) != '\n')) {
		if (symbol == '\\') {
			NewToken.Value += ProcessEscapeSequence();
		} else if (symbol == '"') {
			return NewToken;
		} else {
			NewToken.Value += symbol;
		}
	}

	return Error(NewToken.Position, "unterminated string constant");
}

CToken CScanner::ScanSymbolConstant()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;
	NewToken.Type = CToken::TOKEN_TYPE_CONSTANT_SYMBOL;

	AdvanceOneSymbol();

	if (!InputStream.good()) {
		return Error(NewToken.Position, "unterminated symbol constant");
	}

	char symbol = InputStream.peek();

	if (symbol == '\\') {
		AdvanceOneSymbol();
		NewToken.Value = ProcessEscapeSequence();
	} else {
		NewToken.Value = symbol;
		AdvanceOneSymbol();
	}

	if (!InputStream.good() || AdvanceOneSymbol() != '\'') {
		return Error(NewToken.Position, "unterminated symbol constant");
	}

	return NewToken;
}

CToken CScanner::ScanNumericalConstant()
{
	CToken NewToken;
	NewToken.Position = CurrentPosition;

	string IntegerPart = "0";
	string FractionalPart;
	string ExponentPart;
	string SuffixPart;

	if (InputStream.peek() != '.') {
		IntegerPart = ScanIntegerPart();
	}

	FractionalPart = ScanFractionalPart();

	// NOTE: error state check was here..

	ExponentPart = ScanExponentPart();

	bool FloatConstant = !(FractionalPart.empty() && ExponentPart.empty());

	if (FloatConstant) {
		SuffixPart = ScanFloatSuffix();
	} else {
		SuffixPart = ScanIntegerSuffix();
	}

	if (ErrorState) {
		return CToken(CToken::TOKEN_TYPE_INVALID, "", NewToken.Position);
	}

	NewToken.Type = FloatConstant ? CToken::TOKEN_TYPE_CONSTANT_FLOAT : CToken::TOKEN_TYPE_CONSTANT_INTEGER;
	NewToken.Value = IntegerPart + FractionalPart + ExponentPart + SuffixPart;

	return NewToken;
}

bool CScanner::TryScanNumericalConstant()
{
	bool result = false;
	char symbol = InputStream.peek();

	if (CTraits::IsDigit(symbol)) {
		result = true;
	} else if (symbol == '.') {
		InputStream.ignore();
		if (CTraits::IsDigit(InputStream.peek())) {
			result = true;
		}
		InputStream.putback(symbol);
	}

	return result;
}

string CScanner::ScanHexadecimalInteger()
{
	string result;
	char c;

	while (InputStream.good() && CTraits::IsHexDigit(c = InputStream.peek())) {
		result += c;
		AdvanceOneSymbol();
	}

	return result;
}

string CScanner::ScanOctalInteger()
{
	string result;
	char c;

	while (InputStream.good() && CTraits::IsOctDigit(c = InputStream.peek())) {
		result += c;
		AdvanceOneSymbol();
	}

	return result;
}

string CScanner::ScanIntegerPart()
{
	string result;

	char c = AdvanceOneSymbol();
	result += c;

	if (c == '0') {
		c = InputStream.peek();
		if (c == 'x' || c == 'X') {
			result += 'x';
			AdvanceOneSymbol();
			if (!CTraits::IsHexDigit(InputStream.peek())) {
				Error(CurrentPosition, "invalid hexadecimal constant");
				return "";
			}
			result += ScanHexadecimalInteger();
			if (InputStream.peek() == '.') {
				Error(CurrentPosition, "invalid float constant");
				return "";
			}
		} else if (CTraits::IsOctDigit(c)) {
			result += ScanOctalInteger();
			if (InputStream.peek() == '.') {
				Error(CurrentPosition, "invalid float constant");
				return "";
			}
		} else if (CTraits::IsDigit(c)) {
			Error(CurrentPosition, "invalid octal constant");
			return "";
		}

	} else {
		while (InputStream.good() && CTraits::IsDigit(c = InputStream.peek())) {
			result += c;
			AdvanceOneSymbol();
		}
	}

	return result;
}

string CScanner::ScanFractionalPart()
{
	if (InputStream.peek() != '.') {
		return "";
	}

	string result;
	char c;

	result += AdvanceOneSymbol();

	while (InputStream.good() && CTraits::IsDigit(c = InputStream.peek())) {
		result += c;
		AdvanceOneSymbol();
	}

	return result;
}

string CScanner::ScanExponentPart()
{
	char c;
	c = InputStream.peek();
	if (c != 'e' && c != 'E') {
		return "";
	}

	string result;
	result += AdvanceOneSymbol();

	c = InputStream.peek();

	if (c == '+' || c == '-') {
		result += c;
		AdvanceOneSymbol();
	}

	while (InputStream.good() && CTraits::IsDigit(c = InputStream.peek())) {
		result += c;
		AdvanceOneSymbol();
	}

	return result;
}

string CScanner::ScanFloatSuffix()
{
	char c = InputStream.peek();

	if (c == 'f' || c == 'F' || c == 'l' || c == 'L') {
		AdvanceOneSymbol();
		if (!CTraits::IsValidIdentifierSymbol(InputStream.peek())) {
			return string(1, c);
		} else {
			Error(CurrentPosition, "invalid suffix on float constant");
		}
	} else if (CTraits::IsValidIdentifierSymbol(c)) {
		Error(CurrentPosition, "invalid suffix on float constant");
	}

	return "";
}

string CScanner::ScanIntegerSuffix()
{
	string result;
	char fc = InputStream.peek();

	if (fc == 'u' || fc == 'U' || fc == 'l' || fc == 'L') {
		result += fc;
		AdvanceOneSymbol();
	} else if (CTraits::IsValidIdentifierSymbol(fc)) {
		Error(CurrentPosition, "invalid suffix on integer constant");
		return "";
	}

	char sc = InputStream.peek();

	if (((sc == 'u' || sc == 'U') && (fc != 'u' && fc != 'U')) || ((sc == 'l' || sc == 'L') && (fc != 'l' && fc != 'L'))) {
		result += fc;
		AdvanceOneSymbol();
	} else if (CTraits::IsValidIdentifierSymbol(sc)) {
		Error(CurrentPosition, "invalid suffix on integer constant");
		return "";
	}

	if (CTraits::IsValidIdentifierSymbol(InputStream.peek())) {
		Error(CurrentPosition, "invalid suffix on integer constant");
		return "";
	}

	return result;
}

char CScanner::ProcessEscapeSequence()
{
	// TODO: add numerical (octal and hexadecimal) values of chars escape sequences support
	if (!InputStream.good()) {
		return 0;
	}

	char symbol = AdvanceOneSymbol();
	char result;

	switch (symbol) {
	case '\'':
	case '"':
	case '\\':
	case '?':	// used for trigraphs only..
		result = symbol;
		break;
	case 'a':
		result = '\a';
		break;
	case 'b':
		result = '\b';
		break;
	case 'f':
		result = '\f';
		break;
	case 'n':
		result = '\n';
		break;
	case 'r':
		result = '\r';
		break;
	case 't':
		result = '\t';
		break;
	case 'v':
		result = '\v';
		break;
	default:
		Error(CurrentPosition, "invalid escape sequence");
		return 0;
	}

	return result;
}

bool CScanner::SkipComment()
{
	bool EndMatchState = false;
	bool end = false;
	char symbol;

	if (!InputStream.good()) {
		return false;
	}

	symbol = InputStream.peek();
	if (symbol == '/') {
		InputStream.ignore();
		symbol = InputStream.peek();
		InputStream.putback('/');
		if (symbol != '*') {
			return false;
		}
	} else {
		return false;
	}

	CPosition start = CurrentPosition;

	while (InputStream.good() && !end) {
		symbol = AdvanceOneSymbol();

		if (symbol == '*') {
			EndMatchState = true;
		} else if (EndMatchState && symbol == '/') {
			end = true;
		} else {
			EndMatchState = false;
		}
	}

	if (!InputStream.good()) {
		Error(start, "unterminated comment");
	}

	return true;
}

bool CScanner::SkipWhitespace()
{
	if (!InputStream.good() || !CTraits::IsWhitespace(InputStream.peek())) {
		return false;
	}

	while (InputStream.good() && CTraits::IsWhitespace(InputStream.peek())) {
		AdvanceOneSymbol();
	}

	return true;
}

void CScanner::SkipWhitespaceAndComments()
{
	while (InputStream.good() && (SkipWhitespace() || SkipComment()));
}

char CScanner::AdvanceOneSymbol()
{
	char symbol = InputStream.get();

	if (symbol == '\n') {
		CurrentPosition.Line++;
		CurrentPosition.Column = 1;
	} else {
		CurrentPosition.Column++;
	}

	return symbol;
}

CToken CScanner::Error(const CPosition &Position, const string &Message)
{
	CLogger::Instance()->Log(CLogger::LOG_TYPE_ERROR, Position, Message);
	ErrorState = true;
	return CToken(CToken::TOKEN_TYPE_INVALID, "", Position);
}
