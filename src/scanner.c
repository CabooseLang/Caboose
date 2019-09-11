#include "scanner.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

typedef struct {
	const char *start;
	const char *current;
	int line;
} Scanner;

Scanner scanner;

void initScanner(const char *source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

static bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char c) { return c >= '0' && c <= '9'; }

static bool isAtEnd() { return *scanner.current == '\0'; }

static char advance() {
	scanner.current++;
	return scanner.current[-1];
}

static char peek() { return *scanner.current; }

static char peekNext() {
	if (isAtEnd())
		return '\0';
	return scanner.current[1];
}

static bool match(char expected) {
	if (isAtEnd())
		return false;
	if (*scanner.current != expected)
		return false;

	scanner.current++;
	return true;
}

static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.start = scanner.start;
	token.length = (int)(scanner.current - scanner.start);
	token.line = scanner.line;

	return token;
}

static Token errorToken(const char *message) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner.line;

	return token;
}

static void skipWhitespace() {
	for (;;) {
		char c = peek();
		switch (c) {
			case ' ':
			case '\r':
			case '\t':
				advance();
				break;

			case '\n':
				scanner.line++;
				advance();
				break;

			case '/':
				if (peekNext() == '*') {
					char c;
					// Multiline comments
					advance();
					advance();
					while (true) {
						while (peek() != '*' && !isAtEnd()) {
							if ((c = advance()) == '\n') {
								scanner.line++;
							}
						}

						if (peekNext() == '/')
							break;
						advance();
					}
					advance();
					advance();
				} else if (peekNext() == '/')
					while (peek() != '\n' && !isAtEnd())
						advance();
				else
					return;

				break;
			default:
				return;
		}
	}
}

static TokenType checkKeyword(int start, int length, const char *rest,
							  TokenType type) {
	if (scanner.current - scanner.start == start + length &&
		memcmp(scanner.start + start, rest, length) == 0)
		return type;

	return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
	switch (scanner.start[0]) {
		case 'a':
			return checkKeyword(1, 2, "nd", TOKEN_AND);
		case 'b':
			return checkKeyword(1, 4, "reak", TOKEN_BREAK);
		case 'c':
			if (scanner.current - scanner.start > 1)
				switch (scanner.start[1]) {
					case 'l':
						return checkKeyword(2, 3, "ass", TOKEN_CLASS);
					case 'o':
						return checkKeyword(2, 6, "ntinue", TOKEN_CONTINUE);
				}
			break;
		case 'e':
			return checkKeyword(1, 3, "lse", TOKEN_ELSE);
		case 'f':
			if (scanner.current - scanner.start > 1) {
				switch (scanner.start[1]) {
					case 'a':
						return checkKeyword(2, 3, "lse", TOKEN_FALSE);
					case 'o':
						return checkKeyword(2, 1, "r", TOKEN_FOR);
					case 'u':
						return checkKeyword(2, 1, "n", TOKEN_FUN);
				}
			}
			break;
		case 'i':
			if (scanner.current - scanner.start > 1)
				switch (scanner.start[1]) {
					case 'f':
						return checkKeyword(2, 0, "", TOKEN_IF);
					case 'm':
						return checkKeyword(2, 4, "port", TOKEN_IMPORT);
				}
			break;
		case 'n':
			return checkKeyword(1, 2, "il", TOKEN_NIL);
		case 'o':
			return checkKeyword(1, 1, "r", TOKEN_OR);
		case 'r':
			return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
		case 's':
			if (scanner.current - scanner.start > 1)
				switch (scanner.start[1]) {
					case 'u':
						return checkKeyword(2, 3, "per", TOKEN_SUPER);
					case 't':
						return checkKeyword(2, 4, "atic", TOKEN_STATIC);
				}
			break;
		case 't':
			if (scanner.current - scanner.start > 1)
				switch (scanner.start[1]) {
					case 'h':
						return checkKeyword(2, 2, "is", TOKEN_THIS);
					case 'r':
						return checkKeyword(2, 2, "ue", TOKEN_TRUE);
				}
			break;
		case 'v':
			return checkKeyword(1, 2, "ar", TOKEN_VAR);
		case 'w':
			if (scanner.current - scanner.start > 1)
				switch (scanner.start[1]) {
					case 'h':
						return checkKeyword(2, 3, "ile", TOKEN_WHILE);
					case 'i':
						return checkKeyword(2, 2, "th", TOKEN_WITH);
				}
			break;
	}

	return TOKEN_IDENTIFIER;
}

static Token identifier() {
	while (isAlpha(peek()) || isDigit(peek()))
		advance();

	return makeToken(identifierType());
}

static Token number() {
	while (isDigit(peek()))
		advance();

	// Look for a fractional part.
	if (peek() == '.' && isDigit(peekNext())) {
		// Consume the "."
		advance();

		while (isDigit(peek()))
			advance();
	}

	return makeToken(TOKEN_NUMBER);
}

static Token string(char stringToken) {
	while (peek() != stringToken && !isAtEnd()) {
		if (peek() == '\n')
			scanner.line++;
		else if (peek() == '\\')
			scanner.current++;

		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string.");

	// The closing " or '.
	advance();
	return makeToken(TOKEN_STRING);
}

void backTrack() { scanner.current--; }

Token scanToken() {
	skipWhitespace();

	scanner.start = scanner.current;

	if (isAtEnd())
		return makeToken(TOKEN_EOF);

	char c = advance();

	if (isAlpha(c))
		return identifier();
	if (isDigit(c))
		return number();

	switch (c) {
		case '(':
			return makeToken(TOKEN_LEFT_PAREN);
		case ')':
			return makeToken(TOKEN_RIGHT_PAREN);
		case '{':
			return makeToken(TOKEN_LEFT_BRACE);
		case '}':
			return makeToken(TOKEN_RIGHT_BRACE);
		case '[':
			return makeToken(TOKEN_LEFT_BRACKET);
		case ']':
			return makeToken(TOKEN_RIGHT_BRACKET);
		case ';':
			return makeToken(TOKEN_SEMICOLON);
		case ':':
			return makeToken(TOKEN_COLON);
		case ',':
			return makeToken(TOKEN_COMMA);
		case '.':
			return makeToken(TOKEN_DOT);
		case '/': {
			if (match('='))
				return makeToken(TOKEN_DIVIDE_EQUALS);
			else
				return makeToken(TOKEN_SLASH);
		}
		case '*': {
			if (match('='))
				return makeToken(TOKEN_MULTIPLY_EQUALS);
			else
				return makeToken(TOKEN_STAR);
		}
		case '%':
			return makeToken(TOKEN_PERCENT);
		case '-': {
			if (match('-'))
				return makeToken(TOKEN_DECREMENT);
			else if (match('='))
				return makeToken(TOKEN_MINUS_EQUALS);
			else
				return makeToken(TOKEN_MINUS);
		}
		case '+': {
			if (match('+'))
				return makeToken(TOKEN_INCREMENT);
			else if (match('='))
				return makeToken(TOKEN_PLUS_EQUALS);
			else
				return makeToken(TOKEN_PLUS);
		}
		case '!':
			return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=':
			return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<':
			return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>':
			return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '"':
			return string('"');
		case '\'':
			return string('\'');
	}

	return errorToken("Unexpected character.");
}
