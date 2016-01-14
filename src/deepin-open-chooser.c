#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include <config.h>
#include <libnautilus-private/nautilus-file.h>
#include <libnautilus-private/nautilus-program-choosing.h>

static void
app_chooser_dialog_response_cb (GtkDialog *dialog,
                                gint       response_id,
                                gpointer   user_data)
{
    GtkWindow *parent_window;
    GList *files;
    GAppInfo *info;

    parent_window = user_data;
    files = g_object_get_data (G_OBJECT (dialog), "directory-view:files");

    if (response_id != GTK_RESPONSE_OK)
        goto out;

    info = gtk_app_chooser_get_app_info (GTK_APP_CHOOSER (dialog));

    /* g_signal_emit_by_name (nautilus_signaller_get_current (), "mime-data-changed"); */

    nautilus_launch_application (info, files, parent_window);

    g_object_unref (info);
out:
    gtk_widget_destroy (GTK_WIDGET (dialog));
    gtk_main_quit();
}

static void
choose_program (GList             *files)
{
    GtkWidget *dialog;
    gchar *mime_type;
    GtkWindow *parent_window;

    mime_type = nautilus_file_get_mime_type (files->data);
    g_warning("mime_type: %s\n", mime_type);
    /* parent_window = nautilus_files_view_get_containing_window (view); */
    parent_window = NULL;

    dialog = gtk_app_chooser_dialog_new_for_content_type (parent_window,
                                                          GTK_DIALOG_MODAL |
                                                          GTK_DIALOG_DESTROY_WITH_PARENT |
                                                          GTK_DIALOG_USE_HEADER_BAR,
                                                          mime_type);
    g_object_set_data_full (G_OBJECT (dialog),
                            "directory-view:files",
                            files,
                            (GDestroyNotify) nautilus_file_list_free);
    gtk_widget_show (dialog);

    g_signal_connect_object (dialog, "response",
                             G_CALLBACK (app_chooser_dialog_response_cb),
                             parent_window, 0);
}

#define OPEN_CHOOSER_TYPE (open_chooser_get_type())
#define OPEN_CHOOSER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), OPEN_CHOOSER_TYPE, Chooser))

typedef struct _Chooser{
    GtkApplication parent;
} OpenChooser;

typedef struct _ChooserClass{
    GtkApplicationClass parent_class;
} OpenChooserClass;

G_DEFINE_TYPE(OpenChooser, open_chooser, GTK_TYPE_APPLICATION);

static
void open_chooser_activate(GApplication* app)
{
    g_printf("Please pass file uri.\n");
}

static
void open_chooser_open(GApplication* app, GFile** files, gint n_files, const gchar* hint)
{
    GList* files_list = NULL;
    for (int i = 0; i < n_files; ++i) {
        NautilusFile* file = nautilus_file_get(files[i]);
        GFileInfo* info = g_file_query_info(files[i], "*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        nautilus_file_update_info(file, info);
        if (info != NULL) g_object_unref(info);
        files_list = g_list_append(files_list, file);
    }

    choose_program(files_list);
    gtk_main();
}

void open_chooser_init(OpenChooser* app)
{
}

void open_chooser_class_init(OpenChooserClass* cls)
{
    G_APPLICATION_CLASS(cls)->activate = open_chooser_activate;
    G_APPLICATION_CLASS(cls)->open = open_chooser_open;
}

int main(int argc, char* argv[])
{
    /* Initialize gettext support */
    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    GObject* app = g_object_new(OPEN_CHOOSER_TYPE,
                                "application-id", "com.deepin.OpenChooser",
                                "flags", G_APPLICATION_HANDLES_OPEN,
                                NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
