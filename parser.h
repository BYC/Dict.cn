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

void offset_reset(struct tag_offset *offset, int n);

size_t pstrcspn(char *str1, char *str2);

int get_tag_offset(char *hstr, size_t len,
                   const char *tag, const char *opt, const char *val,
                   struct tag_offset *offset);



#endif // PARSER_H_INCLUDED
