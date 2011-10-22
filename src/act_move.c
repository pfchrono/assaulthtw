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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "ack.h"
#define NOWHERE -1
bool    check_valid_ride ( CHAR_DATA *  ch );
char *  const	compass_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

char *  const   dir_name        []              =
{
    "to the North", "to the East", "to the South", "to the West", "Up", "Down"
};

char *  const   dir_name_2      []              =
{
    "the north", "the east", "the south", "the west", "above", "below"
};

/* rev_name used to indicate where ch has come from when entering -S- */

char * const    rev_name        []              =
{
   "the South", "the West", "the North", "the East", "Below", "Above"
};


const   sh_int  rev_dir         []              =
{
    2, 3, 0, 1, 5, 4
};

const   sh_int  movement_loss   [SECT_MAX]      =
{
    1, 3, 4, 2, 5, 2, 2, 1, 1, 1, 4, 1, 1,
};



/*
 * Local functions.
 */
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );
bool    has_key         args( ( CHAR_DATA *ch, int key ) );
/*
struct fol_data
{
  struct fol_data *next;
  CHAR_DATA *ch;
};

struct fol_data *fol_stack;
struct fol_data *fol_free;

*/

void move_char( CHAR_DATA *ch, int door )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    char buf[MAX_STRING_LENGTH];
    char move_buf[MAX_STRING_LENGTH];
    char tmp[MAX_STRING_LENGTH];
    char door_name_leave[MSL];
    OBJ_DATA *obj;
    BUILDING_DATA *bld = ch->in_building;
    int xx,yy,x,y,z=ch->z;
    bool from_bld = FALSE;
    int movea = 1;     	
	
    buf[0] = '\0';
    move_buf[0] = '\0';
    door_name_leave[0] = '\0';
    door_name_leave[0] = '\0';
    tmp[0] = '\0';

    if ( !ch || ch == NULL )
	return;
    if ( ch->c_sn == gsn_move )
	ch->c_sn = -1;
    if ( ch->in_vehicle )
    {
	if ( AIR_VEHICLE(ch->in_vehicle->type) && z != Z_AIR )
	{
		send_to_char( "@@gYou must @@elift@@g up in order to fly the aircraft.@@N\n\r", ch );
		return;
	}
	if (SPACE_VEHICLE(ch->in_vehicle->type) && z != Z_SPACE_EARTH )
	{
		send_to_char( "@@gYou must @@elift@@g up in order to fly the spaceship.@@N\n\r", ch );
		return;
	}
    }
    if ( IS_SET(ch->pcdata->pflags,PLR_ASS) )
    {
	move(ch,1,1,0);
	do_look(ch,"");
	return;
    }

    xx = ch->x;
    yy = ch->y;
    if ( door < 0 || door > 3 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }
    if ( IS_SET(ch->effect,EFFECT_CONFUSE) && number_percent() < 50 )
	door = number_range(0,3);
    
    if ( ch->carry_weight > can_carry_w( ch ) && !ch->in_vehicle )
    {
	send_to_char( "You are carrying too much weight!\n\r", ch);
	return;
    }
    if ( ch->carry_number > can_carry_n(ch) )
    {
	send_to_char( "You are carrying too many items.\n\r", ch );
	return;
    }

	if ( ch->in_building )
		from_bld = TRUE;
	if ( ch->in_vehicle )
	{
		if ( continual_flight(ch->in_vehicle) )
		{
			ch->c_sn = -1;
			ch->c_level = door;
		}
		else if ( IS_SET(ch->effect,EFFECT_DRUNK) && number_percent() < 33 )
		{
			send_to_char( "You swirve around in circles.. FUN!\n\r", ch );
			return;
		}
		if ( ch->c_sn == gsn_row )
		{
			ch->c_sn = -1;
			ch->in_vehicle->fuel = 1;
		}
		if ( ch->in_vehicle->fuel <= 0 )
		{
			send_to_char( "You are out of fuel.\n\r", ch );
			if ( AIR_VEHICLE(ch->in_vehicle->type) )
			{
				crash(ch,ch);
				return;
			}
			else if ( IS_SET(ch->in_vehicle->flags,VEHICLE_SWIM) )
			{
				ch->c_sn = gsn_row;
				ch->c_level = door;
				ch->c_time = 15 * PULSE_PER_SECOND;
				send_to_char( "You begin rowing with your hands!\n\r", ch );
			}
			return;
		}
	}
	{
		BUILDING_DATA *bld;
		bld = get_char_building(ch);
		if ( bld != NULL && bld && bld->exit[door] == FALSE && complete(bld) && ch->class != CLASS_PHASER)
		{
			send_to_char( "You cannot exit this way.\n\r", ch );
			if ( my_get_hours(ch,TRUE) == 0 )
				send_to_char( "@@WTIP: You can add more exits to buildings using the @@eMAKE@@W command. Example: make east@@N\n\r", ch );
			return;
		}
		
}

	if ( door == DIR_NORTH )
			ch->y += movea;
	else if ( door == DIR_EAST )
			ch->x += movea;
	else if ( door == DIR_SOUTH )
			ch->y -= movea;
	else if ( door == DIR_WEST )
			ch->x -= movea;
	else
		return;
	real_coords(&ch->x,&ch->y);

	bld = get_building(ch->x,ch->y,z);

	if ( ch->in_vehicle != NULL )
	{
		if ( !bld && (map_table.type[ch->x][ch->y][z] == SECT_WATER || map_table.type[ch->x][ch->y][z] == SECT_OCEAN) && !IS_SET(ch->in_vehicle->flags,VEHICLE_FLOATS) && !IS_SET(ch->in_vehicle->flags,VEHICLE_SWIM))
		{
			send_to_char( "You can't drive over water.\n\r", ch );
			ch->x = xx;
			ch->y = yy;
			return;
		}
	}
	if ((map_table.type[ch->x][ch->y][z] == SECT_NULL || map_table.type[ch->x][ch->y][z] == SECT_OCEAN || INVALID_COORDS(ch->x,ch->y) || ( bld && (!bld->active || (bld->protection > 0 && ch->in_building == NULL && str_cmp(bld->owned,ch->name) ) )) || ( bld && ch->in_vehicle != NULL && !from_bld && bld->type != BUILDING_GARAGE && bld->type != BUILDING_AIRFIELD && bld->type != BUILDING_SHIPYARD)))
	{
		bool cancel = FALSE;
		if ( ch->in_vehicle && AIR_VEHICLE(ch->in_vehicle->type) )
		{
			act( "You hit a wall!", ch, NULL, NULL, TO_CHAR );
			if ( z == Z_AIR )
				z = Z_GROUND;
			x = ch->x;
			y = ch->y;
			ch->x = xx;
			ch->y = yy;
			move(ch,x,y,z);
			crash(ch,ch);
			return;
		}
		else
		{
			if ( map_table.type[ch->x][ch->y][ch->z] == SECT_NULL || INVALID_COORDS(ch->x,ch->y) )
				send_to_char( "You cannot go that way.\n\r", ch );
			else if ( map_table.type[ch->x][ch->y][ch->z] == SECT_OCEAN )
			{
				if ( has_boat(ch) )
					cancel = TRUE;
				else
					send_to_char( "You need some way to cross the water!\n\r", ch );
			}
			else if ( bld && ch->in_vehicle != NULL )
				send_to_char( "You can't enter it while driving!\n\r", ch );
			else if ( bld && bld->protection > 0 )
				send_to_char( "This building is protected.\n\r", ch );
			else 
			{
				sprintf( buf, "You cannot enter %s's property while they are offline.\n\r", bld->owned );
				send_to_char(buf,ch);
			}
		}
		if ( !cancel )
		{
			ch->x = xx;
			ch->y = yy;
			return;
		}
	}
	if ( ch->in_vehicle )
		if ( IS_SET(ch->in_vehicle->flags,VEHICLE_SWIM) )
			if ( map_table.type[ch->x][ch->y][ch->z] != SECT_OCEAN && map_table.type[ch->x][ch->y][ch->z] != SECT_WATER 
			  && map_table.type[xx][yy][ch->z] != SECT_OCEAN && map_table.type[xx][yy][ch->z] != SECT_WATER
			)
			{
				send_to_char( "You cannot take it on land.\n\r", ch );
				ch->x = xx; ch->y = yy;
				return;
			}

	if ( bld )
	{
		int reverse = 1;
		if ( door == DIR_NORTH )
			reverse = DIR_SOUTH;
		if ( door == DIR_EAST )
			reverse = DIR_WEST;
		if ( door == DIR_SOUTH )
			reverse = DIR_NORTH;
		if ( door == DIR_WEST )
			reverse = DIR_EAST;

		if ( bld->exit[reverse] == FALSE && complete(bld) && ch->class != CLASS_PHASER)
		{
			send_to_char( "You cannot enter the building from that direction.\n", ch );
			ch->x = xx;
			ch->y = yy;
	
			return;
		}
	}
	if ( ch->in_vehicle )
	{
		if ( AIR_VEHICLE(ch->in_vehicle->type) )
			ch->in_vehicle->fuel--;
		else
			ch->in_vehicle->fuel -= wildmap_table[map_table.type[ch->x][ch->y][ch->z]].fuel;
		if ( IS_SET(ch->in_vehicle->flags,VEHICLE_EATS_FUEL) )
			ch->in_vehicle->fuel -= number_range(1,3);
	}
	for ( obj = ch->first_carry;obj;obj = obj->next_in_carry_list )
	{
		if ( obj->carried_by != ch )
			continue;
		obj->x = ch->x;
		obj->y = ch->y;
	}

	x = ch->x;
	y = ch->y;

	for ( fch = map_ch[xx][yy][z];fch;fch = fch_next )
	{
		fch_next = fch->next_in_room;
		if ( fch == ch )
			continue;
		if ( !can_see(fch,ch) )
			continue;
//		if ( NOT_IN_ROOM(ch,fch) )
//			continue;
		if ( fch->x != xx || fch->y != yy || fch->z != ch->z )
			continue;
		if ( ch->in_vehicle == NULL )
		{
			sprintf( move_buf, "%s moves away to %s.\n\r", ch->name, dir_name[door] );
		}
		else
		{
			if ( !AIR_VEHICLE(ch->in_vehicle->type))
				sprintf( move_buf, "%s drives away to %s.\n\r", ch->name, dir_name[door] );
			else
				sprintf( move_buf, "%s flies away to %s.\n\r", ch->name, dir_name[door] );
		}
		send_to_char( move_buf, fch );
	        if ( ( fch->leader == ch )  && !COMBAT_LAG(fch) && fch->position == POS_STANDING )
	        {
			if ( !ch->in_vehicle || !AIR_VEHICLE(ch->in_vehicle->type) )
			{
	           		act( "You follow $N $t.", fch, dir_name[door], ch, TO_CHAR );
	           		move_char( fch, door );
			}
		}
	}
	ch->x = xx;
	ch->y = yy;

	for ( fch = map_ch[x][y][z];fch;fch = fch->next_in_room )
	{
		if ( !can_see(fch,ch) || fch == ch )
			continue;
		if ( fch->x == x && fch->y == y && fch->z == ch->z )
		{
			if ( ch->in_vehicle == NULL )
			{
				sprintf( move_buf, "%s wanders in from %s.\n\r", ch->name, rev_name[door] );
			}
			else
			{
				if ( !AIR_VEHICLE(ch->in_vehicle->type))
					sprintf( move_buf, "%s drives in from %s.\n\r", ch->name, rev_name[door] );
				else
					sprintf( move_buf, "%s flies in from %s.\n\r", ch->name, rev_name[door] );
			}
			send_to_char( move_buf, fch );
		}
	}


	if ( ch )
	{
		move ( ch, x, y, ch->z );
		if ( !IS_SET(ch->effect,EFFECT_RUNNING) && !IS_SET(ch->config,CONFIG_CLIENT) )
			do_look( ch, "auto" );
		if ( map_table.type[x][y][ch->z] == SECT_ICE && !ch->in_vehicle )
		{
			ch->c_sn = gsn_move;
			ch->c_level = door;
			ch->c_time = 5;
		}
		ch->position = POS_STANDING;
    		if ( ch->in_vehicle )
			if ( ch->in_vehicle->fuel <= 15 )
				send_to_char( "\n\r@@eYou are low on fuel!!@@N\n\r", ch );
	}
    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    if ( ch->z == Z_AIR && ch->c_level != -1 && ch->in_vehicle && ch->in_vehicle->type != VEHICLE_CHINOOK )
    {
	ch->c_level = DIR_NORTH;
	return;
    }
    if (!COMBAT_LAG(ch) || paintball(ch) )
    {
	move_char(ch, DIR_NORTH);
	return;
    }
    ch->c_sn = gsn_move;
    ch->c_time = 8;
    if ( ch->in_vehicle )
    {
	ch->c_time -= ch->in_vehicle->speed;
    	if ( ch->class == CLASS_DRIVER )
		ch->c_time /= 1.5;
    }
    else if ( ch->class == CLASS_SPRINTER || ch->effect2/100==EFFECT2_SPEED )
	ch->c_time /= 1.5;
    if ( IS_SET(ch->effect,EFFECT_SLOW) || ch->poison / 100 == POISON_TRANQ )
	ch->c_time *= 2;
    ch->c_time += wildmap_table[map_table.type[ch->x][ch->y][ch->z]].speed;
    ch->c_level = DIR_NORTH;
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    if ( ch->z == Z_AIR && ch->c_level != -1 && ch->in_vehicle && ch->in_vehicle->type != VEHICLE_CHINOOK )
    {
	ch->c_level = DIR_EAST;
	return;
    }
    if ( !COMBAT_LAG(ch) || paintball(ch) )
    {
	move_char(ch, DIR_EAST);
	return;
    }
    ch->c_sn = gsn_move;
    ch->c_time = 8;
    if ( ch->in_vehicle )
    {
	ch->c_time -= ch->in_vehicle->speed;
    	if ( ch->class == CLASS_DRIVER )
		ch->c_time /= 1.5;
    }
    else if ( ch->class == CLASS_SPRINTER || ch->effect2/100==EFFECT2_SPEED )
	ch->c_time /= 1.5;
    if ( IS_SET(ch->effect,EFFECT_SLOW) || ch->poison / 100 == POISON_TRANQ )
	ch->c_time *= 2;
    ch->c_time += wildmap_table[map_table.type[ch->x][ch->y][ch->z]].speed;
    ch->c_level = DIR_EAST;
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    if ( ch->z == Z_AIR && ch->c_level != -1 && ch->in_vehicle && ch->in_vehicle->type != VEHICLE_CHINOOK )
    {
	ch->c_level = DIR_SOUTH;
	return;
    }
    if ( !COMBAT_LAG(ch) || paintball(ch) )
    {
	move_char(ch, DIR_SOUTH);
	return;
    }
    ch->c_sn = gsn_move;
    ch->c_time = 8;
    if ( ch->in_vehicle )
    {
	ch->c_time -= ch->in_vehicle->speed;
    	if ( ch->class == CLASS_DRIVER )
		ch->c_time /= 1.5;
    }
    else if ( ch->class == CLASS_SPRINTER || ch->effect2/100==EFFECT2_SPEED )
	ch->c_time /= 1.5;
    if ( IS_SET(ch->effect,EFFECT_SLOW) || ch->poison / 100 == POISON_TRANQ )
	ch->c_time *= 2;
    ch->c_time += wildmap_table[map_table.type[ch->x][ch->y][ch->z]].speed;
    ch->c_level = DIR_SOUTH;
    return;
}

