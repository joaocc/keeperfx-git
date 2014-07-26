/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_draw.c
 *     GUI elements drawing functions.
 * @par Purpose:
 *     On-screen drawing of GUI elements, like buttons, menus and panels.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_draw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_planar.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "bflib_guibtns.h"

#include "front_simple.h"
#include "frontend.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char gui_textbuf[TEXT_BUFFER_LENGTH];

/******************************************************************************/
DLLIMPORT void _DK_draw_button_string(struct GuiButton *gbtn, const char *text);
DLLIMPORT int _DK_draw_text_box(char *text);
DLLIMPORT void _DK_draw_slab64k(long pos_x, long pos_y, long width, long height);
DLLIMPORT void _DK_draw_ornate_slab64k(long pos_x, long pos_y, long width, long height);
DLLIMPORT void _DK_draw_ornate_slab_outline64k(long pos_x, long pos_y, long width, long height);
/******************************************************************************/

/******************************************************************************/

int get_bitmap_max_scale(int img_w,int img_h,int rect_w,int rect_h)
{
    int w,h,m;
    w = 0;
    h = 0;
    for (m=0; m < 5; m++)
    {
        w += img_w;
        h += img_h;
        if (w > rect_w) break;
        if (h > rect_h) break;
    }
    // The image width can't be larger than video resolution
    if (m < 1)
    {
        if (w > lbDisplay.PhysicalScreenWidth)
          return 0;
        m = 1;
    }
    return m;
}

void draw_bar64k(long pos_x, long pos_y, int units_per_px, long width)
{
    //RESCALE
    long body_end;
    long x;
    if (width < 72)
    {
        ERRORLOG("Bar is too small");
        return;
    }
    // Button opening sprite
    struct TbSprite *spr;
    spr = &button_sprite[1];
    x = pos_x;
    LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
    x += spr->SWidth * units_per_px / 16;
    // Button body
    body_end = pos_x + width - 64;
    while (x < body_end)
    {
        spr = &button_sprite[2];
        LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
    }
    x = body_end;
    spr = &button_sprite[2];
    LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
    x += spr->SWidth * units_per_px / 16;
    // Button ending sprite
    spr = &button_sprite[3];
    LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
}

void draw_lit_bar64k(long pos_x, long pos_y, long width)
{
    long body_end;
    long x;
    if (width < 32)
    {
        ERRORLOG("Bar is too small");
        return;
    }
    // opening sprite
    struct TbSprite *spr;
    spr = &button_sprite[7];
    LbSpriteDraw(pos_x/pixel_size, pos_y/pixel_size, spr);
    // body
    body_end = pos_x+width-64;
    for (x = pos_x+32; x<body_end; x+=32)
    {
        spr = &button_sprite[8];
        LbSpriteDraw(x/pixel_size, pos_y/pixel_size, spr);
    }
    spr = &button_sprite[8];
    LbSpriteDraw(body_end/pixel_size, pos_y/pixel_size, spr);
    // ending sprite
    spr = &button_sprite[9];
    LbSpriteDraw((pos_x+width-32)/pixel_size, pos_y/pixel_size, spr);
}

void draw_slab64k_background(long pos_x, long pos_y, long width, long height)
{
    long scr_x, scr_y;
    long scr_w, scr_h;
    long i;
    scr_x = pos_x / pixel_size;
    scr_y = pos_y / pixel_size;
    scr_h = height / pixel_size;
    scr_w = width / pixel_size;
    if (scr_x < 0)
    {
        i = scr_x + width / pixel_size;
        scr_x = 0;
        scr_w = i;
    }
    if (scr_y < 0)
    {
        i = scr_y + scr_h;
        scr_y = 0;
        scr_h = i;
    }
    i = lbDisplay.PhysicalScreenWidth * pixel_size;
    if (scr_x + scr_w > i)
        scr_w = i - scr_x;
    i = MyScreenHeight;
    if (scr_y + scr_h > i)
        scr_h = i - scr_y;
    TbPixel *out;
    out = &lbDisplay.WScreen[scr_x + lbDisplay.GraphicsScreenWidth * scr_y];
    for (i=0; scr_h > i; i++)
    {
        TbPixel *inp;
        inp = &gui_slab[GUI_SLAB_DIMENSION * (i % GUI_SLAB_DIMENSION)];
        if (scr_w >= GUI_SLAB_DIMENSION)
        {
            int k;
            memcpy(out, inp, GUI_SLAB_DIMENSION);
            for (k=GUI_SLAB_DIMENSION; k < scr_w-GUI_SLAB_DIMENSION; k+=GUI_SLAB_DIMENSION) {
                memcpy(out + k, inp, GUI_SLAB_DIMENSION);
            }
            if (width - k > 0) {
                memcpy(out + k, inp, scr_w - k);
            }
        } else
        {
            memcpy(out, inp, scr_w);
        }
        out += lbDisplay.GraphicsScreenWidth;
    }
}

