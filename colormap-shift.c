/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Colormap shiftping plug-in
 * Copyright (C) 2022 Jestin Stoffel <jestin.stoffel@gmail.com>
 *
 * This plug-in takes the colormap and lets you shift the colors in the
 * colormap by a given offset, enabling you to edit indexed images that are
 * meant to be palette shifted.
 *
 */

// #include "config.h"

#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#define PLUG_IN_PROC_SHIFT  "plug-in-colormap-shift"
#define PLUG_IN_BINARY      "colormap-shift"
#define PLUG_IN_ROLE        "gimp-colormap-shift"


/* Declare local functions.
 */
static void       query        (void);
static void       run          (const gchar      *name,
                                gint              nparams,
                                const GimpParam  *param,
                                gint             *nreturn_vals,
                                GimpParam       **return_vals);

static void shift_color_map(guchar* orig,
		guchar** shifted,
		gint palsize,
		gint offset);

static gboolean   shift_dialog (gint32            image_ID,
                                guchar           *map);


const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

typedef struct
{
	guint8     offset;	 /* number of collors to offset the colormap */
} ColorShiftVals;

static const ColorShiftVals defaults = {
	16
};

static ColorShiftVals colorshiftvals = defaults;

MAIN ()

static void
query (void)
{
	static const GimpParamDef shift_args[] =
	{
		{ GIMP_PDB_INT32,     "run-mode",   "The run mode { RUN-NONINTERACTIVE (1) }"  },
		{ GIMP_PDB_IMAGE,     "image",      "Input image"                          },
		{ GIMP_PDB_DRAWABLE,  "drawable",   "Input drawable"                       },
		{ GIMP_PDB_INT8,      "offset",     "The number of collors to shift"          }
	};

	gimp_install_procedure (PLUG_IN_PROC_SHIFT,
			"Shift the colors in the color map",
			"This procedure takes an indexed image and lets you "
			"shift the colors a given offset into the colormap."
			"This allows you to edit the same image using different palettes.",
			"Jestin Stoffel <jestin.stoffel@gmail.com>",
			"Copyright 2022 by Jestin Stoffel",
			"0.0.1 2022",
			"Shift Colors",
			"INDEXED*",
			GIMP_PLUGIN,
			G_N_ELEMENTS (shift_args), 0,
			shift_args, NULL);

	gimp_plugin_menu_register (PLUG_IN_PROC_SHIFT, "<Image>/Colors/Map/Colormap");
	gimp_plugin_menu_register (PLUG_IN_PROC_SHIFT, "<Colormap>");
	gimp_plugin_icon_register (PLUG_IN_PROC_SHIFT, GIMP_ICON_TYPE_ICON_NAME,
			(const guint8 *) GIMP_ICON_COLORMAP);
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
	 GimpParam       **return_vals)
{
	static GimpParam   values[1];
	gint32             image_ID;
	GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
	GimpRunMode        run_mode;
	guchar             map[256];
	gint               i;

	gegl_init (NULL, NULL);

	run_mode = param[0].data.d_int32;

	*nreturn_vals = 1;
	*return_vals  = values;

	values[0].type          = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	image_ID = param[1].data.d_image;

	gint palsize;
	guchar *cmap = gimp_image_get_colormap (image_ID, &palsize);
	guchar* shifted_map = (guchar*)malloc(sizeof(guchar) * palsize);

	for (i = 0; i < 256; i++)
		map[i] = i;

	if (strcmp (name, PLUG_IN_PROC_SHIFT) == 0)
	{
		/*  Make sure that the image is indexed  */
		if (gimp_image_base_type (image_ID) != GIMP_INDEXED)
			status = GIMP_PDB_EXECUTION_ERROR;

		if (status == GIMP_PDB_SUCCESS)
		{
			switch (run_mode)
			{
				case GIMP_RUN_INTERACTIVE:
					// TODO: replace with call to dialog
					// shift_color_map (cmap, &shifted_map, palsize, 16);
					// gimp_image_set_colormap(image_ID, shifted_map, palsize);
					// status = GIMP_PDB_SUCCESS;

					if (! shift_dialog (image_ID, map))
						status = GIMP_PDB_CANCEL;
					break;

				case GIMP_RUN_NONINTERACTIVE:
					if (nparams != 5)
						status = GIMP_PDB_CALLING_ERROR;

					if (status == GIMP_PDB_SUCCESS)
					{
						if (palsize != param[3].data.d_int32)
							status = GIMP_PDB_CALLING_ERROR;

						if (status == GIMP_PDB_SUCCESS)
						{
							for (i = 0; i < palsize; i++)
								map[i] = param[4].data.d_int8array[i];
						}
					}
					break;
			}

			if (status == GIMP_PDB_SUCCESS)
			{
				shift_color_map (cmap, &shifted_map, palsize, 16);
				gimp_image_set_colormap(image_ID, shifted_map, palsize);

				if (run_mode == GIMP_RUN_INTERACTIVE)
					gimp_set_data (PLUG_IN_PROC_SHIFT, map, sizeof (map));

				if (run_mode != GIMP_RUN_NONINTERACTIVE)
					gimp_displays_flush ();
			}
		}
	}
	else
	{
		status = GIMP_PDB_CALLING_ERROR;
	}

	g_free(shifted_map);

	values[0].data.d_status = status;
}

