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

CToken* CToken::Clone() const
{
	return new CToken(*this);
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

float CToken::GetFloatValue() const
{
	throw logic_error("CToken can't have float value");
}

char CToken::GetCharValue() const
{
	throw logic_error("CToken can't have char value");
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

CIntegerConstToken* CIntegerConstToken::Clone() const
{
	return new CIntegerConstToken(*this);
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

CFloatConstToken* CFloatConstToken::Clone() const
{
	return new CFloatConstToken(*this);
}

float CFloatConstToken::GetFloatValue() const
{
	return Value;
}

/******************************************************************************
 * CCharConstToken
 ******************************************************************************/

CCharConstToken::CCharConstToken(const string &AText, const CPosition &APosition) : CToken(TOKEN_TYPE_CONSTANT_CHAR, AText, APosition)
{
	Value = AText[0];
}

CCharConstToken* CCharConstToken::Clone() const
{
	return new CCharConstToken(*this);
}

char CCharConstToken::GetCharValue() const
{
	return Value;
}

/******************************************************************************
 * CScanner
 ******************************************************************************/

map<ETokenType, string> CScanner::TokenTypesNames;

CScanner::CScanner(istream &AInputStream) : InputStream(AInputStream), LastToken(NULL), CurrentPosition(1, 1)
{
	if (!InputStream.good()) {
		throw CScannerException("can't read from input stream", CurrentPosition);
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
		TokenTypesNames[TOKEN_TYPE_CONSTANT_CHAR] = "CONSTANT_CHAR";
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
	char c = InputStream.peek();

	if (CharTraits::IsValidIdentifierChar(c, true)) {
		NewToken = ScanIdentifier();
	} else if (TryScanNumericalConstant()) {
		NewToken = ScanNumericalConstant();
	} else if (CharTraits::IsOperationChar(c)) {
		NewToken = ScanOperation();
	} else if (c == '"') {
		NewToken = ScanStringConstant();
	} else if (c == '\'') {
		NewToken = ScanCharConstant();
	} else {
		NewToken = ScanSingleChar();
	}

	return (LastToken = NewToken);
}

CToken* CScanner::ScanIdentifier()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	while (InputStream.good() && CharTraits::IsValidIdentifierChar(InputStream.peek())) {
		Text += NextChar();
	}

	return new CToken(KeywordTraits::IsKeyword(Text) ? TOKEN_TYPE_KEYWORD : TOKEN_TYPE_IDENTIFIER, Text, StartPosition);
}

CToken* CScanner::ScanOperation()
{
	CPosition StartPosition = CurrentPosition;
	ETokenType Type;

	char fs = NextChar();
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
		Type = TOKEN_TYPE_OPERATION_BITWISE_NOT;
		OperationLength = 1;
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
		NextChar();
	}

	if (OperationLength > 2) {
		Text += ts;
		NextChar();
	}

	return new CToken(Type, Text, StartPosition);
}

CToken* CScanner::ScanSingleChar()
{
	CPosition StartPosition = CurrentPosition;
	ETokenType Type;

	char c = InputStream.peek();

	switch (c) {
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
		throw CScannerException(string("invalid char '") + c + "' encountered", CurrentPosition);
	}

	NextChar();

	return new CToken(Type, string(1, c), StartPosition);
}

CToken* CScanner::ScanStringConstant()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	NextChar();

	char c;

	while (InputStream.good() && ((c = NextChar()) != '\n')) {
		// we don't have to process escape sequences in strings because all strings are passed to GAS as is
		/*if (c == '\\') {
			Text += ProcessEscapeSequence();
		} else*/
		
		if (c == '"') {
			return new CToken(TOKEN_TYPE_CONSTANT_STRING, Text, StartPosition);
		} else {
			Text += c;
		}
	}

	throw CScannerException("unterminated string constant", StartPosition);
}

CToken* CScanner::ScanCharConstant()
{
	CPosition StartPosition = CurrentPosition;
	string Text;

	NextChar();

	if (!InputStream.good()) {
		throw CScannerException("unterminated char constant", StartPosition);
	}

	char c = InputStream.peek();

	if (c == '\\') {
		NextChar();
		Text = ProcessEscapeSequence();
	} else {
		Text = c;
		NextChar();
	}

	if (!InputStream.good() || NextChar() != '\'') {
		throw CScannerException("unterminated char constant", StartPosition);
	}

	return new CCharConstToken(Text, StartPosition);
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
	char c = InputStream.peek();

	if (CharTraits::IsDigit(c)) {
		result = true;
	} else if (c == '.') {
		InputStream.ignore();
		if (CharTraits::IsDigit(InputStream.peek())) {
			result = true;
		}
		InputStream.putback(c);
	}

	return result;
}

string CScanner::ScanHexadecimalInteger()
{
	string result;
	char c;

	while (InputStream.good() && CharTraits::IsHexDigit(c = InputStream.peek())) {
		result += c;
		NextChar();
	}

	return result;
}

string CScanner::ScanOctalInteger()
{
	string result;
	char c;

	while (InputStream.good() && CharTraits::IsOctDigit(c = InputStream.peek())) {
		result += c;
		NextChar();
	}

	return result;
}

string CScanner::ScanIntegerPart()
{
	string result;

	char c = NextChar();
	result += c;

	if (c == '0') {
		c = InputStream.peek();
		if (c == 'x' || c == 'X') {
			result += 'x';
			NextChar();
			if (!CharTraits::IsHexDigit(InputStream.peek())) {
				throw CScannerException("invalid hexadecimal constant", CurrentPosition);
			}
			result += ScanHexadecimalInteger();
			if (InputStream.peek() == '.') {
				throw CScannerException("invalid float constant", CurrentPosition);
			}
		} else if (CharTraits::IsOctDigit(c)) {
			result += ScanOctalInteger();
			if (InputStream.peek() == '.') {
				throw CScannerException("invalid float constant", CurrentPosition);
			}
		} else if (CharTraits::IsDigit(c)) {
			throw CScannerException("invalid octal constant", CurrentPosition);
		}

	} else {
		while (InputStream.good() && CharTraits::IsDigit(c = InputStream.peek())) {
			result += c;
			NextChar();
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

	result += NextChar();

	while (InputStream.good() && CharTraits::IsDigit(c = InputStream.peek())) {
		result += c;
		NextChar();
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
	result += NextChar();

	c = InputStream.peek();

	if (c == '+' || c == '-') {
		result += c;
		NextChar();
	}

	while (InputStream.good() && CharTraits::IsDigit(c = InputStream.peek())) {
		result += c;
		NextChar();
	}

	return result;
}

string CScanner::ScanFloatSuffix()
{
	char c = InputStream.peek();

	if (c == 'f' || c == 'F' || c == 'l' || c == 'L') {
		NextChar();
		if (!CharTraits::IsValidIdentifierChar(InputStream.peek())) {
			return string(1, c);
		} else {
			throw CScannerException("invalid suffix on float constant", CurrentPosition);
		}
	} else if (CharTraits::IsValidIdentifierChar(c)) {
		throw CScannerException("invalid suffix on float constant", CurrentPosition);
	}

	return "";
}

string CScanner::ScanIntegerSuffix()
{
	string result;
	char fc = InputStream.peek();

	if (fc == 'u' || fc == 'U' || fc == 'l' || fc == 'L') {
		result += fc;
		NextChar();
	} else if (CharTraits::IsValidIdentifierChar(fc)) {
		throw CScannerException("invalid suffix on integer constant", CurrentPosition);
	}

	char sc = InputStream.peek();

	if (((sc == 'u' || sc == 'U') && (fc != 'u' && fc != 'U')) || ((sc == 'l' || sc == 'L') && (fc != 'l' && fc != 'L'))) {
		result += fc;
		NextChar();
	} else if (CharTraits::IsValidIdentifierChar(sc)) {
		throw CScannerException("invalid suffix on integer constant", CurrentPosition);
	}

	if (CharTraits::IsValidIdentifierChar(InputStream.peek())) {
		throw CScannerException("invalid suffix on integer constant", CurrentPosition);
	}

	return result;
}

char CScanner::ProcessEscapeSequence()
{
	if (!InputStream.good()) {
		return 0;
	}

	char c = NextChar();
	char result;

	switch (c) {
	case '\'':
	case '"':
	case '\\':
	case '?':	// used for trigraphs only..
		result = c;
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
		throw CScannerException("invalid escape sequence", CurrentPosition);
	}

	return result;
}

bool CScanner::SkipComment()
{
	bool EndMatchState = false;
	bool end = false;
	char c;

	if (!InputStream.good()) {
		return false;
	}

	c = InputStream.peek();
	if (c == '/') {
		InputStream.ignore();
		c = InputStream.peek();
		InputStream.putback('/');
		if (c != '*') {
			return false;
		}
	} else {
		return false;
	}

	CPosition start = CurrentPosition;

	while (InputStream.good() && !end) {
		c = NextChar();

		if (c == '*') {
			EndMatchState = true;
		} else if (EndMatchState && c == '/') {
			end = true;
		} else {
			EndMatchState = false;
		}
	}

	if (!InputStream.good()) {
		throw CScannerException("unterminated comment", start);
	}

	return true;
}

bool CScanner::SkipWhitespace()
{
	if (!InputStream.good() || !CharTraits::IsWhitespace(InputStream.peek())) {
		return false;
	}

	while (InputStream.good() && CharTraits::IsWhitespace(InputStream.peek())) {
		NextChar();
	}

	return true;
}

void CScanner::SkipWhitespaceAndComments()
{
	while (InputStream.good() && (SkipWhitespace() || SkipComment()));
}

char CScanner::NextChar()
{
	char c = InputStream.get();

	if (c == '\n') {
		CurrentPosition.Line++;
		CurrentPosition.Column = 1;
	} else {
		CurrentPosition.Column++;
	}

	return c;
}