void do_west( CHAR_DATA *ch, char *argument )
{
    if ( ch->z == Z_AIR && ch->c_level != -1 && ch->in_vehicle && ch->in_vehicle->type != VEHICLE_CHINOOK )
    {
	ch->c_level = DIR_WEST;
	return;
    }
    if (!COMBAT_LAG(ch) || paintball(ch) )
    {
	move_char(ch, DIR_WEST);
	return;
    }
    ch->c_sn = gsn_move;
    ch->c_time = 8;
    if ( ch->in_vehicle )
    {
	ch->c_time -= ch->in_vehicle->speed;
    	if ( ch->class == CLASS_DRIVER )
		ch->c_time /= 1.5;
    }
    else if ( ch->class == CLASS_SPRINTER || ch->effect2/100==EFFECT2_SPEED )
	ch->c_time /= 1.5;
    if ( IS_SET(ch->effect,EFFECT_SLOW) || ch->poison / 100 == POISON_TRANQ )
	ch->c_time *= 2;
    ch->c_time += wildmap_table[map_table.type[ch->x][ch->y][ch->z]].speed;
    ch->c_level = DIR_WEST;
    return;
}

void do_enter( CHAR_DATA *ch, char *argument )
{
	VEHICLE_DATA *vhc;
	char buf[MSL];

	if ( ( vhc = get_vehicle_char(ch,argument) ) == NULL )
	{
		if ( ( vhc = get_vehicle_from_vehicle( ch->in_vehicle ) ) == NULL )
		{
			send_to_char( "I don't see such vehicle here.\n\r", ch );
			return;
		}
	}
	if ( vhc->driving != NULL )
	{
		if ( vhc->driving == ch )
			send_to_char( "You are already inside.\n\r", ch );
		else
			send_to_char( "There is already someone inside!\n\r", ch );
		return;
	}
	if ( ch->victim != ch )
	{
		send_to_char( "You have lost track of your target.\n\r", ch );
		ch->victim = ch;
	}
	if ( ch->in_vehicle )
	{
		if ( TRANSPORT_VEHICLE(vhc->type) )
		{
			if ( AIR_VEHICLE(ch->in_vehicle->type) )
			{
				send_to_char( "Aircrafts cannot enter other vehicles.\n\r", ch );
				return;
			}
			if ( get_vehicle_from_vehicle(vhc) )
			{
				send_to_char( "There is already a vehicle inside.\n\r", ch );
				return;
			}
			sprintf( buf, "You drive %s into %s's hanger, and enter it.", ch->in_vehicle->desc, vhc->desc );
			act( buf, ch, NULL, NULL, TO_CHAR );
			sprintf( buf, "$n drives %s into %s's hanger, and enters it.", ch->in_vehicle->desc, vhc->desc );
			act( buf, ch, NULL, NULL, TO_ROOM );
			ch->in_vehicle->in_vehicle = vhc;
			vhc->vehicle_in = ch->in_vehicle;
			ch->in_vehicle->driving = NULL;
			ch->in_vehicle = vhc;
			ch->in_vehicle->driving = ch;
			return;
		}
		else
		{
			if ( vhc->in_vehicle == ch->in_vehicle )
			{
				if ( ch->in_vehicle->type == VEHICLE_CHINOOK )
				{
					if ( map_table.type[ch->x][ch->y][ch->z] == SECT_FIELD || map_table.type[ch->x][ch->y][ch->z] == SECT_ROCK || map_table.type[ch->x][ch->y][ch->z] == SECT_SNOW || map_table.type[ch->x][ch->y][ch->z] == SECT_SAND || ch->in_building )
					{
						act( "You land $t on the ground.", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
						act( "$n lands $t on the ground.", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
					}
					else
					{
						send_to_char( "You can't land it here!\n\r", ch );
						return;
					}
				}
				sprintf( buf, "You enter %s, and exit %s's hanger.", vhc->desc, ch->in_vehicle->desc );
				act( buf, ch, NULL, NULL, TO_CHAR );
				sprintf( buf, "$n exits %s's hanger in %s.", ch->in_vehicle->desc, vhc->desc );
				act( buf, ch, NULL, NULL, TO_ROOM );
				vhc->in_vehicle->vehicle_in = NULL;
				vhc->in_vehicle = NULL;
				vhc->driving = ch;
				ch->in_vehicle->driving = NULL;
				ch->in_vehicle = vhc;
				move_vehicle(ch->in_vehicle,ch->x,ch->y,ch->z);
				ch->in_vehicle->timer = 0;
				if ( AIR_VEHICLE(ch->in_vehicle->type) ) do_lift(ch,"");
				if ( SPACE_VEHICLE(ch->in_vehicle->type)) do_lift(ch, "");
			}
			else
				send_to_char( "You are already in a vehicle.\n\r", ch );
			return;
		}
	}
	sprintf( buf, "You climb into %s.\n\r", vhc->desc );
	send_to_char( buf, ch );
	sprintf( buf, "$n climbs into %s.", vhc->desc );
	act( buf, ch, NULL, NULL, TO_ROOM );
	if ( AIR_VEHICLE(vhc->type) )
		ch->c_level = -1;
	if ( vhc->fuel < 10 && AIR_VEHICLE(vhc->type) )
		send_to_char( "@@f@@eWARNING! Your aircraft is LOW on fuel! It will crash!@@N\n\r", ch );
	ch->in_vehicle = vhc;
	vhc->driving = ch;
	vhc->timer = 0;
	return;
}  
void do_exit( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];

	if ( ch->in_vehicle == NULL )
	{
		send_to_char( "You are not inside a vehicle.\n\r", ch );
		return;
	}
	if ( ch->in_vehicle->type == VEHICLE_CREEPER )
	{
		send_to_char( "You are locked in! (If you must go out, use the @@eDestroy@@N command)\n\r", ch );
		sprintf( buf, "%s attempted to get out of the creeper.", ch->name );
		monitor_chan( ch, buf, MONITOR_GEN_MORT);
		return;
	}
	if ( map_table.type[ch->x][ch->y][ch->z]==SECT_OCEAN && !IS_IMMORTAL(ch))
	{
		send_to_char("And what, walk on water?\n\r", ch );
		return;
	}
	if ( AIR_VEHICLE(ch->in_vehicle->type) && ch->z == Z_AIR && ( !ch->in_vehicle->in_building || ch->in_vehicle->in_building->type != BUILDING_AIRFIELD ) )
	{
		BUILDING_DATA *bld = map_bld[ch->x][ch->y][Z_GROUND];
		if ( bld && !bld->owner && 0 )
		{
		}
		else if ( bld )
		{
			move(ch,ch->x,ch->y,Z_GROUND);
			act( "You jump out of $t, and crash into a building! You hit the roof and get staked by metal bars.", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
			act( "$n crashes into the building!", ch, NULL, NULL, TO_ROOM );
			sprintf( buf, "%s is left without a pilot, and crashes on the ground!", ch->in_vehicle->desc );
			send_to_loc(buf,ch->x,ch->y,Z_AIR);
			send_to_loc(buf,ch->x,ch->y,Z_GROUND);
			ch->in_vehicle->driving = NULL;
			extract_vehicle(ch->in_vehicle,FALSE);
			ch->in_vehicle = NULL;
			if ( bld->owner )
				damage(bld->owner,ch,ch->hit*2+1000,DAMAGE_PSYCHIC);
			else
			{
				damage(ch,ch,ch->hit*2+1000,DAMAGE_PSYCHIC);
				return;
			}
			return;
		}
		else
		{
			move(ch,ch->x,ch->y,Z_GROUND);
			act( "You jump out of $t, and land safely on the ground!", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
			act( "$n jumps out of $t, and lands safely on the ground!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
		}
		sprintf( buf, "%s is left without a pilot, and crashes on the ground!", ch->in_vehicle->desc );
		send_to_loc(buf,ch->x,ch->y,Z_AIR);
		send_to_loc(buf,ch->x,ch->y,Z_GROUND);
		ch->in_vehicle->driving = NULL;
		extract_vehicle(ch->in_vehicle,FALSE);
		ch->in_vehicle = NULL;
		return;
	}
	if(SPACE_VEHICLE(ch->in_vehicle->type))
        {
    		if(ch->z != Z_GROUND)
		{
			if(get_obj_wear(ch,"spacesuit") == NULL)
			{
				send_to_char("You need a space suit before you can travel outside to space without a ship!\r\n", ch);
				return;
			} else {
				ch->in_vehicle->driving = NULL;
				ch->in_vehicle = NULL;
				return;
			}
		}
	}

	sprintf( buf, "You climb out of %s.\n\r", ch->in_vehicle->desc );
	send_to_char( buf, ch );
	sprintf( buf, "$n climbes out of %s.", ch->in_vehicle->desc );
	act( buf, ch, NULL, NULL, TO_ROOM );
	if ( ch->in_vehicle->type == VEHICLE_JEEP )
	{
		if ( !str_cmp(ch->in_vehicle->name, "Escape Pod") )
		{
			ch->in_vehicle->driving = NULL;
			extract_vehicle(ch->in_vehicle,TRUE);
			ch->in_vehicle = NULL;
			return;
		}
	}
	ch->in_vehicle->driving = NULL;
	ch->in_vehicle = NULL;
	ch->position = POS_STANDING;
	return;
}
void do_pit( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	char buf[MSL];

	if ( ch->fighttimer > 0 )
	{
		send_to_char( "You cannot enter or leave the pit yet! You are fighting!\n\r", ch);
		return;
	}
	if ( IN_PIT(ch) )
	{
		sprintf( buf, "%s has gone out of the pit!", ch->name );
		info( buf, 0 );
		ch->fighttimer += 200;
                if(map_bld[ch->homex][ch->homey][Z_GROUND] != NULL && map_bld[ch->homex][ch->homey][Z_GROUND]->owner == ch && map_bld[ch->homex][ch->homey][Z_GROUND]->type == BUILDING_HQ)
                {
                    act( "$n has gone out of the pit!", ch, NULL, NULL, TO_ROOM );
                    move ( ch, ch->homex, ch->homey, Z_GROUND );
                    act( "You have gone out of the pit!", ch, NULL, NULL, TO_CHAR );
                    act( "$n has gone out of the pit!", ch, NULL, NULL, TO_ROOM );
                    do_look(ch,"");
                    return;
                }
                send_to_char("@@eYour choosen \"home\" is not valid. Searching for a HQ.@@n\r\n", ch);
		for ( bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( bld->type == BUILDING_HQ )
			{
				act( "$n has gone out of the pit!", ch, NULL, NULL, TO_ROOM );
				move ( ch, bld->x, bld->y, bld->z );
				act( "You have gone out of the pit!", ch, NULL, NULL, TO_CHAR );
				act( "$n has gone out of the pit!", ch, NULL, NULL, TO_ROOM );
				do_look(ch,"");
				return;
			}
		}
		send_to_char( "You have no HQ! Setting default coordinates!\n\r", ch );

		int ranx = number_range(1, 1499);
                int rany = number_range(1, 1499);
                while (map_table.type[ranx][rany][Z_GROUND] == SECT_OCEAN && map_bld[ranx][rany][Z_GROUND] == NULL)
                {
                        ranx = number_range(1, 1499);
                        rany = number_range(1, 1499);
                }
		move ( ch, ranx, rany, Z_GROUND );
//		move ( ch, number_range(4,400), number_range(4,400), Z_GROUND );
		do_look(ch,"");
		return;
	}
	else
	{
		if ( ch->in_vehicle )
			mreturn("Get out of your vehicle first.\n\r", ch );
		if ( ch->z == Z_PAINTBALL )
		{
			send_to_char( "You cannot enter the pit from the game arena.\n\r", ch );
			return;
		}
		if ( ch->in_vehicle && ( AIR_VEHICLE(ch->in_vehicle->type) ))
		{
			send_to_char( "Heh, no.\n\r", ch );
			return;
		}
		for ( bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( bld->type == BUILDING_HQ )
			{
				sprintf( buf, "%s has entered the pit!", ch->name );
				info( buf, 0 );
				act( "$n has entered the pit!", ch, NULL, NULL, TO_ROOM );
				move( ch, number_range(PIT_BORDER_X+1,MAX_MAPS-2), number_range(PIT_BORDER_Y+1,MAX_MAPS-2), Z_PAINTBALL );
				act( "$n has entered the pit!", ch, NULL, NULL, TO_ROOM );
				act( "You have entered the pit!", ch, NULL, NULL, TO_CHAR );
				if ( !IS_IMMORTAL(ch) )
				ch->fighttimer = 360;
				do_look(ch,"");
				return;
			}
		}
		send_to_char( "You must have a headquarters in order to enter the pit!\n\r", ch );
		return;
	}
	return;
}

void do_paintball( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	OBJ_DATA *gun;
	char buf[MSL];

	if ( ch->fighttimer > 0  )
	{
		send_to_char( "You cannot enter or leave the arena yet! You are fighting!\n\r", ch);
		return;
	}
	if ( ch->in_vehicle )
	{
		send_to_char( "In the vehicle?\n\r", ch );
		return;
	}
	if ( paintball(ch) )
	{
		sprintf( buf, "%s has gone out of the paintball arena%s!", ch->name, (!str_cmp(argument,"idler"))?" @@bfor suspected idling@@g" : "" );
		info( buf, 0 );
		if ( ( gun = get_eq_char(ch,WEAR_HOLD_HAND_L) ) != NULL )
			extract_obj(gun);
		ch->fighttimer += 200;
		if(map_bld[ch->homex][ch->homey][Z_GROUND] != NULL && map_bld[ch->homex][ch->homey][Z_GROUND]->owner == ch && map_bld[ch->homex][ch->homey][Z_GROUND]-> type == BUILDING_HQ)
		{
                    act( "$n has gone out of the paintball arena!", ch, NULL, NULL, TO_ROOM );
                    move ( ch, ch->homex, ch->homey, Z_GROUND );
                    act( "You have gone out of the paintball arena!", ch, NULL, NULL, TO_CHAR );
                    act( "$n has gone out of the paintball arena!", ch, NULL, NULL, TO_ROOM );
                    do_look(ch,"");
                    return;
		}
		send_to_char("@@eYour choosen \"home\" is not valid. Searching for a HQ.@@n\r\n", ch);
		for ( bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( bld->owner != ch )
			{
				bld->owner = get_ch(bld->owned);
				continue;
			}
			if ( bld->type == BUILDING_HQ && complete(bld) )
			{
				act( "$n has gone out of the paintball arena!", ch, NULL, NULL, TO_ROOM );
				move ( ch, bld->x, bld->y, bld->z );
				act( "You have gone out of the paintball arena!", ch, NULL, NULL, TO_CHAR );
				act( "$n has gone out of the paintball arena!", ch, NULL, NULL, TO_ROOM );
				do_look(ch,"");
				return;
			}
		}
		send_to_char( "You have no HQ! Setting default coordinates!\n\r", ch );
		int ranx = number_range(1, 1499);
		int rany = number_range(1, 1499);
		while (map_table.type[ranx][rany][Z_GROUND] == SECT_OCEAN && map_bld[ranx][rany][Z_GROUND] == NULL)
		{
			ranx = number_range(1, 1499);
			rany = number_range(1, 1499);
		}
		move ( ch, ranx, rany, Z_GROUND );
//		move ( ch, PIT_BORDER_X -1, PIT_BORDER_Y - 1, Z_GROUND );
		do_look(ch,"");
		return;
	}
	else
	{
		extern bool paintlock;
		if ( ch->z == Z_PAINTBALL )
		{
			send_to_char( "You cannot enter from within the game arena.\n\r", ch );
			return;
		}
		if ( paintlock )
		{
			send_to_char( "The paintball arena is currently locked.\n\r", ch );
			return;
		}
		if ( ch->in_vehicle )
		{
			send_to_char( "Heh, no.\n\r", ch );
			return;
		}
		for ( bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( bld->type == BUILDING_HQ && complete(bld) )
			{
				for ( gun=ch->first_carry;gun;gun = gun->next_in_carry_list )
				{
					if ( gun->item_type == ITEM_BOMB && gun->value[1] != 0 )
						gun->value[1] = 0;
				}
				if ( ( gun = create_object(get_obj_index(OBJ_VNUM_PAINT_GUN),0) ) == NULL )
					return;
				if ( ( get_eq_char(ch,WEAR_HOLD_HAND_L) ) != NULL )
                			unequip_char(ch,get_eq_char(ch,WEAR_HOLD_HAND_L));
				obj_to_char(gun,ch);
		        	equip_char(ch,gun,WEAR_HOLD_HAND_L);
				sprintf( buf, "%s has entered the paintball arena!", ch->name );
				info( buf, 0 );
				act( "$n has entered the paintball arena!", ch, NULL, NULL, TO_ROOM );
				move( ch, number_range(200,300), number_range(200,300), Z_PAINTBALL );
				ch->c_count = 0;
				if ( ch->in_vehicle )
				{
					ch->in_vehicle->driving = NULL;
					ch->in_vehicle = NULL;
				}
				act( "$n has entered the paintball arena!", ch, NULL, NULL, TO_ROOM );
				act( "You have entered the paintball arena!", ch, NULL, NULL, TO_CHAR );
				do_look(ch,"");
				if ( !IS_IMMORTAL(ch) )
					ch->fighttimer = 360;
				save_char_obj(ch);
				return;
			}
		}
		send_to_char( "You must have a headquarters in order to enter the paintball arena!\n\r", ch );
		return;
	}
	return;
}

void crash( CHAR_DATA *ch, CHAR_DATA *attacker )
{
	VEHICLE_DATA *vhc;
	vhc = ch->in_vehicle;
	move(ch,ch->x,ch->y,Z_GROUND);
	act( "$t crashes!", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
	act( "$t crashes!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
	ch->hit = number_range(-2,1);
	ch->in_vehicle->driving = NULL;
	ch->in_vehicle = NULL;
	extract_vehicle(vhc,TRUE);
	if ( ch->hit <= 0 )
	{
		char buf[MSL];
			if ( ch != attacker )
			sprintf( buf, "@@a%s @@Wshot @@a%s@@W down from the sky.@@N", attacker->name, ch->name );
		else
			sprintf( buf, "@@a%s @@Wshould have never missed those flight-school lessons.@@N", ch->name );
		damage(attacker,ch,1000,-1);
		info ( buf, 0 );
		return;
	}
	send_to_char( "You have survived the crash!\n\r", ch );
	act( "$n has survived the crash!", ch, NULL, NULL, TO_ROOM );
	return;
}

void do_teleport (CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	BUILDING_DATA *bld;
	int range;
	int x,y,xx,yy;
	int c,m;
	BUILDING_DATA *bld2;

	if ( ch->in_building && ch->in_building->type == BUILDING_TELEPORTER && complete(ch->in_building) )
	{
		do_teleport_b(ch,argument);
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "You must specify the name of a teleporter device you want to use.\n\r", ch );
		return;
	}
	if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		send_to_char( "You do not carry that teleporter device.\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_TELEPORTER )
	{
		send_to_char( "That is not a teleporter.\n\r", ch );
		return;
	}
	if ( obj->value[2] > 0 )
	{
		char buf[MSL];
		sprintf(buf,"You must wait another estimated %d seconds for the teleporter to recharge.\n\r", obj->value[2]*(PULSE_OBJECTS/PULSE_PER_SECOND) );
		send_to_char(buf,ch);
		return;
	}
	range = obj->value[0];
	bld2 = NULL;
	m = -1;
	for ( xx=ch->x-range;xx<ch->x+range;xx++ )
	for ( yy=ch->y-range;yy<ch->y+range;yy++ )
	{
		x=xx;y=yy;real_coords(&x,&y);
		if ( ( bld = map_bld[x][y][ch->z] ) == NULL )
			continue;
		if ( bld->type != obj->value[1] || bld->owner != ch )
			continue;
		c = (abs(bld->x-ch->x)<abs(bld->y-ch->y))?abs(bld->x-ch->x):abs(bld->y-ch->y);
		if ( c < m || m == -1 ) 
		{
			bld2=bld;
			m = c;
		}
	}
	if ( !bld2 )
		mreturn("There is no building within range.\n\r",ch);

	bld = bld2;
	act( "$n presses a few buttons on $p, and vanishes!", ch, obj, NULL, TO_ROOM );
	act( "You press a few buttons on $p, and vanish!", ch, obj, NULL, TO_CHAR );
	move( ch, bld->x, bld->y, bld->z );
	if ( ch->in_vehicle )
	{
		ch->in_vehicle->driving = NULL;
		ch->in_vehicle = NULL;
	}
	act( "$n appears out of thin air!", ch, NULL, NULL, TO_ROOM );
	do_look(ch,"");
	obj->value[2] = 60;
	sendsound(ch,"telept",40,1,25,"misc","telept.wav"); 
	return;
}

void move( CHAR_DATA *ch, int x, int y, int z )
{
	CHAR_DATA *wch;

//	real_coords(&x,&y);
	if ( ch == map_ch[ch->x][ch->y][ch->z] )
	{
		map_ch[ch->x][ch->y][ch->z] = ch->next_in_room?ch->next_in_room:NULL;
	}
	else
		for ( wch = map_ch[ch->x][ch->y][ch->z];wch;wch = wch->next_in_room )
			if ( wch->next_in_room == ch )
				wch->next_in_room = ch->next_in_room;
	ch->x = x;
	ch->y = y;
	ch->z = z;
	ch->next_in_room = map_ch[ch->x][ch->y][ch->z];
	map_ch[ch->x][ch->y][ch->z] = ch;
	ch->in_building = map_bld[ch->x][ch->y][ch->z];
	if ( ch->position == POS_NUKEM )
		ch->position = POS_STANDING;
	if ( ch->victim != ch )
	{
		send_to_char( "You have lost track of your target.\n\r", ch );
		ch->victim = ch;
	}
	if ( ch->in_vehicle )
		move_vehicle(ch->in_vehicle,x,y,z); // ch->x = x;ch->y=y;ch->z=z;
	return;
}
void move_vehicle( VEHICLE_DATA *vhc, int x, int y, int z )
{
	VEHICLE_DATA *vhc2;

	real_coords(&x,&y);
	if ( vhc == map_vhc[vhc->x][vhc->y][vhc->z] )
	{
		map_vhc[vhc->x][vhc->y][vhc->z] = vhc->next_in_room;
	}
	else
		for ( vhc2 = map_vhc[vhc->x][vhc->y][vhc->z];vhc2;vhc2 = vhc2->next_in_room )
			if ( vhc2->next_in_room == vhc )
				vhc2->next_in_room = vhc->next_in_room;
	vhc->x = x;
	vhc->y = y;
	vhc->z = z;
	vhc->next_in_room = map_vhc[vhc->x][vhc->y][vhc->z];
	map_vhc[x][y][z] = vhc;
	if ( vhc->vehicle_in )
		move_vehicle(vhc->vehicle_in,x,y,z);
	return;
}
void move_obj( OBJ_DATA *obj, int x, int y, int z )
{
	extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];
	OBJ_DATA *obj2;

	real_coords(&x,&y);
	if ( obj->carried_by == NULL )
	{
		if ( obj == map_obj[obj->x][obj->y] )
		{
			map_obj[obj->x][obj->y] = obj->next_in_room;
//			if ( !obj->next_in_room )
//				free(map_obj[obj->x][obj->y]);
		}
		else
		{
			for ( obj2 = map_obj[obj->x][obj->y];obj2;obj2 = obj2->next_in_room )
			{
				if ( obj2->next_in_room == obj )
				{
					obj2->next_in_room = obj->next_in_room;
					break;
				}
			}
		}

		obj->x = x;
		obj->y = y;
		obj->z = z;
		obj->in_building = map_bld[obj->x][obj->y][obj->z];
		if ( obj->in_building && str_cmp(obj->in_building->owned,"nobody") && obj->item_type != ITEM_BOMB )
			{
				free_string(obj->owner);
				obj->owner = str_dup(obj->in_building->owned);
			}
		obj->next_in_room = map_obj[obj->x][obj->y];
		map_obj[obj->x][obj->y] = obj;
	}
	return;
}

void do_lift( CHAR_DATA *ch, char *argument )
{
	int type;

	if ( ch->z != Z_GROUND )
	{
		send_to_char( "You can't lift from here.\n\r", ch );
		return;
	}
	if ( ch->in_vehicle == NULL )
	{
		send_to_char( "You are not even in a vehicle!\n\r", ch );
		return;
	}
	if ( AIR_VEHICLE(ch->in_vehicle->type) )
	{
		type = 2;
	}
	else if (SPACE_VEHICLE(ch->in_vehicle->type))
	{
		act( "You power on the launch sequence and start to glide up.", ch, NULL, NULL, TO_CHAR );
		act( "$t activates, and glides up.", ch, ch->in_vehicle->desc, NULL, TO_ROOM);
		move(ch, ch->x, ch->y, Z_SPACE_EARTH);
		ch->in_building = NULL;
		ch->in_vehicle->in_building = NULL;
		ch->in_vehicle->z = ch->z;
		do_look(ch, "");
		return;
	} else {
		send_to_char( "You can't lift in that!\n\r", ch );
		return;
	}
	if ( ( type == 2 && ch->z == Z_AIR ) )
	{
		send_to_char( "You're already there.\n\r", ch );
		return;
	}
	act( "You power the thrusters, and glide up.", ch, NULL, NULL, TO_CHAR );
	act( "$t's thrusters activate, and it slowly glides up.", ch,ch->in_vehicle->desc,NULL, TO_ROOM);
	move(ch,ch->x,ch->y,Z_AIR);
	ch->in_building = NULL;
	ch->in_vehicle->in_building = NULL;
	ch->in_vehicle->z = ch->z;
	do_look(ch,"");
	return;
}
void do_land( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	VEHICLE_DATA * vhc = ch->in_vehicle;
	int x=1,y=1,type;

	if ( ch->z == Z_AIR )
		type = 2;
	else if(ch->z == Z_SPACE_EARTH && ch->in_vehicle != NULL) {
		if(map_bld[ch->x][ch->y][Z_GROUND] == NULL || map_bld[ch->x][ch->y][Z_GROUND]->type != BUILDING_SPACEYARD)
		{
			send_to_char("@@lYou need to land in a @@CSpaceyard@@l.\r\n", ch);
			return;
		}
		else if(map_bld[ch->x][ch->y][Z_GROUND]->active == FALSE)
		{
			send_to_char("This spaceyard belongs to an offline player.\r\n", ch);
			return;
		}
		move(ch, ch->x, ch->y, Z_GROUND);
		move_vehicle(ch->in_vehicle,ch->x, ch->y, Z_GROUND);
		send_to_char("You land safely.\n\r", ch);
		do_look(ch, "");
		return;
	} else if(ch->z == Z_SPACE_EARTH && get_obj_here(ch, "spacesuit") != NULL && ch->in_vehicle == NULL) {
                        send_to_char("\r\nGo back to your @@espaceship@@n! You'll @@eburn@@n up in the atmosphere.\r\n", ch);
                        return;
	} else if(ch->z == Z_SPACE_EARTH && ch->in_vehicle == NULL) {
//		move(ch, ch->x, ch->y, Z_GROUND);
		send_to_char("@@eYou have NOT landed safely, without a ship. Please tell an immortal how you got up there without a ship/spacesuit.@@n\r\n", ch);
		return;
	} else {
		send_to_char( ".. Land where?\n\r", ch );
		return;
	}
	if ( !ch->in_vehicle || ch->in_vehicle == NULL )
	{
		ch->z = Z_GROUND;
		return;
	}

	if ( type == 2 )
	{
		if ( !vhc || vhc->type != VEHICLE_CHINOOK )
		{
			if ( ( bld = get_building(ch->x,ch->y,Z_GROUND) ) == NULL || bld->type != BUILDING_AIRFIELD || !bld->active )
			{
				VEHICLE_DATA *vhc2;
				for ( vhc2=map_vhc[ch->x][ch->y][Z_GROUND];vhc2;vhc2 = vhc2->next_in_room )
				{
					if ( !vhc2->driving && vhc2->type == VEHICLE_CARRIER )
					{
						act( "You land aboard the aircraft carrier.", ch, NULL, NULL, TO_CHAR );
						act( "$t lands aboard the aircraft carrier.", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
						move(ch,ch->x,ch->y,Z_GROUND);
						ch->in_vehicle->in_vehicle = vhc2;
						vhc2->vehicle_in = ch->in_vehicle;
						ch->in_vehicle->driving = NULL;
						ch->in_vehicle = vhc2;
						ch->in_vehicle->driving = ch;
						ch->c_level = -1;
						return;
					}
				}
				send_to_char( "There is no airfield there.\n\rIf you must land now, you can try @@eEject@@Ning from the plane.", ch );
				return;
			}
/*			if ( !bld->owner || (bld->owner != ch && ( bld->owner->pcdata->alliance == -1 || 
bld->owner->pcdata->alliance != ch->pcdata->alliance ) ) )
			{
				send_to_char( "You do not have permission to land there.\n\r", ch );
				return;
			}*/
			act( "You turn the landing thrusters on, and lower your aircraft.", ch, NULL, NULL, TO_CHAR );
			act( "$t begins landing.", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
		}
		else
		{
			int sect = map_table.type[ch->x][ch->y][Z_GROUND];

			if ( ( bld = get_building(ch->x,ch->y,Z_GROUND) ) != NULL && bld->type != BUILDING_AIRFIELD )
			{
				send_to_char( "You can't land it on top of the building.\n\r", ch );
				return;
			}
			if ( bld && !bld->active ) mreturn("Not inside an offline building.\n\r", ch );
			if ( !bld && sect != SECT_FIELD && sect != SECT_SAND && sect != SECT_ROCK && sect != SECT_SNOW)
			{
				send_to_char( "You can't find a good landing spot in this area.\n\r", ch );
				return;
			}
			act( "You land the chinook.", ch, NULL, NULL, TO_CHAR );
			act( "$n lands $s chinook.", ch, NULL, NULL, TO_ROOM );
		}
		x = ch->x;
		y = ch->y;
		ch->c_level = -1;
	}
	move(ch,x,y,Z_GROUND);
	act( "$n has landed $s $t in the room.", ch, (ch->in_vehicle)?ch->in_vehicle->desc:"", NULL, TO_ROOM );
	if ( ch->in_vehicle && ch->in_vehicle->z != ch->z )
		move_vehicle(ch->in_vehicle,x,y,ch->z);
	return;
}

void do_teleport_b( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	BUILDING_DATA *bld2;
	char buf[MSL];
	int x = 1;

	bld = ch->in_building;
	if ( ch->in_vehicle )
	{
		send_to_char( "You can't teleport while driving.\n\r", ch );
		return;
	}
	if ( !bld->owner )
		return;
	if ( argument[0] == '\0' )
	{
		for ( bld2 = first_active_building;bld2;bld2 = bld2->next_active )
		{
			if ( bld2->type == BUILDING_TELEPORTER && bld2->owner && bld2->owner == bld->owner && bld2 != bld )
			{
				sprintf( buf, "@@a%d) @@c%s at %d/%d  %s\n\r", x, bld2->name, bld2->x, bld2->y, bld2->value[10] >= 1 ? "@@G(@@rPowered@@G)" : "@@R(@@ePowered Down@@R)" );
				send_to_char( buf, ch );
				x++;
			}
		}
		sprintf( buf, "\n\r@@NYou have %d battery packs stored in this teleporter.\n\r",bld->value[10] );
		send_to_char(buf,ch);
		send_to_char( "Select a teleporter number to teleport to.\n\r", ch );
		return;
	}
	x = atoi(argument);
	if ( x < 1 )
	{
		send_to_char( "Invalid teleporter number.\n\r", ch );
		return;
	}
	if ( bld->value[10] <= 0 )
	{
		send_to_char( "There are no more battery packs to power up the teleporter! Buy some more and try again.\n\r", ch );
		return;
	}
	for ( bld2 = first_active_building;bld2;bld2 = bld2->next_active )
	{
		if ( bld2->type == BUILDING_TELEPORTER && bld2->owner && bld2->owner == bld->owner && bld2 != bld )
		{
			if ( --x <= 0 )
			{
				act( "You teleport to another location!", ch, NULL, NULL, TO_CHAR );
				act( "$n teleports to another location!", ch, NULL, NULL, TO_ROOM );
				move(ch,bld2->x,bld2->y,bld2->z);
				act( "$n appears from the teleport portal.", ch, NULL, NULL, TO_ROOM );
				bld->value[10]--;
				return;
			}
		}
	}
	send_to_char( "Teleporter number not found.\n\r", ch );
	return;
}

void do_coords( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];
	char buf[MSL];
	int num,dir,x,y;
	argument = one_argument(argument,arg);

	if ( argument[0] == '\0' || ( dir = parse_direction(ch,argument) ) == -1 )
	{
		if ( IS_SET(ch->config,CONFIG_BLIND) )
		{
			sprintf( buf, "You are at %d/%d\n\r", ch->x, ch->y );
			send_to_char(buf, ch);
			return;
		}
		send_to_char( "Syntax: coords <number of rooms> <direction>\n\rYou have provided an invalid direction.\n\r", ch );
		return;
	}
	num = atoi(arg);
	if ( num <= 0 )
	{
		send_to_char( "Syntax: coords <number of rooms> <direction>\n\rYou have provided an invalid number of rooms.\n\r", ch );
		return;
	}
	if ( ch->z == Z_AIR )
	{
		send_to_char( "This would lead to: Somewhere!\n\r", ch );
		return;
	}

	x = ch->x;
	y = ch->y;
	if ( dir == DIR_NORTH )
		y += num;
	else if ( dir == DIR_SOUTH )
		y -= num;
	else if ( dir == DIR_WEST )
		x -= num;
	else if ( dir == DIR_EAST )
		x += num;

	sprintf( buf, "That would lead to: %d/%d\n\r", x,y);
	send_to_char(buf,ch);
	return;
}

bool has_boat(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch) )
		return TRUE;
	if ( ch->in_vehicle && (IS_SET(ch->in_vehicle->flags,VEHICLE_SWIM) || IS_SET(ch->in_vehicle->flags,VEHICLE_FLOATS) ))
		return TRUE;
/*	if ( my_get_minutes(ch,TRUE) < 10 )
	{
		send_to_char("\n\rFor the first 10 minutes, you are able to  walk across ocean terrain.\n\r", ch );
		return TRUE;
	}*/
	return FALSE;
}

void do_tunnel(CHAR_DATA *ch, char *argument)
{

  if(ch->in_vehicle != NULL)
  {
    send_to_char("Get out of the vehicle first!", ch);
    return;
  }

  if(map_bld[ch->x][ch->y][Z_GROUND] == NULL || map_bld[ch->x][ch->y][Z_GROUND]->type != BUILDING_TUNNEL)
  {
    send_to_char("You can't tunnel just anywhere! Find a tunnel building!\r\n", ch);
    return;
  }
  if(map_bld[ch->x][ch->y][Z_GROUND]->cost != 0)
  {
    send_to_char("Complete it first!\r\n", ch);
    return;
  } 
  if (map_bld[ch->x][ch->y][Z_GROUND]->active == FALSE)
  {
    send_to_char("You can't tunnel in offline players tunnels!\r\n", ch);
    return;
  } else {
    if(ch->z == Z_UNDER)
    {
      send_to_char("You start tunneling up.\r\n", ch);
      ch->c_sn = gsn_tunnel;
      ch->c_time = 6*PULSE_PER_SECOND;
      return;
    } else {
      send_to_char("You start tunelling down.\r\n", ch);
      ch->c_sn = gsn_tunnel;
      ch->c_time = 6*PULSE_PER_SECOND;
      return;
    }
  }

}
