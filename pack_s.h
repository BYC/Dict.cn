#ifndef PACK_S_H_INCLUDED
#define PACK_S_H_INCLUDED

#include "parser.h"

#define BUFSIZE 1024 * 200

struct str_item
{
    char *cat;
    char *cntnt;
    struct str_item *next;
};

struct word_def
{
    struct str_item *basi;                                                      //基本释义
    struct str_item *dual;                                                      //双语释义
    struct str_item *dtil;                                                      //详尽释义
    struct str_item *en;                                                        //英英释义
};

struct word_snt
{
    struct str_item *sort;                                                      //例句
    struct str_item *patt;                                                      //常见句型
    struct str_item *phra;                                                      //常用短语
    struct str_item *coll;                                                      //词汇搭配
    struct str_item *auth;                                                      //经典引文
};

struct word_lrn
{
    struct str_item *ess;                                                       //词语用法
    struct str_item *disc;                                                      //词义辨析
    struct str_item *comn;                                                      //常见错误
    struct str_item *etm;                                                       //词源解说
};

struct word_ask
{
    struct str_item *content;                                                   //提问补充
};

struct word_rel
{
    struct str_item *nfw;                                                       //近反义词
    struct str_item *baike;                                                     //互动百科
    struct str_item *ndw;                                                       //临近单词
};

struct word_struct
{
    char *keyword;
    char *word_level;
    char *phonetic;
    char *shapes;

    int HAS_WORD_DEF;                                                           //判断是否有: 释义
    int HAS_WORD_USE;                                                           //          用例
    int HAS_WORD_LRN;                                                           //          讲解
    int HAS_WORD_ASK;                                                           //          问答
    int HAS_WORD_REF;                                                           //          相关，五大区块

    struct word_def *def;
    struct word_snt *snt;
    struct word_lrn *lrn;
    struct word_ask *ask;
    struct word_rel *rel;

};

char *pstrcat(char **dest, char *src);

int pack_word_struct(char *str, ssize_t str_len, struct word_struct *word);

#endif // PACK_S_H_INCLUDED
