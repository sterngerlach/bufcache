
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* command.c */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "buf_header.h"
#include "util.h"

#define INITIAL_TOKEN_CAPACITY      2
#define INCREMENT_TOKEN_CAPACITY    2

/*
 * コマンドの型定義
 */
typedef void (*command_func_t)(char**, size_t, bool*);

struct command_table {
    char*           name;   /* コマンド名 */
    command_func_t  func;   /* コマンドを処理する関数へのポインタ */
};

/*
 * 変数宣言
 */
struct command_table commands[] = {
    { "help",       command_help    },
    { "init",       command_init    },
    { "buf",        command_buf     },
    { "hash",       command_hash    },
    { "free",       command_free    },
    { "getblk",     command_getblk  },
    { "brelse",     command_brelse  },
    { "set",        command_set     },
    { "reset",      command_reset   },
    { "quit",       command_quit    },
    { NULL,		    NULL            },
};

/*
 * バッファヘッダ
 */
#define BUFFER_HEADER_NUM           12
struct buf_header buf_headers[BUFFER_HEADER_NUM];

/*
 * バッファが初期化されたかどうか
 */
bool is_buf_header_initialized;

/*
 * 指定されたバッファに対応するバッファ番号を取得
 * bufferにNULLが指定された, あるいは指定されたバッファが
 * 配列buf_headersに見つからなかった場合は-1を返す
 */
int get_buffer_no_from_buf_header(const struct buf_header* buffer)
{
    int i = 0;

    if (buffer == NULL)
        return -1;
    
    for (i = 0; i < BUFFER_HEADER_NUM; ++i)
        if (buf_headers[i].blkno == buffer->blkno)
            return i;

    /* 指定されたバッファが見つからなかった */
    return -1;
}

/*
 * ユーザの入力文字列を取得
 */
char* get_input()
{
    char* input = NULL;
    size_t length = 0;
    ssize_t cc = 0;
    
    if ((cc = getline(&input, &length, stdin)) == -1)
    	return NULL;

    return input;
}

/*
 * 入力文字列を分割
 */
bool tokenize_input(char* input, char*** args, size_t* argc)
{
    size_t i;
    size_t len = strlen(input);
    size_t capacity = INITIAL_TOKEN_CAPACITY;
    size_t begin = 0;
    size_t token_num = 0;

    char** tokens = NULL;
    char** new_tokens = NULL;

    tokens = (char**)calloc(capacity, sizeof(char*));
    
    if (tokens == NULL) {
        print_error("tokenize_input", "calloc() failed\n");
        return false;
    }

    /* 最初の空白を読み飛ばす */
    for (i = 0; i < len; ++i) {
        if (!isspace(input[i])) {
            begin = i;
            break;
        }
    }

    for (; i < len; ++i) {
        /* この方法は, getline()関数が返す文字列の末尾に区切り文字(改行文字)が
         * あるため動作する(改行文字などの, isspace()関数が真を返すような文字が
         * 末尾になければ, 入力の最後の引数が取り出せない) */
        if (isspace(input[i])) {
            input[i] = '\0';
            tokens[token_num] = input + begin;
            ++token_num;

            /* トークンの配列が足りない場合は配列を拡張 */
            if (token_num >= capacity) {
                capacity += INCREMENT_TOKEN_CAPACITY;
                new_tokens = (char**)realloc(tokens, sizeof(char*) * capacity);

                if (new_tokens == NULL) {
                    print_error("tokenize_input", "realloc() failed\n");
                    free(tokens);
                    return false;
                }

                tokens = new_tokens;
            }

            /* 連続した空白を読み飛ばす */
            for (++i; i < len; ++i) {
                if (!isspace(input[i])) {
                    begin = i;
                    break;
                }
            }
        }
    }

    tokens[token_num] = NULL;

    *args = tokens;
    *argc = token_num;

    return true;
}

/*
 * 指定されたコマンドを探索
 */
command_func_t search_command(char* name)
{
    struct command_table* command;
    
    for (command = commands; command->name != NULL; ++command)
        if (!strcmp(command->name, name))
            return command->func;

    return NULL;
}

