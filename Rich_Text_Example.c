/*********** Rich Text Editor *****************
 * This is simple example shows how to implement Rich Text Editor
 * using EFL Attribute Factory & Text Cursor
 * 
 * For simplicity sake for this example it do not
 * cache the attribute handles (which is inefficient for large
 * chunk of text)
 * 
 * Apply font/text atrributes on text (bold, italic, color, font-size)
*/

#define EFL_BETA_API_SUPPORT 1
 
#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

#define COMP_TEXT(obj, text)  (strcmp(efl_text_get(efl_content_get((Eo*)obj)), text) == 0)

#define CHECK_BOX_BOLD "Bold"
#define CHECK_BOX_ITALIC "Italic"
#define CHECK_BOX_RED "Red"
#define CHECK_BOX_30 "30"

/************ Rich Text Logic ******************/

static Eo *attribute_factory = NULL;
static Eo *check_bold, *check_italic, *check_red, *check_30;
static Eo *cursor_main, *cursor_start, *cursor_end, *sel_start, *sel_end;

static void
_apply_attribute(Eo *ui_text)
{
   /*
     We have three cases :
      1- If there is Selection, we insert attribute for the selected area.
      2- If the main cursor is inside a word, we insert the attribute for the whole word
      3- If outside word, newly inserted text at current posistion should have the chosen attributes 
    */
   
   if (efl2_text_cursor_equal(sel_start, sel_end)) // if no selection
     {
         efl2_text_cursor_copy(cursor_main, cursor_start);
         efl2_text_cursor_copy(cursor_main, cursor_end);

         efl2_text_cursor_word_start(cursor_start);
         efl2_text_cursor_word_end(cursor_end);

         int start_pos, end_pos, main_pos;
         
         start_pos = efl2_text_cursor_position_get(cursor_start);
         end_pos = efl2_text_cursor_position_get(cursor_end);
         main_pos = efl2_text_cursor_position_get(cursor_main);

         if ( main_pos > start_pos && main_pos < end_pos) // inside word
           {
              efl2_text_attribute_factory_insert(attribute_factory, cursor_start, cursor_end);
           }
         else
           {
              //if before/after word newly inserted text by user should use chosen attributes, how to do it?
           }
     }
     else
       efl2_text_attribute_factory_insert(attribute_factory, sel_start, sel_end);
}

static void
_update_check_boxes()
{
   efl_ui_selectable_selected_set(check_bold, efl2_text_font_properties_weight_get(attribute_factory) == EFL2_TEXT_FONT_WEIGHT_BOLD);
   efl_ui_selectable_selected_set(check_italic, efl2_text_font_properties_font_slant_get(attribute_factory) == EFL2_TEXT_FONT_SLANT_ITALIC);
   efl_ui_selectable_selected_set(check_30, efl2_text_font_properties_font_size_get(attribute_factory) == 30);

   unsigned char r,g,b,a;
   efl2_text_style_foreground_color_get(attribute_factory, &r, &g, &b, &a);
   if (r == 255 && g == 0 && b == 0 && a == 255)
     efl_ui_selectable_selected_set(check_red, EINA_TRUE);
   else
     efl_ui_selectable_selected_set(check_red, EINA_FALSE);
}

static void
_attribute_text_get(Eo *cur_start, Eo *cur_end)
{
   Efl2_Text_Attribute_Handle *handle;
   Eina_Iterator *itr;

   itr = efl2_text_attribute_factory_range_attributes_get(cur_start, cur_end); // Does it work if cur_start == cur_end ?
            
   if (eina_iterator_next((itr), (void **)(void *)&(handle)))   // just work on first handle for simplicity
     efl2_text_attribute_factory_load(attribute_factory, handle);   // is this correct ? if attribute_factory updated will the handle be updated ? We want attribute copy only

   eina_iterator_free(itr);
}

static void
_gui_selection_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = (Eo*)data;


   if (!efl2_text_cursor_equal(sel_start, sel_end))
     {
        efl2_text_attribute_factory_reset(attribute_factory);
        _attribute_text_get(sel_start, sel_end);
        _update_check_boxes();
     }
}

static void
_gui_cursor_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = (Eo*)data;

   if (efl2_text_cursor_equal(sel_start, sel_end)) // only if no selection do work (I'm not sure if it can be called while doing selection as cursor_changed event not working at all !)
     {
        efl2_text_attribute_factory_reset(attribute_factory);
        _attribute_text_get(cursor_main, cursor_main);
        _update_check_boxes();
     }
}