void draw_slab64k(long pos_x, long pos_y, long width, long height)
{
    //_DK_draw_slab64k(pos_x, pos_y, width, height);
    draw_slab64k_background(pos_x, pos_y, width, height);
    struct TbSprite *spr;
    int i;
    for (i=10; i < width-12; i+=16)
    {
        spr = &button_sprite[210];
        LbSpriteDraw((i + pos_x) / pixel_size, (pos_y - 6) / pixel_size, spr);
        spr = &button_sprite[211];
        LbSpriteDraw((i + pos_x) / pixel_size, (height + pos_y) / pixel_size, spr);
    }
    for (i=10; i < height-12; i+=16)
    {
        spr = &button_sprite[212];
        LbSpriteDraw((pos_x - 6) / pixel_size, (i + pos_y) / pixel_size, spr);
        spr = &button_sprite[213];
        LbSpriteDraw((width + pos_x) / pixel_size, (i + pos_y) / pixel_size, spr);
    }
    spr = &button_sprite[206];
    LbSpriteDraw((pos_x - 6) / pixel_size, (pos_y - 6) / pixel_size, spr);
    spr = &button_sprite[207];
    LbSpriteDraw((pos_x + width - 12) / pixel_size, (pos_y - 6) / pixel_size, spr);
    spr = &button_sprite[208];
    LbSpriteDraw((pos_x - 6) / pixel_size, (pos_y + height - 12) / pixel_size, spr);
    spr = &button_sprite[209];
    LbSpriteDraw((pos_x + width - 12) / pixel_size, (pos_y + height - 12) / pixel_size, spr);
}

void draw_ornate_slab64k(long pos_x, long pos_y, long width, long height)
{
    //_DK_draw_ornate_slab64k(pos_x, pos_y, width, height);
    draw_slab64k_background(pos_x, pos_y, width, height);
    struct TbSprite *spr;
    int i;
    for (i=10; i < width-12; i+=32)
    {
        spr = &button_sprite[13];
        LbSpriteDraw((i + pos_x)/pixel_size, (pos_y - 4)/pixel_size, spr);
        spr = &button_sprite[18];
        LbSpriteDraw((i + pos_x)/pixel_size, (height + pos_y)/pixel_size, spr);
    }
    for (i=10; i < height-16; i+=32)
    {
        spr = &button_sprite[15];
        LbSpriteDraw((pos_x - 4)/pixel_size, (i + pos_y)/pixel_size, spr);
        spr = &button_sprite[16];
        LbSpriteDraw((width + pos_x)/pixel_size, (i + pos_y)/pixel_size, spr);
    }
    spr = &button_sprite[12];
    LbSpriteDraw((pos_x - 4)/pixel_size, (pos_y - 4)/pixel_size, spr);
    spr = &button_sprite[14];
    LbSpriteDraw((pos_x + width - 28)/pixel_size, (pos_y - 4)/pixel_size, spr);
    spr = &button_sprite[17];
    LbSpriteDraw((pos_x - 4) / pixel_size, (pos_y + height - 28) / pixel_size, spr);
    spr = &button_sprite[19];
    LbSpriteDraw((pos_x + width - 28) / pixel_size, (pos_y + height - 28) / pixel_size, spr);
    spr = &button_sprite[10];
    LbSpriteDraw((pos_x - 32) / pixel_size, (pos_y - 14) / pixel_size, spr);
    spr = &button_sprite[11];
    LbSpriteDraw((pos_x - 34) / pixel_size, (pos_y + height - 78) / pixel_size, spr);
    lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    spr = &button_sprite[10];
    LbSpriteDraw((pos_x + width - 96) / pixel_size, (pos_y - 14) / pixel_size, spr);
    spr = &button_sprite[11];
    LbSpriteDraw((pos_x + width - 92) / pixel_size, (pos_y + height - 78) / pixel_size, spr);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
}