/*
 * コマンドを実行
 */
void execute_command(char** args, size_t argc, bool* exit_app)
{
    command_func_t func;

    *exit_app = false;

    if (argc == 0) {
        /* print_error("execute_command",
                    "you should enter at least 1 argument\n"); */
        return;
    }

    /* コマンドを探索 */
    if ((func = search_command(args[0])) == NULL) {
        print_error("execute_command",
                    "no such command \'%s\'\n", args[0]);
        return;
    }

    /* コマンドを実行 */
    (*func)(args, argc, exit_app);
}

/*
 * helpコマンド
 */
void command_help(char** args, size_t argc, bool* exit_app)
{
    /* アプリケーションは終了させない */
    *exit_app = false;

    /* パラメータを無視(警告の抑止) */
    (void)args;

    /* 引数のチェック */
    if (argc > 1) {
        print_error("command_help",
                    "help command takes no arguments but %zu was given\n",
                    argc - 1);
        return;
    }

    printf("bufcache help\n"
           "usage: command [arguments]...\n\n"
           "built-in commands:\n"
           "help                       "
           "show help\n"
           "init                       "
           "initialize hash list and free list\n"
           "buf [n ...]                "
           "show all buffer statuses (default, if no parameter is given)\n"
           "                           "
           "show statuses of buffer n (if parameter n is provided)\n"
           "hash [n ...]               "
           "show all hash list statuses (default, if no parameter is given)\n"
           "                           "
           "show statuses of hash list for hash value n (if parameter n is provided)\n"
           "free                       "
           "show free list\n"
           "getblk n                   "
           "call getblk() with block number n\n"
           "brelse n                   "
           "call brelse() with the pointer to the buffer n\n"
           "set n stat [stat ...]      "
           "set status of buffer n\n"
           "reset n stat [stat ...]    "
           "reset status of buffer n\n"
           "quit                       "
           "quit application\n");
}

/*
 * initコマンド
 */
void command_init(char** args, size_t argc, bool* exit_app)
{
    static const int init_blkno[] = { 28, 4, 64, 17, 5, 97,
                                      98, 50, 10, 3, 35, 99 };
    static const int init_blkno_free[] = { 3, 5, 4, 28, 97, 10 };

    size_t i;
    int h;
    struct buf_header* p;

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* パラメータを無視(警告の抑止) */
    (void)args;

    /* 引数のチェック */
    if (argc > 1) {
        print_error("command_init",
                    "init command takes no arguments but %zu were given\n",
                    argc - 1);
        return;
    }

    print_message("command_init", "initializing hash list and free list ... \n");
    
    /* 全てのバッファヘッダを初期化 */
    for (i = 0; i < BUFFER_HEADER_NUM; ++i) {
        buf_headers[i].blkno = init_blkno[i];
        buf_headers[i].stat = STAT_VALID | STAT_LOCKED;
        buf_headers[i].hash_fp = NULL;
        buf_headers[i].hash_bp = NULL;
        buf_headers[i].free_fp = NULL;
        buf_headers[i].free_bp = NULL;
        buf_headers[i].cache_data = NULL;
    }

    /* ハッシュリストの初期化 */
    for (i = 0; i < NHASH; ++i)
        init_hash(&hash_head[i]);

    /* フリーリストの初期化 */
    init_free_list(&free_head);

    /* ハッシュリストにバッファヘッダを追加 */
    for (i = 0; i < BUFFER_HEADER_NUM; ++i) {
        h = hash(buf_headers[i].blkno);
        insert_tail(&hash_head[h], &buf_headers[i]);
    }

    /* フリーリストにバッファヘッダを追加 */
    for (i = 0; i < sizeof(init_blkno_free) / sizeof(init_blkno_free[0]); ++i) {
        p = hash_search(init_blkno_free[i]);
        assert(p != NULL);
        p->stat &= ~STAT_LOCKED;
        insert_free_list_tail(&free_head, p);
    }

    /* バッファは初期化済み */
    is_buf_header_initialized = true;

    print_message("command_init", "done\n");
}

/*
 * bufコマンド
 */
