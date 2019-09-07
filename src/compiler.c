#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

typedef struct {
	Token name;
	int depth;
	bool isUpvalue;
} Local;

typedef struct {
	uint8_t index;
	bool isLocal;
} Upvalue;

typedef enum {
	TYPE_FUNCTION,
	TYPE_INITIALIZER,
	TYPE_METHOD,
	TYPE_SCRIPT,
} FunctionType;

typedef struct Compiler {
	struct Compiler* enclosing;
	ObjFunction* function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int localCount;
	Upvalue upvalues[UINT8_COUNT];
	int scopeDepth;
} Compiler;

typedef struct ClassCompiler {
	struct ClassCompiler* enclosing;

	Token name;
	bool hasSuperclass;
} ClassCompiler;

Parser parser;

Compiler* current = NULL;

ClassCompiler* currentClass = NULL;

static Chunk* currentChunk() {
	return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
	if (parser.panicMode) return;
	parser.panicMode = true;

	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF) fprintf(stderr, " at end");
	else if (token->type == TOKEN_ERROR);
	else fprintf(stderr, " at '%.*s'", token->length, token->start);

	fprintf(stderr, ": %s\n", message);
	parser.hadError = true;
}

static void error(const char* message) {
	errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
	errorAt(&parser.current, message);
}

static void advance() {
	parser.previous = parser.current;

	while (1) {
		parser.current = scanToken();
		if (parser.current.type != TOKEN_ERROR) break;

		errorAtCurrent(parser.current.start);
	}
}

static void consume(TokenType type, const char* message) {
	if (parser.current.type == type) {
		advance();
		return;
	}

	errorAtCurrent(message);
}

static bool check(TokenType type) {
	return parser.current.type == type;
}

static bool match(TokenType type) {
	if (!check(type)) return false;
	advance();
	return true;
}

static void emitByte(uint8_t byte) {
	writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
	emitByte(byte1);
	emitByte(byte2);
}

static void emitLoop(int loopStart) {
	emitByte(OP_LOOP);

	int offset = currentChunk()->count - loopStart + 2;
	if (offset > UINT16_MAX) error("Loop body too large.");

	emitByte((offset >> 8) & 0xff);
	emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
	emitByte(instruction);
	emitByte(0xff);
	emitByte(0xff);
	return currentChunk()->count - 2;
}

static void emitReturn() {
	if (current->type == TYPE_INITIALIZER) emitBytes(OP_GET_LOCAL, 0);
	else emitByte(OP_NIL);

	emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
	int constant = addConstant(currentChunk(), value);
	
	if (constant > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	}

	return (uint8_t) constant;
}

static void emitConstant(Value value) {
	emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
	int jump = currentChunk()->count - offset - 2;
	if (jump > UINT16_MAX) error("Too much code to jump over.");

	currentChunk()->code[offset] = (jump >> 8) & 0xff;
	currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
	compiler->enclosing = current;
	compiler->function = NULL;
	compiler->type = type;
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	compiler->function = newFunction();
	current = compiler;

	if (type != TYPE_SCRIPT) current->function->name = copyString(parser.previous.start, parser.previous.length);

	Local* local = &current->locals[current->localCount++];
	local->depth = 0;
	local->isUpvalue = false;

	if (type != TYPE_FUNCTION) {
		local->name.start = "this";
		local->name.length = 4;
	} else {
		local->name.start = "";
		local->name.length = 0;
	}
}

static ObjFunction* endCompiler() {
	emitReturn();
	ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError)
		disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
#endif

	current = current->enclosing;
	return function;
}

static void beginScope() {
	current->scopeDepth++;
}

static void endScope() {
	current->scopeDepth--;

	while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
		if (current->locals[current->localCount - 1].isUpvalue) emitByte(OP_CLOSE_UPVALUE);
		else emitByte(OP_POP);
		current->localCount--;
	}
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token* name) {
	return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}
