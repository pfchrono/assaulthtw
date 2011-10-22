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
 ~  In order to use any part of this  PA  Diku Mud, you must comply with   ~
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


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ack.h"
#include "tables.h"
#include <math.h>

sh_int get_psuedo_level( CHAR_DATA *ch )
{
  return 80;
}
  

bool	ok_to_use( CHAR_DATA *ch, int value )
{
   if ( value == NO_USE 
   &&   get_trust(ch) < 85 )
   {
      send_to_char( "Only Creators may use this value.\n\r", ch );
      return FALSE;
   }
   return TRUE;
}

bool	check_level_use( CHAR_DATA *ch, int level )
{
   char buf[MAX_STRING_LENGTH];
   char out[MAX_STRING_LENGTH];
   
   if ( get_trust(ch) >= level )
      return TRUE;
   
   sprintf( out, "This option limited to " );
   
   switch ( level )
   {
      case 85:
         safe_strcat( MSL, out, "Creators only.\n\r" );
         break;
      case 84:
         safe_strcat( MSL, out, "Supremes or higher.\n\r" );
         break;
      case 83:
         safe_strcat( MSL, out, "Dieties or higher.\n\r" );
         break;
      case 82:
         safe_strcat( MSL, out, "Immortals or higher.\n\r" );
         break;
      case 81:
         safe_strcat( MSL, out, "Heroes or higher.\n\r" );
         break;
      default:
         sprintf( buf, "level %d players and higher.\n\r", level );
         safe_strcat( MSL, out, buf );
   }
   send_to_char( out, ch );
   return FALSE;
}   

OBJ_DATA * create_teleporter( BUILDING_DATA *bld, int range )
{
	OBJ_DATA *obj;
	char buf[MSL];

	obj = create_object( get_obj_index( OBJ_VNUM_TELEPORTER ), 0 );
	obj->level = range;
	obj->value[0] = range;
	obj->value[1] = bld->type;
	sprintf( buf, "A Series %d, %s-Teleporter", range, capitalize(bld->name) );
	free_string( obj->short_descr );
	free_string( obj->description );
	free_string( obj->name );
	obj->short_descr = str_dup(buf);
	obj->description = str_dup(buf);
	sprintf( buf, "Teleporter %s", bld->name );
	obj->name = str_dup(buf);
	obj->x = bld->x;
	obj->y = bld->y;
	obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
	return obj;
}
OBJ_DATA * create_locator( int range )
{
	OBJ_DATA *obj;
	char buf[MSL];

	obj = create_object( get_obj_index( OBJ_VNUM_LOCATOR ), 0 );
	obj->level = range;
	obj->value[0] = range;
	sprintf( buf, "A Series %d item locator", range );
	free_string( obj->short_descr );
	free_string( obj->description );
	obj->short_descr = str_dup(buf);
	obj->description = str_dup(buf);
	obj->x = 1;
	obj->y = 1;
	obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
	return obj;
}
bool complete( BUILDING_DATA *bld )
{
	if ( build_table[bld->type].act != BUILDING_UNATTACHED )
		if ( !bld->tag )
			return FALSE;
	return (!bld->cost);
}

VEHICLE_DATA *get_vehicle_from_vehicle( VEHICLE_DATA *vhc )
{
	if ( !vhc || vhc == NULL || !TRANSPORT_VEHICLE(vhc->type) )
		return NULL;
	return vhc->vehicle_in;
}

bool is_upgrade( int type )
{
	int i;

	for ( i = 0;i<8;i++ )
		if ( build_table[type].cost != 0 )
			return FALSE;
	return TRUE;
}
bool is_evil( BUILDING_DATA *bld )
{
	if ( bld->timer > 0 || IS_SET(bld->value[1],INST_ONLINE))
		return TRUE;
	return FALSE;
}

