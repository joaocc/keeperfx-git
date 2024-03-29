/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_objects.c
 *     Object things configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for object things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 16 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_objects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_creature.h"
#include "thing_objects.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_objects_file[]="objects.cfg";

const struct NamedCommand objects_common_commands[] = {
  {"OBJECTSCOUNT",    1},
  {NULL,              0},
  };

const struct NamedCommand objects_object_commands[] = {
  {"NAME",            1},
  {"GENRE",           2},
  {"RELATEDCREATURE", 3},
  {NULL,              0},
  };

const struct NamedCommand objects_genres_desc[] = {
  {"NONE",            OCtg_Unknown},
  {"DECORATION",      OCtg_Decoration},
  {"FURNITURE",       OCtg_Furniture},
  {"VALUABLE",        OCtg_Valuable},
  {"SPELLBOOK",       OCtg_Spellbook},
  {"WORKSHOPBOX",     OCtg_WrkshpBox},
  {"FOOD",            OCtg_Food},
  {"POWER",           OCtg_Power},
  {"LAIR",            OCtg_Lair},
  {"EFFECT",          OCtg_Effect},
  {NULL,              0},
  };

/******************************************************************************/
struct ObjectsConfig object_conf;
struct NamedCommand object_desc[OBJECT_TYPES_COUNT];
/******************************************************************************/
struct ObjectConfigStats *get_object_model_stats(ThingModel tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= object_conf.object_types_count))
        return &object_conf.object_cfgstats[0];
    return &object_conf.object_cfgstats[tngmodel];
}

struct ObjectConfig *get_object_model_stats2(ThingModel tngmodel)
{
    if ((tngmodel < 0) || (tngmodel >= object_conf.object_types_count))
        return &game.objects_config[0];
    return &game.objects_config[tngmodel];
}

ThingClass crate_to_workshop_item_class(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= OBJECT_TYPES_COUNT))
        return object_conf.workshop_object_class[0];
    return object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_to_workshop_item_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= OBJECT_TYPES_COUNT))
        return object_conf.object_to_door_or_trap[0];
    return object_conf.object_to_door_or_trap[tngmodel];
}

ThingClass crate_thing_to_workshop_item_class(const struct Thing *thing)
{
    if (thing_is_invalid(thing) || (thing->class_id != TCls_Object))
        return object_conf.workshop_object_class[0];
    ThingModel tngmodel;
    tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= OBJECT_TYPES_COUNT))
        return object_conf.workshop_object_class[0];
    return object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_thing_to_workshop_item_model(const struct Thing *thing)
{
    if (thing_is_invalid(thing) || (thing->class_id != TCls_Object))
        return object_conf.object_to_door_or_trap[0];
    ThingModel tngmodel;
    tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= OBJECT_TYPES_COUNT))
        return object_conf.object_to_door_or_trap[0];
    return object_conf.object_to_door_or_trap[tngmodel];
}

TbBool parse_objects_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        object_conf.object_types_count = 1;
    }
    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,objects_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // OBJECTSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= OBJECT_TYPES_COUNT))
              {
                  object_conf.object_types_count = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
            break;
        default:
            CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
                cmd_num,block_buf,config_textname);
            break;
        }
        skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
    return true;
}

