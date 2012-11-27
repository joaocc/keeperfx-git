/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.h
 *     Header file for vidmode.cpp.
 *     Note that this file is a C header, while its code is CPP.
 * @par Purpose:
 *     Video mode switching/setting function.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_VIDMODE_H
#define DK_VIDMODE_H

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_video.h"
#include "bflib_filelst.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct TbColorTables {
  unsigned char fade_tables[64*256];
  unsigned char ghost[256*256];
  unsigned char flat_colours_tl[2*256];
  unsigned char flat_colours_tr[2*256];
  unsigned char flat_colours_br[2*256];
  unsigned char flat_colours_bl[2*256];
  unsigned char robs_bollocks[256];
};

struct TbAlphaTables {
    unsigned char black[256];
    unsigned char grey[8*256];
    unsigned char orange[8*256];
    unsigned char red[8*256];
    unsigned char blue[8*256];
    unsigned char green[8*256];
};

struct TbGradientTables {
    unsigned char black[256];
    unsigned char grey[8*256];
    unsigned char orange[8*256];
    unsigned char red[8*256];
    unsigned char blue[8*256];
    unsigned char green[8*256];
};
/******************************************************************************/
DLLIMPORT unsigned short _DK_pixels_per_block;
#define pixels_per_block _DK_pixels_per_block
DLLIMPORT unsigned short _DK_units_per_pixel;
#define units_per_pixel _DK_units_per_pixel
DLLIMPORT int _DK_MinimalResolutionSetup;
#define MinimalResolutionSetup _DK_MinimalResolutionSetup

//DLLIMPORT unsigned char _DK_ghost[256*16];
//#define ghost _DK_ghost
//DLLIMPORT unsigned char _DK_fade_tables[256*64];
//#define fade_tables _DK_fade_tables
DLLIMPORT struct TbColorTables _DK_pixmap;
#define pixmap _DK_pixmap
DLLIMPORT extern struct TbLoadFiles _DK_map_flag_load_files[];
//#define map_flag_load_files _DK_map_flag_load_files
/******************************************************************************/
extern struct TbSprite *pointer_sprites;
extern struct TbLoadFiles legal_load_files[];
extern struct TbLoadFiles map_flag_load_files[];
extern unsigned short units_per_pixel_min;
extern long base_mouse_sensitivity;
/******************************************************************************/
TbScreenMode switch_to_next_video_mode(void);
void set_game_vidmode(unsigned short i,unsigned short nmode);
TbScreenMode reenter_video_mode(void);
TbScreenMode get_next_vidmode(unsigned short mode);
TbScreenMode validate_vidmode(unsigned short mode);
TbScreenMode get_failsafe_vidmode(void);
TbScreenMode get_movies_vidmode(void);
TbScreenMode get_frontend_vidmode(void);
void set_failsafe_vidmode(unsigned short nmode);
void set_movies_vidmode(unsigned short nmode);
void set_frontend_vidmode(unsigned short nmode);
char *get_vidmode_name(unsigned short mode);

TbBool setup_screen_mode(unsigned short nmode);
short setup_screen_mode_minimal(unsigned short nmode);
TbBool setup_screen_mode_zero(unsigned short nmode);

short LoadMcgaData(void);
short LoadMcgaDataMinimal(void);
TbBool update_screen_mode_data(long width, long height);
void load_pointer_file(short hi_res);
TbBool load_testfont_fonts(void);
void free_testfont_fonts(void);

TbBool set_pointer_graphic_none(void);
TbBool set_pointer_graphic_menu(void);
TbBool set_pointer_graphic_spell(long group_idx, long frame);
TbBool set_pointer_graphic(long ptr_idx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
