/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_options_data.cpp
 *     GUI menus for game options.
 * @par Purpose:
 *     Structures to show and maintain options screens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Dec 2012 - 11 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_ingame_opts.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"

#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "frontmenu_saves.h"
#include "config_settings.h"
#include "frontmenu_options.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GuiButtonInit frontend_define_keys_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {92},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  82, 117,  82, 117,450,246, frontend_draw_scroll_box,          0, 201,  0,      {94},            0, 0, NULL },
  { 1,  0, 0, 0, 0, frontend_define_key_up,NULL, frontend_over_button,   0, 532, 116, 532, 116, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {17},            0, 0, frontend_define_key_up_maintain },
  { 1,  0, 0, 0, 0, frontend_define_key_down,NULL,frontend_over_button,  0, 532, 350, 532, 350, 26, 14, frontend_draw_slider_button,       0, 201,  0,      {18},            0, 0, frontend_define_key_down_maintain },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 536, 130, 536, 130, 10,220, frontend_draw_define_key_scroll_tab,0,201,  0,      {40},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 130,  95, 130,424, 22, frontend_draw_define_key,          0, 201,  0,      {-1},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 152,  95, 152,424, 22, frontend_draw_define_key,          0, 201,  0,      {-2},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 174,  95, 174,424, 22, frontend_draw_define_key,          0, 201,  0,      {-3},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 196,  95, 196,424, 22, frontend_draw_define_key,          0, 201,  0,      {-4},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 218,  95, 218,424, 22, frontend_draw_define_key,          0, 201,  0,      {-5},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 240,  95, 240,424, 22, frontend_draw_define_key,          0, 201,  0,      {-6},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 262,  95, 262,424, 22, frontend_draw_define_key,          0, 201,  0,      {-7},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 284,  95, 284,424, 22, frontend_draw_define_key,          0, 201,  0,      {-8},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 306,  95, 306,424, 22, frontend_draw_define_key,          0, 201,  0,      {-9},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_define_key,NULL,       frontend_over_button,0,  95, 328,  95, 328,424, 22, frontend_draw_define_key,          0, 201,  0,     {-10},            0, 0, frontend_define_key_maintain },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,     frontend_over_button,27,999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {98},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit frontend_option_buttons[] = {
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {96},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 107,  95, 107,220, 26, frontend_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 133,  95, 133,  2, 88, frontend_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 108, 115, 108,220, 26, frontend_draw_text,                0, 201,  0,      {99},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 142, 146, 142, 48, 32, frontend_draw_icon,               90, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 146, 182, 146, 182, 48, 32, frontend_draw_icon,               89, 201,  0,       {0},            0, 0, NULL },
  { 4, 75, 0, 0, 0, gui_set_sound_volume,NULL,       NULL,               0, 194, 147, 194, 147,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&sound_level}, 127, 0, NULL },
  { 4,  0, 0, 0, 0, gui_set_music_volume,NULL,       NULL,               0, 194, 187, 194, 187,300, 22, frontend_draw_slider,              0, 201,  0,{(long)&music_level}, 127, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 231,  95, 231,220, 26, frontend_draw_scroll_box_tab,      0, 201,  0,      {28},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,  95, 257,  95, 257,  0, 88, frontend_draw_scroll_box,          0, 201,  0,      {89},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 115, 232, 115, 232,220, 26, frontend_draw_text,                0, 201,  0,     {100},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 102, 271, 102, 271,190, 22, frontend_draw_text,                0, 201,  0,     {101},            0, 0, NULL },
  { 4,  0, 0, 0, 0, frontend_set_mouse_sensitivity,NULL,NULL,            0, 304, 271, 304, 271,190, 22, frontend_draw_small_slider,        0, 201,  0,{(long)&fe_mouse_sensitivity}, 7, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_invert_mouse,NULL,     frontend_over_button,0, 102, 303, 102, 303,380, 22, frontend_draw_text,                0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, NULL,               NULL,        NULL,               0, 320, 303,   0,   0,100, 22, frontend_draw_invert_mouse,        0, 201,  0,     {102},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button,26, 999, 357, 999, 357,371, 46, frontend_draw_large_menu_button,   0, 201,  0,      {95},            0, 0, NULL },
  { 0,  0, 0, 0, 0, frontend_change_state,NULL,    frontend_over_button, 1, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, 201,  0,       {6},            0, 0, NULL },
  {-1,  0, 0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiMenu frontend_define_keys_menu =
 { 36, 0, 1, frontend_define_keys_buttons,        0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_option_menu =
 { 39, 0, 1, frontend_option_buttons,             0,          0, 640, 480, frontend_copy_mnu_background,0, NULL,    frontend_init_options_menu,0,0,0,};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/