/* dialog */

#define RESPONSE_RESET 1

enum
{
  COLOR_INDEX,
  COLOR_INDEX_TEXT,
  COLOR_RGB,
  NUM_COLS
};

static  GtkUIManager *shift_ui  = NULL;
static  gboolean      shift_run = FALSE;

static void shift_color_map(guchar* orig,
		guchar** shifted,
		gint palsize,
		gint offset)
{
	gint offset_base = offset * 3;
	for(int i = 0; i < palsize*3; i+=3)
	{
		if(i + offset_base >= palsize * 3)
		{
			// this is the last <offset> values in the color map
			(*shifted)[i] = orig[(i + offset_base) % palsize];
			(*shifted)[i+1] = orig[(i+1 + offset_base) % palsize];
			(*shifted)[i+2] = orig[(i+2 + offset_base) % palsize];
			continue;
		}

		(*shifted)[i] = orig[i+offset_base];
		(*shifted)[i+1] = orig[i+1+offset_base];
		(*shifted)[i+2] = orig[i+2+offset_base];
	}
}

static void
shift_reset_callback (GtkAction       *action,
                      GtkTreeSortable *store)
{
}

static GtkUIManager *
shift_ui_manager_new (GtkWidget    *window,
                      GtkListStore *store)
{
  static const GtkActionEntry actions[] =
  {
    {
      "reset", GIMP_ICON_RESET, "Reset Order", NULL, NULL,
      G_CALLBACK (shift_reset_callback)
    },
  };

  GtkUIManager   *ui_manager = gtk_ui_manager_new ();
  GtkActionGroup *group      = gtk_action_group_new ("Actions");
  GError         *error      = NULL;

  gtk_action_group_set_translation_domain (group, NULL);
  gtk_action_group_add_actions (group, actions, G_N_ELEMENTS (actions), store);

  gtk_ui_manager_insert_action_group (ui_manager, group, -1);
  g_object_unref (group);

  gtk_ui_manager_add_ui_from_string (ui_manager,
                                     "<ui>"
                                     "  <popup name=\"shift-popup\">"
                                     "    <menuitem action=\"reset\" />"
                                     "  </popup>"
                                     "</ui>",
                                     -1, &error);
  if (error)
    {
      g_warning ("error parsing ui: %s", error->message);
      g_clear_error (&error);
    }

  return ui_manager;
}

static gboolean
shift_popup_menu (GtkWidget      *widget,
                  GdkEventButton *event)
{
  GtkWidget *menu = gtk_ui_manager_get_widget (shift_ui, "/shift-popup");

  gtk_menu_set_screen (GTK_MENU (menu), gtk_widget_get_screen (widget));
  gtk_menu_popup (GTK_MENU (menu),
                  NULL, NULL, NULL, NULL,
                  event ? event->button : 0,
                  event ? event->time   : gtk_get_current_event_time ());

  return TRUE;
}