void command_buf(char** args, size_t argc, bool* exit_app)
{
    size_t i;
    long val;
    bool filter[BUFFER_HEADER_NUM];

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_buf",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;

    if (argc > 0) {
        /* バッファ番号が指定された場合 */
        for (i = 0; i < BUFFER_HEADER_NUM; ++i)
            filter[i] = false;

        /* 指定された引数をチェック */
        for (i = 0; i < argc; ++i) {
            /* 引数が整数でない場合はエラー */
            if (!strict_strtol(args[i], &val))
                return;

            /* 引数が範囲外である場合はエラー */
            if (val < 0 || val >= BUFFER_HEADER_NUM) {
                print_error("command_buf",
                            "buffer number %ld is out of range: [%d ... %d]\n",
                            val, 0, BUFFER_HEADER_NUM - 1);
                return;
            }

            /* バッファ番号が重複して指定されていてもよい */
            filter[val] = true;
        }
    } else {
        /* バッファ番号が指定されない場合は, 全てのバッファの状態を表示 */
        for (i = 0; i < BUFFER_HEADER_NUM; ++i)
            filter[i] = true;
    }

    print_message("command_buf", "current buffer header statuses:\n");

    /* バッファの状態を表示 */
    for (i = 0; i < BUFFER_HEADER_NUM; ++i) {
        if (filter[i]) {
            print_buf_header(i, &buf_headers[i]);
            putchar('\n');
        }
    }
}

/*
 * hashコマンド
 */
void command_hash(char** args, size_t argc, bool* exit_app)
{
    size_t i;
    long val;
    bool filter[NHASH];
    struct buf_header* p;

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_hash",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }


    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;

    if (argc > 0) {
        /* ハッシュ値が指定された場合 */
        for (i = 0; i < NHASH; ++i)
            filter[i] = false;

        /* 指定された引数をチェック */
        for (i = 0; i < argc; ++i) {
            /* 引数が整数でない場合はエラー */
            if (!strict_strtol(args[i], &val))
                return;

            /* 引数が範囲外である場合はエラー */
            if (val < 0 || val >= NHASH) {
                print_error("command_hash",
                            "hash value %ld is out of range: [%d ... %d]\n",
                            val, 0, NHASH - 1);
                return;
            }

            /* ハッシュ値が重複して指定されていてもよい */
            filter[val] = true;
        }
    } else {
        /* ハッシュ値が指定されない場合は, 全てのハッシュリストを表示 */
        for (i = 0; i < NHASH; ++i)
            filter[i] = true;
    }

    print_message("command_hash", "hash list:\n");

    for (i = 0; i < NHASH; ++i) {
        if (filter[i]) {
            /* ハッシュ値に対応するハッシュリストを表示 */
            printf("%zu: ", i);

            if (is_hash_list_empty(&hash_head[i])) {
                puts("empty");
            } else {
                for (p = hash_head[i].hash_fp; p != &hash_head[i]; p = p->hash_fp) {
                    print_buf_header(get_buffer_no_from_buf_header(p), p);
                    putchar(' ');
                }
            }

            putchar('\n');
        }
    }
}

/*
 * freeコマンド
 */
void command_free(char** args, size_t argc, bool* exit_app)
{
    struct buf_header* p;

    /* パラメータの無視(警告の抑止) */
    (void)args;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_free",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数のチェック */
    if (argc > 1) {
        print_error("command_free",
                    "free command takes no arguments but %zu were given\n",
                    argc - 1);
        return;
    }

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* フリーリストを表示 */
    print_message("command_free", "all buffer headers of free list:\n");
    
    if (is_free_list_empty(&free_head)) {
        printf("empty");
    } else {
        for (p = free_head.free_fp; p != &free_head; p = p->free_fp) {
            print_buf_header(get_buffer_no_from_buf_header(p), p);
            putchar(' ');
        }
    }

    putchar('\n');
}

/*
 * getblkコマンド
 * getblk n
 * n: 論理ブロック番号(blkno)
 */
