#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <config.h>
#include "nautilus-properties-window.h"

#define PROPERTIES_TYPE (properties_get_type ())
#define PROPERTIES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PROPERTIES_TYPE, Properties))

typedef struct _Properties {
    GtkApplication parent;
} Properties;


typedef struct _PropertiesClass {
    GtkApplicationClass parent_class;
} PropertiesClass;


G_DEFINE_TYPE(Properties, properties, GTK_TYPE_APPLICATION);


static
void properties_activate(GApplication* app)
{
    g_printf("Please pass file uri.\n");
}


static
void properties_open(GApplication* app, GFile** files, gint n_files, const gchar* hint)
{
    int i;
    GList* file_list = NULL;

    for (i = 0; i < n_files; ++i) {
        char* uri = g_file_get_uri(files[i]);
        NautilusFile *file = nautilus_file_get_by_uri(uri);
        g_free(uri);
        file_list = g_list_prepend(file_list, file);
    }

    file_list = g_list_reverse(file_list);

    nautilus_properties_window_present (file_list, NULL, NULL);

    nautilus_file_list_free (file_list);
    gtk_main();
}


void properties_init(Properties* app)
{
}


void properties_class_init(PropertiesClass* app)
{
    G_APPLICATION_CLASS(app)->activate = properties_activate;
    G_APPLICATION_CLASS(app)->open = properties_open;
}


int main(int argc, char *argv[])
{
    /* Initialize gettext support */
    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    int status = -1;

    GObject* app = g_object_new(PROPERTIES_TYPE,
                                "application-id", "com.deepin.properties",
                                "flags", G_APPLICATION_HANDLES_OPEN,
                                NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