void draw_ornate_slab_outline64k(long pos_x, long pos_y, long width, long height)
{
    //_DK_draw_ornate_slab_outline64k(pos_x, pos_y, width, height);
    long x, y;
    struct TbSprite *spr;
    x = pos_x;
    y = pos_y;
    int i;
    for (i=10; i < width - 12; i+=32)
    {
        spr = &button_sprite[13];
        LbSpriteDraw((pos_x + i) / pixel_size, (pos_y - 4) / pixel_size, spr);
        spr = &button_sprite[18];
        LbSpriteDraw((pos_x + i) / pixel_size, (pos_y + height) / pixel_size, spr);
    }
    spr = button_sprite;
    for (i=10; i < height - 16; i+=32)
    {
        spr = &button_sprite[15];
        LbSpriteDraw((x - 4) / pixel_size, (i + y) / pixel_size, spr);
        spr = &button_sprite[16];
        LbSpriteDraw((x + width) / pixel_size, (i + y) / pixel_size, spr);
    }
    spr = &button_sprite[12];
    LbSpriteDraw((x - 4) / pixel_size, (y - 4) / pixel_size, spr);
    spr = &button_sprite[14];
    LbSpriteDraw((width + x - 28) / pixel_size, (y - 4) / pixel_size, spr);
    spr = &button_sprite[17];
    LbSpriteDraw((x - 4) / pixel_size, (y - 28 + height) / pixel_size, spr);
    spr = &button_sprite[19];
    LbSpriteDraw((width + x - 28) / pixel_size, (y - 28 + height) / pixel_size, spr);
    spr = &button_sprite[10];
    LbSpriteDraw((x - 32) / pixel_size, (y - 14) / pixel_size, spr);
    spr = &button_sprite[11];
    LbSpriteDraw((x - 34) / pixel_size, (y - 78 + height) / pixel_size, spr);
    lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    spr = &button_sprite[10];
    LbSpriteDraw((width + x - 96) / pixel_size, (y - 14) / pixel_size, spr);
    spr = &button_sprite[11];
    LbSpriteDraw((width + x - 92) / pixel_size, (y - 78 + height) / pixel_size, spr);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
}

