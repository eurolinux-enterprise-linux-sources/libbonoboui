/*
 * moniker-test.c: Test program for monikers resolving to various interfaces.
 *
 * Authors:
 *   Vladimir Vukicevic (vladimir@ximian.com)
 *   Michael Meeks      (michael@ximian.com)
 *
 * Based on moniker-control-test.c, by Joe Shaw (joe@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#undef GTK_DISABLE_DEPRECATED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <glib.h>
#include <libbonoboui.h>

static int async_done;

typedef enum {
	AS_NONE = 0,
	AS_INTERFACE,
	AS_STREAM,
	AS_STORAGE_FILE_LIST,
	AS_CONTROL
} MonikerTestDisplayAs;

typedef void (*MonikerDisplayFunction) (const char *moniker, CORBA_Environment *ev);

typedef struct {
	MonikerTestDisplayAs   disp_as;
	MonikerDisplayFunction func;
} MonikerTestDisplayers;

typedef struct {
	gchar *requested_interface;
	gchar *requested_moniker;
	MonikerTestDisplayAs display_as;
	gchar *moniker;

	gboolean async, ps, pr, pc;
} MonikerTestOptions;

static gchar **remaining = NULL;

static MonikerTestOptions global_mto = { NULL };

static const GOptionEntry moniker_test_options [] = {
	{ "interface", 'i', 0, G_OPTION_ARG_STRING, &global_mto.requested_interface, "request specific interface", "interface" },
	{ "async",     'a', 0, G_OPTION_ARG_NONE, &global_mto.async, "request asynchronous operation where possible", NULL },
	{ "stream",    's', 0, G_OPTION_ARG_NONE, &global_mto.ps, "request Bonobo/Stream", NULL },
	{ "storage",   'r', 0, G_OPTION_ARG_NONE, &global_mto.pr, "request Bonobo/Storage", NULL },
	{ "control",   'c', 0, G_OPTION_ARG_NONE, &global_mto.pc, "request Bonobo/Control", NULL },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &remaining, "moniker", "moniker" },
	{ NULL }
};

static void
display_as_interface (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown the_unknown;

	the_unknown = bonobo_get_object (moniker, global_mto.requested_interface, ev);
	if (ev->_major == CORBA_NO_EXCEPTION && the_unknown) {
		fprintf (stderr, "Requesting interface %s: SUCCESS\n", global_mto.requested_interface);
		bonobo_object_release_unref (the_unknown, ev);
		return;
	}

	fprintf (stderr, "Requesting interface: %s: EXCEPTION: %s:%s\n",
		 global_mto.requested_interface,
		 BONOBO_EX_REPOID (ev),
		 bonobo_exception_get_text (ev));
}

static void
dump_stream (Bonobo_Stream the_stream, CORBA_Environment *ev)
{
	fprintf (stderr, "Writing stream to stdout...\n");
	do {
		Bonobo_Stream_iobuf *stream_iobuf;
		Bonobo_Stream_read (the_stream, 512, &stream_iobuf, ev);
		if (BONOBO_EX (ev)) {
			bonobo_object_release_unref (the_stream, ev);
			g_error ("got exception %s while reading from stream!",
				 BONOBO_EX_REPOID (ev));
		}

		if (stream_iobuf->_length == 0) {
			CORBA_free (stream_iobuf);
			bonobo_object_release_unref (the_stream, ev);
			return;
		}

		fwrite (stream_iobuf->_buffer, stream_iobuf->_length, 1,
			stdout);

		CORBA_free (stream_iobuf);
	} while (1);
}


static void
disp_stream_async_cb (Bonobo_Unknown     object,
		      CORBA_Environment *ev,
		      gpointer           user_data)
{
	if (BONOBO_EX (ev) || !object) {
		g_error ("Couldn't get Bonobo/Stream interface '%s'",
			 bonobo_exception_get_text (ev));
	} else
		dump_stream (object, ev);

	async_done = 1;
}

static void
display_as_stream (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Stream the_stream;

	if (global_mto.async) {
		bonobo_get_object_async (moniker, "IDL:Bonobo/Stream:1.0", ev,
					 disp_stream_async_cb, NULL);

		if (BONOBO_EX (ev))
			g_error ("Couldn't get Bonobo/Stream '%s'",
				 bonobo_exception_get_text (ev));
	} else {
		the_stream = bonobo_get_object (moniker, "IDL:Bonobo/Stream:1.0", ev);
		if (BONOBO_EX (ev) || !the_stream) {
			g_error ("Couldn't get Bonobo/Stream interface '%s'",
				 bonobo_exception_get_text (ev));
		}

		dump_stream (the_stream, ev);
	}
}

static void
display_as_storage_file_list (const char *moniker, CORBA_Environment *ev)
{
    Bonobo_Storage the_storage;
    Bonobo_Storage_DirectoryList *storage_contents;
    Bonobo_StorageInfo *bsi;
    int i;

    the_storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", ev);
    if (BONOBO_EX (ev) || !the_storage) {
        g_error ("Couldn't get Bonobo/Storage interface");
    }

    storage_contents = Bonobo_Storage_listContents (the_storage,
                                                    "",
                                                    Bonobo_FIELD_CONTENT_TYPE |
                                                    Bonobo_FIELD_SIZE |
                                                    Bonobo_FIELD_TYPE,
                                                    ev);
    if (!storage_contents || (storage_contents && !storage_contents->_buffer)) {
        g_error ("got NULL storage_contents!\n");
    }

    bsi = storage_contents->_buffer;
    printf ("Storage List\n");
    printf ("------------\n");
    for (i = 0; i < storage_contents->_length; i++) {
        printf ("% 3d: %20s % 10d %c %15s\n",
                i,
                bsi[i].name,
                bsi[i].size,
                bsi[i].type == Bonobo_STORAGE_TYPE_DIRECTORY ? 'd' : 'r',
                bsi[i].content_type);
    }

    bonobo_object_release_unref (the_storage, ev);
    /* how do I free the silly dirlist? */
}