TbBool parse_objects_object_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct ObjectConfigStats *objst;
    struct Objects *objdat;
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize the objects array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(object_conf.object_cfgstats)/sizeof(object_conf.object_cfgstats[0]);
        for (i=0; i < arr_size; i++)
        {
            objst = &object_conf.object_cfgstats[i];
            LbMemorySet(objst->code_name, 0, COMMAND_WORD_LEN);
            objst->name_stridx = 201;
            objst->genre = 0;
            if (i < object_conf.object_types_count)
            {
                object_desc[i].name = objst->code_name;
                object_desc[i].num = i;
            } else
            {
                object_desc[i].name = NULL;
                object_desc[i].num = 0;
            }
        }
    }
    // Load the file
    arr_size = object_conf.object_types_count;
    for (i=0; i < arr_size; i++)
    {
        sprintf(block_buf,"object%d",i);
        pos = 0;
        k = find_conf_block(buf,&pos,len,block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        objst = &object_conf.object_cfgstats[i];
        objdat = get_objects_data(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_object_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            cmd_num = recognize_conf_command(buf,&pos,len,objects_object_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,objst->code_name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                break;
            case 2: // GENRE
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(objects_genres_desc, word_buf);
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect object genre \"%s\" in [%s] block of %s file.",
                        word_buf,block_buf,config_textname);
                    break;
                }
                objst->genre = n;
                break;
            case 3: // RELATEDCREATURE
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(creature_desc, word_buf);
                }
                if (n < 0)
                {
                    CONFWRNLOG("Incorrect related creature \"%s\" in [%s] block of %s file.",
                        word_buf,block_buf,config_textname);
                    break;
                }
                objdat->related_creatr_model = n;
                break;
            case 0: // comment
                break;
            case -1: // end of buffer
                break;
            default:
                CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
                    cmd_num,block_buf,config_textname);
                break;
            }
            skip_conf_to_next_line(buf,&pos,len);
        }
#undef COMMAND_TEXT
    }
    return true;
}

