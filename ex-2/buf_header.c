
/* 情報工学科3年 学籍番号61610117 杉浦 圭祐 */
/* buf_header.c */

#include <stdio.h>

#include "buf_header.h"
#include "util.h"

/*
 * 変数宣言
 */
struct buf_header hash_head[NHASH];
struct buf_header free_head;

/*
 * 論理ブロック番号に対応するバッファを, ハッシュリストから検索
 */
struct buf_header* hash_search(int blkno)
{
    int h;
    struct buf_header* p;

    h = hash(blkno);

    for (p = hash_head[h].hash_fp; p != &hash_head[h]; p = p->hash_fp)
        if (p->blkno == blkno)
            return p;

    return NULL;
}

/*
 * 論理ブロック番号に対応するバッファを取得
 */
struct buf_header* getblk(int blkno)
{
    int h;
    struct buf_header* p;
    struct buf_header* f;

    while (1) {
        if ((p = hash_search(blkno)) != NULL) {
            /* バッファが有効でない場合はプログラムを停止 */
            /* assert(p->stat & STAT_VALID); */

            if (p->stat & STAT_LOCKED) {
                /* シナリオ5 */
                /* 要求されたバッファはハッシュリストにあるが,
                 * そのバッファは他のプロセスによってロックされている */
                print_message("getblk", "executing case 5\n");
                print_message("getblk", 
                              "buffer (blkno: %d) is in the hash list %d "
                              "and is being locked by another process\n",
                              blkno, hash(blkno));
                
                /* バッファがフリーになるのを待っているプロセスが存在 */
                p->stat |= STAT_WAITED;

                /* sleep(); */
                print_message("getblk",
                              "sleep until buffer (blkno: %d) becomes free\n", blkno);
                return NULL;
            }
            
            /* シナリオ1 */
            /* 要求された論理ブロックのバッファがハッシュリストに存在し,
             * しかもそのバッファがフリーである(他のプロセスによってロックされていない */
             print_message("getblk", "executing case 1\n");
             print_message("getblk",
                           "buffer (blkno: %d) is in the hash list %d "
                           "and is not being locked by another process\n",
                           blkno, hash(blkno));
            
            /* バッファをフリーリストから削除 */
            remove_from_free_list(p);

            /* バッファをロック状態に設定 */
            p->stat |= STAT_LOCKED;

            return p;
        } else {
            if (is_free_list_empty(&free_head)) {
                /* シナリオ4 */
                /* 要求されたバッファがハッシュリストになく, しかもフリーリストが空である */
                print_message("getblk", "executing case 4\n");
                print_message("getblk",
                              "buffer (blkno: %d) is not in the hash list %d "
                              "and also free list is empty\n",
                              blkno, hash(blkno));

                /* sleep(); */
                print_message("getblk",
                              "Sleep until any buffer becomes free (blkno: %d)\n", blkno);
                return NULL;
            }

            /* フリーリストの先頭のバッファヘッダを削除 */
            f = remove_from_free_list_head(&free_head);
            
            /* フリーリストにバッファヘッダが存在しない場合はプログラムを停止 */
            assert(f != NULL);

            if (f->stat & STAT_DWR) {
                /* シナリオ3 */
                /* キャッシュされたデータをハードディスクに遅延書き込み */
                print_message("getblk", "executing case 3\n");
                print_message("getblk",
                              "buffer (blkno: %d) is lazy-written to harddisk\n", f->blkno);

                /* バッファが有効でない場合はプログラムを停止 */
                assert(f->stat & STAT_VALID);

                /* ハードディスクに書き込むためにカーネルがバッファをロック */
                f->stat |= STAT_LOCKED;

                /* キャッシュされていたデータをハードディスクに書き込み */
                f->stat &= ~STAT_DWR;
                f->stat |= STAT_KRDWR;
                f->stat |= STAT_OLD;
                print_message("getblk",
                              "asynchronous write buffer to disk (blkno: %d)\n", f->blkno);
                continue;
            }
            
            /* シナリオ2 */
            /* 要求された論理ブロックのバッファがハッシュリストに存在しない */
            print_message("getblk", "executing case 2\n");
            print_message("getblk",
                          "buffer (blkno: %d) is not in the hash list %d\n",
                          blkno, hash(blkno));

            /* バッファをロック状態に設定 */
            f->stat |= STAT_LOCKED;

            /* 以前のハッシュリストから削除 */
            remove_from_hash(f);

            /* バッファの論理ブロック番号を更新 */
            f->blkno = blkno;

            /* バッファは無効なデータを保持 */
            f->stat &= ~STAT_VALID;

            /* ハードディスクからデータを読み込み */
            /* f->stat |= STAT_KRDWR;
            print_message("getblk",
                          "loading buffer from Harddisk (blkno: %d)\n", blkno); */
            
            /* データの読み込みの完了 */
            /* f->stat &= ~STAT_KRDWR;
            f->stat |= STAT_VALID;
            print_message("getblk",
                          "Done loading buffer from Harddisk (blkno: %d)\n", blkno); */
            
            /* ハッシュのリストに追加 */
            h = hash(blkno);
            insert_tail(&hash_head[h], f);

            return f;
        }
    }

