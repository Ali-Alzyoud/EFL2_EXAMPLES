/********************* Tagging *********************
 * This is simple example shows how to implement Tags and
 * autocompletion with selected Tag from suggested tags
 *
 */

#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1

#include <Eina.h>
#include <Elementary.h>
#include <Efl_Ui.h>

static Eina_List *sugg_tags_list = NULL;
static Eina_Bool tags_list_active = EINA_FALSE;
static size_t tags_starting_pos = 0;

/************ Tagging Logic ******************/

/**
 * List of available tags (DB/DS)
 */
const char *tags[] = {
    "Anna",
    "Bob",
    "Cliff",
    "Foo",
    "Gail",
    "Leen",
    "Mario",
    "Maya",
    "Monty",
    "Petey",
    "Sal",
    "Shonda",
    "Terry"};

/**
 * The return list must be cleared using _tags_list_clear()
 * Return Eina_List, or NULL if nothing found
 */
static Eina_List *
_tags_list_get(const char *query)
{
   Eina_List *tags_list = NULL;
   size_t size = strlen(query);
   char *query_lo = calloc(size + 1, sizeof(char));
   char *tag_lo;
   char *tag;

   memcpy(query_lo, query, size + 1);
   eina_str_tolower(&query_lo);

#define COPY_TAGS_ITEM(ii)                  \
   do                                       \
   {                                        \
      tag = calloc(tag_size, sizeof(char)); \
      memcpy(tag, tags[ii], tag_size);      \
   } while (0);

   for (size_t ii = 0; ii < LIST_SIZE(tags); ii++)
   {
      size_t tag_size = strlen(tags[ii]) + 1;
      // Skip tags with less charactars than query
      if (size > tag_size - 1)
         continue;

      tag_lo = calloc(tag_size, sizeof(char));
      memcpy(tag_lo, tags[ii], tag_size);
      eina_str_tolower(&tag_lo);

      if (
          // Add tags that has query substring
          0 == strncmp(tag_lo, query_lo, size))
      {
         COPY_TAGS_ITEM(ii);
         tags_list = eina_list_prepend(tags_list, tag);
      }
      else if (
          strstr(tag_lo, query_lo))
      {
         COPY_TAGS_ITEM(ii);
         tags_list = eina_list_append(tags_list, tag);
      }

      free(tag_lo);
      tag_lo = NULL;
   }

   free(query_lo);

   return tags_list;
}

/**
 * Use this function to clear list from _tags_list_get function
 */
static void
_tags_list_clear(Eina_List *list)
{
   char *str;
   if (!list)
      return;

   EINA_LIST_FREE(list, str)
   {
      free(str);
   }
}
/**************************************************/

/************ EFL Tagging Logic ******************/

static void
_update_suggestion_list(const char *query)
{
   if (sugg_tags_list != NULL)
   {
      _tags_list_clear(sugg_tags_list);
   }
   sugg_tags_list = _tags_list_get(query);
}

static void
_hide_suggestion_list(void)
{
   _tags_list_clear(sugg_tags_list);

   tags_list_active = EINA_FALSE;
}

static void
_ui_text_changed_cb(void *data, const Efl_Event *event)
{
   Eo *ui_text = data;
   Efl_Ui_Text_Change_Info *info = (Efl_Ui_Text_Change_Info *)event;

   Efl2_Text_Cursor *curs = efl2_ui_text_cursor_new(ui_text);
   efl2_text_cursor_position_set(curs, info->position);

   if (tags_list_active)
   { // update suggestion list
      Efl2_Text_Cursor *curs2;
      efl2_text_cursor_copy(curs, curs2);
      efl2_text_cursor_word_start(curs);
      efl2_text_cursor_word_end(curs2);

      if (tags_starting_pos != efl2_text_cursor_position_get(curs))
      {
         _hide_suggestion_list();
      }
      else
      {
         _update_suggestion_list(efl2_text_cursor_range_text_get(curs, curs2));
      }

      efl_del(curs2);
   }
   else if (info->insert &&
            info->length == 1 && // One character which is @
            strcmp(info->content, "@") &&
            !efl2_text_cursor_word_start(curs)) // if cursor does not moved then it is new word
   {
      tags_list_active = EINA_TRUE;
      tags_starting_pos = info->position;

      _update_suggestion_list(""); // Initialize list with all available tags, TODO: list must have max limit!
   }
   efl_del(curs);
}

static void
_ui_text_cursor_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *ui_text = data;

   Efl2_Text_Cursor *curs = efl2_text_raw_editable_main_cursor_get(ui_text);

   if (tags_list_active)
   {
      if (tags_starting_pos == efl2_text_cursor_position_get(curs))
      {
         _hide_suggestion_list();
      }
      else
      {
         efl2_text_cursor_word_start(curs);

         if (tags_starting_pos != efl2_text_cursor_position_get(curs))
         {
            _hide_suggestion_list();
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

   // Register callback when text changed to track new changes
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_CHANGED_USER, _ui_text_changed_cb, ui_text);
   efl_event_callback_add(ui_text, EFL2_TEXT_RAW_EDITABLE_CURSOR_CHANGED_MANUAL, _ui_text_cursor_changed_cb, ui_text);

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Quit"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.1),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  _gui_quit_cb, efl_added));
}
EFL_MAIN()
/******************************************/
