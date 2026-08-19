#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAlpha(char c) {
    return (c>='a' && c<='z') ||
            (c>='A' && c<= 'Z') ||
            c == '_';
}

static bool isDigit(char c) {
    return c>='0' && c<='9';
}

static bool isAtEnd() {
    // current가 null이라면 true return.
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current ++;
    return scanner.current[-1];   
}

static char peek() {
    return *scanner.current; // 현재 문자 반환.
}

static char peekNext() {
    if(isAtEnd()) return '\0';
    // 혹시 문자열의 끝을 참조하게 있는 경우를 체크하기 위해 End point를 먼저 확인한다.
    return scanner.current[1]; // 포인터가 지금 가리키고 있는 위치의 1칸 뒤.
}

static bool match(const char expected) {
    if(isAtEnd()) return false;
    if(*scanner.current != expected) return false;

    scanner.current++; // 읽었으니 하나더 점프시킨다.
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
    // token 구조체에 들어가야 하는 변수들 값 할당해주고 넘긴다. 
}
static Token errorToken(const char* message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static void skipWhitespace() {
    for(;;) {
        char c = peek();
        switch (c) { // 확인
            case ' ' : 
            case '\r' :
            case '\t':
                advance(); // 공백은 건너뛰기. 
                break; // 루프 맨 위로 돌아가 다시 peek.
            case '\n':
                scanner.line++;
                advance();
                break;
            // Comments
            case '/':
                if(peekNext() == '/') {
                    // a comment goes until the end of the line.
                    while(peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default :
                return ;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type){
    if(scanner.current - scanner.start == start + length && 
        memcmp(scanner.start+start,rest,length)==0) {
            // 길이가 같아야 하고, 각 위치의 알파벳이 같아야 한다.
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (scanner.start[0]) { // current가 아닌 start로 판별. 
        case 'a': return checkKeyword(1,2,"nd",TOKEN_AND);
        case 'c': return checkKeyword(1,4,"lass",TOKEN_CLASS);
        case 'e': return checkKeyword(1,3,"lse",TOKEN_ELSE);
        case 'f': 
            if (scanner.current - scanner.start > 1) {
                switch(scanner.start[1]) {
                    case 'a' : return checkKeyword(2,3,"lse",TOKEN_FALSE);
                    case 'o' : return checkKeyword(2,1,"r",TOKEN_FOR);
                    case 'u' : return checkKeyword(2,1,"n",TOKEN_FUN);
                }
            }
            break;
        case 'i': return checkKeyword(1,1,"f",TOKEN_IF);
        case 'n': return checkKeyword(1,2,"il",TOKEN_NIL);
        case 'o': return checkKeyword(1,1,"r",TOKEN_OR);
        case 'p': return checkKeyword(1,4,"rint",TOKEN_PRINT);
        case 'r': return checkKeyword(1,5,"eturn",TOKEN_RETURN);
        case 's': return checkKeyword(1,4,"uper",TOKEN_SUPER);
        case 't': 
            if(scanner.current - scanner.start > 1 ) {
                switch(scanner.start[1]) {
                    case 'h' : return checkKeyword(2,2,"is",TOKEN_THIS);
                    case 'r' : return checkKeyword(2,2,"ue",TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1,2,"ar",TOKEN_VAR);
        case 'w': return checkKeyword(1,4,"hile",TOKEN_WHILE);
    }   

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // 첫번쨰 글자가 문자면 숫자도 식별자로 허용한다.
    while(isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

static Token number() {
    while (isDigit(peek())) advance();
    // Look for a fractional part.
    if(peek() == "." && isDigit(peekNext())) {
        // consume the ".".
        advance();

        while(isDigit(peek())) advance();
    }
    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    // 여기서 while문 내부로 들어갈 수가 있나?
    // 앞서 scanToken에서 advance를 하면서 한 글자를 먹는다.
    // 그러니 여기선 " 다음 글자를 포인팅하고 있다.
    while (peek() != '"' && !isAtEnd()) {
        if(peek() == '\n') scanner.line++;
        advance();
    }
    if (isAtEnd()) return errorToken("Unterminated String.");

    // The closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipWhitespace(); // 소스코드의 공백 전체를 없앤다. 실제 토큰을 스캔하는 코드는 이 함수를 나오는 지점부터 시작.

    scanner.start = scanner.current;

    if(isAtEnd()) return makeToken(TOKEN_EOF);
    char c = advance(); // read the next character from the source code.

    if(isAlpha(c)) return identifier();
    if(isDigit(c)) return number();
    switch (c){
        // Single Characters.
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
            );
        case '<' :
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
            );
        case '>' :
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
            );
        case '"' : return string();
    }

    return errorToken("Unexpected character.");
}