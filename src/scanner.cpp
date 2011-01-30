#include "scanner.h"

/******************************************************************************
 * CToken
 ******************************************************************************/

CToken::CToken(ETokenType AType, const string &AText, const CPosition &APosition) : Type(AType), Text(AText), Position(APosition)
{
}

CToken::~CToken()
{
}

ETokenType CToken::GetType() const
{
	return Type;
}

string CToken::GetStringifiedType() const
{
	return CScanner::TokenTypesNames[Type];
}

string CToken::GetText() const
{
	return Text;
}

CPosition CToken::GetPosition() const
{
	return Position;
}

int CToken::GetIntegerValue() const
{
	throw logic_error("CToken can't have integer value");
}

double CToken::GetFloatValue() const
{
	throw logic_error("CToken can't have float value");
}

char CToken::GetSymbolValue() const
{
	throw logic_error("CToken can't have symbol value");
}

/******************************************************************************
 * CIntegerConstToken
 ******************************************************************************/

CIntegerConstToken::CIntegerConstToken(const string &AText, const CPosition &APosition) : CToken(TOKEN_TYPE_CONSTANT_INTEGER, AText, APosition)
{
	stringstream ss;
	ss.str(Text);
	ss >> Value;
}

int CIntegerConstToken::GetIntegerValue() const
{
	return Value;
}

/******************************************************************************
 * CFloatConstToken
 ******************************************************************************/

CFloatConstToken::CFloatConstToken(const string &AText, const CPosition &APosition) : CToken(TOKEN_TYPE_CONSTANT_FLOAT, AText, APosition)
{
	stringstream ss;
	ss.str(Text);
	ss >> Value;
}

double CFloatConstToken::GetFloatValue() const
{
	return Value;
}

/******************************************************************************
 * CSymbolConstToken
 ******************************************************************************/

CSymbolConstToken::CSymbolConstToken(const string &AText, const CPosition &APosition) : CToken(TOKEN_TYPE_CONSTANT_SYMBOL, AText, APosition)
{
	Value = AText[0];
}

char CSymbolConstToken::GetSymbolValue() const
{
	return Value;
}

/******************************************************************************
 * CTraits
 ******************************************************************************/

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

/******************************************************************************
 * CScanner
 ******************************************************************************/

map<ETokenType, string> CScanner::TokenTypesNames;

