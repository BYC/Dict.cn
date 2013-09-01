#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "parser.h"

int pstrspn(char *str1, char *str2)                                             //返回制定字符串needle在
{                                                                               //标签haystack中的位置
    int len = 0;
    char *tmp;

    len = strcspn(str1, ">");
    tmp = (char *)malloc(len);
    strncpy(tmp, str1, len);
    len = 0;
    while(*(tmp + len))
    {
        if((strncmp((const char*)(tmp + len), str2, strlen(str2)) == 0))
        {
            free(tmp);
            return len;
        }
        len++;
    }
    free(tmp);
    return 0;
}

char *pstrcat(char **dest, char *src)
{
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

int is_target_tag(char *str, const char *tag,                                   //实现标签的判断、标签栈元素
                  const char *opt, const char *val,                             //组装的功能;是目标标签返回1
                  off_t *len)                                                   //并设置offset的值是关闭标
{                                                                               //返回0,并设置offset相应值

    off_t slen;
    *len = strcspn(str, ">");
    if(*(str + 1) == '/')
    {
        return 0;
    }

    if(strncmp(++str, tag, strlen(tag)) == 0)
    {
        if(opt == NULL)
            return 1;

        if((slen = pstrspn(str, (char *)opt)))
        {
            str = str + slen + strlen(opt) + 2;                                 //<div    class="abc">
            if((strncmp(str, val, strlen(val)) == 0))                           //   ^_____^____^^
                return 1;                                                       // str sln  sl() 1
        }

    }
    return 0;
}

int tag_cmp(char *str, const char *tag)
{
    while(isspace(*++str))
    {
        //do nothing
    }
    if((strncmp((const char *)str, tag, strlen(tag)) == 0))
        return 1;
    return 0;
}

int opt_cmp(char *str, const char *opt)
{
    while(isspace(*++str))
    {
        //do nothing
    }
    if((strncmp((const char *)str, opt, strlen(opt)) == 0))
        return 1;
    return 0;
}

int is_no_close_tag(char *str)                                                  //排除不进行匹配的TAG
{
    if(strncmp(str, "<br>", 4) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<img", 4) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<meta", 5) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<link", 5) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<input", 6) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<param", 6) == 0)
    {
        return 1;
    }
    if(strncmp(str, "<!--", 4) == 0)
    {
        return 1;
    }
    return 0;
}

void tag_stack_push(struct tag_elem **stack, char *str)
{
    if(!is_no_close_tag(str))
    {
        struct tag_elem *tmp;
        tmp = (struct tag_elem *)malloc(sizeof(struct tag_elem));
        memset(tmp, 0, sizeof(struct tag_elem));
        tmp->tag = str;
        tmp->prev = *stack;
        *stack = tmp;
    }
}

void tag_stack_pop(struct tag_elem **stack)
{
    struct tag_elem *tmp;
    tmp = *stack;
    *stack = (*stack)->prev;
    free(tmp);
}

int get_tag_offset(char *hstr, size_t len,
                   const char *tag, const char *opt, const char *val,
                   struct tag_offset *offset)
{
    int i = 0;
    off_t iner_off = 0;
    struct tag_elem *stack = NULL;

    while(i < len)
    {
        if(*hstr == '<')
        {
            if(is_target_tag(hstr, tag, opt, val, &iner_off))
            {
                tag_stack_push(&stack, hstr);
                offset->o_start = i;
                offset->o_end = i + iner_off;
                i = i + iner_off + 1;
                hstr = hstr + iner_off + 1;
                continue;
            }

            if((stack == NULL))
            {
                i = i + iner_off + 1;
                hstr = hstr + iner_off + 1;
                continue;
            }
            else
            {
                if(*(hstr + 1) == '/')
                {
                    if(stack->prev == NULL)
                    {
                        offset->c_start = i;
                        offset->c_end = i + iner_off;
                        offset->t_len = offset->c_end - offset->o_start + 1;
                        offset->o_len = offset->o_end - offset->o_start + 1;
                        offset->c_len = offset->c_end - offset->c_start + 1;
                        offset->cn_len = offset->c_start - offset->o_end - 1;
                        free(stack);
                        return 1;
                    }
                    tag_stack_pop(&stack);
                    i = i + iner_off + 1;
                    hstr = hstr + iner_off + 1;
                    continue;
                }
                else
                {
                    tag_stack_push(&stack, hstr);
                    i = i + iner_off + 1;
                    hstr = hstr + iner_off + 1;
                    continue;
                }

            }
        }
        hstr++;
        i++;                                                                    //移动字符指针，增加计数器的值
    }
    return 0;
}

char *get_text_in_tag(char *str, struct tag_offset offset)
{
    size_t off;
    char *text;
    off = offset.o_end + 1;
    text = (char *)malloc(offset.cn_len + 1);
    memset(text, 0, offset.cn_len + 1);
    str += off;
    strncpy(text, str, offset.cn_len);
    text[offset.cn_len] = '\0';
    return text;
}

char *get_text_btwn(char *str, off_t start, off_t end)
{
    size_t len;
    char *text;
    len = end - start - 1;
    text = (char *)malloc(len + 1);
    memset(text, 0, len + 1);
    str += (start + 1);
    strncpy(text, str, len);
    text[len]  = '\0';
    return text;
}

char *get_a_string(const char *str)
{
    size_t len = strlen(str) + 1;
    char *text = malloc(len);
    memset(text, 0, len);
    strncpy(text, str, len - 1);
    text[len - 1] = '\0';
    return text;
}

void go_pack_keyword(char *str, size_t len, struct word_struct *word)
{
    char *tmp;
    size_t size;
    struct tag_offset offset1, offset2, offset3;
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
        pstrcat(&word->phonetic, get_text_btwn(tmp, offset2.o_end, offset3.o_start));
        pstrcat(&word->phonetic, get_text_in_tag(tmp, offset3));
        pstrcat(&word->phonetic, get_a_string(" "));

        tmp += (offset2.c_end + 1);
        size -= (offset2.c_end - 1);
        get_tag_offset(tmp, size, "span", NULL, NULL, &offset2);
        get_tag_offset(tmp, size, "bdo", NULL, NULL, &offset3);
        pstrcat(&word->phonetic, get_text_btwn(tmp, offset2.o_end, offset3.o_start));
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

int pack_word_struct(char *str, ssize_t str_len, struct word_struct *word)
{
    size_t size;
    struct tag_offset offset1= {0, 0, 0, 0};
    if(get_tag_offset(str, str_len, "div", "class", "main", &offset1))
    {
        str += (offset1.o_end + 1);
        size = offset1.cn_len;
    }


    if(get_tag_offset(str, size, "div", "class", "word", &offset1))
    {
        go_pack_keyword(str + offset1.o_end + 1, offset1.cn_len, word);
        str += (offset1.c_end + 1);
        size -= (offset1.c_end + 1);
    }

//    if(get_tag_offset(str, size, "div", "class", "section def", &offset1))
//    {
//        go_pack_secdef(str, word);
//        str += (offset1.c_end + 1);
//        size -= (offset1.c_end + 1);
//    }
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
