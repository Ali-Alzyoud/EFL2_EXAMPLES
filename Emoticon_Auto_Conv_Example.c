#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

typedef struct
{
   const char *shortcut;
   const char *unicode;
} emoticon_item;

static Efl_Text_Cursor_Cursor *emoticon_cursor = NULL;
static emoticon_item *active_emoticon_item = NULL;
static Eina_Bool emoticon_inserted = EINA_FALSE;
static size_t emoticon_pos = 0;
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
   char *p1 = first, *p2 = second;

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

static void
_emoticon_release(Eo *ui_text)
{
   if ( ui_text && emoticon_cursor )
      efl_text_cursor_free(ui_text, emoticon_cursor);
   emoticon_cursor = NULL;
   active_emoticon_item = NULL;
}

static void
_ui_text_changed_user(void *data, const Efl_Event *event)
{
   Efl_Ui_Text_Change_Info *info = event->info;
   Eo *ui_text = data;

   if (info)
   {
      Efl_Text_Cursor_Cursor *curs = efl_text_cursor_get(ui_text, EFL_TEXT_CURSOR_GET_TYPE_MAIN);
      if (info->insert)
      {
         if (emoticon_cursor && active_emoticon_item)
         {
            _emoticon_release(ui_text);
         }
         else
         {
            if (!emoticon_cursor)
               emoticon_cursor = efl_text_cursor_new(ui_text);
            efl_text_cursor_position_set(ui_text, emoticon_cursor, (info->position < 5) ? 0 : info->position - 5);

            char *word = efl_canvas_text_range_text_get(ui_text, emoticon_cursor, curs);
            emoticon_item *p = emoticon_list;
            while (p->shortcut)
            {
               if (_string_compare_reverse(word, p->shortcut))
               {
                  efl_text_cursor_position_set(ui_text, emoticon_cursor, efl_text_cursor_position_get(ui_text, curs) - strlen(p->shortcut));
                  efl_canvas_text_range_delete(ui_text, emoticon_cursor, curs);
                  efl_text_markup_interactive_cursor_markup_insert(ui_text, curs, p->unicode);
                  active_emoticon_item = p;
                  break;
               }
               p++;
            }
         }
      }
      else
      {
         if (emoticon_cursor &&
             active_emoticon_item &&
             efl_text_cursor_position_get(ui_text, emoticon_cursor) == info->position &&
             0 == strcmp(info->content, efl_text_markup_util_markup_to_text(active_emoticon_item->unicode)))
         {
            efl_text_cursor_position_set(ui_text, emoticon_cursor, info->position);

            efl_text_cursor_text_insert(ui_text, emoticon_cursor, active_emoticon_item->shortcut);

            size_t s = strlen(active_emoticon_item->shortcut);
            while (s--)
               efl_text_cursor_char_next(ui_text, curs);
         }
         _emoticon_release(ui_text);
      }
   }
}

static void
_ui_text_cursor_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;
   printf("changed\n");

   _emoticon_release(ui_text);
}

static void
_gui_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   efl_exit(0);
}

static void
_gui_setup(void)
{
   Eo *win;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, ELM_WIN_BASIC),
                 efl_text_set(efl_added, "Hello World"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);

   efl_add(EFL_UI_TEXT_CLASS, win,
           efl_content_set(win, efl_added),
           // efl_text_markup_set(efl_added, "<font_size=42 style=far_shadow linesize=5 shadow_color=#55F right_margin=50 align=right><color=#FFF><glow_color=#F00>Hello world</glow_color></color>, This is an <b>EFL.Ui</b> application!</font_size>"),
           // efl_text_markup_set(efl_added, "<font_size=12 font=clean>LVAV LW LT LY Text ff</font_size>"),
           // efl_text_markup_set(efl_added, "La LVAV LW LT LY Text ff"),
           efl_text_markup_set(efl_added, "<font_size=28>Te</font_size>"),
           efl_canvas_text_style_set(efl_added, "main_style", "DEFAULT='font=Arial font_size=30 color=#E0C0A0'"),
           efl_text_interactive_selection_allowed_set(efl_added, EINA_TRUE),
           efl_text_interactive_editable_set(efl_added, EINA_TRUE),
           efl_gfx_hint_weight_set(efl_added, 0.5, 0.5),
           efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
           efl_event_callback_add(efl_added, EFL_UI_TEXT_EVENT_CHANGED_USER, _ui_text_changed_user, efl_added),
           efl_event_callback_add(efl_added, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED, _ui_text_cursor_changed, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_TEXT_EVENT_CURSOR_CHANGED_MANUAL, _ui_text_cursor_changed, efl_added));
}

static EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Loop_Arguments *args = ev->info;

   _gui_setup();
}

EFL_MAIN()
