#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pack_s.h"
#include "parser.h"

char *get_a_string(const char *str)
{
    if(str == NULL)
        return NULL;
    size_t len = strlen(str) + 1;
    char *text = malloc(len);
    memset(text, 0, len);
    strncpy(text, str, len - 1);
    text[len - 1] = '\0';
    return text;
}

char *pstrcat(char **dest, char *src)
{
    if(src == NULL)
        return *dest;
    size_t len;
    len = (*dest ? strlen(*dest) : 0) + strlen(src) + 1;
    char *text = (char *)malloc(len);
    memset(text, 0, len);
    if(*dest)
        strcat(text, *dest);
    strcat(text, src);
    free(*dest);
    free(src);
    *dest = text;
    return text;
}

char *get_text_btwn(char *str, off_t start, off_t end)
{
    size_t len;
    char *tmp;
    char *text;
    len = end - start - 1;
    tmp = str + end - 1;
    str += (start + 1);
    while(isspace(*str) && len > 0)
    {
        str++;
        len--;
        if(len == 0)
            return NULL;
    }
    while(isspace(*tmp))
    {
        tmp--;
        len--;
    }
    text = (char *)malloc(len + 1);
    memset(text, 0, len + 1);
    strncpy(text, str, len);
    text[len]  = '\0';
    return text;
}

char *get_text_in_tag(char *str, struct tag_offset offset)
{
    off_t start, end;
    char *text;

    start = offset.o_end;
    end = offset.c_start;
    text = get_text_btwn(str, start, end);
    return text;
}

char *get_text_in_oneline(char *str, size_t len, char *sepr)
{
    off_t off1 = 0, off2 = 0;
    char *text = NULL;

    while(1)
    {
        while(*(str + off1) != '>')
        {
            off1++;
            if((off1 >= len))
                return text;
        }

FindOff2:
        while(*(str + off2) != '<')
        {
            off2++;
            if(off2 >= len)
                return text;
        }

        if(off2 < off1)
        {
            off2++;
            goto FindOff2;
        }
        pstrcat(&text, get_text_btwn(str, off1, off2));
        if(strncmp(str + off2, "<br", 3) == 0)
            pstrcat(&text, get_a_string("\n"));
        if(text != NULL)
        {
            if(strlen(text) != 0)
                pstrcat(&text, get_a_string(sepr));
        }
        off1++;
        off2++;
    }
}

void creat_string_item(struct str_item **node)
{
    *node = (struct str_item *)malloc(sizeof(struct str_item));
    memset(*node, 0, sizeof(struct str_item));
}