static void
_gui_bold_cb(void *data, const Efl_Event *event EINA_UNUSED)
{   
   if (efl_ui_selectable_selected_get((Eo*)data))
      efl2_text_font_properties_weight_set(attribute_factory, EFL2_TEXT_FONT_WEIGHT_BOLD);
   else
      efl2_text_font_properties_weight_set(attribute_factory, EFL2_TEXT_FONT_WEIGHT_NORMAL);
   
   _apply_attribute((Eo*)data);
}

static void
_gui_italic_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_ui_selectable_selected_get((Eo*)data))
      efl2_text_font_properties_font_slant_set(attribute_factory, EFL2_TEXT_FONT_SLANT_ITALIC);
   else
      efl2_text_font_properties_font_slant_set(attribute_factory, EFL2_TEXT_FONT_SLANT_NORMAL);

   _apply_attribute((Eo*)data);
}

static void
_gui_red_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_ui_selectable_selected_get((Eo*)data))
      efl2_text_style_foreground_color_set(attribute_factory, 255, 0, 0, 255);
   else
      efl2_text_style_foreground_color_set(attribute_factory, 0, 0, 0, 255);

   _apply_attribute((Eo*)data);
}

static void
_gui_30_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_ui_selectable_selected_get((Eo*)data))
      efl2_text_font_properties_font_size_set(attribute_factory, 30);
   else
      efl2_text_font_properties_font_size_set(attribute_factory, 15);

   _apply_attribute((Eo*)data);
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
   Eo *win, *box, *boxHor, *ui_text, *content;
 
   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
 
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);
 
   box = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));
 
   ui_text = efl_add(EFL_UI_TEXT_CLASS, box,
           efl_text_markup_set(efl_added, "Hello World"),
           efl_text_multiline_set(efl_added, EINA_FALSE),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
           efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
           efl_event_callback_add(efl_added, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED, // correct name ??
                                  _gui_selection_changed_cb, efl_added),
           efl_event_callback_add(efl_added, EFL_TEXT_CURSOR_EVENT_CURSOR_CHANGED,   // this event attached to the cursor, will it be received by the user ??
                                  _gui_cursor_changed_cb, efl_added),
           efl_pack(box, efl_added));

   boxHor = efl_add(EFL_UI_BOX_CLASS, win,
                efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                efl_pack(box, efl_added));

   attribute_factory = efl_add(EFL2_TEXT_ATTRIBUTE_FACTORY, efl_main_loop_get());
   cursor_main = efl2_text_raw_editable_main_cursor_get(ui_text);
   cursor_start = efl_add(EFL2_TEXT_CURSOR, ui_text);
   cursor_end = efl_add(EFL2_TEXT_CURSOR, ui_text);
   efl2_text_raw_editable_selection_cursors_get(ui_text, &sel_start, &sel_end); // Can be called on ui_text ?

   //     ----------------  Bold Check Box

    content = efl_add(EFL_UI_TEXT_CLASS, win,
           efl_text_set(efl_added, CHECK_BOX_BOLD));

    check_bold = efl_add(EFL_UI_CHECK_CLASS, boxHor,
           efl_gfx_hint_weight_set(efl_added, 0.25, 1.0),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_SELECTED_CHANGED, _gui_bold_cb, efl_added),
           efl_pack(boxHor, efl_added));

    efl_content_set(check_bold, content);

    //     ----------------  Italic Check Box

    content = efl_add(EFL_UI_TEXT_CLASS, win,
           efl_text_set(efl_added, CHECK_BOX_ITALIC));

    check_italic = efl_add(EFL_UI_CHECK_CLASS, boxHor,
           efl_gfx_hint_weight_set(efl_added, 0.25, 1.0),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_SELECTED_CHANGED, _gui_italic_cb, efl_added),
           efl_pack(boxHor, efl_added));

    efl_content_set(check_italic, content);

    //     ----------------  Red Check Box

    content = efl_add(EFL_UI_TEXT_CLASS, win,
           efl_text_set(efl_added, CHECK_BOX_RED));

    check_red = efl_add(EFL_UI_CHECK_CLASS, boxHor,
           efl_gfx_hint_weight_set(efl_added, 0.25, 1.0),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_SELECTED_CHANGED, _gui_red_cb, efl_added),
           efl_pack(boxHor, efl_added));

    efl_content_set(check_red, content);

    //     ---------------- Font Size 30 Check Box

    content = efl_add(EFL_UI_TEXT_CLASS, win,
           efl_text_set(efl_added, CHECK_BOX_30));

    check_30 = efl_add(EFL_UI_CHECK_CLASS, boxHor,
           efl_gfx_hint_weight_set(efl_added, 0.25, 1.0),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_SELECTED_CHANGED, _gui_30_cb, efl_added),
           efl_pack(boxHor, efl_added));

    efl_content_set(check_30, content);
}
EFL_MAIN()

/******************************************/