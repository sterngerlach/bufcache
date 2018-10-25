
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buf_header.h"
#include "command.h"

int main(int argc, char** argv)
{
    char* input;
    char** tokens;
    size_t token_num;
    bool exit_app;
    /* int i; */
    
    /* 警告の抑制 */
    (void)argc;
    (void)argv;

    /* バッファは初期化されていない */
    is_buf_header_initialized = false;

    while (1) {
        printf("$ ");
        
        /* ユーザ入力の取得 */
        if ((input = get_input()) == NULL)
            break;
        
        /* 入力を分割 */
        if (!tokenize_input(input, &tokens, &token_num))
            break;
        
        /* コマンドを実行 */
        execute_command(tokens, token_num, &exit_app);

        free(tokens);
        free(input);

        if (exit_app)
            break;
    }

    putchar('\n');

    return EXIT_SUCCESS;
}

