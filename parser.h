#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <sys/types.h>

#define BUFSIZE 1024 * 200

struct tag_offset
{
    off_t o_start;
    off_t o_end;
    off_t c_start;
    off_t c_end;
    size_t t_len;
    size_t o_len;
    size_t c_len;
    size_t cn_len;
};

struct tag_elem
{
    char *tag;
    struct tag_elem *prev;
};

struct string_item
{
    char *cat;
    char *cntnt;
    struct string_item *next;
};

struct word_def
{
    struct string_item *basic;                                                  //基本释义
    struct string_item *dual;                                                   //双语释义
    struct string_item *abso;                                                   //详尽释义
    struct string_item *en;                                                     //英英释义
};

struct word_snt
{
    struct string_item *sort;                                                   //例句
    struct string_item *patt;                                                   //常见句型
    struct string_item *phra;                                                   //常用短语
    struct string_item *coll;                                                   //词汇搭配
    struct string_item *auth;                                                   //经典引文
};

struct word_lrn
{
    struct string_item *ess;                                                    //词语用法
    struct string_item *disc;                                                   //词义辨析
    struct string_item *comn;                                                   //常见错误
    struct string_item *etm;                                                    //词源解说
};

struct word_ask
{
    struct string_item *content;                                                //提问补充
};

struct word_rel
{
    struct string_item *nfw;                                                    //近反义词
    struct string_item *baike;                                                  //互动百科
    struct string_item *ndw;                                                    //临近单词
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
    struct word_use *snt;
    struct word_lrn *lrn;
    struct word_ask *ask;
    struct word_rel *rel;

};

int get_tag_offset(char *hstr, size_t len,
                   const char *tag, const char *opt, const char *val,
                   struct tag_offset *offset);

int pack_word_struct(char *str, ssize_t str_len, struct word_struct *word_struct);

#endif // PARSER_H_INCLUDED