TbBool load_objects_config_file(const char *textname, const char *fname, unsigned short flags)
{
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    if (len > MAX_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" is too large.",textname,fname);
        return false;
    }
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_objects_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_objects_object_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" object blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_objects_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global objects config";
    static const char config_campgn_textname[] = "campaign objects config";
    char *fname;
    TbBool result;
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    result = load_objects_config_file(config_global_textname,fname,flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_objects_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given object model.
 */
const char *object_code_name(ThingModel tngmodel)
{
    const char *name;
    name = get_conf_parameter_text(object_desc,tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the object model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the object model if found, otherwise -1
 */
ThingModel object_model_id(const char * code_name)
{
    int i;

    for (i = 0; i < object_conf.object_types_count; ++i) {
        if (strncasecmp(object_conf.object_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

void init_objects(void)
{
    long i;
    game.objects_config[1].ilght.field_0 = 0;
    game.objects_config[1].ilght.field_2 = 0x00;
    game.objects_config[1].ilght.field_3 = 0;
    game.objects_config[1].health = 100;
    game.objects_config[1].field_4 = 20;
    game.objects_config[1].field_5 = 0;
    game.objects_config[2].health = 100;
    game.objects_config[2].field_4 = 0;
    game.objects_config[2].field_5 = 1;
    game.objects_config[2].ilght.is_dynamic = 0;
    game.objects_config[2].field_8 = 1;
    game.objects_config[49].health = 100;
    game.objects_config[49].field_4 = 0;
    game.objects_config[49].field_5 = 1;
    game.objects_config[49].ilght.is_dynamic = 0;
    game.objects_config[49].field_8 = 1;
    game.objects_config[3].health = 100;
    game.objects_config[3].field_4 = 20;
    game.objects_config[4].health = 100;
    game.objects_config[4].field_4 = 20;
    game.objects_config[4].field_5 = 1;
    game.objects_config[4].ilght.is_dynamic = 0;
    game.objects_config[4].field_8 = 1;
    game.objects_config[5].health = 1;
    game.objects_config[2].ilght.field_0 = 0x0600;
    game.objects_config[2].ilght.field_2 = 0x32;
    game.objects_config[2].ilght.field_3 = 5;
    game.objects_config[5].field_4 = 20;
    game.objects_config[5].field_5 = 0;
    game.objects_config[5].ilght.is_dynamic = 1;
    game.objects_config[5].is_heart = 1;
    game.objects_config[5].field_8 = 1;
    game.objects_config[6].field_4 = 8;
    game.objects_config[6].health = 50;
    game.objects_config[7].health = 100;
    game.objects_config[7].field_4 = 0;
    game.objects_config[7].field_5 = 1;
    game.objects_config[7].field_8 = 1;
    game.objects_config[8].health = 100;
    game.objects_config[8].field_4 = 20;
    game.objects_config[8].field_5 = 1;
    game.objects_config[10].health = 1000;
    game.objects_config[10].field_4 = 9;
    game.objects_config[28].health = 100;
    game.objects_config[49].ilght.field_0 = 0x0A00u;
    game.objects_config[49].ilght.field_2 = 0x28;
    game.objects_config[49].ilght.field_3 = 5;
    game.objects_config[4].ilght.field_0 = 0x0700u;
    game.objects_config[4].ilght.field_2 = 0x2F;
    game.objects_config[4].ilght.field_3 = 5;
    game.objects_config[5].ilght.field_0 = 0x0E00u;
    game.objects_config[5].ilght.field_2 = 0x24;
    game.objects_config[5].ilght.field_3 = 5;
    game.objects_config[28].field_4 = 0;
    game.objects_config[28].field_5 = 1;
    game.objects_config[28].ilght.is_dynamic = 0;
    game.objects_config[28].field_8 = 1;
    game.objects_config[11].ilght.field_0 = 0x0400u;
    game.objects_config[11].ilght.field_2 = 0x3E;
    game.objects_config[11].ilght.field_3 = 0;
    game.objects_config[11].field_4 = 10;
    game.objects_config[11].field_5 = 0;
    game.objects_config[11].ilght.is_dynamic = 0;
    game.objects_config[11].field_8 = 1;
    game.objects_config[12].ilght.field_0 = 0x0400u;
    game.objects_config[12].ilght.field_2 = 0x3E;
    game.objects_config[12].ilght.field_3 = 0;
    game.objects_config[12].field_4 = 10;
    game.objects_config[12].field_5 = 0;
    game.objects_config[12].ilght.is_dynamic = 0;
    game.objects_config[12].field_8 = 1;
    game.objects_config[13].field_4 = 10;
    game.objects_config[13].field_5 = 0;
    game.objects_config[13].ilght.field_0 = 0x0400u;
    game.objects_config[13].ilght.field_2 = 0x3E;
    game.objects_config[13].ilght.field_3 = 0;
    game.objects_config[13].ilght.is_dynamic = 0;
    game.objects_config[13].field_8 = 1;
    game.objects_config[14].ilght.field_0 = 0x0400u;
    game.objects_config[14].ilght.field_2 = 0x3E;
    game.objects_config[14].ilght.field_3 = 0;
    game.objects_config[14].field_4 = 10;
    game.objects_config[14].field_5 = 0;
    game.objects_config[14].ilght.is_dynamic = 0;
    game.objects_config[14].field_8 = 1;
    game.objects_config[15].field_4 = 10;
    game.objects_config[15].field_5 = 0;
    game.objects_config[15].ilght.field_0 = 0x0400u;
    game.objects_config[15].ilght.field_2 = 0x3E;
    game.objects_config[15].ilght.field_3 = 0;
    game.objects_config[15].ilght.is_dynamic = 0;
    game.objects_config[15].field_8 = 1;
    game.objects_config[16].ilght.field_0 = 0x0400u;
    game.objects_config[16].ilght.field_2 = 0x3E;
    game.objects_config[16].ilght.field_3 = 0;
    game.objects_config[16].field_4 = 10;
    game.objects_config[16].field_5 = 0;
    game.objects_config[16].ilght.is_dynamic = 0;
    game.objects_config[16].field_8 = 1;
    game.objects_config[17].field_4 = 10;
    game.objects_config[17].field_5 = 0;
    game.objects_config[17].ilght.field_0 = 0x0400u;
    game.objects_config[17].ilght.field_2 = 0x3E;
    game.objects_config[17].ilght.field_3 = 0;
    game.objects_config[17].ilght.is_dynamic = 0;
    game.objects_config[17].field_8 = 1;
    game.objects_config[43].field_4 = 8;
    game.objects_config[43].health = 50;
    game.objects_config[28].ilght.field_0 = 0x0600u;
    game.objects_config[28].ilght.field_2 = 0x2E;
    game.objects_config[28].ilght.field_3 = 5;
    game.objects_config[18].field_4 = 10;
    game.objects_config[18].field_5 = 0;
    game.objects_config[18].ilght.field_0 = 0x0400u;
    game.objects_config[18].ilght.field_2 = 0x3E;
    game.objects_config[18].ilght.field_3 = 0;
    game.objects_config[18].ilght.is_dynamic = 0;
    game.objects_config[19].ilght.field_0 = 0x0400u;
    game.objects_config[19].ilght.field_2 = 0x3E;
    game.objects_config[19].ilght.field_3 = 0;
    game.objects_config[18].field_8 = 1;
    game.objects_config[19].field_4 = 10;
    game.objects_config[19].field_5 = 0;
    game.objects_config[20].ilght.field_0 = 0x0400u;
    game.objects_config[20].ilght.field_2 = 0x3E;
    game.objects_config[20].ilght.field_3 = 0;
    game.objects_config[19].ilght.is_dynamic = 0;
    game.objects_config[19].field_8 = 1;
    game.objects_config[20].field_4 = 10;
    game.objects_config[20].field_5 = 0;
    game.objects_config[20].ilght.is_dynamic = 0;
    game.objects_config[21].ilght.field_0 = 0x0400u;
    game.objects_config[21].ilght.field_2 = 0x3E;
    game.objects_config[21].ilght.field_3 = 0;
    game.objects_config[20].field_8 = 1;
    game.objects_config[21].field_4 = 10;
    game.objects_config[21].field_5 = 0;
    game.objects_config[22].ilght.field_0 = 0x0400u;
    game.objects_config[22].ilght.field_2 = 0x3E;
    game.objects_config[22].ilght.field_3 = 0;
    game.objects_config[21].ilght.is_dynamic = 0;
    game.objects_config[21].field_8 = 1;
    game.objects_config[22].field_4 = 10;
    game.objects_config[22].field_5 = 0;
    game.objects_config[22].ilght.is_dynamic = 0;
    game.objects_config[23].ilght.field_0 = 0x0400u;
    game.objects_config[23].ilght.field_2 = 0x3E;
    game.objects_config[23].ilght.field_3 = 0;
    game.objects_config[22].field_8 = 1;
    game.objects_config[23].field_4 = 10;
    game.objects_config[23].field_5 = 0;
    game.objects_config[45].ilght.field_0 = 0x0400u;
    game.objects_config[45].ilght.field_2 = 0x3E;
    game.objects_config[45].ilght.field_3 = 0;
    game.objects_config[23].ilght.is_dynamic = 0;
    game.objects_config[23].field_8 = 1;
    game.objects_config[45].field_4 = 10;
    game.objects_config[45].field_5 = 0;
    game.objects_config[45].ilght.is_dynamic = 0;
    game.objects_config[46].ilght.field_0 = 0x0400u;
    game.objects_config[46].ilght.field_2 = 0x3E;
    game.objects_config[46].ilght.field_3 = 0;
    game.objects_config[45].field_8 = 1;
    game.objects_config[46].field_4 = 10;
    game.objects_config[46].field_5 = 0;
    game.objects_config[47].ilght.field_0 = 0x0400u;
    game.objects_config[47].ilght.field_2 = 0x3E;
    game.objects_config[47].ilght.field_3 = 0;
    game.objects_config[46].ilght.is_dynamic = 0;
    game.objects_config[46].field_8 = 1;
    game.objects_config[47].field_4 = 10;
    game.objects_config[47].field_5 = 0;
    game.objects_config[47].ilght.is_dynamic = 0;
    game.objects_config[134].ilght.field_0 = 0x0400u;
    game.objects_config[134].ilght.field_2 = 0x3E;
    game.objects_config[134].ilght.field_3 = 0;
    game.objects_config[47].field_8 = 1;
    game.objects_config[134].field_4 = 10;
    game.objects_config[134].field_5 = 0;
    game.objects_config[134].ilght.is_dynamic = 0;
    game.objects_config[87].ilght.field_0 = 0x0400u;
    game.objects_config[87].ilght.field_2 = 0x3E;
    game.objects_config[87].ilght.field_3 = 0;
    game.objects_config[134].field_8 = 1;
    game.objects_config[87].field_4 = 10;
    game.objects_config[87].field_5 = 0;
    game.objects_config[88].ilght.field_0 = 0x0400u;
    game.objects_config[88].ilght.field_2 = 0x3E;
    game.objects_config[88].ilght.field_3 = 0;
    game.objects_config[87].ilght.is_dynamic = 0;
    game.objects_config[88].field_4 = 10;
    game.objects_config[88].field_5 = 0;
    game.objects_config[89].ilght.field_0 = 0x0400u;
    game.objects_config[89].ilght.field_2 = 0x3E;
    game.objects_config[89].ilght.field_3 = 0;
    game.objects_config[88].ilght.is_dynamic = 0;
    game.objects_config[89].field_4 = 10;
    game.objects_config[89].field_5 = 0;
    game.objects_config[90].ilght.field_0 = 0x0400u;
    game.objects_config[90].ilght.field_2 = 0x3E;
    game.objects_config[90].ilght.field_3 = 0;
    game.objects_config[89].ilght.is_dynamic = 0;
    game.objects_config[90].field_4 = 10;
    game.objects_config[90].field_5 = 0;
    game.objects_config[91].ilght.field_0 = 0x0400u;
    game.objects_config[91].ilght.field_2 = 0x3E;
    game.objects_config[91].ilght.field_3 = 0;
    game.objects_config[90].ilght.is_dynamic = 0;
    game.objects_config[91].field_4 = 10;
    game.objects_config[91].field_5 = 0;
    game.objects_config[92].ilght.field_0 = 0x0400u;
    game.objects_config[92].ilght.field_2 = 0x3E;
    game.objects_config[92].ilght.field_3 = 0;
    game.objects_config[91].ilght.is_dynamic = 0;
    game.objects_config[92].field_4 = 10;
    game.objects_config[92].field_5 = 0;
    game.objects_config[93].ilght.field_0 = 0x0400u;
    game.objects_config[93].ilght.field_2 = 0x3E;
    game.objects_config[93].ilght.field_3 = 0;
    game.objects_config[92].ilght.is_dynamic = 0;
    game.objects_config[93].field_4 = 10;
    game.objects_config[93].field_5 = 0;
    game.objects_config[86].ilght.field_0 = 0x0400u;
    game.objects_config[86].ilght.field_2 = 0x3E;
    game.objects_config[86].ilght.field_3 = 0;
    game.objects_config[93].ilght.is_dynamic = 0;
    game.objects_config[86].field_4 = 10;
    game.objects_config[86].field_5 = 0;
    game.objects_config[86].ilght.is_dynamic = 0;
    game.objects_config[109].resistant_to_nonmagic = 1;
    game.objects_config[109].field_8 = 1;
    game.objects_config[94].field_8 = 1;
    game.objects_config[95].field_8 = 1;
    game.objects_config[96].field_8 = 1;
    game.objects_config[97].field_8 = 1;
    game.objects_config[98].field_8 = 1;
    game.objects_config[99].field_8 = 1;
    game.objects_config[106].field_8 = 1;
    game.objects_config[107].field_8 = 1;
    game.objects_config[108].field_8 = 1;
    game.objects_config[128].field_4 = 10;
    for (i=57; i <= 85; i++)
    {
      game.objects_config[i].field_8 = 1;
    }
    game.objects_config[126].field_8 = 1;
    game.objects_config[26].field_8 = 1;
    game.objects_config[27].field_8 = 1;
    game.objects_config[31].field_8 = 1;
    game.objects_config[32].field_8 = 1;
    game.objects_config[114].field_8 = 1;
    game.objects_config[115].field_8 = 1;
    game.objects_config[116].field_8 = 1;
    game.objects_config[117].field_8 = 1;
    game.objects_config[118].field_8 = 1;
    game.objects_config[119].field_8 = 1;
    game.objects_config[125].field_8 = 1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
