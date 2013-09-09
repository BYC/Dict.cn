#include <gtk/gtk.h>
#include "ui.h"
#include "callback.h"

GtkWidget *window;
GtkWidget *paned;
GtkWidget *scrolled_window;
GtkWidget *vbox, *hbox;
GtkWidget *list;
GtkTreeSelection *selection;
GtkWidget *entry, *search_button;
GtkWidget *notebook, *text_view;
static butn_data butndata1; //创建查询结构体

//初始化列表
void init_list(GtkWidget *list)
{

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("List Items",
             renderer, "text", LIST_ITEM, NULL);
    gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), "History");
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

    store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);

    gtk_tree_view_set_model(GTK_TREE_VIEW(list),
                            GTK_TREE_MODEL(store));

    g_object_unref(store);
}

//添加条目到列表模型
void add_to_list(GtkWidget *list, const gchar *str)
{
    GtkListStore *store;
    GtkTreeIter iter, iter_tmp;
    gint row_count = 1;
    store = GTK_LIST_STORE(gtk_tree_view_get_model
                           (GTK_TREE_VIEW(list)));

    gtk_list_store_insert(store, &iter, 0);
    gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);

    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)){
        row_count++;
        iter_tmp = iter;
    }
    if(row_count > WORD_NUMS)
        gtk_list_store_remove(store, &iter_tmp);
}

GtkWidget *generate_main_window()
{
//  创建主窗体
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Dict.cn");
    gtk_widget_set_size_request(window, 650, 420);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_has_resize_grip(GTK_WINDOW(window), TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

//  创建分栏构件
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), paned);

//  创建列表并组装
    list = gtk_tree_view_new();
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 120, -1);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(scrolled_window), list);
    gtk_paned_add1(GTK_PANED(paned), scrolled_window);

    init_list(list);

//  创建输入框
    entry = gtk_entry_new();
    gtk_widget_set_margin_top(entry, 1);
    g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(input_complete2), list);
    g_signal_connect(G_OBJECT(entry), "enter-notify-event", G_CALLBACK(entry_reinput), NULL);

//组装查询结构体、设置列表选择回调
    butndata1.entry = entry;
    butndata1.list = list;

//  Search Button
    search_button = gtk_button_new_with_label("Search");
    g_signal_connect(G_OBJECT(search_button), "button-release-event",
                     G_CALLBACK(input_complete1), &butndata1);

//  创建主选项卡
    text_view = gtk_text_view_new();
    notebook = gtk_notebook_new();
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, NULL);
    gtk_widget_set_size_request(notebook, 460, 360);


//  创建盒子
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), search_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 5);

    gtk_paned_add2(GTK_PANED(paned), vbox);

    gtk_widget_grab_focus(GTK_WIDGET(entry)); //使entry在程序启动时即获取焦点
    return window;
}
