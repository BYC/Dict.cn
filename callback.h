#ifndef CALLBACK_H_INCLUDED
#define CALLBACK_H_INCLUDED

//判断输入有效性
gboolean is_text_valid(const char *text, gchar **tmp);

//退出事件函数
gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);

//输入完成事件函数
gint input_complete1(GtkWidget *widget, GdkEvent *event, butn_data *data); //search按钮
gint input_complete2(GtkWidget *widget, GdkEvent *event, gpointer data);   //输入框回车

//entry内容自动全选函数
gint entry_reinput(GtkWidget *widget, GdkEvent *event, gpointer data );
#endif // CALLBACK_H_INCLUDED
