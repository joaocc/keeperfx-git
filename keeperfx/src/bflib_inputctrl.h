/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_inputctrl.h
 *     Header file for bflib_inputctrl.h.
 * @par Purpose:
 *     Input devices control and polling.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     16 Mar 2009 - 12 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_INPUTCTRL_H
#define BFLIB_INPUTCTRL_H

#include "bflib_basics.h"

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern volatile int lbUserQuit;
/******************************************************************************/
TbBool LbWindowsControl(void);
TbResult LbInputRestate(void);
TbBool LbIsActive(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
