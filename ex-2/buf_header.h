
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* buf_header.h */

#ifndef BUF_HEADER_H
#define BUF_HEADER_H

#include <stdbool.h>
#include <assert.h>

#define NHASH           4

#define STAT_LOCKED     0x00000001      /* バッファはロックされている */
#define STAT_VALID      0x00000002      /* バッファは有効なデータを保持している */
#define STAT_DWR        0x00000004      /* 遅延書込みが必要である */
#define STAT_KRDWR      0x00000008      /* カーネルが入出力している */
#define STAT_WAITED     0x00000010      /* ロックが解除されるのを他のプロセスが待機している */
#define STAT_OLD        0x00000020      /* バッファが保持しているデータが古くなっている */

/*
 * バッファヘッダ
 */
struct buf_header {
    int                 blkno;          /* 論理ブロック番号 */
    struct buf_header*  hash_fp;        /* ハッシュリストの順方向ポインタ */
    struct buf_header*  hash_bp;        /* ハッシュリストの逆方向ポインタ */
    unsigned int        stat;           /* バッファの状態 */
    struct buf_header*  free_fp;        /* フリーリストの順方向ポインタ */
    struct buf_header*  free_bp;        /* フリーリストの逆方向ポインタ */
    char*               cache_data;     /* キャッシュデータ領域へのポインタ */
};

/*
 * 論理ブロック番号に対応するバッファを, ハッシュリストから検索
 */
struct buf_header* hash_search(int blkno);

/*
 * 論理ブロック番号に対応するバッファを取得
 */
struct buf_header* getblk(int blkno);

/*
 * 論理ブロック番号に対応するバッファを解放
 */
void brelse(struct buf_header* buffer);

/*
 * バッファの状態を表示
 */
void print_buf_header(int buffer_no, const struct buf_header* buffer);

/*
 * 外部変数
 */
extern struct buf_header hash_head[NHASH];
extern struct buf_header free_head;

/*
 * 論理ブロック番号(blkno)に対応するハッシュ値の計算
 */
static inline int hash(int blkno)
{
    return blkno % 4;
}

/*
 * ハッシュリストの初期化
 */
static inline void init_hash(struct buf_header* h)
{
    h->hash_fp = h;
    h->hash_bp = h;
}

/*
 * フリーリストの初期化
 */
static inline void init_free_list(struct buf_header* h)
{
    h->free_fp = h;
    h->free_bp = h;
}

/*
 * ハッシュリストの先頭にバッファを追加
 */
static inline void insert_head(struct buf_header* h, struct buf_header* p)
{
    assert(p != NULL);

    p->hash_fp = h->hash_fp;
    p->hash_bp = h;
    h->hash_fp->hash_bp = p;
    h->hash_fp = p;

    return;
}

/*
 * ハッシュリストの末尾にバッファを追加
 */
static inline void insert_tail(struct buf_header* h, struct buf_header* p)
{
    assert(p != NULL);

    p->hash_fp = h;
    p->hash_bp = h->hash_bp;
    h->hash_bp->hash_fp = p;
    h->hash_bp = p;

    return;
}

/*
 * フリーリストの先頭にバッファを追加
 */
static inline void insert_free_list_head(struct buf_header* h, struct buf_header* p)
{
    assert(p != NULL);

    p->free_fp = h->free_fp;
    p->free_bp = h;
    h->free_fp->free_bp = p;
    h->free_fp = p;

    return;
}

/*
 * フリーリストの末尾にバッファを追加
 */
static inline void insert_free_list_tail(struct buf_header* h, struct buf_header* p)
{
    assert(p != NULL);

    p->free_fp = h;
    p->free_bp = h->free_bp;
    h->free_bp->free_fp = p;
    h->free_bp = p;

    return;
}

/*
 * ハッシュリストが空かどうかを判定
 */
static inline bool is_hash_list_empty(struct buf_header* h)
{
    return h->hash_fp == h;
}

/*
 * フリーリストが空かどうかを判定
 */
static inline bool is_free_list_empty(struct buf_header* h)
{
    return h->free_fp == h;
}

/*
 * ハッシュリストから指定されたバッファを削除
 */
static inline void remove_from_hash(struct buf_header* p)
{
    assert(p != NULL);

    p->hash_fp->hash_bp = p->hash_bp;
    p->hash_bp->hash_fp = p->hash_fp;
    p->hash_fp = NULL;
    p->hash_bp = NULL;

    return;
}

/*
 * フリーリストから指定されたバッファを削除
 */
static inline void remove_from_free_list(struct buf_header* h)
{
    assert(h != NULL);

    h->free_fp->free_bp = h->free_bp;
    h->free_bp->free_fp = h->free_fp;
    h->free_fp = NULL;
    h->free_bp = NULL;

    return;
}

/*
 * フリーリストから先頭のバッファを削除して取得
 */
static inline struct buf_header* remove_from_free_list_head(struct buf_header* h)
{
    struct buf_header* p;

    if (is_free_list_empty(h))
        return NULL;

    p = h->free_fp;
    remove_from_free_list(p);

    return p;
}

/*
 * フリーリストから末尾のバッファを削除して取得
 */
static inline struct buf_header* remove_from_free_list_tail(struct buf_header* h)
{
    struct buf_header* p;

    if (is_free_list_empty(h))
        return NULL;

    p = h->free_bp;
    remove_from_free_list(p);

    return p;
}

#endif /* BUF_HEADER_H */

