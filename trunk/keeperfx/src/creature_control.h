/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_control.h
 *     Header file for creature_control.c.
 * @par Purpose:
 *     CreatureControl structure support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 Apr 2009 - 16 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRCTRL_H
#define DK_CRTRCTRL_H

#include "bflib_basics.h"
#include "ariadne.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIZEXY            768
/** Max amount of spells casted at the creature at once. */
#define CREATURE_MAX_SPELLS_CASTED_AT 5
/** Max amount of creatures supported on any map. */
#define CREATURES_COUNT       256
/** Percentage of creature pay increase for every experience level. */
#define CREATURE_PAY_INCREASE_ON_EXP       35
/** Percentage of creature damage increase for every experience level. */
#define CREATURE_DAMAGE_INCREASE_ON_EXP    20
/** Percentage of spell range/area of effect increase for every experience level. */
#define CREATURE_RANGE_INCREASE_ON_EXP     6
/** Percentage of creature health increase for every experience level. */
#define CREATURE_HEALTH_INCREASE_ON_EXP    35
/** Percentage of creature strength increase for every experience level. */
#define CREATURE_STRENGTH_INCREASE_ON_EXP  35
/** Percentage of creature dexterity increase for every experience level. */
#define CREATURE_DEXTERITY_INCREASE_ON_EXP 15
/** Percentage of creature defense increase for every experience level. */
#define CREATURE_DEFENSE_INCREASE_ON_EXP   20
/** Percentage of creature parameter increase for every experience level.
 *  Used for all parameters that have no separate definition. */
#define CREATURE_PROPERTY_INCREASE_ON_EXP  35

#define INVALID_CRTR_CONTROL (game.persons.cctrl_lookup[0])
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;
struct PlayerInfo;

enum CreatureSoundTypes {
    CrSnd_SlappedOuch  = 1,
    CrSnd_PrisonMoan   = 4,
    CrSnd_HandPick     = 5,
};

struct CastedSpellData {
    unsigned char spkind;
    short field_1;
    unsigned char field_3;
};

#define SIZEOF_CreatureControl 776

struct CreatureControl {
    unsigned char index;
    unsigned char flgfield_1;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    long field_5;
    short field_9;
char field_B[18];
    unsigned short field_1D;
    unsigned short thing_idx;
unsigned short field_21;
    unsigned char explevel;
    long field_24;
unsigned char field_28[5];
    struct Coord3d moveto_pos;
unsigned char field_33[6];
    long field_39;
unsigned char field_3D[13];
    long field_4A;
unsigned char field_4E[24];
    unsigned char field_66;
unsigned char field_67;
unsigned short field_68;
    unsigned short lairtng_idx;
    short field_6C;
    short field_6E;
    unsigned short field_70;
    unsigned short field_72;
    unsigned short field_74;
    unsigned short next_in_group;
    unsigned short prev_in_group;
    unsigned short field_7A;
    short last_work_room_id;
    short work_room_id;
    short field_80;
    long field_82;
unsigned char field_86[2];
unsigned char field_88;
  union {
  struct {
    char sbyte_89;
    unsigned char byte_8A;
    unsigned char byte_8B;
    unsigned char byte_8C;
    };
    long long_89;
  };
union {
struct {
  short word_8D;
  unsigned short word_8F;
  };
  long long_8D;
};
union {
struct {
  unsigned char byte_91;
  unsigned char byte_92;
  unsigned char byte_93;
  unsigned char byte_94;
  };
struct {
  short word_91;
  short word_93;
  };
  long long_91;
};
unsigned char field_95;
unsigned char field_96[4];
  union {
  struct {
    unsigned char byte_9A;
    unsigned char byte_9B;
    unsigned char byte_9C;
    unsigned char byte_9D;
    unsigned char byte_9E;
    unsigned char byte_9F;
    unsigned char byte_A0;
    unsigned char byte_A1;
    unsigned char byte_A2;
    unsigned char byte_A3;
    unsigned char byte_A4;
    };
  struct {
    short word_9A;
    short word_9C;
    short word_9E;
    short word_A0;
    short word_A2;
    unsigned char byte_A4_cp2;
    };
  struct {
    short word_9A_cp2;
    long long_9C;
    long long_A0;
    unsigned char byte_A4_cp3;
    };
  struct {
    long long_9A;
    long long_9E;
    short word_A2_cp2;
    unsigned char byte_A4_cp4;
    };
  struct {
    unsigned char byte_9A_cp2;
    long long_9B;
    short word_9F_cp2;
    long long_A1;
    };
  struct {
    unsigned char byte_9A_cp3;
    short word_9B;
    short word_9D;
    short word_9F;
    short word_A1;
    short word_A3;
    };
  };
    unsigned char field_A5;
    unsigned char field_A6;
    unsigned char field_A7[2];
    unsigned char field_A9;
    unsigned char field_AA;
    unsigned char field_AB;
    unsigned char spell_flags;
    unsigned char field_AD;
unsigned char field_AE;
    short field_AF;
    unsigned char field_B1;
unsigned char field_B2[4];
    unsigned char field_B6;
    unsigned char teleport_x;
    unsigned char teleport_y;
unsigned short field_B9;
    struct CoordDelta3d pos_BB;
    unsigned char bloody_footsteps_turns;
    short field_C2;
    short max_speed;
    short max_health;
    short field_C8;
    short field_CA;
    short field_CC;
    unsigned long field_CE;
unsigned char field_D2;
unsigned char field_D3;
    unsigned short field_D4;
    unsigned short field_D6;
    unsigned short field_D8;
    unsigned short field_DA;
    unsigned char target_x;
    unsigned char target_y;
long field_DE[48];
    char instances[48];
    unsigned short field_1CE;
  unsigned short field_1D0;
    char field_1D2;
unsigned char field_1D3;
  struct CastedSpellData casted_spells[CREATURE_MAX_SPELLS_CASTED_AT];
    char field_1E8;
unsigned char field_1E9[46];
    struct Ariadne arid;
unsigned char field_27D;
unsigned char field_27E;
unsigned char field_27F;
    short field_280;
    short field_282;
unsigned char field_284[2];
    unsigned char field_286;
    unsigned char field_287;
    struct Coord3d pos_288;
    long field_28E;
unsigned char field_292[12];
unsigned char field_29E[12];
    unsigned short next_in_room;
    unsigned short prev_in_room;//field_2AC
short field_2AE;
    unsigned char field_2B0;
unsigned char field_2B1[2];
unsigned short field_2B3[3];
unsigned short field_2B9[3];

unsigned char field_2BF[2];
unsigned short field_2C1;
unsigned short field_2C3;
unsigned short field_2C5;
    long field_2C7;
unsigned char field_2CB[32];
    long field_2EB;
    unsigned long field_2EF;
unsigned char field_2F3[11];
    unsigned long field_2FE;
    unsigned char field_302;
unsigned char field_303[3];
unsigned char field_306[2];
};