static void
display_control_async_cb (BonoboWidget       *widget,
			  CORBA_Environment  *ev,
			  gpointer            user_data)
{
	if (BONOBO_EX (ev)) {
		GtkWidget *label = gtk_label_new ("Failed to activate");
		char      *err;
		g_warning ("Exception '%s'", (err = bonobo_exception_get_text (ev)));
		g_free (err);
		gtk_widget_destroy (GTK_WIDGET (widget));
		bonobo_window_set_contents (BONOBO_WINDOW (user_data), label); 
	} else
		bonobo_control_frame_control_activate (
			bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));
	async_done = 1;
}

static void
display_as_control (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Control     the_control;
	GtkWidget         *widget;
	BonoboUIContainer *ui_container;

	GtkWidget *window;

	window = bonobo_window_new ("moniker-test", moniker);
	ui_container = bonobo_window_get_ui_container (BONOBO_WINDOW (window));
	gtk_window_set_default_size (GTK_WINDOW (window), 400, 350);

	if (global_mto.async) {
		widget = bonobo_widget_new_control_async (
			moniker, BONOBO_OBJREF (ui_container),
			display_control_async_cb, window);
	} else {
		the_control = bonobo_get_object (moniker, "IDL:Bonobo/Control:1.0", ev);
		if (BONOBO_EX (ev) || !the_control)
			g_error ("Couldn't get Bonobo/Control interface: '%s'",
				 bonobo_exception_get_text (ev));

		widget = bonobo_widget_new_control_from_objref (
			the_control, BONOBO_OBJREF (ui_container));

		bonobo_object_release_unref (the_control, ev);
	}

	if (BONOBO_EX (ev) || !widget)
		g_error ("Couldn't get a widget from the_control");

	if (!global_mto.async)
		bonobo_control_frame_control_activate (
			bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));

	bonobo_window_set_contents (BONOBO_WINDOW (window), widget);

	g_signal_connect (window, "destroy",
			  G_CALLBACK (bonobo_main_quit), NULL);

	gtk_widget_show_all (window);
	bonobo_main ();
}

static MonikerTestDisplayers displayers[] = {
	{ AS_INTERFACE, display_as_interface },
	{ AS_STREAM, display_as_stream },
	{ AS_STORAGE_FILE_LIST, display_as_storage_file_list },
	{ AS_CONTROL, display_as_control },
	{0 }
};

static void
do_moniker_magic (void)
{
	CORBA_Environment ev;
	MonikerTestDisplayers *iter = displayers;
	CORBA_exception_init (&ev);

	while (iter->disp_as) {
		if (iter->disp_as == global_mto.display_as) {
			(*iter->func) (global_mto.requested_moniker, &ev);
			CORBA_exception_free (&ev);
			return;
		}
		iter++;
	}

	g_error ("Didn't find handler!");
}

int
main (int argc, char **argv)
{
	GnomeProgram *program;
	GOptionContext *context = NULL;

	free (malloc (8)); /* -lefence */
	
	context = g_option_context_new ("- test-moniker");
	
	g_option_context_add_main_entries (context, moniker_test_options, GETTEXT_PACKAGE);

	program = gnome_program_init ("- test-moniker", VERSION,
			    LIBBONOBOUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_GOPTION_CONTEXT, context,
			    NULL);

	if (global_mto.requested_interface)
		global_mto.display_as = AS_INTERFACE;
	else if (global_mto.ps)
		global_mto.display_as = AS_STREAM;
	else if (global_mto.pr)
		global_mto.display_as = AS_STORAGE_FILE_LIST;
	else if (global_mto.pc)
		global_mto.display_as = AS_CONTROL;
	else {
		fprintf (stderr, "%s", g_option_context_get_help (context, TRUE, NULL));
		return 1;
	}

	if (remaining!=NULL)
		global_mto.requested_moniker = remaining[0];

	if (global_mto.requested_moniker == NULL) {
		fprintf (stderr, "%s", g_option_context_get_help (context, TRUE, NULL));
		return 1;
	}

	fprintf (stderr, "Resolving moniker '%s' as ", global_mto.requested_moniker);
	switch (global_mto.display_as) {
        case AS_INTERFACE:
		fprintf (stderr, "%s", global_mto.requested_interface);
		break;
        case AS_STREAM:
		fprintf (stderr, "IDL:Bonobo/Stream:1.0");
		break;
        case AS_STORAGE_FILE_LIST:
		fprintf (stderr, "IDL:Bonobo/Storage:1.0");
		break;
        case AS_CONTROL:
		fprintf (stderr, "IDL:Bonobo/Control:1.0");
		break;
        default:
		fprintf (stderr, "???");
		break;
	}
	if (global_mto.async)
		fprintf (stderr, " asynchronously");
	fprintf (stderr, "\n");

	bonobo_activate ();

	async_done = 0;

	do_moniker_magic ();

	while (global_mto.async && !async_done)
		g_main_context_iteration (NULL, TRUE);

	g_object_unref (program);

	return bonobo_ui_debug_shutdown ();
}
