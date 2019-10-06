/*********** Suggestions Context Menu *****************
 * This is simple example shows how to implement Suggestions Context
 * menu on text using EFL Text Context Menu
 * 
 * 
 * On Right Click/Long Press Show a List Of Word Suggestions
*/

#define EFL_BETA_API_SUPPORT 1
#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

static const char *suggestions[] = {
   "hello",
   "Hallo",
   "Halo",
   "Hilo",
};

/************ EFL Suggestions Context Menu Logic ******************/

Eo *cursor_start, *cursor_end;

static const char* word_at_position(Eo *ui_text, Eina_Position2D pos)
{
   //Get word at position
   efl2_text_cursor_coord_set(cursor_start,pos.x, pos.y);
   efl2_text_cursor_copy(cursor_start, cursor_end);

   efl2_text_cursor_word_start(cursor_start);
   efl2_text_cursor_word_end(cursor_end);
   efl2_text_cursor_char_next(cursor_end); // workaround to get word end

   return efl2_text_cursor_range_text_get(cursor_start, cursor_end);
}

static void
_pointer_down_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ui_text = (Eo*)data;

   efl2_ui_text_context_menu_clear(ui_text);

   if (event)
     {
          Eina_Position2D pos = efl_input_pointer_position_get(event->info);
          const char *text = word_at_position(ui_text, pos);

          if (!strcmp(text,"Heloo"))
            {
               int i;
               for (i = 0 ; i < sizeof(suggestions) / sizeof(char *); i++)
                 {
                    // ali.m
                    // (1) Why ELM enum and evas smart callback (should not this use new interfaces EFL events)
                    // (2) User can not control items order in the list, for example he want to add items at the top or bottom.
                    efl2_ui_text_context_menu_item_add(ui_text, suggestions[i], NULL, ELM_ICON_NONE, NULL, NULL);
                 }
            }
     }
}

/****************************************************/





/************ Application Logic ******************/
static void
_gui_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   efl_exit(0);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *box;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);

   box = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));

   Eo *ui_text = efl_add(EFL_UI_TEXT_CLASS, box,
           efl2_text_markup_set(efl_added, "Heloo World"),
           efl_text_multiline_set(efl_added, EINA_FALSE),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
           efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
           efl_pack(box, efl_added));

   // ali.m
   // This is a hack to initialize menu before showing it (It still has issues and does not work probably with efl.ui.text)
   // I do not think this is the right way to do it, user should listen to EFL2_TEXT_RAW_EDITABLE_EVENT_CONTEXT_PREOPEN
   efl_event_callback_add(ui_text, EFL_EVENT_POINTER_DOWN, _pointer_down_cb, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));

   cursor_start = efl_add(EFL2_TEXT_CURSOR, ui_text);
   cursor_end = efl_add(EFL2_TEXT_CURSOR, ui_text);
}
EFL_MAIN()

/******************************************/