CScanner::CScanner(istream &AInputStream) : InputStream(AInputStream), LastToken(NULL), CurrentPosition(1, 1)
{
	if (!InputStream.good()) {
		throw CException("can't read from input stream", CurrentPosition);
	}

	InputStream >> noskipws;

	if (TokenTypesNames.empty()) {
		TokenTypesNames[TOKEN_TYPE_INVALID] = "INVALID";
		TokenTypesNames[TOKEN_TYPE_IDENTIFIER] = "IDENTIFIER";
		TokenTypesNames[TOKEN_TYPE_KEYWORD] = "KEYWORD";
		TokenTypesNames[TOKEN_TYPE_BLOCK_START] = "BLOCK_START";
		TokenTypesNames[TOKEN_TYPE_BLOCK_END] = "BLOCK_END";
		TokenTypesNames[TOKEN_TYPE_LEFT_PARENTHESIS] = "LEFT_PARENTHESIS";
		TokenTypesNames[TOKEN_TYPE_RIGHT_PARENTHESIS] = "RIGHT_PARENTHESIS";
		TokenTypesNames[TOKEN_TYPE_LEFT_SQUARE_BRACKET] = "LEFT_SQUARE_BRACKET";
		TokenTypesNames[TOKEN_TYPE_RIGHT_SQUARE_BRACKET] = "RIGHT_SQUARE_BRACKET";
		TokenTypesNames[TOKEN_TYPE_CONSTANT_INTEGER] = "CONSTANT_INTEGER";
		TokenTypesNames[TOKEN_TYPE_CONSTANT_FLOAT] = "CONSTANT_FLOAT";
		TokenTypesNames[TOKEN_TYPE_CONSTANT_SYMBOL] = "CONSTANT_SYMBOL";
		TokenTypesNames[TOKEN_TYPE_CONSTANT_STRING] = "CONSTANT_STRING";
		TokenTypesNames[TOKEN_TYPE_OPERATION_PLUS] = "OPERATION_PLUS";
		TokenTypesNames[TOKEN_TYPE_OPERATION_MINUS] = "OPERATION_MINUS";
		TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK] = "OPERATION_ASTERISK";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SLASH] = "OPERATION_SLASH";
		TokenTypesNames[TOKEN_TYPE_OPERATION_PERCENT] = "OPERATION_PERCENT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_ASSIGN] = "OPERATION_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_PLUS_ASSIGN] = "OPERATION_PLUS_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_MINUS_ASSIGN] = "OPERATION_MINUS_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN] = "OPERATION_ASTERISK_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SLASH_ASSIGN] = "OPERATION_SLASH_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_PERCENT_ASSIGN] = "OPERATION_PERCENT_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_EQUAL] = "OPERATION_EQUAL";
		TokenTypesNames[TOKEN_TYPE_OPERATION_NOT_EQUAL] = "OPERATION_NOT_EQUAL";
		TokenTypesNames[TOKEN_TYPE_OPERATION_LESS_THAN] = "OPERATION_LESS_THAN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_GREATER_THAN] = "OPERATION_GREATER_THAN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL] = "OPERATION_LESS_THAN_OR_EQUAL";
		TokenTypesNames[TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL] = "OPERATION_GREATER_THAN_OR_EQUAL";
		TokenTypesNames[TOKEN_TYPE_OPERATION_LOGIC_AND] = "OPERATION_LOGIC_AND";
		TokenTypesNames[TOKEN_TYPE_OPERATION_LOGIC_OR] = "OPERATION_LOGIC_OR";
		TokenTypesNames[TOKEN_TYPE_OPERATION_LOGIC_NOT] = "OPERATION_LOGIC_NOT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_AMPERSAND] = "OPERATION_AMPERSAND";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_OR] = "OPERATION_BITWISE_OR";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_NOT] = "OPERATION_BITWISE_NOT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_XOR] = "OPERATION_BITWISE_XOR";
		TokenTypesNames[TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN] = "OPERATION_AMPERSAND_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN] = "OPERATION_BITWISE_OR_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN] = "OPERATION_BITWISE_NOT_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN] = "OPERATION_BITWISE_XOR_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SHIFT_LEFT] = "OPERATION_SHIFT_LEFT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SHIFT_RIGHT] = "OPERATION_SHIFT_RIGHT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN] = "OPERATION_SHIFT_LEFT_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN] = "OPERATION_SHIFT_RIGHT_ASSIGN";
		TokenTypesNames[TOKEN_TYPE_OPERATION_DOT] = "OPERATION_DOT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_INDIRECT_ACCESS] = "OPERATION_INDIRECT_ACCESS";
		TokenTypesNames[TOKEN_TYPE_OPERATION_INCREMENT] = "OPERATION_INCREMENT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_DECREMENT] = "OPERATION_DECREMENT";
		TokenTypesNames[TOKEN_TYPE_OPERATION_CONDITIONAL] = "OPERATION_CONDITIONAL";
		TokenTypesNames[TOKEN_TYPE_SEPARATOR_COMMA] = "SEPARATOR_COMMA";
		TokenTypesNames[TOKEN_TYPE_SEPARATOR_SEMICOLON] = "SEPARATOR_SEMICOLON";
		TokenTypesNames[TOKEN_TYPE_SEPARATOR_COLON] = "SEPARATOR_COLON";
		TokenTypesNames[TOKEN_TYPE_EOF] = "EOF";
	}
}

CScanner::~CScanner()
{
	delete LastToken;
}

const CToken* CScanner::GetToken()
{
	return LastToken;
}

const CToken* CScanner::Next()
{
	delete LastToken;
	LastToken = NULL;

	SkipWhitespaceAndComments();

	if (!InputStream.good()) {
		return (LastToken = new CToken(TOKEN_TYPE_EOF, "", CurrentPosition));
	}

	CToken *NewToken = NULL;
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

CToken* CScanner::ScanIdentifier()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	while (InputStream.good() && CTraits::IsValidIdentifierSymbol(InputStream.peek())) {
		Text += AdvanceOneSymbol();
	}

	return new CToken(CTraits::IsKeyword(Text) ? TOKEN_TYPE_KEYWORD : TOKEN_TYPE_IDENTIFIER, Text, StartPosition);
}

