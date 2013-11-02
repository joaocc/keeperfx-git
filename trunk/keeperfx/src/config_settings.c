/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_settings.c
 *     List of language-specific strings support.
 * @par Purpose:
 *     Support of configuration files for game strings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Nov 2011 - 01 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_settings.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_keybrd.h"
#include "bflib_video.h"
#include "bflib_cpu.h"

#include "config.h"
#include "game_merge.h"
#include "vidmode.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
unsigned char i_can_see_levels[] = {15, 20, 25, 30,};
/******************************************************************************/
DLLIMPORT int _DK_load_settings(void);
DLLIMPORT void _DK_save_settings(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void setup_default_settings(void)
{
    // CPU status variable
    struct CPU_INFO cpu_info;
    const struct GameSettings default_settings = {
     0, 4, 3, 0, 1, 0, 127, 90, 1, 0, 1,
     {
      {KC_UP, KMod_NONE},       {KC_DOWN, KMod_NONE},
      {KC_LEFT, KMod_NONE},     {KC_RIGHT, KMod_NONE},
      {KC_LCONTROL, KMod_NONE}, {KC_LSHIFT, KMod_NONE},
      {KC_DELETE, KMod_NONE},   {KC_PGDOWN, KMod_NONE},
      {KC_HOME, KMod_NONE},     {KC_END, KMod_NONE},
      {KC_T, KMod_NONE},        {KC_L, KMod_NONE},
      {KC_L, KMod_SHIFT},       {KC_P, KMod_SHIFT},
      {KC_T, KMod_ALT},         {KC_T, KMod_SHIFT},
      {KC_H, KMod_NONE},        {KC_W, KMod_NONE},
      {KC_S, KMod_NONE},        {KC_T, KMod_CONTROL},
      {KC_G, KMod_NONE},        {KC_B, KMod_NONE},
      {KC_H, KMod_SHIFT},       {KC_G, KMod_SHIFT},
      {KC_B, KMod_SHIFT},       {KC_F, KMod_NONE},
      {KC_A, KMod_NONE},        {KC_LSHIFT, KMod_NONE},
      {KC_NUMPAD0, KMod_NONE},  {KC_BACK, KMod_NONE},
      {KC_P, KMod_NONE},        {KC_M, KMod_NONE},
     }, 1, 0, 6};
    LbMemoryCopy(&settings, &default_settings, sizeof(struct GameSettings));
    cpu_detect(&cpu_info);
    settings.video_scrnmode = get_next_vidmode(Lb_SCREEN_MODE_INVALID);
    if ((cpu_get_family(&cpu_info) > CPUID_FAMILY_PENTIUM) && (is_feature_on(Ft_HiResVideo)))
    {
        SYNCDBG(6,"Updating to hires video mode");
        settings.video_scrnmode = get_next_vidmode(settings.video_scrnmode);
    }
}

TbBool load_settings(void)
{
    SYNCDBG(6,"Starting");
    char *fname;
    long len;
    fname = prepare_file_path(FGrp_Save,"settings.dat");
    len = LbFileLengthRnc(fname);
    if (len == sizeof(struct GameSettings))
    {
      if (LbFileLoadAt(fname, &settings) == sizeof(struct GameSettings))
          return true;
    }
    setup_default_settings();
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return false;
}

short save_settings(void)
{
  char *fname;
  fname=prepare_file_path(FGrp_Save,"settings.dat");
  LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
  return true;
}

int get_creature_can_see_subtiles(void)
{
    return i_can_see_levels[settings.view_distance % 4];
}
/******************************************************************************/
