#define EFL_BETA_API_SUPPORT 1
 
#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

/************ Spell Check Logic ******************/
static const char *suggestions[] = {
   "hello",
   "Hallo",
   "Halo",
   "Hilo",
};

static const char *dictionary[] = {
   "hello"
};

static Eina_Bool
_spell_check_word(const char *str)
{
   int i;
   for (i = 0 ; i < sizeof(dictionary) / sizeof(char *); i++)
     {
        if (!strcmp(str, dictionary[i]))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

/**************************************************/





/************ EFL Suggestions Context Menu Logic ******************/
static void
_menu_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ui_text = (Eo*)data;

   efl2_ui_text_context_menu_clear(ui_text);

   if(event)
   {
        Eina_Position2D pos = efl_input_pointer_position_get(event->info);

        Eo *cursor_start = efl2_ui_text_cursor_new(ui_text);
        Eo *cursor_end = efl2_ui_text_cursor_new(ui_text);

        efl2_text_cursor_coord_set(cursor_start,pos.x, pos.y);
        efl2_text_cursor_coord_set(cursor_end,pos.x, pos.y);

        efl2_text_cursor_word_start(cursor_start);
        efl2_text_cursor_word_end(cursor_end);
        efl2_text_cursor_char_next(cursor_end); // workaround to get word end
        
        char * text = efl2_text_cursor_range_text_get(cursor_start, cursor_end);

        efl_del(cursor_start);
        efl_del(cursor_end);

        if((text[0] != 'H' && text[0] != 'h') || _spell_check_word(text)) //simple check for only words start with 'h'
           return;

        int i;
        for (i = 0 ; i < sizeof(suggestions) / sizeof(char *); i++)
        {
            efl2_ui_text_context_menu_item_add(ui_text, suggestions[i], NULL, ELM_ICON_NONE, NULL, NULL);
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
 
   efl_event_callback_add(ui_text, EFL_EVENT_POINTER_DOWN, _menu_cb, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()

/******************************************/