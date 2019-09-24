/*********** Placeholder Example *****************
 * This is simple example shows how to set placeholder
 * and apply styles to it
 * 
*/

#define EFL_BETA_API_SUPPORT 1
#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>



/************ Application Logic ******************/
static void
_gui_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   efl_exit(0);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *box, *ui_text;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Hello World"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);

   box = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));

   Eo *ui_text = efl_add(EFL2_UI_TEXT_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
           efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
           efl_pack(box, efl_added));
   
   //set placeholder text
   efl2_text_set(efl_part(ui_text, "efl.text_guide"), "Some Text");

   /*
    How to set Font, font-size, text color ....
   
    The following does not work using the old design :
     efl_text_markup_set(efl_part(ui_text, "efl.text_guide"), "<b>Bold Text</b>");
     efl_text_normal_color_set(efl_part(ui_text, "efl.text_guide"), 255,0,0,255);
   */

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl2_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));

}
EFL_MAIN()

/******************************************/