struct CreatureStats { // sizeof = 230
  unsigned short job_primary;
  unsigned short job_secondary;
  unsigned short jobs_not_do;
  unsigned char eye_effect;
  unsigned short health;
  unsigned char heal_requirement;
  unsigned char heal_threshold;
  unsigned char strength;
  unsigned char armour;
  unsigned char dexterity;
  unsigned char fear;
  unsigned char defence;
  unsigned char luck;
  unsigned char recovery;
  unsigned short hunger_rate;
  unsigned char hunger_fill;
  unsigned short annoy_level;
  unsigned char lair_size;
  unsigned char hurt_by_lava;
  unsigned char sleep_exp_slab;
  unsigned char sleep_experience;
short exp_for_hitting;
short gold_hold;
short training_cost;
short scavenger_cost;
short scavenge_require;
  unsigned char scavenge_value;
  unsigned long to_level[10];
  unsigned char base_speed;
short grow_up;
  unsigned char grow_up_level;
  unsigned char entrance_force;
short max_angle_change;
short eye_height;
short field_57[14];
short field_73;
  unsigned short size_xy;
  unsigned short size_yz;
  unsigned short walking_anim_speed;
  unsigned char flying;
  unsigned char immune_to_gas;
  unsigned char attack_preference;
short field_of_view;
  unsigned char instance_spell[10];
  unsigned char instance_level[10];
  unsigned char research_value;
  unsigned char humanoid_creature;
  unsigned char piss_on_dead;
  unsigned char training_value;
short pay;
  unsigned char manufacture_value;
  unsigned char hearing;
  unsigned char entrance_rooms[3];
  unsigned char entrance_slabs_req[3];
  unsigned char visual_range;
  unsigned char partner_training;
  short torture_time;
  short annoy_no_lair;
  short annoy_no_hatchery;
  short annoy_woken_up;
  short annoy_on_dead_friend;
  short annoy_sulking;
  short annoy_no_salary;
  short annoy_slapped;
  short annoy_on_dead_enemy;
  short annoy_in_temple;
  short annoy_sleeping;
  short annoy_got_wage;
  short annoy_in_torture;
  short annoy_win_battle;
  short annoy_untrained_time;
  short annoy_untrained;
  short field_C4;
  short annoy_queue;
  short annoy_will_not_do_job;
  unsigned short job_stress;
  short annoy_job_stress;
  unsigned short jobs_anger;
  short annoy_others_leaving;
  unsigned char slaps_to_kill;
short lair_enemy;
short hero_vs_keeper_cost;
    unsigned char rebirth;
    unsigned char can_see_invisible;
    unsigned char can_go_locked_doors;
    unsigned char bleeds;
    unsigned char affected_by_wind;
    unsigned short thing_size_xy;
    unsigned short thing_size_yz;
short annoy_eat_food;
short annoy_in_hand;
short damage_to_boulder;
};

struct Persons {
    struct CreatureControl *cctrl_lookup[CREATURES_COUNT];
    struct CreatureControl *cctrl_end;
};

struct CreatureSound {
    long index;
    long count;
};

struct CreatureSounds {
    struct CreatureSound snd01;
    struct CreatureSound snd02;
    struct CreatureSound snd03;
    struct CreatureSound snd04;
    struct CreatureSound snd05;
    struct CreatureSound snd06;
    struct CreatureSound snd07;
    struct CreatureSound snd08;
    struct CreatureSound snd09;
    struct CreatureSound snd10;
    struct CreatureSound snd11;
};
#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern struct CreatureSounds creature_sounds[];
/******************************************************************************/
struct CreatureControl *creature_control_get(long cctrl_idx);
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing);
TbBool creature_control_invalid(const struct CreatureControl *cctrl);
void clear_creature_instance(struct Thing *thing);
long i_can_allocate_free_control_structure(void);
struct CreatureControl *allocate_free_control_structure(void);
void delete_control_structure(struct CreatureControl *cctrl);
void delete_all_control_structures(void);

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, long a2, struct Coord3d *pos);
TbBool disband_creatures_group(struct Thing *thing);
void play_creature_sound(struct Thing *thing, long snd_idx, long a3, long a4);
void reset_creature_eye_lens(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
