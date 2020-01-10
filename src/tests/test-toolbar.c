#undef GTK_DISABLE_DEPRECATED

#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <bonobo/bonobo-ui-main.h>
#include <glib/gi18n.h>

#include <bonobo/bonobo-ui-toolbar.h>
#include <bonobo/bonobo-ui-toolbar-button-item.h>

static GtkWidget *
prepend_item (BonoboUIToolbar *toolbar,
	      const char *icon_file_name,
	      const char *label,
	      gboolean expandable)
{
	GtkWidget *item;
	GdkPixbuf *pixbuf;

	if (icon_file_name)
		pixbuf = gdk_pixbuf_new_from_file (icon_file_name, NULL);
	else
		pixbuf = NULL;

	item = bonobo_ui_toolbar_button_item_new (pixbuf, label);

	if (pixbuf)
		g_object_unref (pixbuf);

	bonobo_ui_toolbar_insert (toolbar, BONOBO_UI_TOOLBAR_ITEM (item), 0);
	bonobo_ui_toolbar_item_set_expandable (BONOBO_UI_TOOLBAR_ITEM (item), expandable);
	gtk_widget_show (item);

	return item;
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *toolbar;
	GtkWidget *frame;
	GtkWidget *item;
	GnomeProgram *program;

	/* ElectricFence rules. */
	free (malloc (1));

	program = gnome_program_init ("test-toolbar", VERSION,
			    LIBBONOBOUI_MODULE,
			    argc, argv, NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_widget_show (frame);
	gtk_container_add (GTK_CONTAINER (window), frame);

	toolbar = bonobo_ui_toolbar_new ();
	gtk_widget_show (toolbar);
	gtk_container_add (GTK_CONTAINER (frame), toolbar);

	/* bonobo_ui_toolbar_set_orientation (BONOBO_UI_TOOLBAR (toolbar), GTK_ORIENTATION_VERTICAL); */

	{ /* Test late icon adding */
		GtkWidget *image;

		item = prepend_item (BONOBO_UI_TOOLBAR (toolbar), NULL, "Debian", FALSE);

		image = gtk_image_new_from_file ("/usr/share/pixmaps/gnome-debian.png");
		bonobo_ui_toolbar_button_item_set_image (
			BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), image);
	}

	{ /* Test late label setting */
		item = prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-emacs.png", NULL, FALSE);
		bonobo_ui_toolbar_button_item_set_label (
			BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), "Emacs");
	}


	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/apple-green.png", "Green apple", FALSE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/apple-red.png", "Red apple (exp)", TRUE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-applets.png", "Applets", FALSE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-gimp.png", "Gimp (exp)", TRUE);

	gtk_widget_show (window);

	gtk_main ();

	g_object_unref (program);

	return 0;
}
