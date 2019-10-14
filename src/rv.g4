grammar rv;

// START OF PARSER
program: block EOF;

block: statement* returnStatement?;

statement:
	';'
	| varList '=' expressionList
	| functionCall
	| label
	| 'break'
	| 'goto' NAME
	| 'do' block 'end'
	| 'while' expression 'do' block 'end'
	| 'repeat' block 'till' expression
	| 'if' expression 'then' block (
		'elif' expression 'then' block
	)* ('else' block)? 'end'
	| 'for' NAME '=' expression ',' expression (',' expression)? 'do' block 'end'
	| 'for' nameList 'of' expressionList 'do' block 'end'
	| 'fun' functionName functionBody
	| 'scoped' 'fun' NAME functionBody
	| 'scoped' nameList ('=' expressionList)?;

returnStatement: 'return' expressionList? ';'?;

label: '<' NAME '>';

functionName: NAME ('.' NAME)* (':' NAME)?;

varList: var_ (',' var_)*;

nameList: NAME (',' NAME)*;

expressionList: expression (',' expression)*;

expression:
	'none'
	| 'false'
	| 'true'
	| number
	| string
	| '...'
	| functionDefinition
	| prefixExpression
	| dictionaryConstructor
	| <assoc = right> expression operatorPower expression
	| operatorUnary expression
	| expression operatorMulDivMod expression
	| expression operatorAddSub expression
	| <assoc = right> expression operatorStringConcat expression
	| expression operatorComparison expression
	| expression operatorAnd expression
	| expression operatorOr expression
	| expression operatorBitwise expression;

prefixExpression: varOrExpression nameAndArgs*;

functionCall: varOrExpression nameAndArgs+;

varOrExpression: var_ | '(' expression ')';

var_: (NAME | '(' expression ')' varSuffix) varSuffix*;

varSuffix: nameAndArgs* ('[' expression ']' | '.' NAME);

nameAndArgs: (':' NAME)? args;

args: '(' expressionList? ')' | dictionaryConstructor | string;

functionDefinition: 'fun' functionBody;

functionBody: '(' parametersList? ')' block 'end';

parametersList: nameList (',' '...')? | '...';

dictionaryConstructor: '{' keyList? '}';

keyList: key (keySeparator key)* keySeparator?;

key:
	'[' expression ']' ':' expression
	| NAME ':' expression
	| expression;

keySeparator: ',';

operatorOr: 'or';

operatorAnd: 'and';

operatorComparison: '<' | '>' | '<=' | '>=' | '!=' | '==';

operatorStringConcat: '@';

operatorAddSub: '+' | '-';

operatorMulDivMod: '*' | '/' | '%' | '//';

operatorBitwise: '&' | '|' | '~' | '<<' | '>>';

operatorUnary: 'not' | '?' | '-' | '~';

operatorPower: '**';

number: INT | FLOAT;

string: NORMALSTRING | CHARSTRING | LONGSTRING;
// END OF PARSER

// START OF LEXER
NAME: [a-zA-Z_][a-zA-Z_0-9]*;

NORMALSTRING: '"' ( EscapeSequence | ~('\\' | '"'))* '"';

CHARSTRING: '\'' ( EscapeSequence | ~('\'' | '\\'))* '\'';

LONGSTRING: '[' NESTED_STR ']';

fragment NESTED_STR: '=' NESTED_STR '=' | '[' .*? ']';

INT: Digit+;

FLOAT:
	Digit+ '.' Digit* ExponentPart?
	| '.' Digit+ ExponentPart?
	| Digit+ ExponentPart;

fragment ExponentPart: [eE] [+-]? Digit+;
fragment EscapeSequence:
	'\\' [abfnrtvz"'\\]
	| '\\' '\r'? '\n'
	| DecimalEscape;
fragment DecimalEscape:
	'\\' Digit
	| '\\' Digit Digit
	| '\\' [0-2] Digit Digit;
fragment Digit: [0-9];
COMMENT: '--[' NESTED_STR ']' -> channel(HIDDEN);
MULTILINE_COMMENT:
	'--' (
		// --
		| '[' '='* // --[==
		| '[' '='* ~('=' | '[' | '\r' | '\n') ~('\r' | '\n')* // --[==AA
		| ~('[' | '\r' | '\n') ~('\r' | '\n')* // --AAA
	) ('\r\n' | '\r' | '\n' | EOF) -> channel(HIDDEN);
WHITESPACE: [ \t\u000C\r\n]+ -> skip;
SHEBANG: '#' '!' ~('\n' | '\r')* -> channel(HIDDEN);
// END OF LEXER
