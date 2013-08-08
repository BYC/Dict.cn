#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED
#include <gtk/gtk.h>

#define WORD_NUMS 50 //历史搜索中存储的最多单词数量

enum
{
    LIST_ITEM = 0,
    N_COLUMNS
};

typedef struct{
    GtkWidget *entry;
    GtkWidget *list;
}butn_data; //查询结构体

GtkWidget *generate_main_window();
void init_list(GtkWidget *list);
void add_to_list(GtkWidget *list, const gchar *str);

#endif // UI_H_INCLUDED
