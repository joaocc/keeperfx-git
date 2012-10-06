/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_strings.c
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
#include "config_strings.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_guibtns.h"

#include "config.h"
#include "config_campaigns.h"
#include "game_merge.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool reset_strings(char **strings)
{
  int text_idx;
  char **text_arr;
  text_arr = strings;
  text_idx = STRINGS_MAX;
  while (text_idx >= 0)
  {
    *text_arr = lbEmptyString;
    text_arr++;
    text_idx--;
  }
  return true;
}

TbBool create_strings_list(char **strings,char *strings_data,char *strings_data_end)
{
  int text_idx;
  char *text_ptr;
  char **text_arr;
  text_arr = strings;
  text_idx = STRINGS_MAX;
  text_ptr = strings_data;
  while (text_idx >= 0)
  {
    if (text_ptr >= strings_data_end)
    {
      break;
    }
    *text_arr = text_ptr;
    text_arr++;
    char chr_prev;
    do {
            chr_prev = *text_ptr;
            text_ptr++;
    } while ((chr_prev != '\0') && (text_ptr < strings_data_end));
    text_idx--;
  }
  return (text_idx < STRINGS_MAX);
}

/**
 * Loads the language-specific strings data for game interface.
 */
TbBool setup_gui_strings_data(void)
{
  char *strings_data_end;
  char *fname;
  short result;
  long filelen;
  long loaded_size;
  SYNCDBG(8,"Starting");

  fname = prepare_file_fmtpath(FGrp_FxData,"gtext_%s.dat",get_language_lwrstr(install_info.lang_id));
  filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("GUI Strings file does not exist or can't be opened");
    SYNCLOG("Strings file name is \"%s\"",fname);
    return false;
  }
  gui_strings_data = (char *)LbMemoryAlloc(filelen + 256);
  if (gui_strings_data == NULL)
  {
    ERRORLOG("Can't allocate memory for GUI Strings data");
    SYNCLOG("Strings file name is \"%s\"",fname);
    return false;
  }
  strings_data_end = gui_strings_data+filelen+255;
  loaded_size = LbFileLoadAt(fname, gui_strings_data);
  if (loaded_size < 16)
  {
    ERRORLOG("GUI Strings file couldn't be loaded or is too small");
    return false;
  }
  // Resetting all values to empty strings
  reset_strings(gui_strings);
  // Analyzing strings data and filling correct values
  result = create_strings_list(gui_strings, gui_strings_data, strings_data_end);
  // Updating strings inside the DLL
  LbMemoryCopy(_DK_strings, gui_strings, DK_STRINGS_MAX*sizeof(char *));
  SYNCDBG(19,"Finished");
  return result;
}

TbBool free_gui_strings_data(void)
{
  // Resetting all values to empty strings
  reset_strings(gui_strings);
  // Freeing memory
  LbMemoryFree(gui_strings_data);
  gui_strings_data = NULL;
  return true;
}

/**
 * Loads the language-specific strings data for the current campaign.
 */
TbBool setup_campaign_strings_data(struct GameCampaign *campgn)
{
  char *strings_data_end;
  char *fname;
  short result;
  long filelen;
  SYNCDBG(18,"Starting");
  fname = prepare_file_path(FGrp_Main,campgn->strings_fname);
  filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("Campaign Strings file does not exist or can't be opened");
    return false;
  }
  campgn->strings_data = (char *)LbMemoryAlloc(filelen + 256);
  if (campgn->strings_data == NULL)
  {
    ERRORLOG("Can't allocate memory for Campaign Strings data");
    return false;
  }
  strings_data_end = campgn->strings_data+filelen+255;
  long loaded_size;
  loaded_size = LbFileLoadAt(fname, campgn->strings_data);
  if (loaded_size < 16)
  {
    ERRORLOG("Campaign Strings file couldn't be loaded or is too small");
    return false;
  }
  // Resetting all values to empty strings
  reset_strings(campgn->strings);
  // Analyzing strings data and filling correct values
  result = create_strings_list(campgn->strings, campgn->strings_data, strings_data_end);
  SYNCDBG(19,"Finished");
  return result;
}

const char * gui_string(unsigned int index)
{
    if (index >= STRINGS_MAX)
        return lbEmptyString;
    return gui_strings[index];
}
const char * cmpgn_string(unsigned int index)
{
    if ((campaign.strings == NULL) || (index >= STRINGS_MAX))
        return lbEmptyString;
    return campaign.strings[index];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
