/************** Emoticon Auto Convertor Example **************
 * This is simple example shows how to implement Emoticon
 * shortcuts auto-convertor.
 *
 * That converts emoticon shortcuts to its unicode value, for
 * example:
 *       :-D will be converted to 😃
 *       <3 will be converted to ❤
 *       and so on...
 *
 */

#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

/*********** Emoticon Auto Conver Logic ***********/
typedef struct
{
   const char *shortcut;
   const char *unicode;
} emoticon_item;

static const emoticon_item emoticon_list[] = {
    {":-D", "&#x1F603;"},    // 😃 &#x1F603;
    {"X-D", "&#x1F606;"},    // 😆 &#x1F606;
    {";-)", "&#x1F609;"},    // 😉 &#x1F609;
    {">:-)", "&#x1F608;"},   // 😈 &#x1F608;
    {"O:-)", "&#x1F607;"},   // 😇 &#x1F607;
    {":-)", "&#x1F642;"},    // 🙂 &#x1F642;
    {":-*", "&#x1F618;"},    // 😘 &#x1F618;
    {":-P", "&#x1F61B;"},    // 😛 &#x1F61B;
    {";-P", "&#x1F61C;"},    // 😜 &#x1F61C;
    {":-|", "&#x1F610;"},    // 😐 &#x1F610;
    {"-_-", "&#x1F611;"},    // 😑 &#x1F611;
    {"B-)", "&#x1F60E;"},    // 😎 &#x1F60E;
    {":-/", "&#x1F615;"},    // 😕 &#x1F615;
    {":-O", "&#x1F62E;"},    // 😮 &#x1F62E;
    {"D-:", "&#x1F626;"},    // 😦 &#x1F626;
    {":'-(", "&#x1F622;"},   // 😢 &#x1F622;
    {":-@", "&#x1F624;"},    // 😤 &#x1F624;
    {":poop:", "&#x1F4A9;"}, // 💩 &#x1F4A9;
    {"<3", "&#x2764;"},      // ❤ &#x2764;
                             //  {"</3", ""}, // &#1F494;
    {":-(", "&#x2639;"},     // ☹ &#x2639;
    {NULL, NULL}};

static int
_string_compare_reverse(const char *first, const char *second)
{
   const char *p1 = first, *p2 = second;

   if (!first || !second)
        return 0;

   while (*p1++)
        ;
   while (*p2++)
        ;

   while (p2 != second)
     {
        p1--;
        p2--;
        if (*p1 != *p2 || (p1 == first && p2 != second))
          {
             return 0;
          }
     }

   return 1;
}

static emoticon_item *
_emoticon_shortcut_matcher_get(const char *shortcut)
{
   emoticon_item *p = (emoticon_item *)emoticon_list;
   while (p->shortcut)
     {
        if (_string_compare_reverse(shortcut, p->shortcut))
          {
               return p;
          }
        p++;
     }
   return NULL;
}
/**************************************************/

/********* Efl Emoticon Auto Conver Logic *********/
static Eo *emoticon_cursor = NULL;
static emoticon_item *active_emoticon_item = NULL;

static void
_emoticon_release(void)
{
   if (emoticon_cursor)
        efl_del(emoticon_cursor);
   emoticon_cursor = NULL;
   active_emoticon_item = NULL;
}

static void
_ui_text_changed_user(void *data, const Efl_Event *event)
{
   Efl2_Text_Change_Info *info = event->info;
   Eo *ui_text = data;

   if (info)
     {
        Eo *curs = efl2_text_raw_editable_main_cursor_get(ui_text);
        if (info->insert)
          {
             if (emoticon_cursor && active_emoticon_item)
               {
                  _emoticon_release();
               }
             else
               {
                  if (!emoticon_cursor)
                       emoticon_cursor = efl2_ui_text_cursor_new(ui_text);

                  efl2_text_cursor_position_set(emoticon_cursor, (info->position < 5) ? 0 : info->position - 5);

                  char *word = efl2_text_cursor_range_text_get(emoticon_cursor, curs);
                  emoticon_item *item;
                  if ((item = _emoticon_shortcut_matcher_get(word)))
                    {
                       efl2_text_cursor_position_set(emoticon_cursor, efl2_text_cursor_position_get(curs) - strlen(item->shortcut));
                       efl2_text_cursor_range_delete(emoticon_cursor, curs);
                       efl2_text_markup_insert(curs, item->unicode);
                       active_emoticon_item = item;
                    }
               }
          }
        else
          {
             if (emoticon_cursor &&
                 active_emoticon_item &&
                 efl2_text_cursor_position_get(emoticon_cursor) == info->position &&
                 0 == strcmp(info->content, efl_text_markup_util_markup_to_text(active_emoticon_item->unicode)))
                {
                   efl2_text_cursor_position_set(emoticon_cursor, info->position);

                   efl2_text_markup_insert(emoticon_cursor, active_emoticon_item->shortcut);

                   size_t s = strlen(active_emoticon_item->shortcut);
                   while (s--)
                        efl2_text_cursor_char_next(ui_text, curs);
                }
             _emoticon_release();
          }
     }
}

static void
_ui_text_cursor_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   _emoticon_release();
}
/**************************************************/

/*************** Application Logic ****************/
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
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // when the user clicks "close" on a window there is a request to delete
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);

   box = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(360, 240)));

   ui_text = efl_add(EFL2_UI_TEXT_CLASS, box,
                     efl2_text_markup_set(efl_added, "Enter emoji shortcuts... <3 ❤"),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.9),
                     efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
                     efl_pack(box, efl_added));

   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, _ui_text_changed_user, ui_text);

   efl_event_callback_add(efl2_text_raw_editable_main_cursor_get(ui_text), EFL2_TEXT_CURSOR_EVENT_CHANGED, _ui_text_cursor_changed, ui_text);
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED, _ui_text_cursor_changed, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}

EFL_MAIN()
/**************************************************/

/* EOF */