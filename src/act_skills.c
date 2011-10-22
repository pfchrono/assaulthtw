/*~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
 ~  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        ~
 ~  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   ~
 ~                                                                         ~
 ~  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          ~
 ~  Chastain, Michael Quan, and Mitchell Tse.                              ~
 ~                                                                         ~
 ~  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              ~
 ~  ACK!MUD is modified Merc2.0/2.1/2.2 code (c)Stephen Zepp 1998 Ver: 4.3 ~
 ~                                                                         ~
 ~  In order to use any part of this Merc Diku Mud, you must comply with   ~
 ~  both the original Diku license in 'license.doc' as well the Merc       ~
 ~  license in 'license.txt', and the Ack!Mud license in 'ack_license.txt'.~
 ~  In particular, you may not remove any of these copyright notices.      ~
 ~                                                                         ~
 ~           _______      _____                                            ~
 ~          /  __  /\    / ___ \       222222        PA_MUD by Amnon Kruvi ~
 ~         /______/ /   / /___\ \            2       PA_MUD is modified    ~
 ~        / _______/   / _______ \           2       Ack!Mud, v4.3         ~
 ~       /_/          /_/       \_\        2                               ~
 ~                                      2                                  ~
 ~                                     2222222                             ~
 ~                                                                         ~
 ~                                                                         ~
 ~   Years of work have been invested to create DIKU, Merc, Ack and PA.    ~
 ~   Please show your respect by following the licenses, and issuing       ~
 ~   credits where due.                                                    ~
 ~                                                                         ~
 ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ack.h"
#include "config.h"

void check_skill( CHAR_DATA *ch, int gsn, int chance, char *name );

const struct skill_type         skill_table[MAX_SKILL+1]   =
{
	{ "Armor Repair", 	gsn_repair, 	FALSE, "Fixing Damaged Armor." },
	{ "Sneak", 		gsn_sneak, 	FALSE, "Invisibility on the map/from turrets." },
	{ "Target", 		gsn_target, 	FALSE, "Locking on to an enemy before firing." },
	{ "Computers", 		gsn_computer, 	FALSE, "Hacking enemy bases." },
	{ "Explosive Research", gsn_exresearch, FALSE, "Upgrading bombs." },
	{ "Dodge", 		gsn_dodge, 	FALSE, "Dodging Player vs. Player attacks." },
	{ "Economics", 		gsn_economics, 	TRUE,  "Mastering the tricks of the trade." },
	{ "Building", 		gsn_building, 	TRUE,  "Building structures." },
	{ "Combat", 		gsn_combat, 	TRUE,  "Armor usage, armor piercing, head shots." },
	{ "Bombs",		gsn_arm, 	FALSE,  "Quick handling of explosives." },
	{ "Blind Combat",	gsn_blind_combat, FALSE,  "Seeing while blind." },
	{ "Grenades", 		gsn_grenades, 	FALSE, "Accurate Grenade throwing." },
	{ "Conquer", 		gsn_conquer, 	FALSE, "Taking control over enemy buildings." },
	{ "Concentration",	gsn_concentration, FALSE, "Reduces chances of being distracted." },
	{ "None", -1, FALSE, "" },
};
const struct ability_type ability_table[] =
{
  // Name               	Eco     Bld     Cmb     Desc
  { "Fix",              	0,      3,      0,      "Repair a building's HP" }, 
  { "Nuclear Containment",	0,	5,	2,	"Reduced nuclear fallout time in your own buildings" },
  { "Peacecraft",		5,	5,	0,	"10% Reduction to damage done to you and by you" },
  { "Fast Healing",		0,	0,	4,	"Recover from bad effects faster" },
  { "Overclock",		3,	0,	0,	"Your laptop batteries last longer" },
  { "Salvage",			2,	0,	3,	"Gain a 10% cash refund when you lose a building" },
  { "Money Safe",		4,	0,	0,	"Regain 50% of your money if the bank its in is destroyed" },
  { "Databank",			2,	3,	0,	"Reduces cost of upgrading" },
  { NULL,               	-1,     -1,     -1,     NULL },
};
 
char * const wear_name[MAX_WEAR] =
{
	"Head",
	"Eyes",
	"Face",
	"Shoulders",
	"Arms",
	"Left Hand",
	"Right Hand",
	"Waist",
	"Body",
	"Legs",
	"Feet",
};
void check_skill( CHAR_DATA *ch, int gsn, int chance, char *name )
{
	char buf[MSL];
	return;
	if ( gsn >= MAX_SKILL )
		return;
	if ( number_percent() < chance && ch->pcdata->skill[gsn] < 100 && ch->pcdata->lastskill <= 0 )
	{
		sprintf( buf, "You have become better at %s!\n\r", name );
		send_to_char(buf,ch);
		ch->pcdata->skill[gsn]++;
		ch->pcdata->lastskill = 75;
	}
	return;
}

void  do_repair( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	BUILDING_DATA *bld;
	char buf[MSL];

	if ( argument[0] == '\0' )
	{
		sprintf( buf, "Your armor repair skill is at %d%%.\n\r", ch->pcdata->skill[gsn_repair]);
		send_to_char( buf, ch );
		return;
	}
	if ( ( bld = get_char_building(ch) ) == NULL )
	{
		send_to_char( "You must be in an armory.\n\r", ch );
		return;
	}
	if ( !bld->owner || bld->owner != ch || !complete(bld) || bld->type != BUILDING_ARMORER )
	{
		send_to_char( "You must be in one of your completed armories.\n\r", ch );
		return;
	}
	if ( ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "You must specify a piece of armor to repair.\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_ARMOR )
	{
		send_to_char( "It is not armor.\n\r", ch );
		return;
	}
	if ( obj->value[1] == 0 )
	{
		send_to_char( "There is no need to repair it.\n\r", ch );
		return;
	}
	if ( ( get_eq_char(ch,WEAR_HOLD_HAND_L) ) != NULL )
		unequip_char(ch,get_eq_char(ch,WEAR_HOLD_HAND_L));
	equip_char(ch,obj,WEAR_HOLD_HAND_L);
	act( "You begin repairing $p. Type ""@@eStop@@N"" at any time to stop.", ch, obj, NULL, TO_CHAR );
	act( "$n begins repairing $p.", ch, obj, NULL, TO_ROOM );
	ch->c_time = 30;
	ch->c_sn = gsn_repair;
	ch->c_level = ch->pcdata->skill[gsn_repair];
	return;
}
void act_repair( CHAR_DATA *ch, int level )
{
	char buf[MSL];
	OBJ_DATA *obj;
	int r;

	if ( ( obj = get_eq_char(ch,WEAR_HOLD_HAND_L) ) == NULL || obj->item_type != ITEM_ARMOR )
	{
		send_to_char( "For some reason, you are no longer holding the armor you were repairing!\n\r", ch );
		ch->c_sn = -1;
		return;
	}
	if ( ch->in_building == NULL || ch->in_building->type != BUILDING_ARMORER )
	{
		send_to_char( "For some reason, you are no longer in an armory.\n\r", ch );
		ch->c_sn = -1;
		return;
	}

	if ( obj->value[1] <= 0 )
	{
		obj->value[1] = 0;
		send_to_char( "You finish repairing.\n\r", ch );
		act( "$n finishes repairing.\n\r", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = -1;
		return;
	}
	check_skill(ch,gsn_repair,5,"Armor Repair");
	r = number_range(0,level)+1;
	if ( number_percent() < (100-level)/5 )
	{
		sprintf(buf,"You mess up @@e(@@R%d@@e)@@N!\n\r", r );
		send_to_char(buf,ch);
		act( "$n glitches a little.", ch, NULL, NULL, TO_ROOM );
		if ( number_percent() < 2 )
		{
			extract_obj(obj);
			ch->c_sn = -1;
			send_to_char( "You accidently ruin what you were working on.\n\r", ch );
			return;
		}
		else
			obj->value[1]+=r;
	}
	else
	{
		act( "$n continues repairing.", ch, NULL, NULL, TO_ROOM );
		sprintf(buf,"You continue repairing %s @@a(@@c%d@@a)@@N.\n\r", obj->short_descr, r );
		send_to_char(buf,ch);
		obj->value[1]-=r;
	}
	ch->c_time = 30;
	return;
}
void do_sneak( CHAR_DATA *ch, char *argument )
{
	int dir;

	if ( argument[0] == '\0' )
	{
		char buf[MSL];
		sprintf( buf, "Your sneak skill is at %d%%.\n\r", ch->pcdata->skill[gsn_sneak] );
		send_to_char( buf, ch );
		if ( ch->position == POS_SNEAKING )
		{
			ch->position = POS_STANDING;
			send_to_char( "You stop sneaking.\n\r", ch );
		}
		return;
	}
	if ( ch->in_vehicle )
	{
		send_to_char( "Not while in a vehicle.\n\r", ch );
		return;
	}
	else if ( !str_prefix(argument,"north") )
		dir = DIR_NORTH;
	else if ( !str_prefix(argument,"east" ) )
		dir = DIR_EAST;
	else if ( !str_prefix(argument,"west") )
		dir = DIR_WEST;
	else if ( !str_prefix(argument,"south") )
		dir = DIR_SOUTH;
	else
	{
		send_to_char( "Invalid direction.\n\r", ch );
		return;
	}
	act( "You start sneaking.", ch, NULL, NULL, TO_CHAR );
	if ( number_percent() > ch->pcdata->skill[gsn_sneak] )
		act( "$n starts sneaking.", ch, NULL, NULL, TO_ROOM );
	ch->c_time = 20-(ch->pcdata->skill[gsn_sneak]/6.6);
	ch->c_sn = gsn_sneak;
	ch->c_level = dir;
	return;
}
void act_sneak( CHAR_DATA *ch, int level )
{
	move_char(ch,level);
	ch->position = POS_SNEAKING;
	return;
}

void do_target( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim=NULL;
	OBJ_DATA *weapon;
	int range;
	int time;
	BUILDING_DATA *bld = NULL;
	bool list = FALSE;

	if ( IN_PIT(ch) )
		mreturn( "Not in the pit.\n\r", ch );
	if ( argument[0] == '\0' )
	{
//		send_to_char( "Target whom?\n\r", ch );
//		return;
		list = TRUE;
	}
	else
	{
		if ( ( victim = get_char_world(ch,argument) ) == NULL )
		{
			send_to_char( "You can't find that target.\n\r", ch );
			return;
		}
	
		if ( victim == ch )
		{
			act( "You stop targetting $N.", ch, NULL, ch->victim, TO_CHAR );
			ch->victim = ch;
			return;
		}
	}
	if ( victim != NULL && victim->in_building && !list )
		if ( !open_bld(victim->in_building) && (victim->x != ch->x || victim->y != ch->y || victim->z != ch->z) )
			mreturn ("You can't target that person through the wall!\n\r", ch );

	if ( ch->in_building && !open_bld(ch->in_building) && bld == NULL )
	{
		send_to_char( "You can't target from inside the building.\n\r", ch );
		return;
	}
        if ( (  ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_L ) ) == NULL || weapon->item_type != ITEM_WEAPON ) && ch->in_vehicle == NULL && bld == NULL )
        {
	        if (  ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_R ) ) == NULL )
		{
                	send_to_char( "You aren't holding a weapon!\n\r", ch );
                	return; 
		}
        }
	if ( ch->in_vehicle == NULL && bld == NULL )
	{
        	if (  weapon->item_type != ITEM_WEAPON )
        	{
        	        send_to_char( "This isn't a gun you're holding!\n\r", ch );
        	        return;
        	}
        	range = 1 + weapon->value[4];
        	if ( ch->class == CLASS_SNIPER )
        	        range += 1;
        	if ( range < 1 )
        	        range = 1;
		if ( ch->in_building && ch->in_building->type == BUILDING_SNIPER_TOWER && complete(ch->in_building) )
			range += 3;
	}
	else if ( bld != NULL )
	{
		range = 6;
	}
	else
	{
		if ( AIR_VEHICLE(ch->in_vehicle->type) )
		{
			if ( ch->z != Z_AIR )
			{
				send_to_char( "The craft isn't operational until you lift!\n\r", ch );
				return;
			}
			range = 3;
		}
		else
			range = 1;
	}

	if ( list )
	{
		char buf[MSL];
		int x,y,xx,yy;

		sprintf( buf, "You are able to target the following people:\n\r\n\r" );
		for ( xx=ch->x-range;xx<=ch->x+range;xx++ )
		for ( yy=ch->y-range;yy<=ch->y+range;yy++ )
		{
			x = xx;y=yy;real_coords(&x,&y);
			for ( victim = map_ch[x][y][ch->z];victim;victim = victim->next_in_room )
			{
				sprintf( buf+strlen(buf), "@@g%s @@c(@@a%d@@c/@@a%d@@c)\n\r", victim->name, x,y);
			}
			if (ch->z != Z_AIR && ( ch->in_vehicle || (weapon &&  !IS_SET(weapon->value[3], WEAPON_HITS_AIR))))
			for ( victim = map_ch[x][y][Z_AIR];victim;victim = victim->next_in_room )
				sprintf( buf+strlen(buf), "@@g%s @@c(@@a%d@@c/@@a%d@@c) @@aFlying in %s@@N\n\r", victim->name, x,y,(victim->in_vehicle)?victim->in_vehicle->desc:"Nothing");

		}
		send_to_char(buf,ch);
		return;
	}

	if ( ch->z != victim->z )
	{
		if ( (ch->z == Z_GROUND || ch->z == Z_AIR) && (victim->z == Z_GROUND || victim->z == Z_AIR) )
		{
			if ( (weapon && !IS_SET(weapon->value[3], WEAPON_HITS_AIR)) )
			{
				send_to_char( "You can't lock on to air targets using this weapon.\n\r", ch );
				return;
			}
		}
		else
		{
			send_to_char( "Your target is not in range.\n\r", ch );
			return;
		}
	}
	if ( ch->victim && ch->victim == victim )
	{
		send_to_char( "They are already targetted.\n\r", ch );
		return;
	}
/*	if ( !NOT_IN_ROOM(ch,victim) )
	{
		send_to_char( "He's right here!\n\r", ch );
		return;
	}*/

	if ( ch->in_vehicle || bld != NULL )
		time =  (40 - ((ch->pcdata->skill[gsn_target] / 20) - (victim->pcdata->skill[gsn_target] / 20))) * 2;
	else
		time =  (40 - ((ch->pcdata->skill[gsn_target] / 20) - (victim->pcdata->skill[gsn_target] / 20))) - weapon->value[6];

	if ( time < 5 )
		time = 5;

	if ( victim->z == Z_AIR && ch->z == Z_GROUND )
		time = 1;

	if ( victim->in_room->vnum == ROOM_VNUM_WMAP && (IS_BETWEEN(victim->x,ch->x-range,ch->x+range)) && (IS_BETWEEN(victim->y,ch->y-range,ch->y+range)))
	{
		if (( IN_PIT(ch) && !IN_PIT(victim) ) || ( !IN_PIT(ch) && IN_PIT(victim) ) )
			return;
		act( "You begin targetting $N!", ch, NULL, victim, TO_CHAR );
		act( "$n begins targetting $N!", ch, NULL, victim, TO_ROOM );
		ch->victim = victim;
		ch->c_sn = gsn_target;
		ch->c_time = time;
		ch->c_level = 1;
		return;
	}
	else
	{
		send_to_char( "Your target is not in range.\n\r", ch );
		return;
	}
	return;
}
void act_target (CHAR_DATA *ch, int level )
{
	OBJ_DATA *weapon;
	int range;

	if ( !ch->victim || ch->victim == NULL )
	{
		send_to_char( "For some reason, your victim isn't online anymore.\n\r", ch );
		ch->victim = ch;
		return;
	}
	if ( ch->victim == ch )
	{
		send_to_char( "For some reason, you were targetting yourself!\n\r", ch );
		ch->victim = ch;
		return;
	}
	if ( ch->victim->in_building )
		if ( !open_bld(ch->victim->in_building) )
		{
			ch->victim = ch;
			mreturn ("Your target has taken cover inside a building!\n\r", ch );
		}

        if ( ( ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_L ) ) == NULL || weapon->item_type != ITEM_WEAPON ) && ch->in_vehicle == NULL )
        {
	        if ( ( ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_R ) ) == NULL || weapon->item_type != ITEM_WEAPON ) && ch->in_vehicle == NULL )
		{
                	send_to_char( "For some reason, you aren't holding a weapon anymore!\n\r", ch );
			ch->victim = ch;
                	return;
		}
        }

	if ( (ch->pcdata->alliance == -1 || ch->pcdata->alliance != ch->victim->pcdata->alliance ) )
		check_skill(ch,gsn_target,15,"Target");
	if ( ch->in_vehicle == NULL  )
	{
        	if ( weapon->item_type != ITEM_WEAPON )
        	{
        	        send_to_char( "This isn't a gun you're holding!\n\r", ch );
        	        return;
        	}
        	range = 1 + weapon->value[4];
        	if ( ch->class == CLASS_SNIPER )
        	        range += 1;
        	if ( range < 1 )
        	        range = 1;
		if ( ch->in_building && ch->in_building->type == BUILDING_SNIPER_TOWER )
			range += 3;
	}
	else
	{
//		range = get_ship_weapon_range(ch->in_vehicle);
		range = 1;
	}

	if ( ch->victim->in_room->vnum != ROOM_VNUM_WMAP || (!IS_BETWEEN(ch->victim->x,ch->x-range,ch->x+range)) || (!IS_BETWEEN(ch->victim->y,ch->y-range,ch->y+range)) )
	{
		send_to_char( "Your victim got away!\n\r", ch );
		ch->victim = ch;
		return;
	}
	if ( ch->z != ch->victim->z )
	{
		if ( ch->in_vehicle != NULL || (weapon && !IS_SET(weapon->value[3],WEAPON_HITS_AIR) ) )
		{
			send_to_char( "Your victim got away!\n\r", ch );
			ch->victim = ch;
			return;
		}
	}
	act( "You are locked on $N!", ch, NULL, ch->victim, TO_CHAR );
	act( "$n is locked on!", ch, NULL, NULL, TO_ROOM );
	return;	
}