static bool identifiersEqual(Token* a, Token* b) {
	if (a->length != b->length) return false;
	return memcmp(a->start, b->start, a->length) == 0;
}
static int resolveLocal(Compiler* compiler, Token* name) {
	for (int i = compiler->localCount - 1; i >= 0; i--) {
		Local* local = &compiler->locals[i];
		if (identifiersEqual(name, &local->name)) {
			if (local->depth == -1) {
				error("Cannot read local variable in its own initializer.");
			}
			return i;
		}
	}

	return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal) {
	int upvalueCount = compiler->function->upvalueCount;
	for (int i = 0; i < upvalueCount; i++) {
		Upvalue* upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->isLocal == isLocal) return i;
	}

		if (upvalueCount == UINT8_COUNT) {
		error("Too many closure variables in function.");
		return 0;
	}

	compiler->upvalues[upvalueCount].isLocal = isLocal;
	compiler->upvalues[upvalueCount].index = index;
	return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
	if (compiler->enclosing == NULL) return -1;
	int local = resolveLocal(compiler->enclosing, name);
	
	if (local != -1) {
		compiler->enclosing->locals[local].isUpvalue = true;
		return addUpvalue(compiler, (uint8_t)local, true);
	}

	int upvalue = resolveUpvalue(compiler->enclosing, name);
	if (upvalue != -1) return addUpvalue(compiler, (uint8_t)upvalue, false);
	return -1;
}

static void addLocal(Token name) {
	if (current->localCount == UINT8_COUNT) {
		error("Too many local variables in function.");
		return;
	}

	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
	local->isUpvalue = false;
}

static void declareVariable() {
		if (current->scopeDepth == 0) return;

	Token* name = &parser.previous;
	for (int i = current->localCount - 1; i >= 0; i--) {
		Local* local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scopeDepth) {
			break;     }
		
		if (identifiersEqual(name, &local->name)) {
			error("Variable with this name already declared in this scope.");
		}
	}

	addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
	consume(TOKEN_IDENTIFIER, errorMessage);
	declareVariable();
	if (current->scopeDepth > 0) return 0;
	return identifierConstant(&parser.previous);
}

static void markInitialized() {
	if (current->scopeDepth == 0) return;
	current->locals[current->localCount - 1].depth = current->scopeDepth;
}
static void defineVariable(uint8_t global) {
	if (current->scopeDepth > 0) {
		markInitialized();
		return;
	}

	emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList() {
	uint8_t argCount = 0;
	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			expression();

			if (argCount == 255) error("Cannot have more than 255 arguments.");
			argCount++;
		} while (match(TOKEN_COMMA));
	}

	consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return argCount;
}

static void and_(bool canAssign) {
	int endJump = emitJump(OP_JUMP_IF_FALSE);

	emitByte(OP_POP);
	parsePrecedence(PREC_AND);

	patchJump(endJump);
}

static void binary(bool canAssign) {
		TokenType operatorType = parser.previous.type;

		ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

		switch (operatorType) {
		case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
		case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
		case TOKEN_GREATER:       emitByte(OP_GREATER); break;
		case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
		case TOKEN_LESS:          emitByte(OP_LESS); break;
		case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
		case TOKEN_PLUS:          emitByte(OP_ADD); break;
		case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
		case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
		case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
		default:
			return;   }
}

static void call(bool canAssign) {
	uint8_t argCount = argumentList();
	emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign) {
	consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
	uint8_t name = identifierConstant(&parser.previous);

	if (canAssign && match(TOKEN_EQUAL)) {
		expression();
		emitBytes(OP_SET_PROPERTY, name);
	} else if (match(TOKEN_LEFT_PAREN)) {
		uint8_t argCount = argumentList();
		emitBytes(OP_INVOKE, argCount);
		emitByte(name);
	} else emitBytes(OP_GET_PROPERTY, name);
}

static void literal(bool canAssign) {
	switch (parser.previous.type) {
		case TOKEN_FALSE: emitByte(OP_FALSE); break;
		case TOKEN_NIL: emitByte(OP_NIL); break;
		case TOKEN_TRUE: emitByte(OP_TRUE); break;
		default:
			return;
	}
}

static void grouping(bool canAssign) {
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
	double value = strtod(parser.previous.start, NULL);
	emitConstant(NUMBER_VAL(value));
}
static void or_(bool canAssign) {
	int elseJump = emitJump(OP_JUMP_IF_FALSE);
	int endJump = emitJump(OP_JUMP);

	patchJump(elseJump);
	emitByte(OP_POP);

	parsePrecedence(PREC_OR);
	patchJump(endJump);
}