void draw_round_slab64k(long pos_x, long pos_y, long width, long height)
{
    unsigned short drwflags_mem;
    drwflags_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox((pos_x + 4) / pixel_size, (pos_y + 4) / pixel_size, (width - 8) / pixel_size, (height - 8) / pixel_size, 1);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    int x,y;
    struct TbSprite *spr;
    long i;
    spr = gui_panel_sprites;
    for (i = 0; i < width - 68; i += 26)
    {
        x = pos_x + i + 34;
        y = pos_y + height - 4;
        LbSpriteDraw(x / pixel_size, pos_y / pixel_size, &spr[242]);
        LbSpriteDraw(x / pixel_size, y / pixel_size, &spr[248]);
    }
    for (i = 0; i < height - 56; i += 20)
    {
        x = pos_x + width - 4;
        y = i + 28 + pos_y;
        LbSpriteDraw(pos_x / pixel_size, y / pixel_size, &spr[244]);
        LbSpriteDraw(x / pixel_size, y / pixel_size, &spr[246]);
    }
    x = pos_x + width - 34;
    y = pos_y + height - 28;
    LbSpriteDraw(pos_x / pixel_size, pos_y / pixel_size, &spr[241]);
    LbSpriteDraw(x / pixel_size, pos_y / pixel_size, &spr[243]);
    LbSpriteDraw(pos_x / pixel_size, y / pixel_size, &spr[247]);
    LbSpriteDraw(x / pixel_size, y / pixel_size, &spr[249]);
    lbDisplay.DrawFlags = drwflags_mem;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one panel sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_gui_panel_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &gui_panel_sprites[spridx];
    if ((spr <= gui_panel_sprites) || (spr >= end_gui_panel_sprites) || (spr->SHeight < 1))
        return 16;
    units_per_px = ((gbtn->height*fraction/100) * 16 + 8) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one panel sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_gui_panel_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &gui_panel_sprites[spridx];
    if ((spr <= gui_panel_sprites) || (spr >= end_gui_panel_sprites) || (spr->SWidth < 1))
        return 16;
    units_per_px = ((gbtn->width*fraction/100) * 16 + 8) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one button sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_button_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &button_sprite[spridx];
    if ((spr <= button_sprite) || (spr >= end_button_sprites) || (spr->SHeight < 1))
        return 16;
    units_per_px = ((gbtn->height*fraction/100) * 16 + 8) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one button sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_button_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &button_sprite[spridx];
    if ((spr <= button_sprite) || (spr >= end_button_sprites) || (spr->SWidth < 1))
        return 16;
    units_per_px = ((gbtn->width*fraction/100) * 16 + 8) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_frontend_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &frontend_sprite[spridx];
    if ((spr <= frontend_sprite) || (spr >= frontend_end_sprite) || (spr->SHeight < 1))
        return 16;
    units_per_px = ((gbtn->height*fraction/100) * 16 + 8) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_frontend_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    int units_per_px;
    struct TbSprite *spr;
    spr = &frontend_sprite[spridx];
    if ((spr <= frontend_sprite) || (spr >= frontend_end_sprite) || (spr->SWidth < 1))
        return 16;
    units_per_px = ((gbtn->width*fraction/100) * 16 + 8) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/** Draws a string on GUI button.
 *  Note that the source text buffer may be damaged by this function.
 * @param gbtn Button to draw text on.
 * @param text Text to be displayed. The buffer may be changed by this function.
 *     It should have at least TEXT_BUFFER_LENGTH in size.
 */
void draw_button_string(struct GuiButton *gbtn, char *text)
{
    unsigned long flgmem;
    static unsigned char cursor_type = 0;
    //_DK_draw_button_string(gbtn, text);
    flgmem = lbDisplay.DrawFlags;
    long cursor_pos = -1;
    if ((gbtn->gbtype == Lb_EDITBTN) && (gbtn == input_button))
    {
        cursor_type++;
        if ((cursor_type & 0x02) == 0)
          cursor_pos = input_field_pos;
        LbLocTextStringConcat(text, " ", TEXT_BUFFER_LENGTH);
        lbDisplay.DrawColour = LbTextGetFontFaceColor();
        lbDisplayEx.ShadowColour = LbTextGetFontBackColor();
    }
    LbTextSetJustifyWindow(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size, gbtn->width/pixel_size);
    LbTextSetClipWindow(gbtn->scr_pos_x/pixel_size, gbtn->scr_pos_y/pixel_size,
        gbtn->width/pixel_size, gbtn->height/pixel_size);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER | Lb_TEXT_UNDERLNSHADOW;
    if (cursor_pos >= 0) {
        // Mind the order, 'cause inserting makes positions shift
        LbLocTextStringInsert(text, "\x0B", cursor_pos+1, TEXT_BUFFER_LENGTH);
        LbLocTextStringInsert(text, "\x0B", cursor_pos, TEXT_BUFFER_LENGTH);
    }
    unsigned long w,h;
    w = 4;
    h = ((gbtn->height - text_string_height(text))/2 - 4);
    LbTextDraw(w/pixel_size, h/pixel_size, text);
    LbTextSetJustifyWindow(0/pixel_size, 0/pixel_size, 640/pixel_size);
    LbTextSetClipWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    lbDisplay.DrawFlags = flgmem;
}

void draw_message_box_at(long startx, long starty, long box_width, long box_height, long spritesx, long spritesy)
{
    struct TbSprite *spr;
    long x,y;
    long n;

    // Draw top line of sprites
    x = startx;
    y = starty;
    {
        spr = &frontend_sprite[25];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 26];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    x = startx;
    {
        spr = &frontend_sprite[25];
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 26];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[30];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
    // Draw centered line of sprites
    spr = &frontend_sprite[25];
    x = startx;
    y += spr->SHeight * units_per_pixel / 16;
    {
        spr = &frontend_sprite[40];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 41];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[45];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
    // Draw bottom line of sprites
    spr = &frontend_sprite[40];
    x = startx;
    y += spr->SHeight * units_per_pixel / 16;
    {
        spr = &frontend_sprite[47];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 48];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[52];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
}

TbBool draw_text_box(const char *text)
{
    long spritesy,spritesx;
    long box_width,box_height;
    long startx,starty;
    long n;
    LbTextSetFont(frontend_font[1]);
    n = LbTextStringWidth(text);
    if (n < (4*108)) {
        spritesy = 1;
        spritesx = n / 108;
    } else {
        spritesx = 4;
        spritesy = n / (3*108);
    }
    if (spritesy > 4) {
      ERRORLOG("Text too long for error box");
    }
    if (spritesx < 2) {
        spritesx = 2;
    } else
    if (spritesx > 4) {
        spritesx = 4;
    }
    box_width = (108 * spritesx + 18) * units_per_pixel / 16;
    box_height = 92 * units_per_pixel / 16;
    startx = (lbDisplay.PhysicalScreenWidth - box_width) / 2;
    starty = (lbDisplay.PhysicalScreenHeight - box_height) / 2;
    draw_message_box_at(startx, starty, box_width, box_height, spritesx, spritesy);
    // Draw the text inside box
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    int tx_units_per_px;
    tx_units_per_px = ((box_height/4)*13/11) * 16 / LbTextLineHeight();
    LbTextSetWindow(startx, starty, box_width, box_height);
    n = LbTextLineHeight() * tx_units_per_px / 16;
    return LbTextDrawResized(0, (box_height - spritesy * n) / 2, tx_units_per_px, text);
}

int scroll_box_get_units_per_px(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    int width;
    int spridx;
    int i;
    width = 0;
    spridx = 40;
    spr = &frontend_sprite[spridx];
    for (i = 6; i > 0; i--)
    {
        width += spr->SWidth;
        spr++;
    }
    return (gbtn->width * 16 + 8) / width;
}

void draw_scroll_box(struct GuiButton *gbtn, int units_per_px, int num_rows)
{
    struct TbSprite *spr;
    int pos_x,pos_y;
    int spridx;
    int delta;
    int i;
    lbDisplay.DrawFlags = 0;
    pos_y = gbtn->scr_pos_y;
    { // First row
        pos_x = gbtn->scr_pos_x;
        spr = &frontend_sprite[25];
        for (i = 6; i > 0; i--)
        {
            LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
            pos_x += spr->SWidth * units_per_px / 16;
            spr++;
        }
        spr = &frontend_sprite[25];
        pos_y += spr->SHeight * units_per_px / 16;
    }
    // Further rows
    while (num_rows > 0)
    {
        spridx = 40;
        if (num_rows < 3)
          spridx = 33;
        spr = &frontend_sprite[spridx];
        pos_x = gbtn->scr_pos_x;
        for (i = 6; i > 0; i--)
        {
            LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
            pos_x += spr->SWidth * units_per_px / 16;
            spr++;
        }
        spr = &frontend_sprite[spridx];
        pos_y += spr->SHeight * units_per_px / 16;
        delta = 3;
        if (num_rows < 3)
            delta = 1;
        num_rows -= delta;
    }
    // Last row
    spr = &frontend_sprite[47];
    pos_x = gbtn->scr_pos_x;
    for (i = 6; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
        spr++;
    }
}

void draw_gui_panel_sprite_left(long x, long y, int units_per_px, long spridx)
{
    struct TbSprite *spr;
    if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
      return;
    spr = &gui_panel_sprites[spridx];
    LbSpriteDrawResized(x/pixel_size, y/pixel_size, units_per_px, spr);
}

void draw_gui_panel_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap)
{
    struct TbSprite *spr;
    if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
      return;
    spr = &gui_panel_sprites[spridx];
    LbSpriteDrawRemap(x/pixel_size, y/pixel_size, spr, &pixmap.fade_tables[remap*256]);
}

