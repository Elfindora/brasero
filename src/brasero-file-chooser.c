/***************************************************************************
 *            brasero-file-chooser.c
 *
 *  lun mai 29 08:53:18 2006
 *  Copyright  2006  Rouquier Philippe
 *  brasero-app@wanadoo.fr
 ***************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib-object.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkfilechooserwidget.h>

#include "eggtreemultidnd.h"

#include "brasero-file-chooser.h"
#include "brasero-uri-container.h"

static void brasero_file_chooser_class_init (BraseroFileChooserClass *klass);
static void brasero_file_chooser_init (BraseroFileChooser *sp);
static void brasero_file_chooser_iface_uri_container_init (BraseroURIContainerIFace *iface);
static void brasero_file_chooser_finalize (GObject *object);

static gchar **
brasero_file_chooser_get_selected_uris (BraseroURIContainer *container);
static gchar *
brasero_file_chooser_get_selected_uri (BraseroURIContainer *container);

static void
brasero_file_chooser_uri_activated_cb (GtkFileChooser *widget,
				       BraseroFileChooser *chooser);
static void
brasero_file_chooser_uri_selection_changed_cb (GtkFileChooser *widget,
					       BraseroFileChooser *chooser);
struct BraseroFileChooserPrivate {
	GtkWidget *chooser;
};

static GObjectClass *parent_class = NULL;

GType
brasero_file_chooser_get_type ()
{
	static GType type = 0;

	if(type == 0) {
		static const GTypeInfo our_info = {
			sizeof (BraseroFileChooserClass),
			NULL,
			NULL,
			(GClassInitFunc)brasero_file_chooser_class_init,
			NULL,
			NULL,
			sizeof (BraseroFileChooser),
			0,
			(GInstanceInitFunc)brasero_file_chooser_init,
		};

		static const GInterfaceInfo uri_container_info =
		{
			(GInterfaceInitFunc) brasero_file_chooser_iface_uri_container_init,
			NULL,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_ALIGNMENT, 
					       "BraseroFileChooser",
					       &our_info,
					       0);

		g_type_add_interface_static (type,
					     BRASERO_TYPE_URI_CONTAINER,
					     &uri_container_info);
	}

	return type;
}

static void
brasero_file_chooser_iface_uri_container_init (BraseroURIContainerIFace *iface)
{
	iface->get_selected_uri = brasero_file_chooser_get_selected_uri;
	iface->get_selected_uris = brasero_file_chooser_get_selected_uris;
}

static void
brasero_file_chooser_class_init (BraseroFileChooserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = brasero_file_chooser_finalize;
}

static void
brasero_file_chooser_set_multi_DND (GtkWidget *widget, gpointer null_data)
{
	/* we explore everything until we reach a treeview (there are two) */
	if (GTK_IS_TREE_VIEW (widget)) {
		GtkTargetList *list;
		GdkAtom target;
		gboolean found;
		guint num;

		list = gtk_drag_source_get_target_list (widget);
		target = gdk_atom_intern ("text/uri-list", TRUE);
		found = gtk_target_list_find (list, target, &num);
		gtk_target_list_unref (list);

		if (found) {
			gtk_tree_view_set_rubber_banding (GTK_TREE_VIEW (widget), TRUE);
			egg_tree_multi_drag_add_drag_support (GTK_TREE_VIEW (widget));
		}
	}
	else if (GTK_IS_CONTAINER (widget)) {
		gtk_container_foreach (GTK_CONTAINER (widget),
				       brasero_file_chooser_set_multi_DND,
				       NULL);
	}
}

static void
brasero_file_chooser_init (BraseroFileChooser *obj)
{
	GtkFileFilter *filter;

	obj->priv = g_new0 (BraseroFileChooserPrivate, 1);

	obj->priv->chooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (obj->priv->chooser), FALSE);

	gtk_widget_show (obj->priv->chooser);

	gtk_container_add (GTK_CONTAINER (obj), obj->priv->chooser);

	g_signal_connect (obj->priv->chooser,
			  "file-activated",
			  G_CALLBACK (brasero_file_chooser_uri_activated_cb),
			  obj);
	g_signal_connect (obj->priv->chooser,
			  "selection-changed",
			  G_CALLBACK (brasero_file_chooser_uri_selection_changed_cb),
			  obj);

	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (obj->priv->chooser), TRUE);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (obj->priv->chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Audio files only"));
	gtk_file_filter_add_mime_type (filter, "audio/*");
	gtk_file_filter_add_mime_type (filter, "application/ogg");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (obj->priv->chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Movies only"));
	gtk_file_filter_add_mime_type (filter, "video/*");
	gtk_file_filter_add_mime_type (filter, "application/ogg");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (obj->priv->chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Image files only"));
	gtk_file_filter_add_mime_type (filter, "image/*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (obj->priv->chooser), filter);

	/* this is a hack/workaround to add support for multi DND */
	gtk_container_foreach (GTK_CONTAINER (obj->priv->chooser),
			       brasero_file_chooser_set_multi_DND,
			       NULL);
}

static void
brasero_file_chooser_finalize (GObject *object)
{
	BraseroFileChooser *cobj;

	cobj = BRASERO_FILE_CHOOSER (object);
	g_free (cobj->priv);

	G_OBJECT_CLASS(parent_class)->finalize(object);
}

GtkWidget *
brasero_file_chooser_new ()
{
	gpointer *obj;
	
	obj = g_object_new (BRASERO_TYPE_FILE_CHOOSER, NULL);
	
	return GTK_WIDGET (obj);
}

static char *
brasero_file_chooser_get_selected_uri (BraseroURIContainer *container)
{
	BraseroFileChooser *chooser;

	chooser = BRASERO_FILE_CHOOSER (container);
	return gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (chooser->priv->chooser));
}

static char **
brasero_file_chooser_get_selected_uris (BraseroURIContainer *container)
{
	BraseroFileChooser *chooser;
	GSList *list, *iter;
	gchar **uris;
	gint i;

	chooser = BRASERO_FILE_CHOOSER (container);
	list = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (chooser->priv->chooser));

	uris = g_new0 (gchar*, g_slist_length (list) + 1);
	i = 0;

	for (iter = list; iter; iter = iter->next) {
		uris [i] = iter->data;
		i++;
	}

	g_slist_free (list);

	return uris;
}

static void
brasero_file_chooser_uri_activated_cb (GtkFileChooser *widget,
				       BraseroFileChooser *chooser)
{
	brasero_uri_container_uri_activated (BRASERO_URI_CONTAINER (chooser));
}

static void
brasero_file_chooser_uri_selection_changed_cb (GtkFileChooser *widget,
					       BraseroFileChooser *chooser)
{
	brasero_uri_container_uri_selected (BRASERO_URI_CONTAINER (chooser));
}