CToken* CScanner::ScanOperation()
{
	CPosition StartPosition = CurrentPosition;
	ETokenType Type;

	char fs = AdvanceOneSymbol();
	char ss = InputStream.get();
	char ts = InputStream.peek();

	InputStream.clear();
	InputStream.putback(ss);

	int OperationLength = 2;

	switch (fs) {
	case '+':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_PLUS_ASSIGN;
			break;
		case '+':
			Type = TOKEN_TYPE_OPERATION_INCREMENT;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_PLUS;
			OperationLength = 1;
		}
		break;
	case '-':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_MINUS_ASSIGN;
			break;
		case '-':
			Type = TOKEN_TYPE_OPERATION_DECREMENT;
			break;
		case '>':
			Type = TOKEN_TYPE_OPERATION_INDIRECT_ACCESS;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_MINUS;
			OperationLength = 1;
		}
		break;
	case '*':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_ASTERISK_ASSIGN;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_ASTERISK;
			OperationLength = 1;
		}
		break;
	case '/':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_SLASH_ASSIGN;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_SLASH;
			OperationLength = 1;
		}
		break;
	case '%':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_PERCENT_ASSIGN;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_PERCENT;
			OperationLength = 1;
		}
		break;
	case '~':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_BITWISE_NOT_ASSIGN;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_BITWISE_NOT;
			OperationLength = 1;
		}
		break;
	case '^':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_BITWISE_XOR_ASSIGN;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_BITWISE_XOR;
			OperationLength = 1;
		}
		break;
	case '&':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_AMPERSAND_ASSIGN;
			break;
		case '&':
			Type = TOKEN_TYPE_OPERATION_LOGIC_AND;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_AMPERSAND;
			OperationLength = 1;
		}
		break;
	case '|':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_BITWISE_OR_ASSIGN;
			break;
		case '|':
			Type = TOKEN_TYPE_OPERATION_LOGIC_OR;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_BITWISE_OR;
			OperationLength = 1;
		}
		break;
	case '!':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_NOT_EQUAL;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_LOGIC_NOT;
			OperationLength = 1;
		}
		break;
	case '=':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_EQUAL;
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_ASSIGN;
			OperationLength = 1;
		}
		break;
	case '<':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_LESS_THAN_OR_EQUAL;
			break;
		case '<':
			if (ts == '=') {
				Type = TOKEN_TYPE_OPERATION_SHIFT_LEFT_ASSIGN;
				OperationLength = 3;
			} else {
				Type = TOKEN_TYPE_OPERATION_SHIFT_LEFT;
			}
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_LESS_THAN;
			OperationLength = 1;
		}
		break;
	case '>':
		switch (ss) {
		case '=':
			Type = TOKEN_TYPE_OPERATION_GREATER_THAN_OR_EQUAL;
			break;
		case '>':
			if (ts == '=') {
				Type = TOKEN_TYPE_OPERATION_SHIFT_RIGHT_ASSIGN;
				OperationLength = 3;
			} else {
				Type = TOKEN_TYPE_OPERATION_SHIFT_RIGHT;
			}
			break;
		default:
			Type = TOKEN_TYPE_OPERATION_GREATER_THAN;
			OperationLength = 1;
		}
		break;
	case '.':
		Type = TOKEN_TYPE_OPERATION_DOT;
		OperationLength = 1;
		break;
	case '?':
		Type = TOKEN_TYPE_OPERATION_CONDITIONAL;
		OperationLength = 1;
		break;
	}

	string Text = string(1, fs);

	if (OperationLength > 1) {
		Text += ss;
		AdvanceOneSymbol();
	}

	if (OperationLength > 2) {
		Text += ts;
		AdvanceOneSymbol();
	}

	return new CToken(Type, Text, StartPosition);
}