int get_char_cost( CHAR_DATA *ch )
{
	BUILDING_DATA *bld;
	int cost = 0;

	for ( bld = ch->first_building;bld;bld = bld->next_owned )
	{
		cost += bld->level;
	}
	return cost;
}

bool IS_BETWEEN(int x,int x1, int x2)
{
	if ( x1 > x2 )
		if ( x >= x2 && x <= x1 )
			return TRUE;
	if ( x1 < x2 )
		if ( x <= x2 && x >= x1 )
			return TRUE;
	if ( x1 == x2 )
		if ( x == x1 )
			return TRUE;
	return FALSE;
}

bool building_can_shoot( BUILDING_DATA *bld, CHAR_DATA *ch, int range )
{
	BUILDING_DATA *bld2;

	if ( ch == NULL )
		return FALSE;
	if ( bld->owner == ch )
		return FALSE;
	if ( IS_IMMORTAL(ch) )
		return FALSE;
	if ( (bld->level < 5 && sneak(ch) ) || sneak(ch) )
		return FALSE;
	if ( ( bld2 = ch->in_building ) != NULL && complete(bld2) && !open_bld(bld2) )
		return FALSE;
	if ( bld2 && bld2 == bld )
		return FALSE;
	
	return TRUE;
}

bool open_bld( BUILDING_DATA *bld )
{
	if ( !complete(bld) )
		return TRUE;

	if ( build_table[bld->type].act == BUILDING_WALLS
	  || build_table[bld->type].act == BUILDING_UNATTACHED
	  || bld->type == BUILDING_WAR_CANNON
	  || bld->type == BUILDING_SNIPER_TOWER
	)
		return TRUE;
	return FALSE;
}
int get_rank( CHAR_DATA *ch )
{
	int rank;

	if ( IS_IMMORTAL(ch) )
		return 9999;

/*	rank = ((ch->pcdata->tbkills/4) + (ch->pcdata->tpkills/3)) - ( ch->pcdata->deaths/3);
	rank += 1; */
	rank = (ch->rank / 100)+1;
	if ( rank < 1 )
		rank = 1;
	else if ( rank > 9998 )
		rank = 9998;
	return rank;
}

int get_bit_value( int bit )
{
	switch (bit)
	{
		case 1: return BIT_1;
		case 2: return BIT_2;
		case 3: return BIT_3;
		case 4: return BIT_4;
		case 5: return BIT_5;
		case 6: return BIT_6;
		case 7: return BIT_7;
		case 8: return BIT_8;
		case 9: return BIT_9;
		case 10: return BIT_10;
		case 11: return BIT_11;
		case 12: return BIT_12;
		case 13: return BIT_13;
		case 14: return BIT_14;
		case 15: return BIT_15;
		case 16: return BIT_16;
		case 17: return BIT_17;
		case 18: return BIT_18;
		case 19: return BIT_19;
		case 20: return BIT_20;
		case 21: return BIT_21;
		case 22: return BIT_22;
		case 23: return BIT_23;
		case 24: return BIT_24;
		case 25: return BIT_25;
		case 26: return BIT_26;
		case 27: return BIT_27;
		case 28: return BIT_28;
		case 29: return BIT_29;
		case 30: return BIT_30;
		case 31: return BIT_31;
		case 32: return BIT_32;
		default: return 0;
	}
	return 0;
}

bool sneak( CHAR_DATA *ch )
{
	int chance,sect;
	OBJ_DATA *obj;
	if ( !ch )
		return TRUE;

	if ( ch->z == Z_PAINTBALL )
		return FALSE;

	if ( ch->class == CLASS_DARKOP && ch->in_vehicle == NULL && number_percent() < 22 ) return TRUE;
	if ( ( obj = get_eq_char(ch,WEAR_BODY) ) != NULL && obj->item_type == ITEM_SUIT && obj->value[0] == 0 && number_percent() < obj->value[1] ) return TRUE;

	if ( ch->position != POS_SNEAKING )
		return FALSE;
	sect = map_table.type[ch->x][ch->y][ch->z];
	chance = ch->pcdata->skill[gsn_sneak];
	chance += ch->pcdata->skill[gsn_combat] * 1.5;
	if ( sect == SECT_ROCK )
		chance -= 7;
	else if ( sect == SECT_FIELD )
		chance -= 5;
	else if ( sect == SECT_FOREST )
		chance += 10;

	if ( chance > 90 ) chance = 90;

	if ( number_percent() < chance )
		return TRUE;
	return FALSE;
}

