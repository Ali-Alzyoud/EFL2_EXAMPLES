/******************* Undo/Redo Handler ********************
 * This is simple example shows how to implement Undo and
 * Redo handling on Efl.Ui.Text.
 *
 * For simplicity sake this example store up to 5 changes
 * in history list to restore from, or recall to.
 *
 * Also shows how to handle Undo/Redo events triggered
 * by user action (Ctrl+Z and Ctrl+Y), or programmatically
 * using callback on button click.
 *
*/

#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

/******************** Undo/Redo logic ********************/

#define MAX_HISTORY_SIZE 5

static int history_index = 0;
static Eina_List *history_list = NULL;

static char *
_history_next_get(void)
{
   int size = eina_list_count(history_list);

   if (0 == size || (size - 1) == history_index)
   {
      return NULL;
   }

   history_index++;

   return (char *)eina_list_data_get(eina_list_nth_list(history_list, history_index));
}

static char *
_history_prev_get(void)
{
   int size = eina_list_count(history_list);

   if (0 == size || 0 == history_index)
   {
      return NULL;
   }

   history_index--;

   return (char *)eina_list_data_get(eina_list_nth_list(history_list, history_index));
}

static void
_history_append(char *text)
{
   int size = eina_list_count(history_list);
   char *new_text = NULL;

   if (!text)
      return;

   new_text = calloc(sizeof(char), strlen(text) + 1);
   if (!new_text)
      return;

   strcpy(new_text, text);

   if (0 != size && history_index + 1 < size)
   {
      char *n_text = NULL;
      Eina_List *nl = eina_list_nth_list(history_list, history_index + 1), *l;

      EINA_LIST_FOREACH(nl, l, n_text)
      {
         nl = eina_list_remove(l, n_text);
         free(n_text);
      }
   }
   history_list = eina_list_append(history_list, new_text);
   history_index++;
}

/*********************************************************/

/*************** EFL Undo/Redo Callback(s) ***************/

static void
_ui_text_undo_common(Eo* ui_text) {
   char *text = _history_prev_get();

   if ( text ) {
      efl2_text_markup_set(ui_text, text);
   }
}

static void
_ui_text_redo_common(Eo* ui_text) {
   char *text = _history_next_get();

   if ( text ) {
      efl2_text_markup_set(ui_text, text);
   }
}

static void
_ui_text_undo_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _ui_text_undo_common(ui_text);
}

static void
_ui_text_redo_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _ui_text_redo_common(ui_text);
}

static void
_ui_text_undo_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _ui_text_undo_common(ui_text);
}

static void
_ui_text_redo_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _ui_text_redo_common(ui_text);
}

static void
_ui_text_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   char *text = efl2_text_markup_get(ui_text);

   if ( text ) {
      _history_append(text);
   }
}

static void
_ui_text_update_text_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   Efl2_Text_Cursor *curs = efl2_text_raw_editable_main_cursor_get(ui_text);

   efl2_text_cursor_text_insert(curs, "EFL");
}

/*********************************************************/

/******************* Application Logic *******************/

static void
_gui_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   efl_exit(0);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *box, ui_text;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // when the user clicks "close" on a window there is a request to delete
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added),
                 efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));
   attribute_factory = efl_add(EFL2_TEXT_ATTRIBUTE_FACTORY, efl_main_loop_get());

   ui_text = efl_add(EFL2_UI_TEXT_CLASS, box,
                     efl2_text_markup_set(efl_added, "Hello World"),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.7),
                     efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
                     efl_pack(box, efl_added));

   // Register callback when text changed to track new changes
   efl_event_callback_add(ui_text, EFL2_CANVAS_TEXT_EVENT_CHANGED, _ui_text_changed_cb, ui_text);

   // Register callback when redu event called to recall future state (if exists)
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST, _ui_text_redo_cb, ui_text);

   // Register callback when undo event called to restore previous state (if exists)
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_EVENT_UNDO_REQUEST, _ui_text_undo_cb, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Undo"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _ui_text_undo_clicked_cb, ui_text));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Redo"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _ui_text_redo_clicked_cb, ui_text));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Update Text"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _ui_text_update_text_clicked_cb, ui_text));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()

/*********************************************************/

/* EOF */
