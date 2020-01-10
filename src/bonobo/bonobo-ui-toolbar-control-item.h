/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-control-item.c: A special toolbar item for controls.
 *
 * Author:
 *	Jon K Hellan (hellan@acm.org)
 *
 * Copyright 2000 Jon K Hellan.
 */
#ifndef _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_
#define _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_

#include <glib.h>
#include "bonobo-ui-toolbar-button-item.h"
#include "bonobo-widget.h"

/* All private / non-installed header */

G_BEGIN_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM            (bonobo_ui_toolbar_control_item_get_type ())
#define BONOBO_UI_TOOLBAR_CONTROL_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM, BonoboUIToolbarControlItem))
#define BONOBO_UI_TOOLBAR_CONTROL_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM, BonoboUIToolbarControlItemClass))
#define BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM))
#define BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM))

typedef enum {
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL,
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON,
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE
} BonoboUIToolbarControlDisplay;

typedef struct _BonoboUIToolbarControlItemPrivate BonoboUIToolbarControlItemPrivate;

typedef struct {
	GtkToolButton parent;

	GtkWidget    *widget;   /* The widget (of a control, or custom */
        BonoboWidget *control;	/* The wrapped control - if a control, or NULL */
	GtkWidget    *box;      /* Box to pack innards inside to avoid reparenting */
	GtkWidget    *button;	/* GtkToolItem's internal button */

	BonoboUIToolbarControlDisplay hdisplay;
	BonoboUIToolbarControlDisplay vdisplay;
} BonoboUIToolbarControlItem;

typedef struct {
	GtkToolButtonClass parent_class;
} BonoboUIToolbarControlItemClass;

GType       bonobo_ui_toolbar_control_item_get_type    (void) G_GNUC_CONST;
GtkWidget    *bonobo_ui_toolbar_control_item_new         (Bonobo_Control control_ref);
GtkWidget    *bonobo_ui_toolbar_control_item_new_widget  (GtkWidget *custom_in_proc_widget);
GtkWidget    *bonobo_ui_toolbar_control_item_construct   (BonoboUIToolbarControlItem *control_item,
							  GtkWidget                  *widget,
							  Bonobo_Control              control_ref);
void          bonobo_ui_toolbar_control_item_set_display (BonoboUIToolbarControlItem    *item,
							  BonoboUIToolbarControlDisplay  hdisplay,
							  BonoboUIToolbarControlDisplay  vdisplay);

G_END_DECLS

#endif /* _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_ */
