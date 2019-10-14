#include <stdio.h>
#include <string.h>
#include "common.h"
#include "lexer.h"

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Lexer;

Lexer lexer;

void initLexer(const char *src)
{
    lexer.start = src;
    lexer.current = src;
    lexer.line = 1;
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isDone()
{
    return *lexer.current == '\0';
}

static char advance()
{
    ++lexer.current;
    return lexer.current[-1];
}

static char getCurrent()
{
    return *lexer.current;
}

static char getNext()
{
    if (isDone())
        return '\0';
    return lexer.current[1];
}

static bool match(char expected)
{
    if (isDone())
        return false;
    if (*lexer.current != expected)
        return false;

    ++lexer.current;

    return true;
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;

    return token;
}

static Token errorToken(const char *msg)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = (int)strlen(msg);
    token.line = lexer.line;

    return token;
}

static void skipWS()
{
    while (true)
    {
        char c = getCurrent();

        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            ++lexer.line;
            advance();
            break;
        case '-':
            if (getNext() == '-')
            {
                while (getCurrent() != '\n' && !isDone()) // a comment ends at the end of the line
                    advance();
            }
            else
                return;
            break;

        default:
            return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char *rest, TokenType type)
{
    if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0)
        return type;

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType()
{
    switch (lexer.start[0])
    {
    case 'a':
        return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (lexer.current - lexer.start > 1)
        {
            switch (lexer.start[1])
            {
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
        return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return checkKeyword(1, 2, "one", TOKEN_NONE);
    case 'o':
        return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (lexer.current - lexer.start > 1)
        {
            switch (lexer.start[1])
            {
            case 'h':
                return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (isAlpha(getCurrent()) || isDigit(getCurrent()))
        advance();

    return makeToken(identifierType());
}

static Token number()
{
    while (isDigit(getCurrent()))
        advance();

    // looking for a float
    if (getCurrent() == '.' && isDigit(getNext()))
    {
        advance();

        while (isDigit(getCurrent()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string()
{
    while (getCurrent() != '\'' && !isDone())
    {
        if (getCurrent() == '\n')
            ++lexer.line;
        advance();
    }

    if (isDone())
        return errorToken("Strings must begin and end with single quotes");

    // The closing single quote.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken()
{
    skipWS();

    lexer.start = lexer.current;

    if (isDone())
        return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlpha(c))
        return identifier();
    if (isDigit(c))
        return number();

    switch (c)
    {
    case '(':
        return makeToken(TOKEN_LPAREN);
    case ')':
        return makeToken(TOKEN_RPAREN);
    case '{':
        return makeToken(TOKEN_LBRACE);
    case '}':
        return makeToken(TOKEN_RBRACE);
    case ';':
        return makeToken(TOKEN_SEMICOLON);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_ASTERISK);
    case '!':
        return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(match('=') ? TOKEN_DOUBLE_EQUAL : TOKEN_EQUAL);
    case '<':
        return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '\'':
        return string();
    }

    return errorToken("Unrecognized character...");
}
