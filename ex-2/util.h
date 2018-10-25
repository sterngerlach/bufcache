
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* util.h */

#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdbool.h>

/*
 * 文字列のカラー出力
 */
#define ANSI_ESCAPE_COLOR_BLACK     "\033[0;30m"
#define ANSI_ESCAPE_COLOR_RED       "\033[1;31m"
#define ANSI_ESCAPE_COLOR_GREEN     "\033[1;32m"
#define ANSI_ESCAPE_COLOR_YELLOW    "\033[1;33m"
#define ANSI_ESCAPE_COLOR_BLUE      "\033[1;34m"
#define ANSI_ESCAPE_COLOR_MAGENTA   "\033[1;35m"
#define ANSI_ESCAPE_COLOR_CYAN      "\033[1;36m"
#define ANSI_ESCAPE_COLOR_GRAY      "\033[1;37m"
#define ANSI_ESCAPE_COLOR_RESET     "\033[0m"

/*
 * 文字列(10進数の整数表現)を整数に変換
 */
bool strict_strtol(const char* nptr, long* valptr);

/*
 * メッセージを表示
 */
void print_message(const char* function, const char* format, ...);

/*
 * エラーメッセージを表示
 */
void print_error(const char* function, const char* format, ...);

#endif /* UTIL_H */

