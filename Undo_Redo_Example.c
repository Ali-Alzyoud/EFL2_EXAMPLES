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

static Eina_List *history_index = NULL;
static Eina_List *history_list = NULL;
static Eina_Bool history_list_can_merge = EINA_FALSE;

static Efl2_Text_Change_Info *
_history_next_get(void)
{
   int size = eina_list_count(history_list);

   if (0 == size || !eina_list_next(history_index))
     {
        return NULL;
     }

   history_index = eina_list_next(history_index);

   return (Efl2_Text_Change_Info *)eina_list_data_get(history_index);
}

static Efl2_Text_Change_Info *
_history_prev_get(void)
{
   int size = eina_list_count(history_list);

   if (0 == size || !eina_list_prev(history_index))
     {
        return NULL;
     }

   history_index = eina_list_prev(history_index);

   return (Efl2_Text_Change_Info *)eina_list_data_get(history_index);
}

static void
_history_append(Efl2_Text_Change_Info *info)
{
   int size = eina_list_count(history_list);
   Efl2_Text_Change_Info *new_info = NULL;

   if (size == MAX_HISTORY_SIZE)
     {
        return;
     }

   if (!new_info)
        return;

   Eina_List *ll;
   history_list = eina_list_split_list(history_list, history_index,
                                       &ll);

   Efl2_Text_Change_Info *inf = NULL;
   EINA_LIST_FREE(history_list, inf)
     {
        eina_stringshare_del(inf->content);
        free(inf);
     }

   new_info = (Efl2_Text_Change_Info *)calloc(1, sizeof(*new_info));
   if (!new_info)
        return;

   memcpy(new_info, info, sizeof(*new_info));

   Efl2_Text_Change_Info *head_inf = eina_list_data_get(history_index);

   eina_stringshare_ref(new_info->content);

   if (history_list_can_merge && (new_info->insert == head_inf->insert))
        new_info->merge = EINA_TRUE;

   history_index = eina_list_append(history_index, new_info);


   history_index = eina_list_last(history_index);
   if (history_list == NULL)
     {
        history_list = history_index;
     }

   history_list_can_merge = EINA_TRUE;
}

static void
_user_undo(Eo *ui_text)
{
   Efl2_Text_Change_Info *info = _history_prev_get();

   while (info)
     {
        _user_undo_redo_do(ui_text, info, EINA_FALSE);

        if (info->merge)
          {
             info = _history_prev_get();
          }
        else
          {
             break;
          }
     }
}


static void
_user_undo_redo_do(Eo *ui_text, Efl2_Text_Change_Info *inf, Eina_Bool undo)
{
   printf("%s: %s", (undo) ? "Undo" : "Redo",
         inf->content);

   Efl_Text_Cursor *mcur = efl2_text_raw_editable_main_cursor_get(ui_text);
   efl2_text_cursor_position_set(mcur, inf->position);

   if ((inf->insert && undo) || (!inf->insert && !undo))
     {
        Efl_Text_Cursor  *end;
        end = efl2_ui_text_cursor_new(ui_text);
        efl2_text_cursor_position_set(end, inf->position + inf->length);

        efl2_text_cursor_range_delete(mcur, end);
        efl_del(end);
     }
   else
     {
        efl2_text_cursor_text_insert(mcur, inf->content);
     }

   /* No matter what, once we did an undo/redo we don't want to merge,
    * even if we got backt to the top of the stack. */
   history_list_can_merge = EINA_FALSE;
}

static void
_user_redo(Eo *ui_text)
{
   Efl2_Text_Change_Info *info = _history_next_get();

   while (info)
     {
        _user_undo_redo_do(ui_text, info, EINA_TRUE);

        if (info->merge)
          {
             info = _history_next_get();
          }
        else
          {
             break;
          }
     }
}

/*********************************************************/

/*************** EFL Undo/Redo Callback(s) ***************/

static void
_ui_text_undo_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _user_undo(ui_text);
}

static void
_ui_text_redo_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   _user_redo(ui_text);
}

static void
_ui_text_undo_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   //ali.m How to force ui_text to fire undo event ?
   //I do not want to hack it by direcly call _ui_text_undo_common(ui_text);
}

static void
_ui_text_redo_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   //ali.m How to force ui_text to fire redo event ?
   //I do not want to hack it by direcly call _ui_text_redo_common(ui_text);
}

static void
_ui_text_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;

   _history_append((Efl2_Text_Change_Info *)event);
}

static void
_ui_text_cur_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   history_list_can_merge = EINA_FALSE;
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
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_CHANGED_USER, _ui_text_changed_cb, ui_text);

   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_CURSOR_CHANGED_MANUAL, _ui_text_cur_changed_cb, ui_text);

   // Register callback when redu event called to recall future state (if exists)
   // ali.m
   // (1) EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST will be available for efl.ui.text ?
   // (2) I think user expect event name be like  EFL2_UI_TEXT_EVENT_REDO_REQUEST ?
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
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()

/*********************************************************/

/* EOF */