    return NULL;
}

/*
 * 論理ブロック番号に対応するバッファを解放
 */
void brelse(struct buf_header* buffer)
{
    /* バッファがロックされていない場合はプログラムを停止 */
    assert(buffer->stat & STAT_LOCKED);

    /* wakeup(); */
    print_message("brelse",
                  "wakeup processes waiting for any buffer\n");

    if (buffer->stat & STAT_WAITED)
        print_message("brelse",
                      "wakeup processes waiting for buffer of blkno %d\n",
                      buffer->blkno);

    /* 割り込みの禁止(クリティカルセクション) */
    /* raise_cpu_level(); */
    print_message("raise_cpu_level", "entering critical section ...\n");
    
    if ((buffer->stat & STAT_VALID) && !(buffer->stat & STAT_OLD)) {
        /* バッファに有効なデータが含まれていて, バッファのデータが古くない場合 */
        /* フリーリストの末尾にバッファを挿入 */
        print_message("brelse",
                      "buffer (blkno: %d) contains valid data "
                      "that is not marked as STAT_OLD\n",
                      buffer->blkno);
        print_message("brelse",
                      "buffer is inserted to the tail of the free list\n");
        insert_free_list_tail(&free_head, buffer);
    } else {
        /* バッファに有効なデータが含まれていないか, バッファのデータが古い場合 */
        /* フリーリストの先頭にバッファを挿入 */
        print_message("brelse",
                      "buffer (blkno: %d) does not contain valid data, "
                      "or buffer holds old data\n",
                      buffer->blkno);
        print_message("brelse",
                      "buffer is inserted to the head of the free list\n");
        insert_free_list_head(&free_head, buffer);
    }

    buffer->stat &= ~STAT_LOCKED;
    buffer->stat &= ~STAT_WAITED;
    buffer->stat &= ~STAT_OLD;
    
    /* 割り込みの許可 */
    /* lower_cpu_level(); */
    print_message("lower_cpu_level", "leaving critical section ...\n");
}

/*
 * バッファの状態を表示
 */
void print_buf_header(int buffer_no, const struct buf_header* buffer)
{
    printf("["
           ANSI_ESCAPE_COLOR_CYAN "%2d "
           ANSI_ESCAPE_COLOR_RED "%2d "
           ANSI_ESCAPE_COLOR_GREEN "%c%c%c%c%c%c"
           ANSI_ESCAPE_COLOR_RESET "]",
           buffer_no, buffer->blkno,
           (buffer->stat & STAT_OLD ? 'O' : '-'),
           (buffer->stat & STAT_WAITED ? 'W' : '-'),
           (buffer->stat & STAT_KRDWR ? 'K' : '-'),
           (buffer->stat & STAT_DWR ? 'D' : '-'),
           (buffer->stat & STAT_VALID ? 'V' : '-'),
           (buffer->stat & STAT_LOCKED ? 'L' : '-'));
}

