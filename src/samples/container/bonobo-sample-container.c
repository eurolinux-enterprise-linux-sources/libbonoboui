#undef GTK_DISABLE_DEPRECATED

#include <unistd.h>
#include <bonobo.h>
#include <glib.h>

#define APPNAME "TestContainer"
#define APPVERSION "1.0"

#define UI_XML "Bonobo_Sample_Container-ui.xml"

static gchar **files = NULL;
static gboolean use_gtk_window = FALSE;
static const GOptionEntry options[] = {
	{ "gtk", 'g', 0, G_OPTION_ARG_NONE, &use_gtk_window,
	  "Use GtkWindow instead of BonoboWindow (default)", NULL },
	{ G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "FILE" },
	{ NULL }
};

/*
 * FIXME: TODO:
 *
 *  + Add a Menu item 'activate' to do a control_frame_activate /
 *  trigger a UI merge / de-merge.
 *  + Add gtk-only mode
 */
static void
window_destroyed (GtkWindow * window, gpointer user_data)
{
        g_warning ("FIXME: should count toplevels");
        bonobo_main_quit ();
}

static void
window_title (GtkWindow * window, const char *moniker, gboolean use_gtk)
{
        char *title;

        title = g_strdup_printf ("%s - in a %s", moniker,
                                 g_type_name_from_instance ((gpointer)
                                                            window));
        gtk_window_set_title (window, title);
        g_free (title);
}

static void
verb_HelpAbout (BonoboUIComponent * uic, gpointer user_data,
                 const char *cname)
{
        g_message
            ("Unfortunately I cannot use gnome_about API - it would introduce more dependencies to libbonoboui");
}

static void
verb_FileExit (BonoboUIComponent * uic, gpointer user_data,
                const char *cname)
{
        bonobo_main_quit ();
}

static void
verb_Activate (BonoboUIComponent * uic, gpointer user_data,
                const char *cname)
{
        bonobo_control_frame_control_activate
             (bonobo_widget_get_control_frame
             (BONOBO_WIDGET
             (bonobo_window_get_contents
             (BONOBO_WINDOW (user_data)))));
}

static BonoboUIVerb bonobo_app_verbs[] = {
        BONOBO_UI_VERB ("FileExit", verb_FileExit),
        BONOBO_UI_VERB ("HelpAbout", verb_HelpAbout),
        BONOBO_UI_VERB ("Activate", verb_Activate),
        BONOBO_UI_VERB_END
};

static void
window_create (const char *moniker, gboolean use_gtk)
{
        GtkWidget *window;
        GtkWidget *control;
        BonoboUIContainer *ui_container;

        if (use_gtk) {
                window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
                ui_container = CORBA_OBJECT_NIL;
        } else {
                BonoboUIComponent *ui_comp =
                    bonobo_ui_component_new_default ();
                window = bonobo_window_new (APPNAME, APPNAME);
                ui_container =
                    bonobo_window_get_ui_container (BONOBO_WINDOW
                                                    (window));
                bonobo_ui_component_set_container (ui_comp,
                                                   BONOBO_OBJREF
                                                   (ui_container),
                                                   NULL);
                bonobo_ui_util_set_ui (ui_comp, "", UI_XML,
                                       APPNAME, NULL);
                bonobo_ui_component_add_verb_list_with_data (ui_comp,
                                                             bonobo_app_verbs,
                                                             window);
        }

        window_title (GTK_WINDOW (window), moniker, use_gtk);

        control = bonobo_widget_new_control (moniker,
                                             BONOBO_OBJREF (ui_container));

        if (!control) {
                g_warning ("Couldn't create a control for '%s'",
                           moniker);
                return;
        }

        if (use_gtk) {
                gtk_container_add (GTK_CONTAINER (window), control);
        } else
                bonobo_window_set_contents (BONOBO_WINDOW (window),
                                             control);

        g_signal_connect (window, "destroy",
                           G_CALLBACK (window_destroyed ), NULL);

        gtk_widget_show_all (window);
}

int
main (int argc, char *argv[])
{
        int i;
        GOptionContext *context;
        GnomeProgram *program;
        
        context = g_option_context_new (NULL);
        g_option_context_add_main_entries (context, options, "bonobo-sample-container");
        
        program = gnome_program_init (APPNAME, APPVERSION,
                                      LIBBONOBOUI_MODULE,
                                      argc, argv,
                                      GNOME_PARAM_GOPTION_CONTEXT, context,
                                      GNOME_PARAM_NONE);

        /* Check for argument consistency. */
        if (files == NULL) {
                g_message ("Must specify a filename");
                return 1;
        }

        for (i = 0; i < g_strv_length (files); i++) {
                char *moniker;

                /* FIXME: we should do some auto-detection here */
                moniker = g_strdup_printf ("file:%s", files[i]);

                window_create (moniker, use_gtk_window);
        }

        bonobo_main ();

        g_object_unref (program);

        return 0;
}