void command_getblk(char** args, size_t argc, bool* exit_app)
{
    long blkno;
    struct buf_header* p;

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_getblk",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;

    /* 引数のチェック */
    if (argc != 1) {
        print_error("command_getblk",
                    "getblk command takes 1 argument but %zu were given\n",
                    argc);
        return;
    }

    /* 引数(論理ブロック番号)が整数でない場合はエラー */
    if (!strict_strtol(args[0], &blkno))
        return;
    
    print_message("command_getblk",
                  "executing getblk() with blkno %d ...\n", (int)blkno);

    /* 論理ブロック番号に対応するバッファを取得 */
    p = getblk((int)blkno);

    print_message("command_getblk", "done\n");

    /* 結果を表示 */
    print_message("command_getblk", "result: ");

    if (p == NULL)
        printf("null");
    else
        print_buf_header(get_buffer_no_from_buf_header(p), p);

    putchar('\n');
}

/*
 * brelseコマンド
 * brelse n
 * n: 論理ブロック番号(blkno)
 */
void command_brelse(char** args, size_t argc, bool* exit_app)
{
    long blkno;
    struct buf_header* p;

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_brelse",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;

    /* 引数のチェック */
    if (argc != 1) {
        print_error("command_brelse",
                    "brelse command takes 1 argument but %zu were given\n",
                    argc);
        return;
    }

    /* 引数(論理ブロック番号)が整数でない場合はエラー */
    if (!strict_strtol(args[0], &blkno))
        return;
    
    /* 指定された論理ブロック番号に対応するバッファを探索 */
    p = hash_search((int)blkno);

    /* バッファが見つからなかった場合はエラー */
    if (p == NULL) {
        print_error("command_brelse",
                    "buffer (blkno: %d) was not found in the hash list %d\n",
                    (int)blkno, hash((int)blkno));
        return;
    }

    /* バッファがロックされていない場合はエラー */
    if (!(p->stat & STAT_LOCKED)) {
        print_error("command_brelse",
                    "buffer (blkno: %d) is not locked by the process\n",
                    (int)blkno);
        return;
    }

    print_message("command_brelse",
                  "executing brelse() with blkno %d ...\n", (int)blkno);

    /* バッファを解放 */
    brelse(p);

    print_message("command_brelse", "done\n");
}

/*
 * setコマンド
 * set n stat [stat ...]
 * n: 論理ブロック番号(blkno)
 * stat: バッファの状態
 */
void command_set(char** args, size_t argc, bool* exit_app)
{
    size_t i;
    long blkno;
    char stat;
    unsigned int new_stat;
    struct buf_header* p;

    print_message("command_set", 
                  "warning: this command may cause inconsistencies and various issues\n");

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_set",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;
    
    /* 引数の個数のチェック */
    if (argc < 2) {
        print_error("command_set",
                    "set command takes at least 2 arguments but only %zu were given\n",
                    argc);
        return;
    } else if (argc > 7) {
        print_error("command_set",
                    "set command takes at most 7 arguments but %zu were given\n",
                    argc);
        return;
    }

    /* 引数のチェック */
    /* 論理ブロック番号が整数でない場合はエラー */
    if (!strict_strtol(args[0], &blkno))
        return;
    
    /* 指定された論理ブロック番号に対応するバッファを探索 */
    p = hash_search((int)blkno);

    /* バッファが見つからなかった場合はエラー */
    if (p == NULL) {
        print_error("command_set",
                    "buffer (blkno: %d) was not found in the hash list %d\n",
                    (int)blkno, hash((int)blkno));
        return;
    }

    /* バッファに付加する新たな状態 */
    new_stat = 0;
    
    for (i = 1; i < argc; ++i) {
        /* 引数が1文字でない場合はエラー */
        if (strlen(args[i]) != 1) {
            print_error("command_set",
                        "invalid argument: %s\n", args[i]);
            return;
        }

        /* バッファに設定する状態の生成 */
        stat = (char)toupper(*args[i]);

        switch (stat) {
            case 'L':
                new_stat |= STAT_LOCKED;
                break;
            case 'V':
                new_stat |= STAT_VALID;
                break;
            case 'D':
                new_stat |= STAT_DWR;
                break;
            case 'K':
                new_stat |= STAT_KRDWR;
                break;
            case 'W':
                new_stat |= STAT_WAITED;
                break;
            case 'O':
                new_stat |= STAT_OLD;
                break;
            default:
                print_error("command_set",
                            "invalid argument: %s\n", args[i]);
                return;
        }
    }

    print_message("command_set",
                  "setting buffer header (blkno: %d) status ... \n", p->blkno);

    /* バッファに新たな状態を設定 */
    p->stat |= new_stat;

    print_message("command_set", "done\n");

    print_message("command_set",
                  "current buffer header (blkno: %d) status: ", p->blkno);
    print_buf_header(get_buffer_no_from_buf_header(p), p);
    putchar('\n');
}

