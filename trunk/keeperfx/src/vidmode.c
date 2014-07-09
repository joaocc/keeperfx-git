/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode.c
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Functions to change video mode in DK way.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "vidmode.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"

#include "vidfade.h"
#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_heap.h"
#include "gui_draw.h"
#include "gui_parchment.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "creature_graphics.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
TbScreenMode switching_vidmodes[] = {
  Lb_SCREEN_MODE_320_200_8,
  Lb_SCREEN_MODE_640_400_8,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  Lb_SCREEN_MODE_INVALID,
  };

TbScreenMode failsafe_vidmode = Lb_SCREEN_MODE_320_200_8;
TbScreenMode movies_vidmode   = Lb_SCREEN_MODE_320_200_8;
TbScreenMode frontend_vidmode = Lb_SCREEN_MODE_640_480_8;

//struct IPOINT_2D units_per_pixel;
unsigned short units_per_pixel_min;
long base_mouse_sensitivity = 256;

short force_video_mode_reset = true;

struct TbSprite *pointer_sprites;
struct TbSprite *end_pointer_sprites;
unsigned char * pointer_data;

unsigned char *nocd_raw;
unsigned char *nocd_pal;

struct TbSprite *end_map_font;
struct TbSprite *end_map_hand;
TbSpriteData map_font_data;
TbSpriteData end_map_font_data;
TbSpriteData map_hand_data;
TbSpriteData end_map_hand_data;
/******************************************************************************/

extern struct TbSetupSprite setup_sprites_minimal[];
extern struct TbSetupSprite setup_sprites[];
#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
extern struct TbSetupSprite setup_testfont[];
extern struct TbLoadFiles testfont_load_files[];
#endif

extern struct TbLoadFiles mcga_load_files[];
extern struct TbLoadFiles vres256_load_files[];
extern struct TbLoadFiles mcga_load_files_minimal[];
extern struct TbLoadFiles vres256_load_files_minimal[];
extern struct TbLoadFiles low_res_pointer_load_files[];
extern struct TbLoadFiles low_res_small_pointer_load_files[];
extern struct TbLoadFiles hi_res_pointer_load_files[];
extern struct TbLoadFiles hi_res_small_pointer_load_files[];
extern struct TbLoadFiles legal_load_files[];
extern struct TbLoadFiles game_load_files[];
extern struct TbLoadFiles nocd_load_files[];
extern struct TbLoadFiles map_flag_load_files[];
/******************************************************************************/

/**
 * Loads VGA 256 graphics files, for high resolution modes.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadVRes256Data(long scrbuf_size)
{
    int i;
    // Update size of the parchment buffer, as it is also used as screen buffer
    if (scrbuf_size < 640*480)
        scrbuf_size = 640*480;
    i = LbDataFindStartIndex(vres256_load_files,(unsigned char **)&hires_parchment);
    if (i>=0) {
        vres256_load_files[i].SLength = scrbuf_size;
    }
    // Load the files
    if (LbDataLoadAll(vres256_load_files)) {
        return 0;
    }
    return 1;
}

/**
 * Loads MCGA graphics files, for low resolution mode.
 * It is modified version of LbDataLoadAll, optimized for maximum
 * game speed on very slow machines.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadMcgaData(void)
{
  struct TbLoadFiles *load_files;
  void *mem;
  struct TbLoadFiles *t_lfile;
  int ferror;
  int ret_val;
  int i;
  load_files = mcga_load_files;
  LbDataFreeAll(load_files);
  ferror = 0;
  i = 0;
  t_lfile = &load_files[i];
  // Allocate some low memory, only to be sure that
  // it will be free when this function ends
  mem = LbMemoryAllocLow(0x10000u);
  while (t_lfile->Start != NULL)
  {
    // Don't allow loading flags
    t_lfile->Flags = 0;
    ret_val = LbDataLoad(t_lfile);
    if (ret_val == -100)
    {
      ERRORLOG("Can't allocate memory for MCGA files element \"%s\".", t_lfile->FName);
      ferror++;
    } else
    if ( ret_val == -101 )
    {
      ERRORLOG("Can't load MCGA file \"%s\".", t_lfile->FName);
      ferror++;
    }
    i++;
    t_lfile = &load_files[i];
  }
  if (mem != NULL)
    LbMemoryFree(mem);
  return (ferror == 0);
}

/**
 * Loads MCGA graphics files, for low resolution mode.
 * Loads only most importand files, where no GUI is needed.
 * @return Returns true if all files were loaded, false otherwise.
 */