void check_building_destroyed( BUILDING_DATA *bld )
{
	CHAR_DATA *bch;

	if ( bld == NULL )
		return;
	bch = get_ch(bld->owned);
	if ( bch == NULL )
		return;

	if ( bld->type == BUILDING_BANK && bld->value[5] > 1 )
	{
		if ( has_ability(bch,6) ) //money safe
		{
			send_to_char("You manage to keep 50% of the bank's deposited money.\n\r", bch );
			gain_money(bch,bld->value[5]/2);
		}
	}
	else if ( bld->type == BUILDING_PSYCHIC_LAB )
	{
		BUILDING_DATA *bld2;
		int x=0;
		char buf[MSL];
		for ( bld2=bch->first_building;bld2;bld2 = bld2->next_owned )
		{
			if ( build_table[bld2->type].act == BUILDING_DEFENSE && bld2->value[10] == DAMAGE_PSYCHIC )
			{
				bld->value[10] = DAMAGE_BULLETS;
				x++;
			}
		}
		if ( x > 0 )
		{
			sprintf(buf,"%d of your turrets have been reverted back to bullet damage.\n\r", x );
			send_to_char(buf,bch);
		}
	}

	if ( build_table[bld->type].act != BUILDING_UNATTACHED )
	{
		bld->tag = 1;
		check_hq_connection(bld);
	}
	return;
}

int get_item_limit( BUILDING_DATA *bld )
{
	if ( bld == NULL )
		return 0;
	if ( bld->type == BUILDING_WAREHOUSE )
		return bld->level * 20;
	return 20;
}

void send_warning( CHAR_DATA *ch, BUILDING_DATA *bld, CHAR_DATA *victim )
{
	char buf[MSL];
	if ( ch == victim )
		return;
	if ( victim->in_vehicle && victim->in_vehicle->type == VEHICLE_BOMBER )
		return;

	sprintf( buf, "@@yYour %s, at %d/%d, is firing at %s!\n\r", bld->name, bld->x, bld->y, victim->name );
	send_to_char( buf, ch );
	return;
}

void update_ranks( CHAR_DATA *ch )
{
	int i,min,rank,max=0;
	bool stop = FALSE;

	if ( !ch )
		return;

	min = 0;
	rank = get_rank(ch);
	for ( i = 0;i<30;i++ )
	{
		if ( !stop && (rank_table[i].name == NULL || !str_cmp(rank_table[i].name,ch->name) ) )
		{
			min = i;
			stop = TRUE;
//			break;
		}
		if ( !stop && rank_table[i].rank < rank_table[min].rank )
			min = i;
		if ( rank_table[i].rank > max )
			max = rank_table[i].rank;
	}
	if ( rank_table[min].name != NULL )
		free_string(rank_table[min].name);
	rank_table[min].name = str_dup(ch->name);
	rank_table[min].rank = rank;
	save_ranks();
	if ( rank > max )
	{
		web_data.highest_rank = rank;
		update_web_data(WEB_DATA_HIGHEST_RANK,ch->name);
	}
	return;
}

bool defense_building( BUILDING_DATA *bld )
{
	if ( build_table[bld->type].act == BUILDING_DEFENSE )
			return TRUE;
	return FALSE;
}