/*
 * resetコマンド
 * reset n stat [stat ...]
 * n: 論理ブロック番号(blkno)
 * stat: バッファの状態
 */
void command_reset(char** args, size_t argc, bool* exit_app)
{
    size_t i;
    long blkno;
    char stat;
    unsigned int new_stat;
    struct buf_header* p;

    print_message("command_reset",
                  "warning: this command may cause inconsistencies and various issues\n");

    /* アプリケーションは終了させない */
    *exit_app = false;

    /* バッファが初期化されていない場合はエラー */
    if (!is_buf_header_initialized) {
        print_error("command_reset",
                    "buffer headers are not initialized. "
                    "You should call init command first\n");
        return;
    }

    /* 引数の操作(コマンド名の除去) */
    ++args;
    --argc;
    
    /* 引数の個数のチェック */
    if (argc < 2) {
        print_error("command_reset",
                    "set command takes at least 2 arguments but only %zu were given\n",
                    argc);
        return;
    } else if (argc > 7) {
        print_error("command_reset",
                    "set command takes at most 7 arguments but %zu were given\n",
                    argc);
        return;
    }

    /* 引数のチェック */
    /* 論理ブロック番号が整数でない場合はエラー */
    if (!strict_strtol(args[0], &blkno))
        return;
    
    /* 指定された論理ブロック番号に対応するバッファを探索 */
    p = hash_search((int)blkno);

    /* バッファが見つからなかった場合はエラー */
    if (p == NULL) {
        print_error("command_reset",
                    "buffer (blkno: %d) was not found in the hash list %d\n",
                    (int)blkno, hash((int)blkno));
        return;
    }

    /* バッファから外す設定 */
    new_stat = 0;
    
    for (i = 1; i < argc; ++i) {
        /* 引数が1文字でない場合はエラー */
        if (strlen(args[i]) != 1) {
            print_error("command_reset",
                        "invalid argument: %s\n", args[i]);
            return;
        }

        /* バッファに設定する状態の生成 */
        stat = (char)toupper(*args[i]);

        switch (stat) {
            case 'L':
                new_stat |= STAT_LOCKED;
                break;
            case 'V':
                new_stat |= STAT_VALID;
                break;
            case 'D':
                new_stat |= STAT_DWR;
                break;
            case 'K':
                new_stat |= STAT_KRDWR;
                break;
            case 'W':
                new_stat |= STAT_WAITED;
                break;
            case 'O':
                new_stat |= STAT_OLD;
                break;
            default:
                print_error("command_reset",
                            "invalid argument: %s\n", args[i]);
                return;
        }
    }

    print_message("command_reset",
                  "setting buffer header (blkno: %d) status ... \n", p->blkno);

    /* バッファに新たな状態を設定 */
    p->stat &= ~new_stat;

    print_message("command_reset", "done\n");

    print_message("command_reset", "current buffer header (blkno: %d) status: ", p->blkno);
    print_buf_header(get_buffer_no_from_buf_header(p), p);
    putchar('\n');

}

/*
 * quitコマンド
 */
void command_quit(char** args, size_t argc, bool* exit_app)
{
    /* パラメータの無視 */
    (void)args;

    /* 引数のチェック */
    if (argc > 1) {
        print_error("command_quit",
                    "quit command takes no arguments but %zu were given\n",
                    argc - 1);
        *exit_app = false;
        return;
    }

    puts("Bye-bye");
    
    /* アプリケーションを終了させる */
    *exit_app = true;
}