static void string(bool canAssign) {
	emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}
//[<>]? .+\n
static void namedVariable(Token name, bool canAssign) {
/* Global Variables read-named-variable < Local Variables named-local
	uint8_t arg = identifierConstant(&name);
*/
	uint8_t getOp, setOp;
	int arg = resolveLocal(current, &name);
	if (arg != -1) {
		getOp = OP_GET_LOCAL;
		setOp = OP_SET_LOCAL;
	} else if ((arg = resolveUpvalue(current, &name)) != -1) {
		getOp = OP_GET_UPVALUE;
		setOp = OP_SET_UPVALUE;
	} else {
		arg = identifierConstant(&name);
		getOp = OP_GET_GLOBAL;
		setOp = OP_SET_GLOBAL;
	}
	
	if (canAssign && match(TOKEN_EQUAL)) {
		expression();
		emitBytes(setOp, (uint8_t)arg);
	} else emitBytes(getOp, (uint8_t)arg);
}

static void variable(bool canAssign) {
	namedVariable(parser.previous, canAssign);
}

static Token syntheticToken(const char* text) {
	Token token;
	token.start = text;
	token.length = (int)strlen(text);
	return token;
}

static void pushSuperclass() {
	if (currentClass == NULL) return;
	namedVariable(syntheticToken("super"), false);
}

static void super_(bool canAssign) {
	if (currentClass == NULL) {
		error("Cannot use 'super' outside of a class.");
	} else if (!currentClass->hasSuperclass) {
		error("Cannot use 'super' in a class with no superclass.");
	}

	consume(TOKEN_DOT, "Expect '.' after 'super'.");
	consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
	uint8_t name = identifierConstant(&parser.previous);

		namedVariable(syntheticToken("this"), false);

	if (match(TOKEN_LEFT_PAREN)) {
		uint8_t argCount = argumentList();

		pushSuperclass();
		emitBytes(OP_SUPER, argCount);
		emitByte(name);
	} else {
		pushSuperclass();
		emitBytes(OP_GET_SUPER, name);
	}
}

static void this_(bool canAssign) {
	if (currentClass == NULL) error("Cannot use 'this' outside of a class.");
	else variable(false);
}

static void unary(bool canAssign) {
	TokenType operatorType = parser.previous.type;

	parsePrecedence(PREC_UNARY);

	switch (operatorType) {
		case TOKEN_BANG: emitByte(OP_NOT); break;
		case TOKEN_MINUS: emitByte(OP_NEGATE); break;
		default:
			return;
	}
}

