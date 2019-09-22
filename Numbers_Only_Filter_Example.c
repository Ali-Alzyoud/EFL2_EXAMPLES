/*********** Numbers Only Filter *****************
 * This is simple example shows how to implement a
 * filter for Efl.Ui.Text that accepts only numbers
 * and filter out other characters.
 *
*/

#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

/******************** Callbacks *********************/

#define IS_DIGIT(x) ((x >= '0') && (x <= '9'))

static void
_ui_text_changed_user_pre_cb(void *data, const Efl_Event *event)
{
   Eo *ui_text = (Eo *)data;
   Efl2_Ui_Text_Change_Info *info = event;


   if (info->insert)
     {
        // Efl_Ui_Text_Change_Info::content type is const char
        // which is the same type as in Efl2.Ui.Raw_Editable's "change,user" event
        // so how could i change text?
        char *p = info->content;

        // Most of the time (unless it is paste operation) "change,user,pre"
        // event will be triggered for only one character input.
        // So we can either accept or deny new character
        // what about deny flag or something?
        if (info->length == 1)
          {
             if (!IS_DIGIT(*p))
               {
                  *p = '\0';
                  info->length = 0;
               }
             return;
          }

        char *new_content = (char *)calloc(info->length + 1, sizeof(char));
        memset(new_content, 0, info->length + 1);
        char *b = new_content;

        while (*p)
          {
             if (IS_DIGIT(*p))
               {
                  *b++ = *p;
               }
             p++;
          }
        *b = '\0';

        // can i free info->content and let it point to new location?
        // so i don't have to re-copy text back to its buffer?
        memcpy(info->content, new_content, info->length + 1);

        free(new_content);

        // Should i update info->length property with new string length?
        info->length = strlen(info->content);
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
                     efl2_text_markup_set(efl_added, "123"),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
                     efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
                     efl_pack(box, efl_added));

   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_CHANGED_USER_PRE, _ui_text_changed_user_pre_cb, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()
/******************************************/