short LoadMcgaDataMinimal(void)
{
  // Load the files
  if (LbDataLoadAll(mcga_load_files_minimal))
    return 0;
  return 1;
}

TbScreenMode get_next_vidmode(unsigned short mode)
{
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (i=0;i<maxmodes;i++)
  {
    if (switching_vidmodes[i]==mode) break;
  }
//  SYNCMSG("SEL IDX %d ALL %d SEL %d PREV %d",i,maxmodes,switching_vidmodes[i],mode);
  i++;
  if (i>=maxmodes)
  {
    i=0;
  } else
  if (switching_vidmodes[i] == Lb_SCREEN_MODE_INVALID)
  {
    i=0;
  }
  return switching_vidmodes[i];
}

void set_game_vidmode(unsigned short i,unsigned short nmode)
{
  switching_vidmodes[i]=(TbScreenMode)nmode;
}

TbScreenMode validate_vidmode(unsigned short mode)
{
  int i;
  int maxmodes=sizeof(switching_vidmodes)/sizeof(TbScreenMode);
  // Do not allow to enter higher modes on low memory systems
  if ((features_enabled & Ft_HiResVideo) == 0)
    return failsafe_vidmode;
  for (i=0;i<maxmodes;i++)
  {
    if (switching_vidmodes[i] == mode) return switching_vidmodes[i];
  }
  return failsafe_vidmode;
}

TbScreenMode get_failsafe_vidmode(void)
{
  return failsafe_vidmode;
}

void set_failsafe_vidmode(unsigned short nmode)
{
  failsafe_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_movies_vidmode(void)
{
  return movies_vidmode;
}

void set_movies_vidmode(unsigned short nmode)
{
  movies_vidmode=(TbScreenMode)nmode;
}

TbScreenMode get_frontend_vidmode(void)
{
  return frontend_vidmode;
}

void set_frontend_vidmode(unsigned short nmode)
{
  frontend_vidmode=(TbScreenMode)nmode;
}

void load_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  if ((features_enabled & Ft_BigPointer) == 0)
  {
    if (hi_res)
      ldfiles = hi_res_small_pointer_load_files;
    else
      ldfiles = low_res_small_pointer_load_files;
  } else
  {
    if (hi_res)
      ldfiles = hi_res_pointer_load_files;
    else
      ldfiles = low_res_pointer_load_files;
  }
  if ( LbDataLoadAll(ldfiles) )
    ERRORLOG("Unable to load pointer files");
  LbSpriteSetup(pointer_sprites, end_pointer_sprites, pointer_data);
}

TbBool set_pointer_graphic_none(void)
{
  LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  return true;
}