void draw_gui_panel_sprite_ocleft(long x, long y, int units_per_px, long spridx, TbPixel color)
{
    struct TbSprite *spr;
    if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
      return;
    spr = &gui_panel_sprites[spridx];
    LbSpriteDrawOneColour(x/pixel_size, y/pixel_size, spr, color);
}

void draw_gui_panel_sprite_centered(long x, long y, int units_per_px, long spridx)
{
    struct TbSprite *spr;
    if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
      return;
    spr = &gui_panel_sprites[spridx];
    x -= ((spr->SWidth*(long)pixel_size) >> 1);
    y -= ((spr->SHeight*(long)pixel_size) >> 1);
    LbSpriteDrawResized(x/pixel_size, y/pixel_size, units_per_px, spr);
}

void draw_gui_panel_sprite_occentered(long x, long y, int units_per_px, long spridx, TbPixel color)
{
    struct TbSprite *spr;
    if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
      return;
    spr = &gui_panel_sprites[spridx];
    x -= ((spr->SWidth*(long)pixel_size) >> 1);
    y -= ((spr->SHeight*(long)pixel_size) >> 1);
    LbSpriteDrawOneColour(x/pixel_size, y/pixel_size, spr, color);
}

void draw_button_sprite_left(long x, long y, int units_per_px, long spridx)
{
    struct TbSprite *spr;
    spr = &button_sprite[spridx];
    if ((spr <= button_sprite) || (spr >= end_button_sprites))
      return;
    LbSpriteDrawResized(x/pixel_size, y/pixel_size, units_per_px, spr);
}

