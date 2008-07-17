/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * brasero
 * Copyright (C) Philippe Rouquier 2007 <bonfire-app@wanadoo.fr>
 * 
 *  Brasero is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 * brasero is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with brasero.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef _BRASERO_VIDEO_TREE_MODEL_H_
#define _BRASERO_VIDEO_TREE_MODEL_H_

#include <glib-object.h>

G_BEGIN_DECLS

/* This DND target when moving nodes inside ourselves */
#define BRASERO_DND_TARGET_SELF_FILE_NODES	"GTK_TREE_MODEL_ROW"

struct _BraseroDNDVideoContext {
	GtkTreeModel *model;
	GList *references;
};
typedef struct _BraseroDNDVideoContext BraseroDNDVideoContext;

typedef enum {
	BRASERO_VIDEO_TREE_MODEL_NAME		= 0,
	BRASERO_VIDEO_TREE_MODEL_MIME_ICON,
	BRASERO_VIDEO_TREE_MODEL_SIZE,
	BRASERO_VIDEO_TREE_MODEL_EDITABLE,
	BRASERO_VIDEO_TREE_MODEL_COL_NUM
} BraseroVideoProjectColumn;

#define BRASERO_TYPE_VIDEO_TREE_MODEL             (brasero_video_tree_model_get_type ())
#define BRASERO_VIDEO_TREE_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BRASERO_TYPE_VIDEO_TREE_MODEL, BraseroVideoTreeModel))
#define BRASERO_VIDEO_TREE_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BRASERO_TYPE_VIDEO_TREE_MODEL, BraseroVideoTreeModelClass))
#define BRASERO_IS_VIDEO_TREE_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BRASERO_TYPE_VIDEO_TREE_MODEL))
#define BRASERO_IS_VIDEO_TREE_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), BRASERO_TYPE_VIDEO_TREE_MODEL))
#define BRASERO_VIDEO_TREE_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), BRASERO_TYPE_VIDEO_TREE_MODEL, BraseroVideoTreeModelClass))

typedef struct _BraseroVideoTreeModelClass BraseroVideoTreeModelClass;
typedef struct _BraseroVideoTreeModel BraseroVideoTreeModel;

struct _BraseroVideoTreeModelClass
{
	BraseroVideoProjectClass parent_class;
};

struct _BraseroVideoTreeModel
{
	BraseroVideoProject parent_instance;
};

GType brasero_video_tree_model_get_type (void) G_GNUC_CONST;

BraseroVideoTreeModel *
brasero_video_tree_model_new (void);

BraseroVideoFile *
brasero_video_tree_model_path_to_file (BraseroVideoTreeModel *self,
				       GtkTreePath *path);
GtkTreePath *
brasero_video_tree_model_file_to_path (BraseroVideoTreeModel *self,
				       BraseroVideoFile *file);

G_END_DECLS

#endif /* _BRASERO_VIDEO_TREE_MODEL_H_ */