CToken* CScanner::ScanSingleSymbol()
{
	CPosition StartPosition = CurrentPosition;
	ETokenType Type;

	char symbol = InputStream.peek();

	switch (symbol) {
	case '{':
		Type = TOKEN_TYPE_BLOCK_START;
		break;
	case '}':
		Type = TOKEN_TYPE_BLOCK_END;
		break;
	case '(':
		Type = TOKEN_TYPE_LEFT_PARENTHESIS;
		break;
	case ')':
		Type = TOKEN_TYPE_RIGHT_PARENTHESIS;
		break;
	case '[':
		Type = TOKEN_TYPE_LEFT_SQUARE_BRACKET;
		break;
	case ']':
		Type = TOKEN_TYPE_RIGHT_SQUARE_BRACKET;
		break;
	case ';':
		Type = TOKEN_TYPE_SEPARATOR_SEMICOLON;
		break;
	case ',':
		Type = TOKEN_TYPE_SEPARATOR_COMMA;
		break;
	case ':':
		Type = TOKEN_TYPE_SEPARATOR_COLON;
		break;
	default:
		throw CException(string("invalid symbol '") + symbol + "' encountered", CurrentPosition);
	}

	AdvanceOneSymbol();

	return new CToken(Type, string(1, symbol), StartPosition);
}

CToken* CScanner::ScanStringConstant()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	AdvanceOneSymbol();

	char symbol;

	while (InputStream.good() && ((symbol = AdvanceOneSymbol()) != '\n')) {
		if (symbol == '\\') {
			Text += ProcessEscapeSequence();
		} else if (symbol == '"') {
			return new CToken(TOKEN_TYPE_CONSTANT_STRING, Text, StartPosition);
		} else {
			Text += symbol;
		}
	}

	throw CException("unterminated string constant", StartPosition);
}

CToken* CScanner::ScanSymbolConstant()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	AdvanceOneSymbol();

	if (!InputStream.good()) {
		throw CException("unterminated symbol constant", StartPosition);
	}

	char symbol = InputStream.peek();

	if (symbol == '\\') {
		AdvanceOneSymbol();
		Text = ProcessEscapeSequence();
	} else {
		Text = symbol;
		AdvanceOneSymbol();
	}

	if (!InputStream.good() || AdvanceOneSymbol() != '\'') {
		throw CException("unterminated symbol constant", StartPosition);
	}

	return new CSymbolConstToken(Text, StartPosition);
}

CToken* CScanner::ScanNumericalConstant()
{
	CPosition StartPosition = CurrentPosition;

	string IntegerPart = "0";
	string FractionalPart;
	string ExponentPart;
	string SuffixPart;

	if (InputStream.peek() != '.') {
		IntegerPart = ScanIntegerPart();
	}

	FractionalPart = ScanFractionalPart();
	ExponentPart = ScanExponentPart();

	bool FloatConstant = !(FractionalPart.empty() && ExponentPart.empty());

	if (FloatConstant) {
		SuffixPart = ScanFloatSuffix();
	} else {
		SuffixPart = ScanIntegerSuffix();
	}

	string Text = IntegerPart + FractionalPart + ExponentPart + SuffixPart;

	CToken *NewToken;
	if (FloatConstant) {
		NewToken = new CFloatConstToken(Text, StartPosition);
	} else {
		NewToken = new CIntegerConstToken(Text, StartPosition);
	}

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
				throw CException("invalid hexadecimal constant", CurrentPosition);
			}
			result += ScanHexadecimalInteger();
			if (InputStream.peek() == '.') {
				throw CException("invalid float constant", CurrentPosition);
			}
		} else if (CTraits::IsOctDigit(c)) {
			result += ScanOctalInteger();
			if (InputStream.peek() == '.') {
				throw CException("invalid float constant", CurrentPosition);
			}
		} else if (CTraits::IsDigit(c)) {
			throw CException("invalid octal constant", CurrentPosition);
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
			throw CException("invalid suffix on float constant", CurrentPosition);
		}
	} else if (CTraits::IsValidIdentifierSymbol(c)) {
		throw CException("invalid suffix on float constant", CurrentPosition);
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
		throw CException("invalid suffix on integer constant", CurrentPosition);
	}

	char sc = InputStream.peek();

	if (((sc == 'u' || sc == 'U') && (fc != 'u' && fc != 'U')) || ((sc == 'l' || sc == 'L') && (fc != 'l' && fc != 'L'))) {
		result += fc;
		AdvanceOneSymbol();
	} else if (CTraits::IsValidIdentifierSymbol(sc)) {
		throw CException("invalid suffix on integer constant", CurrentPosition);
	}

	if (CTraits::IsValidIdentifierSymbol(InputStream.peek())) {
		throw CException("invalid suffix on integer constant", CurrentPosition);
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
		throw CException("invalid escape sequence", CurrentPosition);
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
		throw CException("unterminated comment", start);
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
