
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* command.h */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

/*
 * バッファが初期化されたかどうか
 */
extern bool is_buf_header_initialized;

/*
 * ユーザの入力文字列を取得
 */
char* get_input();

/*
 * 入力文字列を分割
 */
bool tokenize_input(char* input, char*** args, size_t* argc);

/*
 * コマンドを実行
 */
void execute_command(char** args, size_t argc, bool* exit_app);

/*
 * 各種コマンド群
 */
void command_help(char** args, size_t argc, bool* exit_app);
void command_init(char** args, size_t argc, bool* exit_app);
void command_buf(char** args, size_t argc, bool* exit_app);
void command_hash(char** args, size_t argc, bool* exit_app);
void command_free(char** args, size_t argc, bool* exit_app);
void command_getblk(char** args, size_t argc, bool* exit_app);
void command_brelse(char** args, size_t argc, bool* exit_app);
void command_set(char** args, size_t argc, bool* exit_app);
void command_reset(char** args, size_t argc, bool* exit_app);
void command_quit(char** args, size_t argc, bool* exit_app);

#endif /* COMMAND_H */

