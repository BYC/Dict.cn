#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

void offset_reset(struct tag_offset *offset)
{
    offset->o_start = offset->o_end = offset->c_start = offset->c_end
    = offset->cn_len = offset->al_len = 0;
}

size_t pstrcspn(char *str1, char *str2)                                         //返回制定字符串str2
{                                                                               //在标签str1中的位置
    size_t len = 0;
    char *tmp;

    while(*(str1 + len) != '>')
    {
        len++;
    }
    tmp = (char *)malloc(len + 2);
    strncpy(tmp, str1, len + 1);
    tmp[len + 1] = '\0';
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

int is_target_tag(char *str, const char *tag,                                   //实现标签的判断、标签栈元素
                  const char *opt, const char *val,                             //组装的功能;是目标标签返回1
                  off_t *len)                                                   //并设置offset的值是关闭标
{                                                                               //返回0,并设置offset相应值

    off_t slen;
    *len = pstrcspn(str, ">");
    if(*(str + 1) == '/')
    {
        return 0;
    }

    if(strncmp(++str, tag, strlen(tag)) == 0)
    {
        if(opt == NULL)
            return 1;

        if((slen = pstrcspn(str, (char *)opt)))
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
    if(strncmp(str, "<br", 3) == 0)
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

    offset_reset(offset);

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
                        offset->cn_len = offset->c_start - offset->o_end - 1;
                        offset->al_len = offset->c_end - offset->o_start + 1;
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
