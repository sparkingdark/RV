#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "lexer.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool crazyMode;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_ADDSUB,     // + -
    PREC_MULDIV,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;

Program *compilingProgram;

static Program *currentProgram()
{
    return compilingProgram;
}

static void errorAt(Token *token, const char *msg)
{
    if (parser.crazyMode)
        return;

    parser.crazyMode = true;

    fprintf(stderr, "line %d: Error", token->line);

    if (token->type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing
    }
    else
        fprintf(stderr, " at '%.*s'", token->length, token->start);

    fprintf(stderr, ": %s\n", msg);
    parser.hadError = true;
}

static void errorAtCurrent(const char *msg)
{
    errorAt(&parser.current, msg);
}

static void error(const char *msg)
{
    errorAt(&parser.current, msg);
}

static void advance()
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

static void validate(TokenType type, const char *msg)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(msg);
}

static void emitByte(uint8_t byte)
{
    writeProgram(currentProgram(), byte, parser.previous.line);
}

static void emit2Bytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

static uint8_t makeConst(Value value)
{
    int constant = addConst(currentProgram(), value);

    if (constant > UINT8_MAX)
    {
        error("Too many constants in one program");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConst(Value value)
{
    emit2Bytes(OP_CONST, makeConst(value));
}

static void endCompile()
{
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleProgram(currentProgram(), "code");
    }
#endif
}

static void expression();
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary()
{
    // Remember the operator.
    TokenType operatorType = parser.previous.type;

    // Compile the right operand.
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        emit2Bytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_DOUBLE_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emit2Bytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emit2Bytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_ASTERISK:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        return; // Unreachable.
    }
}

static void literal()
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_NONE:
        emitByte(OP_NONE);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

static void group()
{
    expression();
    validate(TOKEN_RPAREN, "Expected ')' after expression");
}

static void number()
{
    emitConst(NUMBER_VAL(strtod(parser.previous.start, NULL)));
}

static void unary()
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return; // Unreachable.
    }
}

ParseRule rules[] = {
    {group, NULL, PREC_NONE},        // TOKEN_LPAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_RPAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_LBRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_RBRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_COMMA
    {NULL, NULL, PREC_NONE},         // TOKEN_DOT
    {unary, binary, PREC_ADDSUB},    // TOKEN_MINUS
    {NULL, binary, PREC_ADDSUB},     // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},         // TOKEN_SEMICOLON
    {NULL, binary, PREC_MULDIV},     // TOKEN_SLASH
    {NULL, binary, PREC_MULDIV},     // TOKEN_ASTERISK
    {unary, NULL, PREC_NONE},        // TOKEN_BANG
    {NULL, binary, PREC_EQUALITY},   // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_EQUAL
    {NULL, binary, PREC_EQUALITY},   // TOKEN_DOUBLE_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_IDENTIFIER
    {NULL, NULL, PREC_NONE},         // TOKEN_STRING
    {number, NULL, PREC_NONE},       // TOKEN_NUMBER
    {NULL, NULL, PREC_NONE},         // TOKEN_AND
    {NULL, NULL, PREC_NONE},         // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},         // TOKEN_ELSE
    {literal, NULL, PREC_NONE},      // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},         // TOKEN_FOR
    {NULL, NULL, PREC_NONE},         // TOKEN_FUN
    {NULL, NULL, PREC_NONE},         // TOKEN_IF
    {literal, NULL, PREC_NONE},      // TOKEN_NONE
    {NULL, NULL, PREC_NONE},         // TOKEN_OR
    {NULL, NULL, PREC_NONE},         // TOKEN_PRINT
    {NULL, NULL, PREC_NONE},         // TOKEN_RETURN
    {NULL, NULL, PREC_NONE},         // TOKEN_SUPER
    {NULL, NULL, PREC_NONE},         // TOKEN_THIS
    {literal, NULL, PREC_NONE},      // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},         // TOKEN_VAR
    {NULL, NULL, PREC_NONE},         // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},         // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},         // TOKEN_EOF
};

static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expression is expected");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char *src, Program *program)
{
    initLexer(src);

    compilingProgram = program;

    parser.hadError = false;
    parser.crazyMode = false;

    advance();
    expression();
    validate(TOKEN_EOF, "EOF is expected");

    endCompile();

    return !parser.hadError;
}