void go_pack_keyword(char *str, size_t len, struct word_struct *word)
{
    char *tmp;
    size_t size;
    struct tag_offset offset[3];

    offset_reset(offset, 3);
    memset(word, 0, sizeof(struct word_struct));

    if(get_tag_offset(str, len, "h1", "class", "keyword", &offset[0]))
    {
        pstrcat(&word->keyword, get_text_in_tag(str, offset[0]));
        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    if(get_tag_offset(str, len, "span", "class", "level-title", &offset[0]))
    {
        pstrcat(&word->word_level, get_text_in_tag(str, offset[0]));
        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    if(get_tag_offset(str, len, "div", "class", "phonetic", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        while(get_tag_offset(tmp, size, "span", NULL, NULL, &offset[1]))
        {
            pstrcat(&word->phonetic,
                    get_text_in_oneline(tmp + offset[1].o_start,
                                        offset[1].al_len, " "));
            pstrcat(&word->phonetic, get_a_string(" "));
            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end - 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    if(get_tag_offset(str, len, "div", "class", "shape", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        while(get_tag_offset(tmp, size, "span", NULL, NULL, &offset[1]))
        {
            get_tag_offset(tmp, size, "label", NULL, NULL, &offset[2]);
            pstrcat(&word->shapes, get_text_in_tag(tmp, offset[2]));
            get_tag_offset(tmp, size, "a", NULL, NULL, &offset[2]);
            pstrcat(&word->shapes, get_text_in_tag(tmp, offset[2]));
            pstrcat(&word->shapes, get_a_string(" "));

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }
    }
}

/*==================================组装释义====================================*/
void go_pack_secdef(char *str, size_t len, struct word_struct *word)
{
    char *tmp, *tmp1, *str_cat;
    size_t size, size1;
    struct str_item **node_now;
    struct tag_offset offset[4];

    offset_reset(offset, 4);
    word->def = (struct word_def *)malloc(sizeof(struct word_def));
    memset(word->def, 0, sizeof(struct word_def));

    /*=================================基本释义=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout basic clearfix", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;                                        //移动指针到基本释义区块
        size = offset[0].cn_len;                                                //设定该区块的局部长度
        node_now = &word->def->basi;                                            //node_now指向基本释义链表头
        while(get_tag_offset(tmp, size, "li", NULL, NULL, &offset[1]))          //获取li 并用offset[2]记下li位置
        {
            creat_string_item(node_now);
            pstrcat(&((*node_now)->cntnt),
                    get_text_in_oneline(tmp + offset[1].o_start,
                                        offset[1].al_len, NULL));

            node_now = &((*node_now)->next);
            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }
        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 2);
    }

    /*=================================双语释义=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout dual", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->def->dual;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================详尽释义=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout detail", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->def->dtil;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 4);
    }

    /*=================================英英释义=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout en", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->def->en;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                get_tag_offset(tmp1, size1, "p", NULL, NULL, &offset[3]);
                pstrcat(&((*node_now)->cntnt),                                  //获取<li>中的文字
                        get_text_btwn(tmp1, offset[2].o_end, offset[3].o_start));
                pstrcat(&((*node_now)->cntnt), get_a_string("\n"));
                pstrcat(&((*node_now)->cntnt),                                  //获取<p>标签内的文字
                        get_text_in_oneline(tmp1 + offset[3].o_start,
                                            offset[3].al_len, NULL));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }
    }

    return;
}

/*==================================组装用例====================================*/
void go_pack_secsnt(char *str, size_t len, struct word_struct *word)
{
    char *tmp, *tmp1, *str_cat;
    size_t size, size1;
    struct str_item **node_now;
    struct tag_offset offset[4];

    offset_reset(offset, 4);
    word->snt = (struct word_snt *)malloc(sizeof(struct word_snt));
    memset(word->snt, 0, sizeof(struct word_snt));

    /*==================================例句===================================*/
    if(get_tag_offset(str, len, "div", "class", "layout sort", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->snt->sort;
        while(get_tag_offset(tmp, size, "li", NULL, NULL, &offset[1]))
        {
            creat_string_item(node_now);
            pstrcat(&((*node_now)->cntnt),
                    get_text_in_oneline(tmp + offset[1].o_start,
                                        offset[1].al_len, NULL));

            node_now = &((*node_now)->next);
            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }
        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 2);
    }

    /*=================================常见句型=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout patt", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->snt->patt;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "div", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================常用短语=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout phra", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->snt->phra;

        while(get_tag_offset(tmp, size, "dl", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "dt", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================词汇搭配=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout coll", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->snt->coll;

        while(get_tag_offset(tmp, size, "ul", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "b", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================经典引文=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout auth", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->snt->auth;

        while(get_tag_offset(tmp, size, "ul", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;

            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, "—————"));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    return;
}

/*==================================组装用例====================================*/
void go_pack_seclrn(char *str, size_t len, struct word_struct *word)
{
    char *tmp, *tmp1, *str_cat;
    size_t size, size1;
    struct str_item **node_now;
    struct tag_offset offset[3];

    offset_reset(offset, 3);
    word->lrn = (struct word_lrn *)malloc(sizeof(struct word_lrn));
    memset(word->lrn, 0, sizeof(struct word_lrn));

    /*================================词语用法=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout ess", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->lrn->ess;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================词义辨析=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout discrim", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->lrn->disc;

        while(get_tag_offset(tmp, size, "dl", NULL, NULL, &offset[1]))
        {
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            creat_string_item(node_now);
            (*node_now)->cat = str_cat;
            pstrcat(&((*node_now)->cntnt),
                    get_text_in_oneline(tmp + offset[1].o_start,
                                        offset[1].al_len, "\n"));

            node_now = &((*node_now)->next);
            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================常见错误=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout comn", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->lrn->comn;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset[1]))
        {
            tmp1 = tmp + offset[1].o_end + 1;
            size1 = offset[1].cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset[2]);
            str_cat = get_text_in_oneline(tmp + offset[2].o_start + 1,
                                          offset[2].al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset[2]))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset[2].o_start,
                                            offset[2].al_len, "\n"));

                node_now = &((*node_now)->next);
                tmp1 += (offset[2].c_end + 1);
                size1 -= (offset[2].c_end + 1);
            }

            tmp += (offset[1].c_end + 1);
            size -= (offset[1].c_end + 1);
        }

        str += (offset[0].c_end + 1);
        len -= (offset[0].c_end + 1);
        offset_reset(offset, 3);
    }

    /*=================================词源解说=================================*/
    if(get_tag_offset(str, len, "div", "class", "layout etm", &offset[0]))
    {
        tmp = str + offset[0].o_end + 1;
        size = offset[0].cn_len;
        node_now = &word->lrn->etm;

        get_tag_offset(tmp, size, "ul", NULL, NULL, &offset[1]);
        creat_string_item(node_now);
        tmp1 = get_text_in_oneline(tmp + offset[1].o_start,
                                   offset[1].al_len, "\n");
        if(strlen(tmp1))
            pstrcat(&((*node_now)->cntnt), tmp1);
    }

    return;
}

/*==================================组装问答====================================*/
//void go_pack_secask(char *str, size_t len, struct word_struct *word)
//{
//
//    return;
//}

/*==================================组装问答====================================*/
void go_pack_secrel(char *str, size_t len, struct word_struct *word)
{


    return;
}

/*==================================组装单词====================================*/
int pack_word_struct(char *str, ssize_t str_len, struct word_struct *word)
{
    struct tag_offset offset;
    offset_reset(&offset, 1);

    if(get_tag_offset(str, str_len, "div", "class", "main", &offset))
    {
        str += (offset.o_end + 1);
        str_len = offset.cn_len;
        offset_reset(&offset, 1);
    }

    if(get_tag_offset(str, str_len, "div", "class", "word", &offset))
    {
        go_pack_keyword(str + offset.o_end + 1, offset.cn_len, word);
        str += (offset.c_end + 1);
        str_len -= (offset.c_end + 1);
        offset_reset(&offset, 1);
    }

    if(get_tag_offset(str, str_len, "div", "class", "section def", &offset))
    {
        go_pack_secdef(str + offset.o_end + 1, offset.cn_len, word);
        str += (offset.c_end + 1);
        str_len -= (offset.c_end + 1);
        offset_reset(&offset, 1);
    }

    if(get_tag_offset(str, str_len, "div", "class", "section sent", &offset))
    {
        go_pack_secsnt(str + offset.o_end + 1, offset.cn_len, word);
        str += (offset.c_end + 1);
        str_len -= (offset.c_end + 1);
        offset_reset(&offset, 1);
    }

    if(get_tag_offset(str, str_len, "div", "class", "section learn", &offset))
    {
        go_pack_seclrn(str + offset.o_end + 1, offset.cn_len, word);
        str += (offset.c_end + 1);
        str_len -= (offset.c_end + 1);
        offset_reset(&offset, 1);
    }

//    if(get_tag_offset(str, size, "div", "class", "section ask", &offset))
//    {
//        go_pack_secask(str + offset.o_end + 1, offset.cn_len, word);
//        str += (offset.c_end + 1);
//        str_len -= (offset.c_end + 1);
//        offset_reset(&offset, 1);
//    }

//    if(get_tag_offset(str, size, "div", "class", "section nwd", &offset))
//    {
//        go_pack_secrel(str + offset.o_end + 1, offset.cn_len, word);
//        str += (offset.c_end + 1);
//        str_len -= (offset.c_end + 1);
//    }

    return 1;
}
