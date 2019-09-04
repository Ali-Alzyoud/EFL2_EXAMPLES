
#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

Eo * attribute_factory = NULL;
Eina_List *handles_list = NULL;
Eo * cursor_start = NULL;
Eo * cursor_end = NULL;

/************ Spell Check Logic ******************/
const char* dictionary[] = {
   "hello",
   "world",
   "one",
   "two",
   "this",
};

static Eina_Bool
_spell_check_word(const char* str)
{
   int i;
   for (i = 0 ; i < sizeof(dictionary) / sizeof(char*); i++)
     {
        if (!strcmp(str,dictionary[i]))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

/**************************************************/







/************ EFL Spell Check Logic ******************/

static Efl2_Text_Attribute_Handle*
mark_misspelled(Eo *start,Eo *end)
{
   Efl2_Text_Attribute_Handle *handle = efl2_text_attribute_factory_insert(attribute_factory, start, end);
   efl2_text_attribute_factory_ref(handle);
   return handle;
}

static void
mark_clear()
{
   Eina_List *l;
   EINA_LIST_FOREACH(handles_list, l, handle)
     {
        efl2_text_attribute_factory_unref(handle);
        efl2_text_attribute_factory_del(handle);
     }
   handles_list = eina_list_free(handles_list);
}

static Eina_Bool
_ui_text_spell_check_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo * ui_text= (Eo *)data;
   Eina_Bool correct;

   efl2_text_style_underline_clear(attribute_factory);
   efl2_text_style_underline_color_set(attribute_factory, 255, 0, 0, 255);

   mark_clear()

   efl2_text_cursor_paragraph_first(cursor_start);
   efl2_text_cursor_paragraph_first(cursor_end);
   efl_text_cursor_word_end(cursor_end);


   //ali.m Ugly code to iterate words
   while (!efl2_text_cursor_equal(cursor_start,cursor_end))
     {
        const char * word = efl2_text_cursor_range_text_get(cursor_start,cursor_end);
        correct = _spell_check_word(efl2_text_cursor_range_text_get(cursor_start,cursor_end));
        if (!correct)
        {
           handles_list = eina_list_append(handles_list, mark_misspelled(cursor_start, cursor_end));
        }
        
        int word_start = efl2_text_cursor_position_get(cursor_start);
        efl2_text_cursor_char_next(cursor_end);
        efl2_text_cursor_word_end(cursor_end);
        efl2_text_cursor_copy(cursor_end, cursor_start);
        efl2_text_cursor_word_start(cursor_start);
        if (word_start == efl2_text_cursor_position_get(cursor_start))
          break;
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
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
                     efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
                     efl_pack(box, efl_added));

   cursor_start = efl2_ui_text_cursor_new(ui_text);
   cursor_end = efl2_ui_text_cursor_new(ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Spell Check"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _ui_text_spell_check_cb, ui_text));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()
/******************************************/
