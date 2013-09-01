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

GtkWidget *generate_main_window(); //创建主窗体函数
void init_list(GtkWidget *list);   //初始化列表函数
void add_to_list(GtkWidget *list, const gchar *str);  //添加历史记录到列表

#endif // UI_H_INCLUDED
