#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <sys/types.h>

struct tag_offset
{
    off_t o_start;
    off_t o_end;
    off_t c_start;
    off_t c_end;
    size_t cn_len;
    size_t al_len;
};

struct tag_elem
{
    char *tag;
    struct tag_elem *prev;
};

void offset_reset(struct tag_offset *offset);

int get_tag_offset(char *hstr, size_t len,
                   const char *tag, const char *opt, const char *val,
                   struct tag_offset *offset);

size_t pstrcspn(char *str1, char *str2);

int is_target_tag(char *str, const char *tag,                                   //实现标签的判断、标签栈元素
                  const char *opt, const char *val,                             //组装的功能;是目标标签返回1
                  off_t *len);                                                  //并设置offset的值是关闭标
                                                                                //返回0,并设置offset相应值

int tag_cmp(char *str, const char *tag);

int opt_cmp(char *str, const char *opt);

int is_no_close_tag(char *str);

void tag_stack_push(struct tag_elem **stack, char *str);

void tag_stack_pop(struct tag_elem **stack);

int get_tag_offset(char *hstr, size_t len,
                   const char *tag, const char *opt, const char *val,
                   struct tag_offset *offset);



#endif // PARSER_H_INCLUDED
