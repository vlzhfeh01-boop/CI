#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() { // 대화형 모드
    char line[1024];
    for(;;){
        printf("> ");
        //fgets : 키보드에서 한 줄을 읽어 line 배열에 저장한다.
        if(!fgets(line, sizeof(line),stdin)) {
            printf("\n");
            break;
        }
        interpret(line);
    }
}
// OS에서 자주 사용하는 트릭이다. 크기가 큰 소스코드를 읽고 싶을때 이런 방식 활용.
// seek -> tell , size 확인. 이후 malloc으로 size만큼 할당. 배열의 마지막에 \n 으로 마지막임을 표시.
static char* readFile(const char* path) {
    FILE* file = fopen(path,"rb");
    if(file == NULL) {
        fprintf(stderr, "Could not openfile \"%s\".\n", path);
        exit(74);
    }

    fseek(file,0L,SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file); // file stream의 처음으로 되돌아간다.

    char* buffer = (char*)malloc(fileSize+1);
    if(buffer == NULL) {
        fprintf(stderr, "Not enought memory to read \"%s\".\n",path);
        exit(74);
    }
    // fileSize만큼의 바이트를 읽어서 buffer에 채워 넣고, 실제로 읽은 바이트 수 반환.
    size_t bytesRead = fread(buffer, sizeof(char),fileSize,file);
    if(bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n",path);
        exit(74);
    }
    buffer[bytesRead] = '\0'; // NULL 종료 문자. 

    fclose(file);
    return buffer;
}
// 대화형모드가 아닌 파일 path를 전달했을 경우
static void runFile(const char* path) {
    char* source = readFile(path); // 마지막에 \0을 채워넣는다.
    InterpretResult result = interpret(source);
    free(source);

    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    initVM(); // VM module init.
    
    if(argc == 1) { // 인자가 없으면 대화형 repl 모드
        repl();
    } else if (argc==2) // 인자가 하나면 그 파일을 스크립트로 실행
    {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
    }
    
    freeVM();

    return 0;
}