TbBool set_pointer_graphic_menu(void)
{
  if (frontend_sprite == NULL)
  {
    WARNLOG("Frontend sprites not loaded, setting pointer to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  LbMouseChangeSpriteAndHotspot(&frontend_sprite[1], 0, 0);
  return true;
}

TbBool set_pointer_graphic_spell(long group_idx, long frame)
{
  long i,x,y;
  struct TbSprite *spr;
  SYNCDBG(8,"Setting to group %d",(int)group_idx);
  if (pointer_sprites == NULL)
  {
    WARNLOG("Pointer sprites not loaded, setting to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if ((group_idx < 0) || (group_idx >= SPELL_POINTER_GROUPS))
  {
    WARNLOG("Group index out of range, setting pointer to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if (is_feature_on(Ft_BigPointer))
  {
    y = 32;
    x = 32;
    i = 8*group_idx + (frame%8);
  } else
  {
    y = 78;
    x = 26;
    i = group_idx;
  }
  spr = &pointer_sprites[40+i];
  SYNCDBG(8,"Activating pointer %d",40+i);
  if ((spr >= pointer_sprites) && (spr < end_pointer_sprites))
  {
    LbMouseChangeSpriteAndHotspot(spr, x/2, y/2);
  } else
  {
    WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)i);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

TbBool set_pointer_graphic(long ptr_idx)
{
  long x,y;
  struct TbSprite *spr;
  SYNCDBG(8,"Setting to %d",(int)ptr_idx);
  if (pointer_sprites == NULL)
  {
    WARNLOG("Pointer sprites not loaded, setting to none");
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  switch (ptr_idx)
  {
  case   0:
  case   1:
  case   2:
  case   4:
  case  15:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case   3:
      spr = &pointer_sprites[ptr_idx];
      x = 17; y = 29;
      break;
  case   5:
  case   6:
  case   7:
  case   8:
  case   9:
  case  10:
  case  11:
  case  12:
  case  13:
  case  14:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  16:
  case  17:
  case  18:
  case  19:
  case  20:
  case  21:
  case  22:
  case  23:
  case  24:
      spr = &pointer_sprites[ptr_idx];
      x = 20; y = 20;
      break;
  case  25:
  case  26:
  case  27:
  case  28:
  case  29:
  case  30:
  case  31:
  case  32:
  case  33:
  case  34:
  case  35:
  case  36:
  case  37:
  case  38:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 38;
      break;
  case  39:
  // 40..144 are spell pointers
  case  47:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  case  96:
  case  97:
  case  98:
  case  99:
  case 100:
  case 101:
  case 102:
  case 103:
      spr = &pointer_sprites[ptr_idx];
      x = 12; y = 15;
      break;
  default:
    WARNLOG("Unrecognized Mouse Pointer index, %d",ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
    return false;
  }
  if ((spr >= pointer_sprites) && (spr < end_pointer_sprites))
  {
    LbMouseChangeSpriteAndHotspot(spr, x, y);
  } else
  {
    WARNLOG("Sprite %d exceeds buffer, setting pointer to none",(int)ptr_idx);
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  }
  return true;
}

void unload_pointer_file(short hi_res)
{
  struct TbLoadFiles *ldfiles;
  set_pointer_graphic_none();
  if ((features_enabled & Ft_BigPointer) == 0)
  {
    if (hi_res)
      ldfiles = hi_res_small_pointer_load_files;
    else
      ldfiles = low_res_small_pointer_load_files;
  } else
  {
    if (hi_res)
      ldfiles = hi_res_pointer_load_files;
    else
      ldfiles = low_res_pointer_load_files;
  }
  LbDataFreeAll(ldfiles);
}

TbBool init_fades_table(void)
{
    char *fname;
    long i;
    static const char textname[] = "fade table";
    fname = prepare_file_path(FGrp_StdData,"tables.dat");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    if (LbFileLoadAt(fname, &pixmap) != sizeof(struct TbColorTables))
    {
        compute_fade_tables(&pixmap,engine_palette,engine_palette);
        LbFileSaveAt(fname, &pixmap, sizeof(struct TbColorTables));
    }
    lbDisplay.FadeTable = pixmap.fade_tables;
    TbPixel cblack = 144;
    // Update black color
    for (i=0; i < 8192; i++)
    {
        if (pixmap.fade_tables[i] == 0) {
            pixmap.fade_tables[i] = cblack;
        }
    }
    return true;
}

TbBool init_alpha_table(void)
{
    char *fname;
    static const char textname[] = "alpha color table";
    fname = prepare_file_path(FGrp_StdData,"alpha.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &alpha_sprite_table) != sizeof(struct TbAlphaTables))
    {
        compute_alpha_tables(&alpha_sprite_table,engine_palette,engine_palette);
        LbFileSaveAt(fname, &alpha_sprite_table, sizeof(struct TbAlphaTables));
    }
    return true;
}

TbBool init_rgb2idx_table(void)
{
    char *fname;
    static const char textname[] = "rgb-to-index color table";
    fname = prepare_file_path(FGrp_StdData,"colours.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &colours) != sizeof(TbRGBColorTable))
    {
        compute_rgb2idx_table(colours,engine_palette);
        LbFileSaveAt(fname, &colours, sizeof(TbRGBColorTable));
    }
    return true;
}

TbBool init_redpal_table(void)
{
    char *fname;
    static const char textname[] = "red-blended color table";
    fname = prepare_file_path(FGrp_StdData,"redpal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &red_pal) != 256)
    {
        compute_shifted_palette_table(red_pal, engine_palette, engine_palette, 20, -10, -10);
        LbFileSaveAt(fname, &red_pal, 256);
    }
    return true;
}

TbBool init_whitepal_table(void)
{
    char *fname;
    static const char textname[] = "white-blended color table";
    fname = prepare_file_path(FGrp_StdData,"whitepal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &white_pal) != 256)
    {
        compute_shifted_palette_table(white_pal, engine_palette, engine_palette, 48, 48, 48);
        LbFileSaveAt(fname, &white_pal, 256);
    }
    return true;
}

void init_colours(void)
{
    init_rgb2idx_table();
    init_redpal_table();
    init_whitepal_table();
}

char *get_vidmode_name(unsigned short mode)
{
  TbScreenModeInfo *mdinfo;
  mdinfo = LbScreenGetModeInfo(mode);
  return   mdinfo->Desc;
}

TbBool setup_screen_mode(unsigned short nmode)
{
    TbScreenModeInfo *mdinfo;
    unsigned int flg_mem;
    long lens_mem;
    short was_minimal_res;
    SYNCDBG(4,"Setting up mode %d",(int)nmode);
    if (!force_video_mode_reset)
    {
      if ((nmode == lbDisplay.ScreenMode) && (!MinimalResolutionSetup))
      {
        SYNCDBG(6,"Mode %d already active, no changes.",(int)nmode);
        return true;
      }
    }
    lens_mem = game.numfield_1B;
    flg_mem = lbDisplay.DrawFlags;
    was_minimal_res = (MinimalResolutionSetup || force_video_mode_reset);
    set_pointer_graphic_none();
    if (LbGraphicsScreenHeight() < 200)
    {
        WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)lbDisplay.ScreenMode);
    } else
    if (LbGraphicsScreenHeight() < 400)
    {
        if (MinimalResolutionSetup)
        {
          if (lbDisplay.ScreenMode != nmode)
            LbScreenReset();
          LbDataFreeAll(mcga_load_files_minimal);
          ERRORLOG("MCGA Minimal not allowed (Reset)");
          MinimalResolutionSetup = 0;
        } else
        {
          reset_eye_lenses();
          reset_heap_manager();
          reset_heap_memory();
          set_pointer_graphic_none();
          unload_pointer_file(0);
          if (lbDisplay.ScreenMode != nmode)
            LbScreenReset();
          LbDataFreeAll(mcga_load_files);
        }
    } else
    // so (LbGraphicsScreenHeight() >= 400)
    {
        if (MinimalResolutionSetup)
        {
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset();
          LbDataFreeAll(vres256_load_files_minimal);
          MinimalResolutionSetup = 0;
        } else
        {
          reset_eye_lenses();
          reset_heap_manager();
          reset_heap_memory();
          set_pointer_graphic_none();
          unload_pointer_file(1);
          if ((lbDisplay.ScreenMode != nmode) || (MinimalResolutionSetup))
            LbScreenReset();
          LbDataFreeAll(vres256_load_files);
        }
    }

    mdinfo = LbScreenGetModeInfo(nmode);
    if (mdinfo->Height < 200)
    {
        ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
        force_video_mode_reset = true;
        return false;
    } else
    if (mdinfo->Height < 400)
    {
        SYNCDBG(6,"Entering low-res mode %d, resolution %dx%d.",(int)nmode,(int)mdinfo->Width,(int)mdinfo->Height);
        if (!LoadMcgaData())
        {
          ERRORLOG("Loading Mcga files failed");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, engine_palette, 2, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(0);
    } else
    // so (mdinfo->Height >= 400)
    {
        SYNCDBG(6,"Entering hi-res mode %d, resolution %dx%d.",(int)nmode,(int)mdinfo->Width,(int)mdinfo->Height);
        if (!LoadVRes256Data((long)mdinfo->Width*(long)mdinfo->Height))
        {
          ERRORLOG("Unable to load VRes256 data files");
          force_video_mode_reset = true;
          return false;
        }
        if ((lbDisplay.ScreenMode != nmode) || (was_minimal_res))
        {
            if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, engine_palette, 1, 0) != 1)
            {
              ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                  mdinfo->Desc,(int)nmode);
              force_video_mode_reset = true;
              return false;
            }
        }
        load_pointer_file(1);
    }
    LbScreenClear(0);
    LbScreenSwap();
    update_screen_mode_data(mdinfo->Width, mdinfo->Height);
    if (parchment_loaded)
      reload_parchment_file(mdinfo->Width >= 640);
    reinitialise_eye_lens(lens_mem);
    LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1, (MyScreenHeight/pixel_size) >> 1);
    lbDisplay.DrawFlags = flg_mem;
    if (!setup_heap_memory())
    {
      force_video_mode_reset = true;
      return false;
    }
    setup_heap_manager();
    game.numfield_C &= ~0x0004;
    force_video_mode_reset = false;
    SYNCDBG(8,"Finished");
    return true;
}

TbBool update_screen_mode_data(long width, long height)
{
  if ((width >= 640) && (height >= 400))
  {
    pixel_size = 1;
  } else
  {
    pixel_size = 2;
  }
  MyScreenWidth = width * (long)pixel_size;
  MyScreenHeight = height * (long)pixel_size;
  pixels_per_block = 16 * (long)pixel_size;
  units_per_pixel = (width>height?width:height)/40;// originally was 16 for hires, 8 for lores
  units_per_pixel_min = (width>height?height:width)/40;// originally 10 for hires
  if (MinimalResolutionSetup)
    LbSpriteSetupAll(setup_sprites_minimal);
  else
    LbSpriteSetupAll(setup_sprites);
  LbMouseSetup(NULL);
  LbMouseChangeMoveRatio(base_mouse_sensitivity*units_per_pixel/16, base_mouse_sensitivity*units_per_pixel/16);
  LbMouseSetPointerHotspot(0, 0);
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  return true;
}

short setup_screen_mode_minimal(unsigned short nmode)
{
  unsigned int flg_mem;
  TbScreenModeInfo *mdinfo;
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  if (!force_video_mode_reset)
  {
    if ((nmode == lbDisplay.ScreenMode) && (MinimalResolutionSetup))
      return 1;
  }
  flg_mem = lbDisplay.DrawFlags;
  if (LbGraphicsScreenHeight() < 200)
  {
      WARNLOG("Unhandled previous Screen Mode %d, Reset skipped",(int)lbDisplay.ScreenMode);
  } else
  if (LbGraphicsScreenHeight() < 400)
  {
      if (MinimalResolutionSetup)
      {
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(mcga_load_files_minimal);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        set_pointer_graphic_none();
        unload_pointer_file(0);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(mcga_load_files);
      }
  } else
  {
      if (MinimalResolutionSetup)
      {
        set_pointer_graphic_none();
        unload_pointer_file(1);
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(vres256_load_files_minimal);
        MinimalResolutionSetup = 0;
      } else
      {
        reset_eye_lenses();
        reset_heap_manager();
        reset_heap_memory();
        if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
          LbScreenReset();
        LbDataFreeAll(vres256_load_files);
      }
  }
  mdinfo = LbScreenGetModeInfo(nmode);
  if (mdinfo->Height < 200)
  {
      ERRORLOG("Unhandled Screen Mode %d, setup failed",(int)nmode);
      force_video_mode_reset = true;
      return 0;
  } else
  if (mdinfo->Height < 400)
  {
      SYNCDBG(17,"Preparing minimal low resolution mode");
      MinimalResolutionSetup = 1;
      if ( !LoadMcgaDataMinimal() )
      {
        ERRORLOG("Unable to load minimal MCGA files");
        return 0;
      }
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, engine_palette, 2, 0) != 1)
          {
            ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                mdinfo->Desc,(int)nmode);
            force_video_mode_reset = true;
            return 0;
          }
      }
  } else
  {
      SYNCDBG(17,"Preparing minimal high resolution mode");
      MinimalResolutionSetup = 1;
      frontend_load_data_from_cd();
      if ( LbDataLoadAll(vres256_load_files_minimal) )
      {
        ERRORLOG("Unable to load vres256_load_files_minimal files");
        force_video_mode_reset = true;
        return 0;
      }
      frontend_load_data_reset();
      if ((nmode != lbDisplay.ScreenMode) || (force_video_mode_reset))
      {
          if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, engine_palette, 1, 0) != 1)
          {
             ERRORLOG("Unable to setup screen resolution %s (mode %d)",
                 mdinfo->Desc,(int)nmode);
             force_video_mode_reset = true;
             return 0;
          }
      }
  }
  LbScreenClear(0);
  LbScreenSwap();
  update_screen_mode_data(mdinfo->Width, mdinfo->Height);
  lbDisplay.DrawFlags = flg_mem;
  force_video_mode_reset = false;
  return 1;
}

TbBool setup_screen_mode_zero(unsigned short nmode)
{
  TbScreenModeInfo *mdinfo;
  SYNCDBG(4,"Setting up mode %d",(int)nmode);
  mdinfo = LbScreenGetModeInfo(nmode);
  LbPaletteDataFillBlack(engine_palette);
  if (LbScreenSetup((TbScreenMode)nmode, mdinfo->Width, mdinfo->Height, engine_palette, 2, 0) != 1)
  {
      ERRORLOG("Unable to setup screen resolution %s (mode %d)",
          mdinfo->Desc,(int)nmode);
      return false;
  }
  force_video_mode_reset = true;
  return true;
}

TbScreenMode reenter_video_mode(void)
{
 TbScreenMode scrmode;
 scrmode=validate_vidmode(settings.video_scrnmode);
 if ( setup_screen_mode(scrmode) )
  {
      settings.video_scrnmode = scrmode;
  } else
  {
      SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
          get_vidmode_name(scrmode),(int)scrmode);
      scrmode=get_failsafe_vidmode();
      if ( !setup_screen_mode(scrmode) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return Lb_SCREEN_MODE_INVALID;
      }
      settings.video_scrnmode = scrmode;
      save_settings();
  }
  SYNCLOG("Switched video to %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
  return scrmode;
}

TbScreenMode switch_to_next_video_mode(void)
{
    TbScreenMode scrmode;
    scrmode = get_next_vidmode(lbDisplay.ScreenMode);
    if ( setup_screen_mode(scrmode) )
    {
        settings.video_scrnmode = scrmode;
    } else
    {
        SYNCLOG("Can't enter %s (mode %d), falling to failsafe mode",
            get_vidmode_name(scrmode),(int)scrmode);
        scrmode = get_failsafe_vidmode();
        if ( !setup_screen_mode(scrmode) )
        {
          FatalError = 1;
          exit_keeper = 1;
          return Lb_SCREEN_MODE_INVALID;
        }
        settings.video_scrnmode = scrmode;
    }
    SYNCLOG("Switched video to %s (mode %d)", get_vidmode_name(scrmode),(int)scrmode);
    save_settings();
    if (game.numfield_C & 0x20)
      setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
    else
      setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
//    reinit_all_menus();
    return scrmode;
}

#if (BFDEBUG_LEVEL > 0)
TbBool load_testfont_fonts(void)
{
  if ( LbDataLoadAll(testfont_load_files) )
  {
    ERRORLOG("Unable to load testfont_load_files files");
    return false;
  }
  LbSpriteSetupAll(setup_testfont);
  return true;
}

void free_testfont_fonts(void)
{
  LbDataFreeAll(testfont_load_files);
}
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
