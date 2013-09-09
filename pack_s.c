#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pack_s.h"
#include "parser.h"

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

        if(strncmp(str + off2, "<br", 3) == 0)
            pstrcat(&text, get_a_string("\n"));
        if(off2 < off1)
        {
            off2++;
            goto FindOff2;
        }
        pstrcat(&text, get_text_btwn(str, off1, off2));
        if(strlen(text) != 0)
        {
            pstrcat(&text, get_a_string(sepr));
        }
        off1++;
    }
}

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

void creat_string_item(struct str_item **node)
{

    *node = (struct str_item *)malloc(sizeof(struct str_item));
    memset(*node, 0, sizeof(struct str_item));
}

void go_pack_keyword(char *str, size_t len, struct word_struct *word)
{
    char *tmp;
    size_t size;
    struct tag_offset offset1, offset2, offset3;

    offset_reset(&offset1);
    offset_reset(&offset2);
    offset_reset(&offset3);
    memset(word, 0, sizeof(struct word_struct));

    if(get_tag_offset(str, len, "h1", "class", "keyword", &offset1))
    {
        pstrcat(&word->keyword, get_text_in_tag(str, offset1));
        str += (offset1.c_end + 1);
        len -= (offset1.c_end + 1);
    }

    if(get_tag_offset(str, len, "div", "class", "phonetic", &offset1))
    {
        tmp = str + offset1.o_end + 1;
        size = offset1.cn_len;
        get_tag_offset(tmp, size, "span", NULL, NULL, &offset2);
        get_tag_offset(tmp, size, "bdo", NULL, NULL, &offset3);
        pstrcat(&word->phonetic,
                get_text_btwn(tmp, offset2.o_end, offset3.o_start));
        pstrcat(&word->phonetic, get_text_in_tag(tmp, offset3));
        pstrcat(&word->phonetic, get_a_string(" "));

        tmp += (offset2.c_end + 1);
        size -= (offset2.c_end - 1);
        get_tag_offset(tmp, size, "span", NULL, NULL, &offset2);
        get_tag_offset(tmp, size, "bdo", NULL, NULL, &offset3);
        pstrcat(&word->phonetic,
                get_text_btwn(tmp, offset2.o_end, offset3.o_start));
        pstrcat(&word->phonetic, get_text_in_tag(tmp, offset3));

        str += (offset1.c_end + 1);
        len -= (offset1.c_end - 1);
    }

    if(get_tag_offset(str, len, "div", "class", "shape", &offset1))
    {
        tmp = str + offset1.o_end + 1;
        size = offset1.cn_len;
        while(get_tag_offset(tmp, size, "span", NULL, NULL, &offset2))
        {
            get_tag_offset(tmp, size, "label", NULL, NULL, &offset3);
            pstrcat(&word->shapes, get_text_in_tag(tmp, offset3));
            get_tag_offset(tmp, size, "a", NULL, NULL, &offset3);
            pstrcat(&word->shapes, get_text_in_tag(tmp, offset3));
            pstrcat(&word->shapes, get_a_string(" "));

            tmp += (offset2.c_end + 1);
            size -= (offset2.c_end + 1);
        }
    }
}

