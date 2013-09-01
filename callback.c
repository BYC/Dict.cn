#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ui.h"

gboolean is_text_valid(const char *text, gchar **tmp)       //判断是否是有效输
{
    int i, start, end, len;                                 //入并截取有效部分
    start = 0;
    end = 0;
    len = 0;
    while(!isalpha(*(text + start)) && (*(text + start) != '\000'))
        start++;
    end+=start;
    while(isalpha(*(text + end)) && (*(text + end) != '\000'))
        end++;
    if(!(len = end - start))
        return FALSE;
    *tmp = (gchar *)calloc(len + 1, sizeof(char)); // +1 用于存放"\0"


    for(i = 0; i < len; i++)
        *(*tmp + i)  = tolower(*(text + start + i));
    *(*tmp + len) = '\000';
    return TRUE;
}

gboolean is_exsit_in_list(GtkWidget *list, gchar *text)
{
    GtkTreeModel *liststore;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *str;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    liststore = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
    if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter))
        return FALSE;

    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(liststore), &iter, LIST_ITEM, &str, -1);
        if((strcmp(text, str)) == 0)
        {
            gtk_tree_selection_select_iter(selection, &iter);
            g_free(str);
            return TRUE;
        }
        g_free(str);
    }
    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter));

    return FALSE;
}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

gint input_complete1(GtkWidget *widget, GdkEvent *event, butn_data *data)  //button
{
    const gchar *text;
    gchar *tmp;
    text = gtk_entry_get_text(GTK_ENTRY(data->entry));
    if(!is_text_valid(text, &tmp))
    {
        gtk_widget_grab_focus(data->entry);
        return FALSE;
    }
    if(is_exsit_in_list(data->list, tmp))
    {
        gtk_widget_grab_focus(data->entry);
        return FALSE;
    }
    add_to_list(data->list, tmp);
    gtk_widget_grab_focus(data->entry);
    free(tmp);
    return FALSE;
}

gint input_complete2(GtkWidget *widget, gpointer data)              //entry
{
    const gchar *text;
    gchar *tmp;
    text = gtk_entry_get_text(GTK_ENTRY(widget));
    if(!is_text_valid(text, &tmp))
    {
        gtk_widget_grab_focus(widget);
        return FALSE;
    }
    if(is_exsit_in_list(GTK_WIDGET(data), tmp))
    {
        gtk_widget_grab_focus(widget);
        return FALSE;
    }
    add_to_list(GTK_WIDGET(data), tmp);
    gtk_widget_grab_focus(widget);
    free(tmp);
    return FALSE;
}

//entry的自动选择函数
gint entry_reinput( GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data )
{
    gtk_widget_grab_focus(widget);
    return FALSE;
}

void list_item_selected(GtkWidget *widget, gpointer data){
    GtkTreeView *list;
    GtkTreeModel *liststore;
    GtkTreeIter iter;
    gchar *str;

    list = gtk_tree_selection_get_tree_view(GTK_TREE_SELECTION(widget));
    liststore = gtk_tree_view_get_model(list);
    gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &liststore, &iter);

    gtk_tree_model_get(liststore, &iter, LIST_ITEM, &str, -1);
    gtk_entry_set_text(GTK_ENTRY(data), str);
    gtk_tree_iter_free(&iter);
    g_free(str);
}