static gboolean
shift_button_press (GtkWidget      *widget,
                    GdkEventButton *event)
{
  if (gdk_event_triggers_context_menu ((GdkEvent *) event))
    return shift_popup_menu (widget, event);

  return FALSE;
}

static void
shift_response (GtkWidget       *dialog,
                gint             response_id,
                GtkTreeSortable *store)
{
  switch (response_id)
    {
    case RESPONSE_RESET:
      shift_reset_callback (NULL, store);
      break;

    case GTK_RESPONSE_OK:
      shift_run = TRUE;
      /* fallthrough */

    default:
      gtk_main_quit ();
      break;
    }
}

static gboolean
shift_dialog (gint32  image_ID,
              guchar *map)
{
  GtkWidget       *dialog;
  GtkWidget       *vbox;
  GtkWidget       *box;
  GtkWidget       *iconview;
  GtkListStore    *store;
  GtkCellRenderer *renderer;
  GtkTreeIter      iter;
  guchar          *cmap;
  gint             palsize, i;
  gboolean         valid;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new ("Shift Colormap", PLUG_IN_ROLE,
                            NULL, 0,
                            gimp_standard_help_func, PLUG_IN_PROC_SHIFT,

                            "_Reset",  RESPONSE_RESET,
                            "_Cancel", GTK_RESPONSE_CANCEL,
                            "_OK",     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gimp_window_set_transient (GTK_WINDOW (dialog));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, TRUE, TRUE, 0);

  cmap = gimp_image_get_colormap (image_ID, &palsize);

  g_return_val_if_fail ((palsize > 0) && (palsize <= 256), FALSE);

  store = gtk_list_store_new (NUM_COLS,
                              G_TYPE_INT, G_TYPE_STRING, GIMP_TYPE_RGB,
                              G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  for (i = 0; i < palsize; i++)
    {
      GimpRGB  rgb;
      GimpHSV  hsv;
      gint     index = map[i];

      gimp_rgb_set_uchar (&rgb,
                          cmap[index * 3],
                          cmap[index * 3 + 1],
                          cmap[index * 3 + 2]);
      gimp_rgb_to_hsv (&rgb, &hsv);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          COLOR_RGB,        &rgb,
                          -1);
    }

  g_free (cmap);

  shift_ui = shift_ui_manager_new (dialog, store);

  iconview = gtk_icon_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);

  gtk_box_pack_start (GTK_BOX (vbox), iconview, TRUE, TRUE, 0);

  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (iconview),
                                    GTK_SELECTION_SINGLE);
  gtk_icon_view_set_orientation (GTK_ICON_VIEW (iconview),
                                 GTK_ORIENTATION_VERTICAL);
  gtk_icon_view_set_columns (GTK_ICON_VIEW (iconview), 16);
  gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (iconview), 0);
  gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (iconview), 0);
  gtk_icon_view_set_reorderable (GTK_ICON_VIEW (iconview), FALSE);

  renderer = gimp_cell_renderer_color_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (iconview), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (iconview), renderer,
                                  "color", COLOR_RGB,
                                  NULL);
  g_object_set (renderer,
                "width", 0,
                NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (iconview), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (iconview), renderer,
                                  NULL);
  g_object_set (renderer,
                "size-points", 0.0,
                "xalign",      0.0,
                "ypad",        0,
                NULL);

  g_signal_connect (iconview, "popup-menu",
                    G_CALLBACK (shift_popup_menu),
                    NULL);

  g_signal_connect (iconview, "button-press-event",
                    G_CALLBACK (shift_button_press),
                    NULL);

  box = gimp_hint_box_new (("Drag and drop colors to rearrange the colormap.  "
                             "The numbers shown are the original indices.  "
                             "Right-click for a menu with sort options."));

  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (shift_response),
                    store);

  gtk_widget_show_all (dialog);

  gtk_main ();

  i = 0;

  for (valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
       valid;
       valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter))
    {
      gint index;

      gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                          COLOR_INDEX, &index,
                          -1);
      map[i++] = index;
    }

  gtk_widget_destroy (dialog);

  return shift_run;
}