void draw_button_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap)
{
    struct TbSprite *spr;
    spr = &button_sprite[spridx];
    if ((spr <= button_sprite) || (spr >= end_button_sprites))
      return;
    LbSpriteDrawRemap(x/pixel_size, y/pixel_size, spr, &pixmap.fade_tables[remap*256]);
}

void draw_frontend_sprite_left(long x, long y, int units_per_px, long spridx)
{
    struct TbSprite *spr;
    spr = &frontend_sprite[spridx];
    if ((spr <= frontend_sprite) || (spr >= frontend_end_sprite))
        return;
    LbSpriteDrawResized(x, y, units_per_px, spr);
}

void draw_string64k(long x, long y, const char * text)
{
    unsigned short drwflags_mem;
    drwflags_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextDraw(x/pixel_size, y/pixel_size, text);
    lbDisplay.DrawFlags = drwflags_mem;
}

TbBool frontmenu_copy_background_at(const struct TbRect *bkgnd_area, int units_per_px)
{
    int img_width, img_height;
    img_width = 640;
    img_height = 480;
    const unsigned char *srcbuf = frontend_background;
    // Only 8bpp supported for now
    if (LbGraphicsScreenBPP() != 8)
        return false;
    // Do the drawing
    copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
        img_width*units_per_px/16,img_height*units_per_px/16,bkgnd_area->left,bkgnd_area->top,srcbuf,img_width,img_height);
    // Burning candle flames
    return true;
}

long get_frontmenu_background_area_rect(int rect_x, int rect_y, int rect_w, int rect_h, struct TbRect *bkgnd_area)
{
    int img_width, img_height;
    img_width = 640;
    img_height = 480;
    // Parchment bitmap scaling
    int units_per_px, units_per_px_max;
    units_per_px = max(16*rect_w/img_width, 16*rect_h/img_height);
    units_per_px_max = min(16*7*rect_w/(6*img_width), 16*4*rect_h/(3*img_height));
    if (units_per_px > units_per_px_max)
        units_per_px = units_per_px_max;
    // The image width can't be larger than video resolution
    if (units_per_px < 1) {
        units_per_px = 1;
    }
    // Set rectangle coords
    bkgnd_area->left = rect_x + (rect_w-units_per_px*img_width/16)/2;
    bkgnd_area->top = rect_y + (rect_h-units_per_px*img_height/16)/2;
    if (bkgnd_area->top < 0) bkgnd_area->top = 0;
    bkgnd_area->right = bkgnd_area->left + units_per_px*img_width/16;
    bkgnd_area->bottom = bkgnd_area->top + units_per_px*img_height/16;
    if (bkgnd_area->bottom > rect_y+rect_h) bkgnd_area->bottom = rect_y+rect_h;
    return units_per_px;
}

/**
 * Draws menu background.
 */
void draw_frontmenu_background(int rect_x,int rect_y,int rect_w,int rect_h)
{
    // Validate parameters with video mode
    TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(LbScreenActiveMode());
    if (rect_w == POS_AUTO)
      rect_w = mdinfo->Width-rect_x;
    if (rect_h == POS_AUTO)
      rect_h = mdinfo->Height-rect_y;
    if (rect_w<0) rect_w=0;
    if (rect_h<0) rect_h=0;
    // Get background area rectangle
    struct TbRect bkgnd_area;
    int units_per_px;
    units_per_px = get_frontmenu_background_area_rect(rect_x, rect_y, rect_w, rect_h, &bkgnd_area);
    // Draw it
    frontmenu_copy_background_at(&bkgnd_area, units_per_px);
    SYNCDBG(9,"Done");
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
