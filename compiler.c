#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode; // true면 다른 에러가 발생하는 것을 억제한다.
}   Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREc_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
} Precedence;
//  처음 선언부터 아래로 내려갈수록 우선순위가 높아진다. 
// unary가 + 보다 우선순위가 높으니깐, -a.b + c 의 경우에는 -a.b 까지 수행되고 + 앞에서 멈춘다.

Parser parser;

Chunk* compilingChunk;

//미래의 변경에 대비한 간접 참조 패턴이다.
// 컴파일러 코드 안에서 Chunk에 접근해야 하는 곳이 한 군데가 아니다.
// 나중에는 현재 컴파일 중인 함수의 청크 처럼, 더 복잡한 로직으로 접근해야 한다.
// 이렇게 함수로 바꿔놓으면, 일일이 접근하던 코드를 묶어서 고치면 된다.
static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    if(parser.panicMode) return; // 먼저 앞선 에러가 있었는지 확인. 
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error",token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'",token->length, token->start);
    }

    fprintf(stderr, ": %s\n",message);
    parser.hadError = true;
}

// 왜 같은 내용을 이름 바꿔서 두개 선언하는 거지?
static void error(const char* message) {
    errorAt(&parser.current, message);
}
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;
    // 여러개의 에러 토큰을 뱉어내는 상황 처리.
    // 어디에 레어가 있는지 계속 보고한다.
    // 파서의 나머지 부분이 에러 토큰의 존재를 절대 신경 쓰지 않아도 되도록 전부 흡수.
    for(;;) {
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    return errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(),byte,parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn(){
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    // constant는 index를 받는다.
    int constant = addConstant(currentChunk(), value); 
    if(constant > UINT8_MAX) { // 하나의 chunk에 최대 256개까지만 받는다.
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    // pushed the instruction onto the stack at runtime.
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
}
// Parentheses
static void grouping() {
    expression();
    // 이 함수 내부에 들어온 이산 left paren이 읽혔다는 걸 의미. 
    // 자동으로 ) 가 필요함을 알 수 있다.
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}
// literal
static void number() {
    // strtod : convert a string to a floating point number.
    // token for the has already been consumed and is stored in previou
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // minus를 따로 저장해놨다가 expression operand compile 이후에
    // 실행지점에서 negate를 수행.
    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        
    default:
        return; // Unreachable.
    }
}

static void parsePrecedence(Precedence precedence) {

}

static void expression(){
    parsePrecedence(PREC_ASSIGNMENT); // parse the lowest precedence level.
}

bool compile(const char* source, Chunk* chunk){
    initScanner(source);
    compilingChunk = chunk; // compile에 사용될 chunk를 전역 변수로 넘긴다.

    // init variables.
    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF,"Expect end of expression.");
    endCompiler();
    return !parser.hadError; // error가 있으면 false를 반환한다.
}