void sendsound( CHAR_DATA *ch, char *file, int V, int I, int P, char *T, char *filename )
{
	char buf[MSL];
	if ( !IS_SET(ch->config,CONFIG_SOUND) )
		return;
	sprintf( buf, "\n\r!!SOUND(%s V=%d L=%d P=%d T=%s U=%s/MSP/%s)", file, V,I,P,T,WEBSITE,filename );
	send_to_char(buf,ch);
	return;
}

int check_dodge( CHAR_DATA *ch, int chance )
{
	int newchance;
	newchance = chance - ( (chance / 100) * (ch->pcdata->skill[gsn_dodge] /2) );
	if ( ch->pcdata->skill[gsn_dodge] < 100 && number_percent() < 2 )
	{
		send_to_char( "You have become better at dodge!\n\r", ch );
		ch->pcdata->skill[gsn_dodge]++;
		save_char_obj(ch);
	}
	return newchance;
}

bool in_range( CHAR_DATA *ch, CHAR_DATA *victim, int range )
{
	if ( !ch || !victim )
		return FALSE;
	if ( (IS_BETWEEN(victim->x,ch->x-range,ch->x+range)) && (IS_BETWEEN(victim->y,ch->y-range,ch->y+range)))
		return TRUE;
	else
		return FALSE;
}

void create_obj_atch( CHAR_DATA *ch, int index )
{
	OBJ_DATA *obj;
	obj = create_object(get_obj_index(index),0);
	obj->x = ch->x;
	obj->y = ch->y;
	obj->z = ch->z;
	obj_to_room(obj,ch->in_room);
	return;
}
bool blind_spot( CHAR_DATA *ch, int x, int y )
{
	int range = 5;
	if ( !ch->in_vehicle )
		return FALSE;

	if ( IS_SET(ch->in_vehicle->flags,VEHICLE_SWIM) )
		range = 8;
	else if ( AIR_VEHICLE(ch->in_vehicle->type) )
		range = 6;
	else if ( ch->in_vehicle->type == VEHICLE_CREEPER )
		range = 2;

	if ( (x < ch->x - range || x > ch->x + range || y < ch->y - range || y > ch->y + range))
		return TRUE;
	return FALSE;
}