void do_computer( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	char arg1[MSL];
	char arg2[MSL];

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if ( !str_cmp(arg1,"defend"))
	{
		if ( ( bld = ch->in_building ) == NULL )
		{
			send_to_char( "You can't defend the building while you are not inside.\n\r", ch );
			return;
		}
		if ( bld->value[8] == 0 )
		{
			send_to_char( "There are no traces of a hacker here.\n\r", ch );
			return;
		}
		act( "You begin defending against the hacker.", ch, NULL, NULL, TO_CHAR );
		act( "$n goes to the computer terminal and begins defending against the hacker.", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = gsn_computer;
		ch->c_level = 1;
		ch->c_time = 30 - ( ch->pcdata->skill[gsn_computer] / 4 );
		return;
	}
	else if ( !str_cmp(arg1,"repair"))
	{
		if ( ( bld = ch->in_building ) == NULL )
		{
			send_to_char( "You can't repair the building while you are not inside.\n\r", ch );
			return;
		}
		if ( bld->value[3] == 0 )
		{
			send_to_char( "There are no traces of a virus here.\n\r", ch );
			return;
		}
/*		if ( bld->value[3] > 0 )
		{
			send_to_char( "The virus has not yet reached your system. You should use the 'Defend' option now.\n\r", ch );
			return;
		}*/
		act( "You begin defending against the virus.", ch, NULL, NULL, TO_CHAR );
		act( "$n goes to the computer terminal and begins defending against the virus.", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = gsn_computer;
		ch->c_level = 2;
		ch->c_time = 40 - ( ch->pcdata->skill[gsn_computer] / 4 );
		return;
	}
	else if ( !str_cmp(arg1,"scan"))
	{
		if ( ( bld = ch->in_building ) == NULL )
		{
			send_to_char( "You can't repair the building while you are not inside.\n\r", ch );
			return;
		}
		if ( bld->type != BUILDING_HQ )
		{
			send_to_char( "You need to be in a headquarters.\n\r", ch );
			return;
		}
		act( "You begin resetting your system.", ch, NULL, NULL, TO_CHAR );
		act( "$n goes to the computer terminal and begins resetting the systems.", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = gsn_computer;
		ch->c_level = 9;
		ch->c_time = 80 - ( ch->pcdata->skill[gsn_computer] / 4 );
		return;
	}
	if ( !str_cmp(arg1,"protect"))
	{
		OBJ_DATA *obj;
		OBJ_DATA *comp = NULL;
		for ( obj = ch->first_carry;obj;obj = obj->next_in_carry_list )
			if ( obj->item_type == ITEM_COMPUTER )
				if ( comp == NULL || obj->value[0] > comp->value[0] )
					comp = obj;

		if ( comp == NULL )
		{
			send_to_char( "You must be carrying a computer in your inventory!\n\r", ch );
			return;
		}
		if ( comp->value[0] <= 2 )
		{
			send_to_char( "This computer is no good for setting up dummies.\n\r", ch );
			return;
		}
		if ( ( bld = ch->in_building ) == NULL || bld->type != BUILDING_HQ || !complete(bld) )
		{
			send_to_char( "You must be in a headquarters.\n\r", ch );
			return;
		}
		act( "You begin setting some dummy directories on the mainframe.", ch, NULL, NULL, TO_CHAR );
		act( "$n begins setting up dummy directories on the mainframe.", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = gsn_computer;
		ch->c_level = 8;
		ch->c_time = 60 - ( ch->pcdata->skill[gsn_computer] / 4 );
		return;
	}
	else if ( !str_cmp(arg1,"backdoor"))
	{
		CHAR_DATA *wch;
		OBJ_DATA *obj;
		OBJ_DATA *comp = NULL;
		bool found = FALSE;
		int range;

		if ( arg2[0] == '\0' )
		{
			send_to_char( "Install a back door at whose computer?\n\r", ch );
			return;
		}
		for ( obj = ch->first_carry;obj;obj = obj->next_in_carry_list )
			if ( obj->item_type == ITEM_COMPUTER )
				if ( comp == NULL || obj->value[0] > comp->value[0] )
					comp = obj;

		if ( comp == NULL )
		{
			send_to_char( "You must be carrying a computer in your inventory!\n\r", ch );
			return;
		}
		if ( ( wch = get_char_world(ch,arg2) ) == NULL )
		{
			send_to_char( "No such player.\n\r", ch );
			return;
		}
		if ( wch == ch )
		{
			send_to_char( "You don't really need to hack yourself... You already know where you are.\n\r", ch );
			return;
		}
		range = comp->value[0];
 		if ( !IS_BETWEEN(wch->x,comp->carried_by->x-range,comp->carried_by->x+range) || !IS_BETWEEN(wch->y,comp->carried_by->y+range,comp->carried_by->y-range) || wch->z != ch->z )
		{
			send_to_char( "Your computer doesn't have that range.\n\r", ch );
			return;
		}
		for ( obj = wch->first_carry;obj;obj = obj->next_in_carry_list )
		{
			if ( obj->item_type == ITEM_COMPUTER && obj->value[3] == 0 )
			{
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char( "Either your target does not carry a computer, or all of their computers are already infected.\n\r", ch );
			return;
		}
		ch->c_sn = gsn_computer;
		ch->c_time = 40 - ( ch->pcdata->skill[gsn_computer] / 4 );
		ch->c_level = 5;
		ch->victim = wch;
		act( "You begin hacking $N's computer!", ch, NULL,wch, TO_CHAR );
		act( "$n pulls up his laptop, and begins hacking someone.", ch, NULL, NULL, TO_ROOM );
		return;
	}
	else if ( !str_cmp(arg1,"connect"))
	{
		int x,y,z,range,xx,yy,xxx,yyy,r1,r2,r;
		OBJ_DATA *obj;
		OBJ_DATA *comp;
		OBJ_DATA *bounce = NULL;
		CHAR_DATA *wch;
		char arg3[MSL];
		char buf[MSL];

		argument = one_argument(argument,arg3);

		if ( !is_number(arg2) || !is_number(arg3) )
		{
			send_to_char( "You must insert valid coordinates.\n\r", ch );
			return;
		}

		x = atoi(arg2);
		y = atoi(arg3);
		if ( x < 0 || x >= MAX_MAPS || y < 0 || y >= MAX_MAPS ) mreturn("Those are invalid coordinates.\n\r", ch );
		real_coords(&x,&y);
		comp = get_best_laptop(ch);

		if ( !comp )
		{
			send_to_char( "You must be carrying a computer in your inventory!\n\r", ch );
			return;
		}

		if ( comp->value[1] <= 0 )
		{
			send_to_char( "Its battery is dead.\n\r", ch );
			return;
		}

		r = 15;
		if ( comp->value[0] > r ) r = comp->value[0];
		z = ch->z;
		r1 = 0-r;
		r2 = 0-r;
		for ( xxx=x-r;xxx<x+r;xxx++ )
		{
		if ( bounce ) break;
		for ( yyy=y-r;yyy<y+r;yyy++ )
		{
			range = (abs(r1)>abs(r2))?abs(r1):abs(r2);
			xx = xxx;yy = yyy;real_coords(&xx,&yy);
			for ( wch = map_ch[xx][yy][ch->z];wch;wch = wch->next_in_room ) {
				if ( wch != ch )
				{
					if ( IS_SET(ch->effect,EFFECT_ENCRYPTION) ) continue;
				}
				else if ( comp->value[0] >= range )
				{
					bounce = comp; continue;
				}

				for ( obj = wch->first_carry;obj;obj = obj->next_in_carry_list ) {
					if ( obj->item_type != ITEM_COMPUTER ) continue;
					if ( obj->value[1] <= 0 ) continue;
					if ( wch != ch && obj->value[3] == 0 ) continue;
					if ( obj->value[0] < range ) continue;
					bounce = obj; break;
				}
			}
			if ( bounce ) break;
			for ( obj=map_obj[xx][yy];obj;obj = obj->next_in_room ) {
				if ( obj->z != ch->z ) continue;
				if ( obj->value[3] == 0 ) continue;
				if ( obj->value[1] <= 0 ) continue;
				if ( obj->item_type != ITEM_COMPUTER ) continue;
				if ( obj->value[0] < range ) continue;
				bounce = obj; break;
			}
			if ( bounce ) break;
			r2++;
		}
		r1++;
		r2 = 0-r;
		}

		if ( !bounce) mreturn("Those coordinates are not in range of your laptop or any transmitters.\n\r", ch );
		if ( bounce != comp )
		{
			sprintf(buf,"You bounce your connection over %s.\n\r", bounce->short_descr );
			send_to_char(buf,ch);
		}
		if ( ( bld = get_building(x,y,z) ) == NULL )
		{
			send_to_char( "There is no building over there.\n\r", ch );
			return;
		}
		if ( is_evil(bld) || !complete(bld) || !str_cmp(bld->owned,"nobody") || build_table[bld->type].act == BUILDING_WALLS )
		{
			send_to_char( "It has no computer network.\n\r", ch );
			return;
		}
		if ( ( wch = get_ch(bld->owned) ) == NULL )
		{
			send_to_char( "Its mainframe is shut down.\n\r", ch );
			return;
		}
		if ( IN_PIT(wch) || paintball(wch) )
		{
			send_to_char( "That's not fair...\n\r", ch );
			return;
		}
		ch->c_sn = gsn_computer;
		ch->c_level = 7;
		ch->c_time = 40;
		ch->bvictim = bld;
		act( "$n pulls out a laptop and begins typing...", ch, NULL, NULL, TO_ROOM );
		send_to_char( "Connecting...\n\r", ch );
		sendsound(ch,"waitcon",40,1,25,"misc","waitcon.wav"); 
		ch->c_obj = comp;

		if ( bounce != comp )
			if ( !bounce->carried_by )
				if ( --bounce->value[1] <= 0 )
				{
					sprintf(buf,"%s has run out of battery, and is destroyed.\n\r", bounce->short_descr);
					send_to_char(buf,ch);
					extract_obj(bounce);
				}

	}
	else
	{
		send_to_char( "Syntax: Computer Defend - Defends against connected hackers.\n\rComputer Repair - Repairs viruses that have entered the system.\n\rComputer Scan - Scans and repairs evil programs.\n\rComputer Protect - Creates dummy directories in a whole base\n\rComputer Connect <x> <y> <bounce?> - Attempts to establish a connection to a given building. Must carry a laptop. You can also set a player's name to Bounce the connection through.\n\rComputer Backdoor - Uploads a trojan to the target player's laptop, if carrying one, which you can track at a Hackers Hideout.", ch );
		return;
	}
	return;
}

void act_computer( CHAR_DATA *ch, int level )
{
	BUILDING_DATA *bld;
	int chance;

	if ( ( bld = ch->in_building ) == NULL && ( ch->c_level == 1 || ch->c_level == 2 || ch->c_level == 8 || ch->c_level == 9) )
	{
		send_to_char( "For some reason, you are no longer in a building.\n\r", ch );
		ch->c_sn = -1;
		ch->c_level = 0;
		return;
	}
/*	if ( number_percent() < 5 && ch->pcdata->skill[gsn_computer] < 100)
	{
		send_to_char( "You have become better at Computers!\n\r", ch );
		ch->pcdata->skill[gsn_computer]++;
	}*/
	if ( level == 1 )
	{
		if ( bld->value[8] == 0 )
		{
			send_to_char( "The hacker has already left your system!!\n\r", ch );
			ch->c_sn = -1;
			ch->c_level = 0;
			return;
		}
		ch->c_time = 30 - ( ch->pcdata->skill[gsn_computer] / 4 );
		act( "You work hard on the computer terminal - Trying to defend against the hacker.", ch, NULL, NULL, TO_CHAR );
		act( "$n works hard on the computer terminal - Trying to defend against the hacker.", ch, NULL, NULL, TO_ROOM );
	}
	if ( level == 2 )
	{
		if ( bld->value[3] == 0 )
		{
			send_to_char( "The virus has been cleared out of your system!!\n\r", ch );
			ch->c_sn = -1;
			ch->c_level = 0;
			return;
		}
		ch->c_time = 40 - ( ch->pcdata->skill[gsn_computer] / 4 );
		act( "You work hard on the computer terminal - Trying to defend against the virus.", ch, NULL, NULL, TO_CHAR );
		act( "$n works hard on the computer terminal - Trying to defend against the virus.", ch, NULL, NULL, TO_ROOM );
	}
	if ( level == 3 || level == 4 )
	{
		char buf[MSL];
		if ( ch->bvictim == NULL )
		{
			send_to_char( "Your target was shut down.\n\r", ch );
			ch->c_sn = -1;
			ch->bvictim = NULL;
			ch->c_level = 0;
			return;
		}
		if ( ch->bvictim->value[3] > 0 )
		{
			sprintf( buf, "Estimated time: Less than %d seconds.\n\r", ch->bvictim->value[3] * 10 );
			act( "$n continues uploading the virus.", ch, NULL, NULL, TO_ROOM );
			send_to_char( buf, ch );
			ch->c_time = 40;
		}
		else
		{
			if ( level == 3 )
				send_to_char( "Your virus was successfully implanted!\n\r", ch );
			else
			{
				send_to_char( "You have upgraded the virus!\n\r", ch );
				ch->bvictim->value[3]--;
			}
			ch->c_sn = -1;
			ch->c_level = -1;
			free_string(ch->bvictim->attacker);
			ch->bvictim->attacker = str_dup(ch->name);
			ch->bvictim = NULL;
			return;
		}
		return;
	}
	if ( level == 5 )
	{
		OBJ_DATA *obj;
		bool found = FALSE;

		if ( ch->victim == ch )
		{
			send_to_char( "Your victim has left!\n\r", ch );
			ch->c_sn = -1;
			return;
		}
		for ( obj = ch->victim->first_carry;obj;obj = obj->next_in_carry_list )
		{
			if ( obj->item_type == ITEM_COMPUTER )
			{
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char( "For some reason, your target is no longer carrying a computer.\n\r", ch );
			return;
		}
		obj->value[3] = 1;
		send_to_char( "You have uploaded the back-door trojan!\n\r", ch );
		ch->victim = ch;
		ch->c_sn = -1;
		ch->c_level = 0;		
		return;
	}
	if ( level == 6 )
	{
		CHAR_DATA *bch;
		if ( ch->bvictim == NULL )
		{
			send_to_char( "Your target was shut down.\n\r", ch );
			ch->c_sn = -1;
			ch->bvictim = NULL;
			ch->c_level = 0;
			return;
		}
		chance = 10;
		if ( ch->class == CLASS_HACKER )
			chance *= 2;
		if ( number_percent() < chance )
		{
			char buf[MSL];
			if ( ( bch = get_ch(ch->bvictim->owned) ) == NULL )
			{
				send_to_char( "The mainframe was shut down.\n\r", ch );
				return;
			}
			if ( bch->pcdata->alliance == -1 )
			{
				send_to_char( "The owner of this structure is not a member of any alliance.\n\r", ch );
				return;
			}
			if ( bch->pcdata->alliance == ch->pcdata->alliance )
			{
				send_to_char( "You're already a member of that alliance.\n\r", ch );
				return;
			}
			if ( ch->pcdata->alliance != -1 )
				alliance_table[ch->pcdata->alliance].members--;
			alliance_table[bch->pcdata->alliance].members++;
			ch->pcdata->alliance = bch->pcdata->alliance;
			sprintf( buf, "@@gYou have inserted yourself into %s @@gdatabase.@@N\n\r", alliance_table[ch->pcdata->alliance].name );
			send_to_char(buf,ch);
		}
		return;
	}
	else if ( level == 7 )
	{
		OBJ_DATA *obj;
		bool found = FALSE;
		for ( obj = ch->victim->first_carry;obj;obj = obj->next_in_carry_list )
		{
			if ( obj->item_type == ITEM_COMPUTER )
			{
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			ch->c_sn = -1;
			send_to_char( "For some reason, you are no longer carrying a computer.\n\r", ch );
			return;
		}
		if ( !ch->bvictim || ch->bvictim == NULL )
		{
			ch->c_sn = -1;
			return;
		}

		if ( ch->bvictim->value[8] != 0 )
		{
			send_to_char( "Connection Refused.\n\r", ch );
			act( "$n takes his computer and places it back in $s inventory.", ch, NULL, NULL, TO_ROOM );
			ch->c_sn = -1;
			return;
		}
		send_to_char( "Connection Established.\n\r", ch );
		act( "$n watches $s computer, grinning.", ch, NULL, NULL, TO_ROOM );
		free_string(ch->bvictim->attacker);
		ch->bvictim->attacker = str_dup(ch->name);
		ch->c_sn = -1;
		ch->c_level = 0;
		ch->position=POS_HACKING;
		ch->bvictim->value[8] = (obj->level/10) + obj->value[4];
		return;
	}
	else if ( level == 8 )
	{
		OBJ_DATA *obj;
		OBJ_DATA *comp = NULL;
		BUILDING_DATA *bld2;
		char buf[MSL];
		for ( obj = ch->first_carry;obj;obj = obj->next_in_carry_list )
			if ( obj->item_type == ITEM_COMPUTER )
				if ( comp == NULL || obj->value[0] > comp->value[0] )
					comp = obj;

		if ( comp == NULL )
		{
			ch->c_sn = -1;
			send_to_char( "For some reason, you are no longer carrying a computer.\n\r", ch );
			return;
		}
		if ( comp->value[0] <= 2 )
		{
			ch->c_sn = -1;
			send_to_char( "This computer is no good for setting up dummies.\n\r", ch );
			return;
		}

		for ( bld2 = first_building;bld2;bld2 = bld2->next )
			if ( !str_cmp(bld2->owned,bld->owned) )
			{
				bld2->directories = comp->value[0];
				bld2->real_dir = number_range(1,bld2->directories);
				bld2->password = number_range(10000,99999);
			}
		sprintf( buf, "You set up %d directories, and only number %d is real!\n\r", comp->value[0], bld->real_dir );
		send_to_char( buf, ch );
		ch->c_sn = -1;
		ch->c_level = 0;
		return;
	}
	if ( level == 9 )
	{
		BUILDING_DATA *bld2;
		if ( bld->type != BUILDING_HQ )
		{
			send_to_char( "For some reason, you are no longer in a HQ!\n\r", ch );
			ch->c_sn = -1;
			return;
		}
		for ( bld2 = first_building;bld2;bld2 = bld2->next )
			if ( !str_cmp(bld2->owned,bld->owned) )
			{
				if ( IS_SET(bld2->value[1],INST_SPOOF) )
					REMOVE_BIT(bld2->value[1],INST_SPOOF);
			}
		send_to_char( "Scan complete.\n\r", ch );
		ch->c_sn = -1;
		return;
	}

	if ( bld->value[3] == 0 && level == 2 )
	{
		send_to_char( "The virus has been cleared.\n\r", ch );
		ch->c_sn = -1;
		ch->c_level = 0;
		return;
	}
	chance = 25;
	if ( ch->class == CLASS_HACKER )
		chance *= 2;
	if ( number_percent() < chance )
	{
		if ( level == 2 )
		{
			send_to_char( "You have defeated the virus!\n\r", ch );
			bld->value[3] = 0;
			bld->value[4] = 24;
			ch->c_sn = -1;
			ch->c_level = 0;
			free_string(bld->attacker);
			bld->attacker = str_dup("None");
			return;
		}
		if ( level == 1 && number_percent() < 25 )
		{
			CHAR_DATA *vch;
			send_to_char( "You got that hacker!\n\r", ch );
			ch->c_sn = -1;
			for ( vch = first_char;vch;vch = vch->next )
			{
				if ( vch->bvictim == ch->in_building )
				{
					send_to_char( "Connection Terminated by system administrator...\n\r", vch );
                			vch->position = POS_STANDING;
                			vch->bvictim->value[8] = 0;
					vch->c_sn = -1;
                			vch->bvictim = NULL;
                			return;					
				}
			}
		}
	}	
	return;
}

void do_exresearch( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Research which explosive?\n\r", ch );
		return;
	}
	if ( ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "You do not carry that explosive.\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_BOMB )
	{
		send_to_char( "That is not an explosive.\n\r", ch );
		return;
	}
	ch->c_sn = gsn_exresearch;
	ch->c_time = 40;
	ch->c_level = 0;
	if ( ( get_eq_char(ch,WEAR_HOLD_HAND_L) ) != NULL )
		unequip_char(ch,get_eq_char(ch,WEAR_HOLD_HAND_L));
	equip_char(ch,obj,WEAR_HOLD_HAND_L);
	act( "You begin researching $p.", ch, obj, NULL, TO_CHAR );
	act( "$n begins researching $p.", ch, obj, NULL, TO_ROOM );
	return;
}

void act_exresearch( CHAR_DATA *ch, int level )
{
	OBJ_DATA *obj;
	int chance;

	if ( ( obj = get_eq_char(ch,WEAR_HOLD_HAND_L) ) == NULL || obj->item_type != ITEM_BOMB )
	{
		send_to_char( "For some reason, you are no longer holding the explosive you were researching!\n\r", ch );
		ch->c_sn = -1;
		return;
	}
	if ( level == 10 )
	{
		send_to_char( "You have completed your research!\n\r", ch );
		obj->value[2] += dice(1,ch->pcdata->skill[gsn_exresearch]/2);
		if ( obj->value[6] > 0 )
			obj->value[6]--;
		ch->c_sn = -1;
		ch->c_level = 0;
		return;
	}
	send_to_char( "You continue researching.\n\r", ch );
	chance = (ch->pcdata->skill[gsn_exresearch]/3)*2;
	if ( number_percent() < ((100-level)/5) + obj->level && number_percent() > chance )
	{
		send_to_char( "You mess up!\n\r", ch );
		act( "$n glitches a little.", ch, NULL, NULL, TO_ROOM );
		if ( number_percent() < 25 )
		{
			int dam = dice(obj->weight,obj->value[2]);
			if ( dam >= ch->max_hit )
				dam = ch->max_hit - 1;
			act( "$p EXPLODES!!", ch, obj, NULL, TO_CHAR );
			act( "$p EXPLODES!!", ch, obj, NULL, TO_ROOM );
			extract_obj(obj);
			ch->c_sn = -1;
			damage(ch,ch,dam,DAMAGE_BLAST);
			return;
		}
		else
			level--;
	}
	act( "$n continues researching.\n\r", ch, NULL, NULL, TO_ROOM );
	ch->c_time = 40;
	ch->c_level = level + 1;
	return;
}

void do_proficiencies(CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	char arg[MSL];
	int i;

	if ( argument[0] == '\0' )
	{
		do_skills(ch,"");
		sprintf( buf, "\n\rYou have %d proficiency points.\n\r", ch->pcdata->prof_points );
		send_to_char(buf, ch );
		send_to_char( "\n\rSyntax: Proficiencies Add <prof>\n\r", ch );
		send_to_char( "        Proficiencies Abilities\n\r", ch );
		return;
	}
	argument = one_argument(argument,arg);
	if ( !str_prefix(arg,"abilities") )
	{
		char buf2[MSL];
		sprintf(buf,"@@d");
		sprintf(buf2,"@@r");
		send_to_char( "\n\rName                Eco Bld Cmb  Desc\n\r----------------------------------------------------------------\n\r", ch );
		for ( i=0;ability_table[i].economics!=-1;i++ )
		{
			if ( !has_ability(ch,i) )
				sprintf(buf+strlen(buf),"%-20s %d   %d   %d   %s\n\r", ability_table[i].name, ability_table[i].economics,ability_table[i].building,ability_table[i].combat,ability_table[i].desc );
			else
				sprintf(buf2+strlen(buf2),"%-20s             %s\n\r", ability_table[i].name, ability_table[i].desc );
		}
		send_to_char(buf,ch);
		send_to_char(buf2,ch);
		return;
	}
	else if ( !str_prefix(arg,"add") )
	{
		if ( ch->pcdata->prof_points <= 0 )
		{
			send_to_char( "You don't have any more proficiency points!\n\r", ch );
			return;
		}
        	for ( i = 0;skill_table[i].gsn != -1;i++ )
        	{
                	if ( !skill_table[i].prof )
				continue;
			if ( str_cmp(skill_table[i].name,argument) )
				continue;
			ch->pcdata->skill[skill_table[i].gsn]++;
			ch->pcdata->prof_points--;
			sprintf( buf, "@@WYou have increased your @@a%s@@W proficiency!@@N\n\r", skill_table[i].name );
			send_to_char(buf,ch);
			save_char_obj(ch);
			return;
		}
		send_to_char("No such proficiency.\n\r", ch );
		return;
	}
	else
	{
		send_to_char( "Invalid option. Options are:\n\rProficiencies ADD <prof>   -   Proficiencies add electronics\n\rProficiencies Abilities - List the prof. abilities\n\r", ch );
		return;
	}
	return;
}

void do_fix ( CHAR_DATA *ch, char * argument)
{
	BUILDING_DATA *bld = ch->in_building;
	if ( !bld || !complete(bld) )
	{
		send_to_char( "You can't fix anything here.\n\r", ch );
		return;
	}
	if ( !has_ability(ch,0) )
	{
		send_to_char( "You do not have enough proficiency points to use this skill.\n\r", ch );
		return;
	}
	if ( bld->hp >= bld->maxhp )
	{
		send_to_char( "This building doesn't need fixing.\n\r", ch );
		return;
	}
	ch->c_sn = gsn_fix;
	ch->c_time = 16;
	send_to_char( "You begin fixing the building.\n\r", ch );
	act( "$n begins fixing the building.", ch, NULL, NULL, TO_ROOM );
	return;
}
void act_fix( CHAR_DATA *ch, int level )
{
	BUILDING_DATA *bld = ch->in_building;
	int x = ch->pcdata->skill[gsn_building];
	if ( !bld || !complete(bld) )
	{
		send_to_char( "You can't fix anything here.\n\r", ch );
		ch->c_sn = -1;
		return;
	}
	bld->hp = URANGE(1,bld->hp+x,bld->maxhp);
	if ( bld->hp >= bld->maxhp )
	{
		ch->c_sn = -1;
		send_to_char( "You finish fixing the building.\n\r", ch );
		return;
	}
	send_to_char( "You continue working on the building.\n\r", ch );
	act( "$n continues working on the building.", ch, NULL, NULL, TO_ROOM );
	ch->c_time = 16;
	return;
}

void do_train(CHAR_DATA *ch, char *argument)
{
	int skill,cost,i,disc;
	char buf[MSL];

	if ( argument[0] == '\0' )
	{
		send_to_char( "Which skill would you like to train?\n\r", ch );
		if ( IS_NEWBIE(ch) )
			send_to_char( "TIP: Type \"Skills\" to see the list.\n\r", ch );
		return;
	}
	skill = -1;
	for ( i=0;skill_table[i].gsn != -1;i++ )
	{
		if ( skill_table[i].gsn == -1 )
			break;
		if ( !str_prefix(argument,skill_table[i].name) )
		{
			skill = skill_table[i].gsn;
			break;
		}
	}
	if ( skill == -1 )
	{
		send_to_char( "There is no such skill.\n\r", ch );
		return;
	}
	if ( skill_table[skill].prof )
	{
		send_to_char( "You cannot train proficiencies with experience points.\n\r", ch );
		return;
	}
	cost = (ch->pcdata->skill[skill] +1) * 3;
	disc = get_rank(ch)/100;
	if ( disc > cost - 10 )
		disc = cost - 10;
	cost -= (cost * disc)/100;
	if ( ch->pcdata->experience < cost )
	{
		sprintf( buf, "This will require %d experience points, you only have %d.\n\r", cost, ch->pcdata->experience );
		send_to_char(buf,ch);
		return;
	}
	ch->pcdata->skill[skill]++;
	ch->pcdata->experience -= cost;
	save_char_obj(ch);
	sprintf( buf, "You train your %s skill.\n\r", skill_table[i].name );
	send_to_char(buf,ch);
	return;
}
bool blind_combat_check(CHAR_DATA *ch)
{
	if ( number_percent() < ch->pcdata->skill[gsn_blind_combat] )
		return TRUE;
//	send_to_char( "You feel around your inventory, but couldn't find it.\n\r", ch );
	return FALSE;
}

void do_conquer(CHAR_DATA *ch, char *argument)
{
	char buf[MSL];
	char hq[MSL];
	BUILDING_DATA *bld = ch->in_building;
	BUILDING_DATA *bld2;
	int lab=-1,lab2=-1,b=0;
	int build[MAX_POSSIBLE_BUILDING]={0};
/*	for(b=0;b<MAX_POSSIBLE_BUILDING;b++)
		build[b]=0;
	b=0;*/
	if ( !bld || bld == NULL )
	{
		send_to_char( "You must be in a building you wish to conquer.\n\r", ch );
		return;
	}
	if ( !bld->owner )
	{
		send_to_char( "How did you even get inside?\n\r", ch );
		return;
	}
	if ( bld->owner == ch )
	{
		send_to_char( "This building already belongs to you.\n\r", ch );
		return;
	}
	if ( get_rank(ch)<2 ) mreturn("You can't capture buildings until you hit rank 2.\n\r",ch);
	if ( get_rank(bld->owner)<2 ) mreturn("You can't capture buildings of low-ranking players.\n\r",ch);
	if ( allied(bld->owner,ch) ) mreturn("You can't capture allied buildings.\n\r", ch );
	if ( build_table[bld->type].act == BUILDING_LAB )
		lab = bld->type;
	else if ( build_table[build_table[bld->type].requirements].act == BUILDING_LAB )
		lab = build_table[bld->type].requirements;
	else if ( build_table[build_table[build_table[bld->type].requirements].requirements].act == BUILDING_LAB )
		lab = build_table[build_table[bld->type].requirements].requirements;
	hq[0] = '\0';
	for ( bld2=ch->first_building;bld2;bld2 = bld2->next_owned )
	{
		b++;
		build[bld2->type]++;
		if ( build_table[bld2->type].act == BUILDING_LAB )
			lab2 = bld2->type;
		else if ( build_table[build_table[bld2->type].requirements].act == BUILDING_LAB )
			lab2 = build_table[bld2->type].requirements;
		else if ( build_table[build_table[build_table[bld2->type].requirements].requirements].act == BUILDING_LAB )
			lab2 = build_table[build_table[bld2->type].requirements].requirements;
		else if (bld2->type == BUILDING_HQ )
			sprintf(hq+strlen(hq),"HQ at: %d/%d\n\r", bld2->x,bld2->y );

	}
	if ( !build[BUILDING_HQ] && bld->type != BUILDING_HQ )
	{
		send_to_char( "You can't capture buildings unless you have a Headquarters.\n\r", ch );
		return;
	}
	else if ( build[BUILDING_HQ] >= MAX_HQS_ALLOWED && bld->type == BUILDING_HQ )
	{
		send_to_char( "You can't capture another headquarters.\n\r", ch );
		sprintf(hq+strlen(hq),"%d HQs found.\n\r", build[BUILDING_HQ]);
		send_to_char(hq,ch);
		return;
	}
	if ( bld->type != BUILDING_HQ && build[bld->type] >= build_table[bld->type].max )
	{
		send_to_char( "You cannot have any more of this building type.\n\r", ch );
		return;
	}
	if ( build[BUILDING_HQ] > MAX_HQS_ALLOWED ) build[BUILDING_HQ] = MAX_HQS_ALLOWED;
	ch->blimit=building_limits[build[BUILDING_HQ]];
	if ( b >= ch->blimit && bld->type != BUILDING_HQ )
		mreturn ("You have already hit your building limit.\n\r", ch );
	if ( lab != -1 && lab2 != -1 && lab != lab2 )
	{
		send_to_char( "You cannot conquer this building. It will conflict with your current technology tree.\n\r", ch );
		return;
	}
	act( "You begin conquering the building!\n\r", ch,NULL,NULL,TO_CHAR );
	act( "$n begins conquering the building!\n\r", ch,NULL,NULL,TO_ROOM );
	ch->c_sn = gsn_conquer;
	ch->c_time = 16;
	ch->c_level = 10;
	sprintf(buf, "@@e%s is attempting to take over your %s (%d/%d)!@@N\n\r", ch->name, bld->name, bld->x,bld->y);
	send_to_char(buf,bld->owner);
	return;
}

void act_conquer(CHAR_DATA *ch, int level)
{
//	int chance=0;
	BUILDING_DATA *bld;

	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "For some reason, you are no longer inside a building.\n\r", ch );
		ch->c_sn = -1;
		return;
	}
	act( "You continue taking over...", ch, NULL,NULL,TO_CHAR );
	act( "$n continues taking over...", ch, NULL,NULL,TO_ROOM );
/*	chance = ((10000/bld->maxhp)*bld->hp)/100;
	chance -= (ch->pcdata->skill[gsn_conquer]/4)*3;
	if ( number_percent() < chance )
	{
		act( "You have failed taking control.", ch, NULL, NULL, TO_CHAR );
		act( "$n has failed assuming control of the building.", ch, NULL, NULL, TO_ROOM );
		ch->c_sn = -1;
		ch->c_level = 0;
		return;
	}*/
	if ( --ch->c_level <= 0 )
	{
		BUILDING_DATA *bld2;
		char buf[MSL];
		sprintf(buf, "@@e%s has taken over your %s (%d/%d)!@@N\n\r", ch->name, bld->name, bld->x,bld->y);
		send_to_char(buf,bld->owner);

		send_to_char( "You have successfully taken over the building!\n\r", ch );
		act( "$n has successfully taken control of the building!", ch, NULL, NULL, TO_ROOM );
		sprintf(buf,"%s's (%d) %s captured by %s.",bld->owned,(bld->owner)?my_get_hours(bld->owner,TRUE):0,bld->name,ch->name);
		log_f(buf);
		if ( bld->owner && bld == bld->owner->first_building )
			bld->owner->first_building = bld->next_owned;
		if ( bld->prev_owned )
			bld->prev_owned->next_owned = bld->next_owned;
		if ( bld->next_owned )
			bld->next_owned->prev_owned = bld->prev_owned;
		free_string(bld->owned);
		bld->owned = str_dup(ch->name);
		bld2 = ch->first_building;
		while (bld2 && bld2->next_owned )bld2 = bld2->next_owned;
		if ( bld2 )
		{
			bld->next_owned = NULL;
			bld->prev_owned = bld2;
			bld2->next_owned = bld;
		}
		else
		{
			bld->next_owned = NULL;
			bld->prev_owned = NULL;	
			ch->first_building = bld;
		}
		bld->timer = 0;
		bld->active = TRUE;
		bld->owner = ch;
		ch->c_sn = -1;
		check_hq_connection(bld);
		if ( ch->fighttimer < 360 )
		ch->fighttimer = 360;
		return;
	}
	ch->c_time = (((10000/bld->maxhp)*bld->hp)/100) - ((ch->pcdata->skill[gsn_conquer]/4)*3);
	if ( ch->c_time < 16 )
		ch->c_time = 16;
	return;
}
void do_infiltrate(CHAR_DATA *ch, char *argument)
{
	BUILDING_DATA *bld;
	int dir,x,y;

	if ( ch->in_vehicle )
		mreturn("Exit the vehicle first.\n\r", ch );
	if ( ch->in_building )
	{
		send_to_char( "You can't find an entrance!\n\r", ch );
		return;
	}
	if ( ( dir = parse_direction(ch,argument) ) == -1 )
	{
		send_to_char( "Which direction do you want to infiltrate to?\n\r", ch );
		return;
	}
	x=ch->x; y=ch->y;

        if ( dir == DIR_NORTH )
                y++;
        else if ( dir == DIR_SOUTH )
                y--;
        else if ( dir == DIR_EAST )          
                x++;                
        else if ( dir == DIR_WEST )
                x--;

	real_coords(&x,&y);

	if ( ( bld = map_bld[x][y][ch->z] ) == NULL )
	{
		char buf[MSL];
		sprintf(buf, "There is nothing there to infiltrate (%d/%d).\n\r", x,y);
		send_to_char(buf,ch);
		return;
	}
	if ( !bld->active ) mreturn("You can't infiltrate offline buildings.\n\r", ch);
	if ( build_table[bld->type].act == BUILDING_WALLS )
	{
		ch->c_level = bld->value[0]+(bld->level*5);
		act( "You begin climbing up the wall.", ch, NULL, NULL, TO_CHAR );
		act( "$n begins climbing up the wall.", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		ch->c_level = bld->level*5;
		act( "You begin climbing up through the window.", ch, NULL, NULL, TO_CHAR );
		act( "$n begins climbing up through the window.", ch, NULL, NULL, TO_ROOM );
	}
	ch->c_sn = gsn_infiltrate;
	ch->c_time = 32;
	ch->bvictim = bld;
	return;
}
void act_infiltrate(CHAR_DATA *ch, int level)
{
	BUILDING_DATA *bld = ch->bvictim;
	if ( !ch->bvictim )
		return;
	if ( number_percent() < ch->c_level )
	{
		send_to_char( "You couldn't break through this time.\n\r", ch );
		act( "$n couldn't get through.", ch, NULL, NULL, TO_ROOM );
		return;
	}
	act( "$n breaks into the building.", ch, NULL, NULL, TO_ROOM );
	move(ch,bld->x,bld->y,bld->z);
	do_look(ch,"");
	ch->position = POS_SNEAKING;
	return;
}

void do_work(CHAR_DATA *ch, char *argument)
{
	BUILDING_DATA *bld = ch->in_building;

	if ( !bld )
		mreturn( "You can't work here.\n\r", ch );

	if ( !bld->owner || bld->owner != ch || !complete(bld) )
		mreturn( "The owners won't let you work here.\n\r", ch )

	if ( bld->type == BUILDING_BANK )
	{
		act( "You sit next to your desk, and ignore people like a good bank employee.", ch, NULL, NULL, TO_CHAR );
		act( "$n sits next to $s desk, ignoring everyone around him like in real life.", ch, NULL, NULL, TO_ROOM );
	}
	else if ( bld->type == BUILDING_CYBERCAFE )
	{
		OBJ_DATA *obj;
		if ( ( obj = get_best_laptop(ch) ) == NULL )
			mreturn( "You must have a computer to work here.\n\r", ch );
		if ( obj->value[0] <= 0 )
			mreturn( "Your laptop needs a new battery.\n\r", ch );
		ch->c_obj = obj;
		act( "You pull up your computer, and begin hacking websites.", ch, NULL, NULL, TO_CHAR );
		act( "$n pulls up $s computer, and begins hacking websites.", ch, NULL, NULL, TO_ROOM );
	}
	else if ( bld->type == BUILDING_MINE )
	{
		act( "You begin working in the mines.", ch, NULL, NULL, TO_CHAR );
		act( "$n begins working in the mines.", ch, NULL, NULL, TO_ROOM );
	}
	else if ( bld->type == BUILDING_SHOP )
	{
		act("You take your spot behind the counter and wait for a customer.", ch, NULL, NULL, TO_CHAR );
		act( "$n takes $s spot behind the counter and waits for a customer.", ch, NULL, NULL, TO_ROOM );
	}
	else
		mreturn( "You can't work in this building.\n\r", ch );

	ch->c_sn = gsn_work;
	ch->c_time = 20 * PULSE_PER_SECOND;
	ch->c_level = 0;
	return;
}
void act_work(CHAR_DATA *ch,int level)
{
	BUILDING_DATA *bld = ch->in_building;
	char buf[MSL];
	int m;

	if ( !bld )
	{
		ch->c_sn = -1;
		mreturn( "You are no longer in a building!\n\r", ch );
	}

	ch->c_level++;
	if ( bld->type == BUILDING_BANK )
	{
		sprintf( buf, "@@NYou secretly sneak money into your account @@x(@@N$%d@@x)@@N\n\r", bld->level );
		send_to_char(buf,ch);
		bld->value[5]+=bld->level;
		if ( bld->value[5] > 30000 ) bld->value[5] = 30000;
		if ( number_percent() < ch->c_level )
		{
			send_to_char( "Your boss found out you were stealing! In his rage, he erased your entire account.\n\r", ch );
			bld->value[5] = 0;
			ch->c_sn = -1;
			return;
		}
	}
	else if ( bld->type == BUILDING_CYBERCAFE )
	{
		OBJ_DATA *obj;
		int i = number_range(1,5);
		if ( ( obj = ch->c_obj ) == NULL )
		{
			ch->c_sn = -1;
			mreturn( "You have lost your computer.\n\r", ch );
		}
		ch->c_obj = obj;
		switch(i)
		{
			case 1: send_to_char( "You contribute to the further development of free games!.. Actually, you just cracked a commercial game and made it free.", ch ); break;
			case 2: send_to_char( "You break into Microsoft.com and change the logs to point at your highschool bully.", ch ); break;
			default: send_to_char( "You hack and copy a porn site.", ch ); break;
		}
		i = ((obj->value[0] * ch->pcdata->skill[gsn_computer]) / 100)+1;
		sprintf( buf, " @@x(@@N$%d@@x)@@N\n\r", i );
		send_to_char(buf,ch);
		gain_money(ch,i);
		if ( --obj->value[1] <= 0 )
		{
			send_to_char( "Your laptop has shut down! You need a new battery.\n\r", ch );
			ch->c_sn = -1;
			return;
		}
		if ( number_percent() < ch->c_level )
			obj->value[3] = 1;
		if ( number_percent() < ch->c_level && 0 )
		{
			send_to_char( "The authorities have traced your location! You sprint out of your workstation and start whistling like nothing happened.\n\r", ch );
			obj->value[3] = 1;
			ch->c_sn = -1;
			return;
		}

	}
	else if ( bld->type == BUILDING_MINE )
	{
		if ( number_percent() < ch->c_level )
		{
			m = number_range(1,500);
			sprintf(buf, "A %s rock drops right on your head. @@e(@@R%d@@e)@@N\n\r", (m<100)?"small":(m<300)?"heavy":"huge", m );
			send_to_char(buf,ch);
			sprintf(buf, "A %s rock drops right on $n's head.", (m<100)?"small":(m<300)?"heavy":"huge" );
			act(buf,ch,NULL,NULL,TO_ROOM);
			damage(ch,ch,m,-1);
		}
		else
		{
			send_to_char("You sift through the piles of sand and rocks.\n\r", ch );
			act( "$n sifts through the piles of sand and rocks.", ch, NULL, NULL, TO_ROOM );
			m = number_range(1,50);
			if ( m > 5 )
			{
				send_to_char( "You find nothing.\n\r", ch );
			}
			else
			{
				sprintf(buf,"You find a %s! @@a(@@c$%d@@a)@@N\n\r", (m==1)?"silver pebble":(m==2)?"silver chunk":(m==3)?"shiny stone":(m==4)?"tiny piece of gold":"gemstone", m * 10);
				send_to_char(buf,ch);
				gain_money(ch,m*10);
			}
		}
	}
	else if ( bld->type == BUILDING_SHOP )
	{
		sprintf(buf, "A customer walks in.\n\r" );
		m = number_range(1,bld->level);
		if ( number_percent() < ch->c_level )
		{
			m *= 15;
			sprintf(buf+strlen(buf),"He pulls out a gun and robs you!! @@e(@@R$%d@@e)@@N\n\r", m );
			ch->money -= m;
			if ( ch->money <=0 ) ch->money = 0;
		}
		else
		{
			sprintf(buf+strlen(buf),"He buys a couple of things, and leaves. @@a(@@c$%d@@a)@@N\n\r", m );
			gain_money(ch,m);
		}
		send_to_char(buf,ch);
	}
	else
	{
		ch->c_sn = -1;
		return;
	}
	if ( ch->c_level < 16 )
		ch->c_time = (20-(ch->c_level/4)) * PULSE_PER_SECOND;
	else
		ch->c_time = 4 * PULSE_PER_SECOND;
	return;
}