void go_pack_secdef(char *str, size_t len, struct word_struct *word)
{
    char *tmp, *tmp1, *str_cat;
    size_t size, size1;
    struct str_item **node_now;
    struct tag_offset offset1, offset2, offset3, offset4;

    offset_reset(&offset1);
    offset_reset(&offset2);
    offset_reset(&offset3);
    offset_reset(&offset4);
    word->def = (struct word_def *)malloc(sizeof(struct word_def));
    memset(word->def, 0, sizeof(struct word_def));

    if(get_tag_offset(str, len, "ul", "class", "dict-basic-ul", &offset1))
    {
        tmp = str + offset1.o_end + 1;                                          //移动指针到基本释义区块
        size = offset1.cn_len;                                                  //设定该区块的局部长度
        node_now = &word->def->basi;                                            //node_now指向基本释义链表头
        while(get_tag_offset(tmp, size, "li", NULL, NULL, &offset2))            //获取li 并用offset2记下li位置
        {
            creat_string_item(node_now);
//            get_tag_offset(tmp + offset2.o_end + 1, offset2.cn_len, "span",     //offset1记下span位置
//                           NULL, NULL, &offset3);
            pstrcat(&((*node_now)->cntnt),
                    get_text_in_oneline(tmp + offset2.o_start,
                                        offset2.al_len, NULL));
//            get_tag_offset(tmp + offset2.o_end + 1,
//                           offset2.cn_len, "strong", NULL, NULL, &offset3);
//            pstrcat(&((*node_now)->cntnt),
//                    get_text_in_tag(tmp + offset2.o_end + 1, offset3));

            node_now = &((*node_now)->next);
            tmp += (offset2.c_end + 1);
            size -= (offset2.c_end + 1);
        }
        str += (offset1.c_end + 1);
        len -= (offset1.c_end + 1);
        offset_reset(&offset1);
        offset_reset(&offset2);
    }

    if(get_tag_offset(str, len, "div", "class", "layout dual", &offset1))
    {
        tmp = str + offset1.o_end + 1;
        size = offset1.cn_len;
        node_now = &word->def->dual;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset2))
        {
            tmp1 = tmp + offset2.o_end + 1;
            size1 = offset2.cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset3);
            str_cat = get_text_in_oneline(tmp + offset3.o_start + 1,
                                          offset3.al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset3))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset3.o_start,
                                            offset3.al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset3.c_end + 1);
                size1 -= (offset3.c_end + 1);
            }

            tmp += (offset2.c_end + 1);
            size -= (offset2.c_end + 1);
        }

        str += (offset1.c_end + 1);
        len -= (offset1.c_end + 1);
        offset_reset(&offset1);
        offset_reset(&offset2);
        offset_reset(&offset3);
    }

    if(get_tag_offset(str, len, "div", "class", "layout detail", &offset1))
    {
        tmp = str + offset1.o_end + 1;
        size = offset1.cn_len;
        node_now = &word->def->dtil;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset2))
        {
            tmp1 = tmp + offset2.o_end + 1;
            size1 = offset2.cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset3);
            str_cat = get_text_in_oneline(tmp + offset3.o_start + 1,
                                          offset3.al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset3))
            {
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),
                        get_text_in_oneline(tmp1 + offset3.o_start,
                                            offset4.al_len, " "));

                node_now = &((*node_now)->next);
                tmp1 += (offset3.c_end + 1);
                size1 -= (offset3.c_end + 1);
            }

            tmp += (offset2.c_end + 1);
            size -= (offset2.c_end + 1);
        }

        str += (offset1.c_end + 1);
        len -= (offset1.c_end + 1);
        offset_reset(&offset1);
        offset_reset(&offset2);
        offset_reset(&offset3);
    }

    if(get_tag_offset(str, len, "div", "class", "layout en", &offset1))
    {
        tmp = str + offset1.o_end + 1;
        size = offset1.cn_len;
        node_now = &word->def->en;

        while(get_tag_offset(tmp, size, "ol", NULL, NULL, &offset2))
        {
            tmp1 = tmp + offset2.o_end + 1;
            size1 = offset2.cn_len;
            get_tag_offset(tmp, size, "span", NULL, NULL, &offset3);
            str_cat = get_text_in_oneline(tmp + offset3.o_start + 1,
                                          offset3.al_len, NULL);
            while(get_tag_offset(tmp1, size1, "li", NULL, NULL, &offset3))
            {
                get_tag_offset(tmp1, size1, "p", NULL, NULL, &offset4);
                creat_string_item(node_now);
                (*node_now)->cat = str_cat;
                pstrcat(&((*node_now)->cntnt),                                  //获取<li>中的文字
                        get_text_btwn(tmp1, offset3.o_end, offset4.o_start));
                pstrcat(&((*node_now)->cntnt), get_a_string("\n"));
                pstrcat(&((*node_now)->cntnt),                                  //获取<p>标签内的文字
                        get_text_in_oneline(tmp1 + offset4.o_start,
                                            offset4.al_len, NULL));

                node_now = &((*node_now)->next);
                tmp1 += (offset3.c_end + 1);
                size1 -= (offset3.c_end + 1);
            }

            tmp += (offset2.c_end + 1);
            size -= (offset2.c_end + 1);
        }
    }

    return;
}

int pack_word_struct(char *str, ssize_t str_len, struct word_struct *word)
{
    struct tag_offset offset1;
    if(get_tag_offset(str, str_len, "div", "class", "main", &offset1))
    {
        str += (offset1.o_end + 1);
        str_len = offset1.cn_len;
    }

    if(get_tag_offset(str, str_len, "div", "class", "word", &offset1))
    {
        go_pack_keyword(str + offset1.o_end + 1, offset1.cn_len, word);
        str += (offset1.c_end + 1);
        str_len -= (offset1.c_end + 1);
    }

    if(get_tag_offset(str, str_len, "div", "class", "section def", &offset1))
    {
        go_pack_secdef(str + offset1.o_end + 1, offset1.cn_len, word);
        str += (offset1.c_end + 1);
        str_len -= (offset1.c_end + 1);
    }

//    if(get_tag_offset(str, size, "div", "class", "section sent", &offset1))
//    {
//        go_pack_secsent(str, word);
//        str += (offset1.c_end + 1);
//        size -= (offset1.c_end + 1);
//    }
//    if(get_tag_offset(str, size, "div", "class", "section learn", &offset1))
//    {
//        go_pack_seclrn(str, word);
//        str += (offset1.c_end + 1);
//        size -= (offset1.c_end + 1);
//    }
//    if(get_tag_offset(str, size, "div", "class", "section ask", &offset1))
//    {
//        go_pack_secask(str, word);
//        str += (offset1.c_end + 1);
//        size -= (offset1.c_end + 1);
//    }
    return 1;
}