void reset_building(BUILDING_DATA *bld,int type)
{
	activate_building(bld,TRUE);
	bld->type = type;
	bld->maxhp = build_table[type].hp;
	bld->maxshield = build_table[type].shield;
	bld->hp = bld->maxhp;
	bld->shield = bld->maxshield;
	bld->level = 1;
	if ( bld->owned != NULL )
		free_string(bld->owned);
	if ( bld->name != NULL )
		free_string(bld->name);
	bld->owned = str_dup("Nobody");
	bld->owner = NULL;
	bld->name = str_dup(build_table[type].name);
	bld->cost = 0;
}
OBJ_DATA * make_quest_base( int type, int size, int z )
{
	bool bad = TRUE;
	bool skip = FALSE;
	BUILDING_DATA *bld;
	OBJ_DATA *obj = NULL;
	int i,x,y,xx,yy;
	int ox,oy;

	i=0;
	ox = 0;
	oy = 0;
	size /= 1.5;
	while ( bad )
	{
		i++;
		if ( i > 5 )
			return NULL;
		bad = FALSE;
		x = number_range(size+4,MAX_MAPS-size-5);
		y = number_range(size+4,MAX_MAPS-size-5);
		if ( map_table.type[x][y][z] == SECT_NULL )
		{
			bad = TRUE;
			continue;
		}
		for ( xx = x-size-4;xx<x+size+4;xx++ )
			for ( yy = y-size-4;yy<y+size+4;yy++ )
				if ( map_bld[xx][yy][z] )
					bad = TRUE;
		
	}
	for ( xx = x-size;xx<x+size;xx++ )
	{
		for ( yy = y-size;yy<y+size;yy++ )
		{
			skip = !skip;
			if ( number_percent() < 15 )
				skip = !skip;
			if ( skip )
				continue;
			if ( ox == 0 || number_percent() < 5 )
			{
				ox = xx;
				oy = yy;
			}
			bld = create_building(type);
			bld->x = xx;
			bld->y = yy;
			bld->z = z;
			map_bld[bld->x][bld->y][bld->z] = bld;
			reset_building(bld,type);
			bld->timer = 60;
			for ( i=0;i<4;i++ )
				bld->exit[i] = TRUE;
		}
	}
	if ( ox > 0 && oy > 0 )
	{
		char buf[MSL];
		obj = create_object(get_obj_index(OBJ_VNUM_SCAFFOLD),0);
		obj->level = 1;
		obj->value[0] = type;
		sprintf(buf,"@@cA@@a %s @@cScaffold@@N",build_table[type].name);
		free_string(obj->short_descr);
		free_string(obj->description);
		obj->short_descr = str_dup(buf);
		obj->description = str_dup(buf);
		sprintf(buf,"%s Scaffold",build_table[type].name);
		free_string(obj->name);
		obj->name = str_dup(buf);
		obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
		move_obj(obj,ox,oy,z);
	}
	return obj;
}
bool open_scaffold(CHAR_DATA *ch, OBJ_DATA *obj)
{
	BUILDING_DATA *bld;
	int i,buildings=0,same=0;
	char buf[MSL];
	if ( !obj || obj->item_type != ITEM_SCAFFOLD || get_building(obj->x,obj->y,obj->z))
		return FALSE;

        if ( !sysdata.killfest )
	{
        	for ( bld = first_building;bld;bld = bld->next )
        	{
                        if ( str_cmp(ch->name, bld->owned) )
				continue;
			if ( bld->type == obj->value[0] )
				same++;
                        buildings++;
		}
	}
	if ( buildings >= BUILDING_LIMIT )
		return FALSE;
	if ( same >= obj->value[1] )
		return FALSE;
	bld = create_building(obj->value[0]);
	if ( bld == NULL )
		return FALSE;
	reset_building(bld,obj->value[0]);
	bld->x = obj->x;
	bld->y = obj->y;
	bld->z = obj->z;
	map_bld[bld->x][bld->y][bld->z] = bld;
	free_string(bld->owned);
	bld->owned = str_dup(ch->name);
	bld->owner = ch;
	sprintf(buf,"%s @@copens up into the @@a%s@@c!@@N\n\r",obj->short_descr,bld->name);
	send_to_loc(buf,obj->x,obj->y,obj->z);
	bld->cost = 0;
	for ( i=0;i<4;i++ )
		bld->exit[i] = TRUE;
	bld->hp = 1;
	bld->shield = 1;
	extract_obj(obj);
	reset_special_building(bld);
	return TRUE;
}

bool has_ability(CHAR_DATA *ch,int abil)
{
	if ( ch->pcdata->skill[gsn_economics] >= ability_table[abil].economics )
		if ( ch->pcdata->skill[gsn_building] >= ability_table[abil].building )
			if ( ch->pcdata->skill[gsn_combat] >= ability_table[abil].combat )
				return TRUE;
	return FALSE;
}
void reset_special_building(BUILDING_DATA *bld)
{
	return;
}
int get_armor_value(int dt)
{
	if ( dt == DAMAGE_GENERAL )
		return 2;
	else if ( dt == DAMAGE_BULLETS )
		return 3;
	else if ( dt == DAMAGE_BLAST )
		return 4;
	else if ( dt == DAMAGE_ACID )
		return 5;
	else if ( dt == DAMAGE_FLAME )
		return 6;
	else if ( dt == DAMAGE_LASER )
		return 7;
	else if ( dt == DAMAGE_SOUND )
		return 8;
	else
		return -1;

}
bool hidden(CHAR_DATA *victim)
{
	if ( IS_SET(victim->act, PLR_WIZINVIS) || IS_SET(victim->act, PLR_INCOG) )
		return TRUE;
	if ( victim->in_building && victim->in_building->type == BUILDING_CLUB && complete(victim->in_building) )
		return TRUE;
	return FALSE;
}