ParseRule rules[] = {
/* Compiling Expressions rules < Calls and Functions infix-left-paren
	{ grouping, NULL,    PREC_NONE },       */
	{ grouping, call,    PREC_CALL },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       /* Compiling Expressions rules < Classes and Instances not-yet
	{ NULL,     NULL,    PREC_NONE },       */
	{ NULL,     dot,     PREC_CALL },         { unary,    binary,  PREC_TERM },         { NULL,     binary,  PREC_TERM },         { NULL,     NULL,    PREC_NONE },         { NULL,     binary,  PREC_FACTOR },       { NULL,     binary,  PREC_FACTOR },     /* Compiling Expressions rules < Types of Values table-not
	{ NULL,     NULL,    PREC_NONE },       */
	{ unary,    NULL,    PREC_NONE },       /* Compiling Expressions rules < Types of Values table-equal
	{ NULL,     NULL,    PREC_NONE },       */
	{ NULL,     binary,  PREC_EQUALITY },     { NULL,     NULL,    PREC_NONE },       /* Compiling Expressions rules < Types of Values table-comparisons
	{ NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       */
	{ NULL,     binary,  PREC_EQUALITY },     { NULL,     binary,  PREC_COMPARISON },   { NULL,     binary,  PREC_COMPARISON },   { NULL,     binary,  PREC_COMPARISON },   { NULL,     binary,  PREC_COMPARISON }, /* Compiling Expressions rules < Global Variables table-identifier
	{ NULL,     NULL,    PREC_NONE },       */
	{ variable, NULL,    PREC_NONE },       /* Compiling Expressions rules < Strings table-string
	{ NULL,     NULL,    PREC_NONE },       */
	{ string,   NULL,    PREC_NONE },         { number,   NULL,    PREC_NONE },       /* Compiling Expressions rules < Jumping Back and Forth table-and
	{ NULL,     NULL,    PREC_NONE },       */
	{ NULL,     and_,    PREC_AND },          { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       /* Compiling Expressions rules < Types of Values table-false
	{ NULL,     NULL,    PREC_NONE },       */
	{ literal,  NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       /* Compiling Expressions rules < Types of Values table-nil
	{ NULL,     NULL,    PREC_NONE },       */
	{ literal,  NULL,    PREC_NONE },       /* Compiling Expressions rules < Jumping Back and Forth table-or
	{ NULL,     NULL,    PREC_NONE },       */
	{ NULL,     or_,     PREC_OR },           { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       /* Compiling Expressions rules < Superclasses not-yet
	{ NULL,     NULL,    PREC_NONE },       */
	{ super_,   NULL,    PREC_NONE },       /* Compiling Expressions rules < Methods and Initializers not-yet
	{ NULL,     NULL,    PREC_NONE },       */
	{ this_,    NULL,    PREC_NONE },       /* Compiling Expressions rules < Types of Values table-true
	{ NULL,     NULL,    PREC_NONE },       */
	{ literal,  NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },         { NULL,     NULL,    PREC_NONE },       };

static void parsePrecedence(Precedence precedence) {
	advance();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;
	if (prefixRule == NULL) {
		error("Expect expression.");
		return;
	}

	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(canAssign);

	while (precedence <= getRule(parser.current.type)->precedence) {
		advance();
		ParseFn infixRule = getRule(parser.previous.type)->infix;
		infixRule(canAssign);
	}

	if (canAssign && match(TOKEN_EQUAL)) {
		error("Invalid assignment target.");
		expression();
	}
}

static ParseRule* getRule(TokenType type) {
	return &rules[type];
}

void expression() {
	parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		declaration();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
	Compiler compiler;
	initCompiler(&compiler, type);
	beginScope(); 
	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
	
	if (!check(TOKEN_RIGHT_PAREN)) do {
		current->function->arity++;
		if (current->function->arity > 255) errorAtCurrent("Cannot have more than 255 parameters.");
		
		uint8_t paramConstant = parseVariable("Expect parameter name.");
		defineVariable(paramConstant);
	} while (match(TOKEN_COMMA));
	
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	
	block();
	ObjFunction* function = endCompiler();
	emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

	for (int i = 0; i < function->upvalueCount; i++) {
		emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
		emitByte(compiler.upvalues[i].index);
	}
}

static void method() {
	consume(TOKEN_IDENTIFIER, "Expect method name.");
	uint8_t constant = identifierConstant(&parser.previous);

		FunctionType type = TYPE_METHOD;
	if (parser.previous.length == 4 && memcmp(parser.previous.start, "init", 4) == 0) 
		type = TYPE_INITIALIZER;

	function(type);

	emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
	consume(TOKEN_IDENTIFIER, "Expect class name.");
	Token className = parser.previous;
	uint8_t nameConstant = identifierConstant(&parser.previous);
	declareVariable();

	emitBytes(OP_CLASS, nameConstant);
	defineVariable(nameConstant);

	ClassCompiler classCompiler;
	classCompiler.name = parser.previous;
	classCompiler.hasSuperclass = false;
	classCompiler.enclosing = currentClass;
	currentClass = &classCompiler;

	if (match(TOKEN_LESS)) {
		consume(TOKEN_IDENTIFIER, "Expect superclass name.");

		if (identifiersEqual(&className, &parser.previous))
			error("A class cannot inherit from itself.");

		classCompiler.hasSuperclass = true;

		beginScope();
		
		variable(false);
		addLocal(syntheticToken("super"));
		defineVariable(0);

		namedVariable(className, false);
		emitByte(OP_INHERIT);
	}

	consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		namedVariable(className, false);
		method();
	}
	consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

	if (classCompiler.hasSuperclass) endScope();

	currentClass = currentClass->enclosing;
}

static void funDeclaration() {
	uint8_t global = parseVariable("Expect function name.");
	markInitialized();
	function(TYPE_FUNCTION);
	defineVariable(global);
}

static void varDeclaration() {
	uint8_t global = parseVariable("Expect variable name.");

	if (match(TOKEN_EQUAL)) expression();
	else emitByte(OP_NIL);

	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

	defineVariable(global);
}

static void expressionStatement() {
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	emitByte(OP_POP);
}

static void forStatement() {
	beginScope();

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
	if (match(TOKEN_VAR)) varDeclaration();
	else if (match(TOKEN_SEMICOLON));
	else expressionStatement();

	int loopStart = currentChunk()->count;
	int exitJump = -1;
	
	if (!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

		exitJump = emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);
	}

	if (!match(TOKEN_RIGHT_PAREN)) {
		int bodyJump = emitJump(OP_JUMP);

		int incrementStart = currentChunk()->count;
		expression();
		emitByte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

		emitLoop(loopStart);
		loopStart = incrementStart;
		patchJump(bodyJump);
	}

	statement();

	emitLoop(loopStart);

	if (exitJump != -1) {
		patchJump(exitJump);
		emitByte(OP_POP);   }

	endScope();
}
static void ifStatement() {
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition."); 
	int thenJump = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	statement();

	int elseJump = emitJump(OP_JUMP);

	patchJump(thenJump);
	emitByte(OP_POP);

	if (match(TOKEN_ELSE)) statement();
	patchJump(elseJump);
}

static void printStatement() {
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emitByte(OP_PRINT);
}

static void importStatement() {
	consume(TOKEN_STRING, "Expect string after import.");
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
    consume(TOKEN_SEMICOLON, "Expect ';' after import.");

    emitByte(OP_IMPORT);
}

static void returnStatement() {
	if (current->type == TYPE_SCRIPT) error("Cannot return from top-level code.");

	if (match(TOKEN_SEMICOLON)) emitReturn();
	else {
		if (current->type == TYPE_INITIALIZER) error("Cannot return a value from an initializer.");

		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
		emitByte(OP_RETURN);
	}
}

static void whileStatement() {
	int loopStart = currentChunk()->count;

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	int exitJump = emitJump(OP_JUMP_IF_FALSE);

	emitByte(OP_POP);
	statement();

	emitLoop(loopStart);

	patchJump(exitJump);
	emitByte(OP_POP);
}

static void synchronize() {
	parser.panicMode = false;

	while (parser.current.type != TOKEN_EOF) {
		if (parser.previous.type == TOKEN_SEMICOLON) return;

		switch (parser.current.type) {
			case TOKEN_CLASS:
			case TOKEN_FUN:
			case TOKEN_VAR:
			case TOKEN_FOR:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_PRINT:
			case TOKEN_IMPORT:
			case TOKEN_RETURN:
				return;

			default:
								;
		}

		advance();
	}
}

static void declaration() {
	if (match(TOKEN_CLASS)) classDeclaration();
	else if (match(TOKEN_FUN)) funDeclaration();
	else if (match(TOKEN_VAR)) varDeclaration();
	else statement();

	if (parser.panicMode) synchronize();
}

static void statement() {
	if (match(TOKEN_PRINT)) printStatement();
	else if (match(TOKEN_FOR)) forStatement();
	else if (match(TOKEN_IF)) ifStatement();
	else if (match(TOKEN_IMPORT)) importStatement();
	else if (match(TOKEN_RETURN)) returnStatement();
	else if (match(TOKEN_WHILE)) whileStatement();
	else if (match(TOKEN_LEFT_BRACE)) {
		beginScope();
		block();
		endScope();
	} else expressionStatement();
}

ObjFunction* compile(const char* source) {
	initScanner(source);
	Compiler compiler;
	initCompiler(&compiler, TYPE_SCRIPT);

	parser.hadError = false;
	parser.panicMode = false;

	advance();

	while (!match(TOKEN_EOF)) {
		declaration();
	}

	ObjFunction* function = endCompiler();
	return parser.hadError ? NULL : function;
}

void grayCompilerRoots() {
	Compiler* compiler = current;
	while (compiler != NULL) {
		grayObject((Obj*)compiler->function);
		compiler = compiler->enclosing;
	}
}
