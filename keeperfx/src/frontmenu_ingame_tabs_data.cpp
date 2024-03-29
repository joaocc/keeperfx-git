/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_tabs_data.cpp
 *     Main in-game GUI, visible during gameplay.
 * @par Purpose:
 *     Structures to show and maintain tabbed menu appearing ingame.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 11 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontmenu_ingame_tabs.h"
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
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_opts.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void gui_zoom_in(struct GuiButton *gbtn);
void gui_zoom_out(struct GuiButton *gbtn);
void gui_go_to_map(struct GuiButton *gbtn);
void gui_turn_on_autopilot(struct GuiButton *gbtn);
void menu_tab_maintain(struct GuiButton *gbtn);
void gui_area_autopilot_button(struct GuiButton *gbtn);
void maintain_turn_on_autopilot(struct GuiButton *gbtn);
void gui_choose_room(struct GuiButton *gbtn);
void gui_area_event_button(struct GuiButton *gbtn);
void gui_remove_area_for_rooms(struct GuiButton *gbtn);
void gui_area_big_room_button(struct GuiButton *gbtn);
void gui_choose_spell(struct GuiButton *gbtn);
void gui_go_to_next_spell(struct GuiButton *gbtn);
void gui_area_spell_button(struct GuiButton *gbtn);
void gui_choose_special_spell(struct GuiButton *gbtn);
void gui_area_big_spell_button(struct GuiButton *gbtn);
void gui_choose_trap(struct GuiButton *gbtn);
void gui_go_to_next_trap(struct GuiButton *gbtn);
void gui_over_trap_button(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void gui_area_trap_button(struct GuiButton *gbtn);
void gui_go_to_next_door(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void gui_over_door_button(struct GuiButton *gbtn);
void gui_remove_area_for_traps(struct GuiButton *gbtn);
void gui_area_big_trap_button(struct GuiButton *gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void gui_creature_query_background1(struct GuiMenu *gmnu);
void gui_creature_query_background2(struct GuiMenu *gmnu);
void maintain_room(struct GuiButton *gbtn);
void maintain_big_room(struct GuiButton *gbtn);
void maintain_spell(struct GuiButton *gbtn);
void maintain_big_spell(struct GuiButton *gbtn);
void maintain_trap(struct GuiButton *gbtn);
void maintain_door(struct GuiButton *gbtn);
void maintain_big_trap(struct GuiButton *gbtn);
void pick_up_creature_doing_activity(struct GuiButton *gbtn);
void gui_go_to_next_creature_activity(struct GuiButton *gbtn);
void gui_go_to_next_room(struct GuiButton *gbtn);
void gui_over_room_button(struct GuiButton *gbtn);
void gui_area_room_button(struct GuiButton *gbtn);
void pick_up_next_creature(struct GuiButton *gbtn);
void gui_go_to_next_creature(struct GuiButton *gbtn);
void gui_area_anger_button(struct GuiButton *gbtn);
void gui_area_smiley_anger_button(struct GuiButton *gbtn);
void gui_area_experience_button(struct GuiButton *gbtn);
void gui_area_instance_button(struct GuiButton *gbtn);
void maintain_instance(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_up(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void gui_scroll_activity_down(struct GuiButton *gbtn);
void maintain_activity_up(struct GuiButton *gbtn);
void maintain_activity_down(struct GuiButton *gbtn);
void maintain_activity_pic(struct GuiButton *gbtn);
void maintain_activity_row(struct GuiButton *gbtn);
void gui_activity_background(struct GuiMenu *gmnu);
void gui_area_ally(struct GuiButton *gbtn);
void gui_area_stat_button(struct GuiButton *gbtn);
void maintain_event_button(struct GuiButton *gbtn);
void gui_toggle_ally(struct GuiButton *gbtn);
void maintain_ally(struct GuiButton *gbtn);
void maintain_prison_bar(struct GuiButton *gbtn);
void maintain_room_and_creature_button(struct GuiButton *gbtn);
void pick_up_next_wanderer(struct GuiButton *gbtn);
void gui_go_to_next_wanderer(struct GuiButton *gbtn);
void pick_up_next_worker(struct GuiButton *gbtn);
void gui_go_to_next_worker(struct GuiButton *gbtn);
void pick_up_next_fighter(struct GuiButton *gbtn);
void gui_go_to_next_fighter(struct GuiButton *gbtn);
void gui_area_payday_button(struct GuiButton *gbtn);
void gui_area_research_bar(struct GuiButton *gbtn);
void gui_area_workshop_bar(struct GuiButton *gbtn);
void gui_area_player_creature_info(struct GuiButton *gbtn);
void gui_area_player_room_info(struct GuiButton *gbtn);
void spell_lost_first_person(struct GuiButton *gbtn);
void gui_set_tend_to(struct GuiButton *gbtn);
void gui_set_query(struct GuiButton *gbtn);
/******************************************************************************/
struct GuiButtonInit main_menu_buttons[] = {
  { 0,             38, 0, 0,          gui_zoom_in,           NULL,  NULL,               0, 112,   4, 114,   4, 26, 66, gui_area_new_vertical_button,    237, 321,  0,       {0},            0, 0, NULL },
  { 0,             39, 0, 0,         gui_zoom_out,           NULL,  NULL,               0, 110,  70, 114,  70, 26, 66, gui_area_new_vertical_button,    239, 322,  0,       {0},            0, 0, NULL },
  { 0,             37, 0, 0,        gui_go_to_map,           NULL,  NULL,               0,   0,   0,   0,   0, 30, 31, gui_area_new_vertical_button,    304, 323,  0,       {0},            0, 0, NULL },
  { 0,              0, 0, 0,gui_turn_on_autopilot,           NULL,  NULL,               0,   0,  70,   0,  70, 16, 67, gui_area_autopilot_button,       492, 201,  0,       {0},            0, 0, maintain_turn_on_autopilot },
  { 0,              0, 0, 0,                 NULL,           NULL,  NULL,               0,  68,   0,  68,   0, 68, 16, gui_area_new_normal_button,      499, 722,&options_menu, {0},        0, 0, NULL },
  { 3,   BID_INFO_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               7,   0, 154,   0, 154, 28, 34, gui_draw_tab,                      7, 447,  0,{(long)&info_tag},     0, 0, menu_tab_maintain },
  { 3,   BID_ROOM_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               2,  28, 154,  28, 154, 28, 34, gui_draw_tab,                      9, 448,  0,{(long)&room_tag},     0, 0, menu_tab_maintain },
  { 3,  BID_SPELL_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               3,  56, 154,  56, 154, 28, 34, gui_draw_tab,                     11, 449,  0,{(long)&spell_tag},    0, 0, menu_tab_maintain },
  { 3,   BID_TRAP_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               4,  84, 154,  84, 154, 28, 34, gui_draw_tab,                     13, 450,  0,{(long)&trap_tag},     0, 0, menu_tab_maintain },
  { 3, BID_CREATR_TAB, 0, 0,    gui_set_menu_mode,           NULL,  NULL,               5, 112, 154, 112, 154, 28, 34, gui_draw_tab,                     15, 451,  0,{(long)&creature_tag}, 0, 0, menu_tab_maintain },
  { 0,             40, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 360, 138, 360, 24, 30, gui_area_event_button,             0, 201,  0,       {0},            0, 0, maintain_event_button },
  { 0,             41, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 330, 138, 330, 24, 30, gui_area_event_button,             0, 201,  0,       {1},            0, 0, maintain_event_button },
  { 0,             42, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 300, 138, 300, 24, 30, gui_area_event_button,             0, 201,  0,       {2},            0, 0, maintain_event_button },
  { 0,             43, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 270, 138, 270, 24, 30, gui_area_event_button,             0, 201,  0,       {3},            0, 0, maintain_event_button },
  { 0,             44, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 240, 138, 240, 24, 30, gui_area_event_button,             0, 201,  0,       {4},            0, 0, maintain_event_button },
  { 0,             45, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 210, 138, 210, 24, 30, gui_area_event_button,             0, 201,  0,       {5},            0, 0, maintain_event_button },
  { 0,             46, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 180, 138, 180, 24, 30, gui_area_event_button,             0, 201,  0,       {6},            0, 0, maintain_event_button },
  { 0,             47, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 150, 138, 150, 24, 30, gui_area_event_button,             0, 201,  0,       {7},            0, 0, maintain_event_button },
  { 0,             48, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138, 120, 138, 120, 24, 30, gui_area_event_button,             0, 201,  0,       {8},            0, 0, maintain_event_button },
  { 0,             49, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  90, 138,  90, 24, 30, gui_area_event_button,             0, 201,  0,       {9},            0, 0, maintain_event_button },
  { 0,             50, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  60, 138,  60, 24, 30, gui_area_event_button,             0, 201,  0,      {10},            0, 0, maintain_event_button },
  { 0,             51, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,  30, 138,  30, 24, 30, gui_area_event_button,             0, 201,  0,      {11},            0, 0, maintain_event_button },
  { 0,             52, 0, 0,       gui_open_event, gui_kill_event,  NULL,               0, 138,   0, 138,   0, 24, 30, gui_area_event_button,             0, 201,  0,      {12},            0, 0, maintain_event_button },
  { 0,              0, 0, 0,                 NULL,           NULL,  NULL,               0,  22, 122,  22, 122, 94, 40, NULL,                              0, 441,  0,       {0},            0, 0, NULL },
  {-1,              0, 0, 0,                 NULL,           NULL,  NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit room_menu_buttons[] = {
  { 0,  6, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,238,  6, 242, 32, 36, gui_area_room_button,             57, 615,  0,       {2},            0, 0, maintain_room },
  { 0,  8, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,238, 38, 242, 32, 36, gui_area_room_button,             79, 625,  0,      {14},            0, 0, maintain_room },
  { 0,  7, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,238, 70, 242, 32, 36, gui_area_room_button,             59, 624,  0,      {13},            0, 0, maintain_room },
  { 0, 10, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,238,102, 242, 32, 36, gui_area_room_button,             67, 618,  0,       {6},            0, 0, maintain_room },
  { 0,  9, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,276,  6, 280, 32, 36, gui_area_room_button,             61, 616,  0,       {3},            0, 0, maintain_room },
  { 0, 18, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,276, 38, 280, 32, 36, gui_area_room_button,             81, 626,  0,      {15},            0, 0, maintain_room },
  { 0, 19, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,276, 70, 280, 32, 36, gui_area_room_button,             83, 627,  0,      {16},            0, 0, maintain_room },
  { 0, 13, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,276,102, 280, 32, 36, gui_area_room_button,             75, 621,  0,       {8},            0, 0, maintain_room },
  { 0, 11, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,314,  6, 318, 32, 36, gui_area_room_button,             65, 617,  0,       {4},            0, 0, maintain_room },
  { 0, 17, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,314, 38, 318, 32, 36, gui_area_room_button,             63, 619,  0,       {5},            0, 0, maintain_room },
  { 0, 16, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,66,314, 70, 318, 32, 36, gui_area_room_button,             69, 623,  0,      {12},            0, 0, maintain_room },
  { 0, 12, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,98,314,102, 318, 32, 36, gui_area_room_button,             73, 628,  0,      {10},            0, 0, maintain_room },
  { 0, 15, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0, 2,352,  6, 356, 32, 36, gui_area_room_button,             71, 622,  0,      {11},            0, 0, maintain_room },
  { 0, 14, 0, 0, gui_choose_room,gui_go_to_next_room,gui_over_room_button,0,34,352, 38, 356, 32, 36, gui_area_room_button,             77, 629,  0,       {9},            0, 0, maintain_room },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, maintain_room },
  { 0, 20, 0, 0, gui_remove_area_for_rooms,NULL,NULL,                 0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, 462,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_room_button,          0, 201,  0,       {0},            0, 0, maintain_big_room },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit spell_menu_buttons[] = {
  { 0, 36, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 238,   6, 242, 32, 36, gui_area_spell_button,           114, 647,  0,      {18},            0, 0, maintain_spell },
  { 0, 21, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 238,  38, 242, 32, 36, gui_area_spell_button,           118, 648,  0,       {2},            0, 0, maintain_spell },
  { 0, 22, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 238,  70, 242, 32, 36, gui_area_spell_button,           108, 649,  0,       {5},            0, 0, maintain_spell },
  { 0, 27, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 238, 102, 242, 32, 36, gui_area_spell_button,           122, 654,  0,      {11},            0, 0, maintain_spell },
  { 0, 35, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 276,   6, 280, 32, 36, gui_area_spell_button,           452, 653,  0,       {3},            0, 0, maintain_spell },
  { 0, 23, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 276,  38, 280, 32, 36, gui_area_spell_button,           116, 650,  0,       {6},            0, 0, maintain_spell },
  { 0, 29, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 276,  70, 280, 32, 36, gui_area_spell_button,           128, 656,  0,      {13},            0, 0, maintain_spell },
  { 0, 34, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_spell_button,           112, 651,  0,       {9},            0, 0, maintain_spell },
  { 0, 24, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 314,   6, 318, 32, 36, gui_area_spell_button,           120, 652,  0,       {7},            0, 0, maintain_spell },
  { 0, 26, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 314,  38, 318, 32, 36, gui_area_spell_button,           110, 661,  0,       {8},            0, 0, maintain_spell },
  { 0, 25, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  66, 314,  70, 318, 32, 36, gui_area_spell_button,           124, 657,  0,      {10},            0, 0, maintain_spell },
  { 0, 28, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 314, 102, 318, 32, 36, gui_area_spell_button,           126, 655,  0,      {12},            0, 0, maintain_spell },
  { 0, 30, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,   2, 352,   6, 356, 32, 36, gui_area_spell_button,           314, 658,  0,      {15},            0, 0, maintain_spell },
  { 0, 31, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  34, 352,  38, 356, 32, 36, gui_area_spell_button,           319, 659,  0,      {14},            0, 0, maintain_spell },
  { 0, 33, 0, 0, gui_choose_special_spell,NULL,   NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_spell_button,           321, 663,  0,      {19},            0, 0, maintain_spell },
  { 0, 32, 0, 0, gui_choose_spell,gui_go_to_next_spell,NULL,          0,  98, 352, 102, 356, 32, 36, gui_area_spell_button,           317, 660,  0,      {16},            0, 0, maintain_spell },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, 201,  0,       {0},            0, 0, maintain_big_spell },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit spell_lost_menu_buttons[] = {
  { 0, 36, 0, 0, spell_lost_first_person,NULL,    NULL,               0,   2, 238,   8, 250, 32, 36, gui_area_new_null_button,        114, 647,  0,      {18},            0, 0, maintain_spell },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  34, 238,  40, 250, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 238,  72, 250, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  98, 238, 104, 250, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   2, 276,   8, 288, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  34, 276,  40, 288, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  72, 288, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 104, 288, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   2, 314,   8, 326, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  34, 314,  40, 326, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 314,  72, 326, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  98, 314, 104, 326, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   8, 364, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  40, 364, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  72, 364, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  98, 352, 104, 364, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_spell_button,         0, 201,  0,       {0},            0, 0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit trap_menu_buttons[] = {
  { 0, 54, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0, 2,238,  6, 242, 32, 36, gui_area_trap_button,            154, 585,  0,       {2},            0, 0, maintain_trap },
  { 0, 55, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,34,238, 38, 242, 32, 36, gui_area_trap_button,            156, 586,  0,       {3},            0, 0, maintain_trap },
  { 0, 56, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,66,238, 70, 242, 32, 36, gui_area_trap_button,            158, 587,  0,       {4},            0, 0, maintain_trap },
  { 0, 67, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,98,238,102, 242, 32, 36, gui_area_trap_button,            162, 589,  0,       {6},            0, 0, maintain_trap },
  { 0, 53, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0, 2,276,  6, 280, 32, 36, gui_area_trap_button,            152, 584,  0,       {1},            0, 0, maintain_trap },
  { 0, 57, 0, 0, gui_choose_trap,gui_go_to_next_trap,gui_over_trap_button,0,34,276, 38, 280, 32, 36, gui_area_trap_button,            160, 588,  0,       {5},            0, 0, maintain_trap },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 276,  70, 280, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  98, 276, 102, 280, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0, 58, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0, 2,314,  6, 318, 32, 36, gui_area_trap_button,            166, 594,  0,       {7},            0, 0, maintain_door },
  { 0, 59, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,34,314, 38, 318, 32, 36, gui_area_trap_button,            168, 595,  0,       {8},            0, 0, maintain_door },
  { 0, 60, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,66,314, 70, 318, 32, 36, gui_area_trap_button,            170, 596,  0,       {9},            0, 0, maintain_door },
  { 0, 61, 0, 0, gui_choose_trap,gui_go_to_next_door,gui_over_door_button,0,98,314,102, 318, 32, 36, gui_area_trap_button,            172, 597,  0,      {10},            0, 0, maintain_door },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   2, 352,   6, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  34, 352,  38, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  66, 352,  70, 356, 32, 36, gui_area_new_null_button,         24, 201,  0,       {0},            0, 0, NULL },
  { 0, 62, 0, 0, gui_remove_area_for_traps,NULL,  NULL,               0,  98, 352, 102, 356, 32, 36, gui_area_new_no_anim_button,     107, 463,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   8, 210,   8, 194,126, 44, gui_area_big_trap_button,          0, 201,  0,       {0},            0, 0, maintain_big_trap },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_menu_buttons[] = {
  { 0, 72, 0, 0, pick_up_next_wanderer,gui_go_to_next_wanderer,NULL,  0,  26, 192,  26, 192, 38, 24, gui_area_new_normal_button,      284, 302,  0,       {0},            0, 0, NULL },
  { 0, 73, 0, 0, pick_up_next_worker,gui_go_to_next_worker,NULL,      0,  62, 192,  62, 192, 38, 24, gui_area_new_normal_button,      282, 303,  0,       {0},            0, 0, NULL },
  { 0, 74, 0, 0, pick_up_next_fighter,gui_go_to_next_fighter,NULL,    0,  98, 192,  98, 192, 38, 24, gui_area_new_normal_button,      286, 304,  0,       {0},            0, 0, NULL },
  { 1,  0, 0, 0, gui_scroll_activity_up,gui_scroll_activity_up,NULL,  0,   4, 192,   4, 192, 22, 24, gui_area_new_normal_button,      278, 201,  0,       {0},            0, 0, maintain_activity_up },
  { 1,  0, 0, 0, gui_scroll_activity_down,gui_scroll_activity_down,NULL,0, 4, 364,   4, 364, 22, 24, gui_area_new_normal_button,      280, 201,  0,       {0},            0, 0, maintain_activity_down },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  0,   0, 196,   0, 218, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,26,220,26,220,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[0+0]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,62,220,62,220,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[0+1]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,0,98,220,98,220,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[0+2]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  1,   0, 220,   0, 242, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,26,244,26,244,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[4+0]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,62,244,62,244,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[4+1]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,1,98,244,98,244,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[4+2]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  2,   0, 244,   0, 266, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,26,268,26,268,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[8]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,62,268,62,268,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[9]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,2,98,268,98,268,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[10]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  3,   0, 268,   0, 290, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,26,292,26,292,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[12]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,62,292,62,292,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[13]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,3,98,292,98,292,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[14]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  4,   0, 292,   0, 314, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,26,316,26,316,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[16]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,62,316,62,316,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[17]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,4,98,316,98,316,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[18]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_next_creature,gui_go_to_next_creature,NULL,  5,   0, 314,   0, 338, 22, 22, gui_area_creatrmodel_button,       0, 733,  0,       {0},            0, 0, maintain_activity_pic },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,26,340,26,340,32,20,gui_area_anger_button,   288, 734,  0,{(long)&activity_list[20]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,62,340,62,340,32,20,gui_area_anger_button,   288, 735,  0,{(long)&activity_list[21]},0,0, maintain_activity_row },
  { 0,  0, 0, 0, pick_up_creature_doing_activity,gui_go_to_next_creature_activity,NULL,5,98,340,98,340,32,20,gui_area_anger_button,   288, 736,  0,{(long)&activity_list[22]},0,0, maintain_activity_row },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit query_menu_buttons[] = {
  { 0,  0, 0, 0, gui_set_query,      NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      475, 432,  0,       {0},            0, 0, NULL },
  { 2, 69, 0, 0, gui_set_tend_to,    NULL,        NULL,               1,  36, 190,  36, 190, 32, 26, gui_area_flash_cycle_button,     350, 307,  0,{(long)&game.creatures_tend_1}, 1, 0, maintain_prison_bar },
  { 2, 70, 0, 0, gui_set_tend_to,    NULL,        NULL,               2,  74, 190,  74, 190, 32, 26, gui_area_flash_cycle_button,     346, 306,  0,{(long)&game.creatures_tend_2}, 1, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 216,   4, 222,132, 24, gui_area_payday_button,          341, 454,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   2, 246,   2, 246, 60, 24, gui_area_research_bar,            61, 452,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  74, 246,  74, 246, 60, 24, gui_area_workshop_bar,            75, 453,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  74, 274,  74, 274, 60, 24, gui_area_player_creature_info,   323, 456,  0,       {0},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  74, 298,  74, 298, 60, 24, gui_area_player_creature_info,   325, 456,  0,       {1},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  74, 322,  74, 322, 60, 24, gui_area_player_creature_info,   327, 456,  0,       {2},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  74, 346,  74, 346, 60, 24, gui_area_player_creature_info,   329, 456,  0,       {3},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 274,   4, 274, 60, 24, gui_area_player_room_info,       324, 455,  0,       {0},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 298,   4, 298, 60, 24, gui_area_player_room_info,       326, 455,  0,       {1},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 322,   4, 322, 60, 24, gui_area_player_room_info,       328, 455,  0,       {2},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_player_room_info,       330, 455,  0,       {3},            0, 0, maintain_room_and_creature_button },
  { 0,  0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 274,  62, 274, 14, 22, gui_area_ally,                     0, 469,  0,       {0},            0, 0, maintain_ally },
  { 0,  0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 298,  62, 298, 14, 22, gui_area_ally,                     0, 469,  0,       {1},            0, 0, maintain_ally },
  { 0,  0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 322,  62, 322, 14, 22, gui_area_ally,                     0, 469,  0,       {2},            0, 0, maintain_ally },
  { 0,  0, 0, 0, gui_toggle_ally,    NULL,        NULL,               0,  62, 346,  62, 346, 14, 22, gui_area_ally,                     0, 469,  0,       {3},            0, 0, maintain_ally },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit event_menu_buttons[] = {
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons1[] = {
  { 0,  0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu2,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, 291,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, 223,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {0},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {1},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {2},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {3},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {4},            0, 0, maintain_instance },
  { 0,  0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {5},            0, 0, maintain_instance },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons2[] = {
  { 0,  0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu3,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  80, 200,  80, 200, 56, 24, gui_area_smiley_anger_button,    466, 291,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  80, 230,  80, 230, 56, 24, gui_area_experience_button,      467, 223,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 262,   4, 262,126, 14, NULL,                              0, 222,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 290,   4, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {4},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 290,  72, 290, 60, 24, gui_area_instance_button,         45, 201,  0,       {5},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 318,   4, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {6},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 318,  72, 318, 60, 24, gui_area_instance_button,         45, 201,  0,       {7},            0, 0, maintain_instance },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {8},            0, 0, maintain_instance },
  { 0,  0, 0, 1, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_instance_button,         45, 201,  0,       {9},            0, 0, maintain_instance },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiButtonInit creature_query_buttons3[] = {
  { 0,  0, 0, 1, NULL,               NULL,        NULL,               0,  44, 374,  44, 374, 52, 20, gui_area_new_normal_button,      473, 433,&creature_query_menu1,{0}, 0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 226,   4, 226, 60, 24, gui_area_stat_button,            331, 292,  0,       {0},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 226,  72, 226, 60, 24, gui_area_stat_button,            332, 293,  0,       {1},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 256,   4, 256, 60, 24, gui_area_stat_button,            333, 295,  0,       {2},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 256,  72, 256, 60, 24, gui_area_stat_button,            334, 294,  0,       {3},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 286,   4, 286, 60, 24, gui_area_stat_button,            335, 296,  0,       {4},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 286,  72, 286, 60, 24, gui_area_stat_button,            336, 297,  0,       {5},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 316,   4, 316, 60, 24, gui_area_stat_button,            337, 298,  0,       {6},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 316,  72, 316, 60, 24, gui_area_stat_button,            338, 299,  0,       {7},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,   4, 346,   4, 346, 60, 24, gui_area_stat_button,            339, 300,  0,       {8},            0, 0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,               0,  72, 346,  72, 346, 60, 24, gui_area_stat_button,            340, 301,  0,       {9},            0, 0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,  0,       {0},            0, 0, NULL },
};

struct GuiMenu main_menu =
 { 1, 0, 1, main_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu room_menu =
 { 2, 0, 1, room_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_menu =
 { 3, 0, 1, spell_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu spell_lost_menu =
 { 38, 0, 1, spell_lost_menu_buttons,                    0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu trap_menu =
 { 4, 0, 1, trap_menu_buttons,                           0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu creature_menu =
 { 5, 0, 1, creature_menu_buttons,                       0,   0, 140, 400, gui_activity_background,     0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu query_menu =
 { 7, 0, 1, query_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 1,};
struct GuiMenu event_menu =
 { 6, 0, 1, event_menu_buttons,                          0,   0, 140, 400, NULL,                        0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu creature_query_menu1 =
 { 31, 0, 1, creature_query_buttons1,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu2 =
 { 35, 0, 1, creature_query_buttons2,             0,          0, 140, 400, gui_creature_query_background1,0,NULL,   NULL,                    0, 0, 1,};
struct GuiMenu creature_query_menu3 =
 { 32, 0, 1, creature_query_buttons3,             0,          0, 140, 400, gui_creature_query_background2,0,NULL,   NULL,                    0, 0, 1,};

struct TiledSprite status_panel = {
    2, 4, {
        { 1, 2,},
        { 3, 4,},
        { 5, 6,},
        {21,22,},
    },
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
