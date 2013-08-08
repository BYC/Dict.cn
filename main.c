#include <stdlib.h>
#include <gtk/gtk.h>
#include "ui.h"

int main (int argc, char *argv[])
{
    GtkWidget *window;
    gtk_init (&argc, &argv);
    window = generate_main_window();
    gtk_widget_show_all(window);
    gtk_main ();

    return 0;
}