int count_buildings(CHAR_DATA *victim)
{
	int i=0;
	BUILDING_DATA *bld;
	if ( !victim || victim == NULL )
		return 999;
	for ( bld = victim->first_building;bld;bld = bld->next_owned )
		i++;
	return i;
}
void clear_basic(CHAR_DATA *ch)
{
	ch->pcdata->pkills = 0;
	ch->pcdata->tpkills = 0;
	ch->pcdata->bkills = 0;
	ch->pcdata->tbkills = 0;
	ch->quest_points = 0;
	ch->pcdata->deaths = 0;
	return;
}
void real_coords(int *x,int *y)
{
        if ( *x < 0 )      
                *x = MAX_MAPS + *x;
        if ( *y < 0 )      
                *y = MAX_MAPS + *y;
        if ( *x >= MAX_MAPS )       
                *x = *x - MAX_MAPS;
        if ( *y >= MAX_MAPS )
                *y = *y - MAX_MAPS;
}

bool check_hq_connection(BUILDING_DATA *bldc)
{
	BUILDING_DATA *bld;
	BUILDING_DATA *bld_next;
	CHAR_DATA *ch = bldc->owner;
	int b=0;
	char buf[MSL];

	if ( !ch || ch == NULL ) return FALSE;

	if ( sysdata.killfest )
	{
		for (bld=ch->first_building;bld;bld = bld->next_owned)
			bld->tag = TRUE;
		return TRUE;
	}
	for (bld=ch->first_building;bld;bld = bld->next_owned)
		bld->tag = build_table[bld->type].act==BUILDING_UNATTACHED;
//	bldc->tag = TRUE;
	for (bld=ch->first_building;bld;bld = bld->next_owned)
		if ( bld->type == BUILDING_HQ )
			tag(bld);
	for (bld=ch->first_building;bld;bld = bld_next)
	{
		bld_next = bld->next_owned;
		if ( bld->tag == FALSE && build_table[bld->type].act != BUILDING_UNATTACHED && !bld->visible )
//			extract_building(bld,TRUE);
//			b++;
			bld->visible = TRUE;
	}
/*	if ( b > 0 )
	{
		sprintf(buf,"@@e%d@@R of your buildings have been disconnected from your Headquarters, and will no longer function properly.@@N\n\r", b );
		send_to_char(buf,ch);
	}*/
        return TRUE;
}        

void tag(BUILDING_DATA *bld)
{
	if (!bld || bld==NULL)
		return;
	if (bld->tag)
		return;
	bld->tag = TRUE;
	tag(find_building(bld,DIR_NORTH));
	tag(find_building(bld,DIR_WEST));
	tag(find_building(bld,DIR_SOUTH));
	tag(find_building(bld,DIR_EAST));
	return;
}

BUILDING_DATA *find_building(BUILDING_DATA *bld,int dir)
{
	int x,y;
	x = bld->x; y = bld->y;
	if ( dir == DIR_NORTH )
		y++;
	else if ( dir == DIR_SOUTH )
		y--;
	else if ( dir == DIR_WEST )
		x--;
	else
		x++;
	real_coords(&x,&y);
	return map_bld[x][y][bld->z];
}
void gain_money(CHAR_DATA *ch,long r)
{
	if ( ch->money + r > MAX_CASH )
		ch->money = MAX_CASH;
	else
		ch->money += r;
}
OBJ_DATA *get_best_laptop(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj2=NULL;
	for ( obj = ch->first_carry;obj;obj = obj->next_in_carry_list )
	{
		if ( obj->item_type != ITEM_COMPUTER ) continue;
		if ( obj->value[1] <= 0 ) continue;
		if ( obj->value[8] == 0 ) continue;
		if ( obj2 == NULL || obj->value[0] > obj2->value[0] )
			obj2 = obj;
	}
	return obj2;
}
