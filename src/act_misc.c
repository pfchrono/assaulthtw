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
#include "mapper.h"
#include "tables.h"

void spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo );
CHAR_DATA *search_dir_name( CHAR_DATA *ch, char *argument, int direction, int range );
void game_in_play(CHAR_DATA *ch, char *name);
void warp( CHAR_DATA *ch, int range);
void sell_item( CHAR_DATA *ch, OBJ_DATA *obj );
void shop_item(CHAR_DATA *ch,char *name,int cost,char *desc);
void shop_title(CHAR_DATA *ch);

extern char * const dir_name[];  
extern char * const dir_name_2[];
extern char * const compass_name[];

bool can_build( int type, int sect, int planet )
{
	int i,sec;

	for ( i=0;i<MAX_BUILDON;i++ )
	{
		sec = build_table[type].buildon[i];
		if ( sec == sect )
			return TRUE;
	}

	return FALSE;
}
int leads_to( int x,int y,int z,int dir )
{
	if ( dir == DIR_NORTH )
		y++;
	else if ( dir == DIR_SOUTH )
		y--;
	else if ( dir == DIR_EAST )
		x++;
	else if ( dir == DIR_WEST )
		x--;
	real_coords(&x,&y);
	return map_table.type[x][y][z];
}
BUILDING_DATA * leads_to_b( int x,int y,int z,int dir )
{
	if ( dir == DIR_NORTH )
		y++;
	else if ( dir == DIR_SOUTH )
		y--;
	else if ( dir == DIR_EAST )
		x++;
	else if ( dir == DIR_WEST )
		x--;
	real_coords(&x,&y);
	return map_bld[x][y][z];
}
void move_coords( int *x,int *y,int dir )
{
	if ( dir == DIR_NORTH )
		*y=*y+1;
	else if ( dir == DIR_SOUTH )
		*y=*y-1;
	else if ( dir == DIR_EAST )
		*x=*x+1;
	else
		*x=*x-1;
	return;
}
int get_loc( char *loc )
{
	if ( !str_prefix(loc,"Head") )
	{
		if ( number_percent() < 70 )
			return WEAR_HEAD;
		else if ( number_percent() < 70 )
			return WEAR_FACE;
		else
			return WEAR_EYES;
	}
	else if ( !str_prefix(loc,"torso") )
	{
		if ( number_percent() < 50 )
			return WEAR_BODY;
		else
			return WEAR_SHOULDERS;
	}
	else if ( !str_prefix(loc,"arms") )
	{
		if ( number_percent() < 50 )
			return WEAR_ARMS;
		else if ( number_percent() < 50 )
			return WEAR_HOLD_HAND_L;
		else
			return WEAR_HOLD_HAND_R;
	}
	else if ( !str_prefix(loc,"legs") )
	{
		if ( number_percent() < 50 )
			return WEAR_LEGS;
		else if ( number_percent() < 50 )
			return WEAR_FEET;
		else
			return WEAR_WAIST;
	}
	return -1;
}
int parse_direction( CHAR_DATA *ch, char *arg )
{
    int door = -1;

         if ( !str_prefix( arg, "north" ) ) door = DIR_NORTH;
    else if ( !str_prefix( arg, "east"  ) ) door = DIR_EAST;
    else if ( !str_prefix( arg, "south" ) ) door = DIR_SOUTH;
    else if ( !str_prefix( arg, "west"  ) ) door = DIR_WEST;
//    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = DIR_UP;
//    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = DIR_DOWN;

    return door;
}

char *building_title[MAX_BUILDING_TYPES] =
{"Core","Superweapons","Defenses","Walls","Labs","Money","Unattached"};
void do_a_build( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	char buf1[MSL];
	char buf2[MSL];
	int i,x,y;
	int rank = get_rank(ch);
	bool found = FALSE;
	BUILDING_DATA *bld;
	BUILDING_DATA *bld2;
	sh_int built[MAX_BUILDING];
	bool build[MAX_BUILDING];
	int count[MAX_BUILDING];
	int buildings = 0;
	int lab = 0;
	sh_int ndist[MAX_BUILDING_TYPES];

	int bact[MAX_BUILDING_TYPES];
	bool tact[MAX_BUILDING_TYPES];
	bool ok;
	bool here = FALSE;
	extern int buildings_lists[MAX_BUILDING_TYPES][MAX_POSSIBLE_BUILDING];

	buf2[0] = '\0';

	for ( i = 0;i < MAX_BUILDING_TYPES;i++ )
	{
		bact[i] = 0;
		tact[i] = TRUE;
		ndist[i] = -1;
	}
	for ( i = 0;i<MAX_BUILDING;i++ )
	{
		if ( sysdata.killfest )
			built[i] = 10;
		else
			built[i] = 0;
		build[i] = FALSE;
		count[i] = 0;
	}
	if ( sysdata.killfest )
		for ( bld = ch->first_building;bld;bld = bld->next_owned )
			build[bld->type] = TRUE;

	for ( bld = ch->first_building;bld;bld = bld->next_owned )
	{
		if ( sysdata.killfest )
		{
			buildings++;
			continue;
		}
		{
			x = 0;
			y = 0;
			count[bld->type]++;

			if ( build_table[bld->type].requirements != 0 )
			{
				y = build_table[bld->type].requirements;
				if ( build_table[build_table[bld->type].requirements].requirements != 0 )
					x = build_table[build_table[bld->type].requirements].requirements;
			}
			if ( complete(bld) )
			{
				if ( bld->level > built[bld->type] )
					built[bld->type] = bld->level;
			}
			if ( built[BUILDING_BLASTER_TURRET] > built[BUILDING_L_TURRET] ) built[BUILDING_L_TURRET] = built[BUILDING_BLASTER_TURRET];
			if ( built[BUILDING_L_TURRET] > built[BUILDING_TURRET] ) built[BUILDING_TURRET] = built[BUILDING_L_TURRET];
			if ( built[BUILDING_TURRET] > built[BUILDING_S_TURRET] ) built[BUILDING_S_TURRET] = built[BUILDING_TURRET];
			if ( built[BUILDING_ICE_WALL] > built[BUILDING_ENERGY_WALL] ) built[BUILDING_ENERGY_WALL] = built[BUILDING_ICE_WALL];
			if ( built[BUILDING_ENERGY_WALL] > built[BUILDING_STRONG_WALL] ) built[BUILDING_STRONG_WALL] = built[BUILDING_ENERGY_WALL];
			if ( built[BUILDING_STRONG_WALL] > built[BUILDING_TALL_WALL] ) built[BUILDING_TALL_WALL] = built[BUILDING_STRONG_WALL];
			if ( built[BUILDING_TALL_WALL] > built[BUILDING_SHORT_WALL] ) built[BUILDING_SHORT_WALL] = built[BUILDING_TALL_WALL];

			if ( x > 0 )
				build[x] = TRUE;
			if ( y > 0 )
				build[y] = TRUE;
			build[bld->type] = TRUE;
			if ( build_table[bld->type].act == BUILDING_LAB )
			{
				sprintf( buf2+strlen(buf2), "%s (at %d/%d)\n\r", bld->name, bld->x, bld->y );
				lab = bld->type;
			}
			if ( build_table[x].act == BUILDING_LAB )
			{
				sprintf( buf2+strlen(buf2), "%s (at %d/%d)\n\r", bld->name, bld->x, bld->y );
				lab = x;
			}
			if ( build_table[y].act == BUILDING_LAB )
			{
				sprintf( buf2+strlen(buf2), "%s (at %d/%d)\n\r", bld->name, bld->x, bld->y );
				lab = y;
			}
			buildings++;
		}
	}
	if ( !built[BUILDING_HQ]>0 )
		for ( i = 0;i<MAX_BUILDING;i++ )
		{
			built[i] = 0;
			build[i] = FALSE;
		}

	built[0] = 1;
	ch->blimit=building_limits[count[BUILDING_HQ]];

	if ( !str_cmp(argument,"here") )
		here = TRUE;
//	else if ( !str_cmp(argument,"here all") )
//	{	here = TRUE; all = TRUE; }

	if ( argument[0] == '\0' || here )
	{
		char s_buf[MSL];

		sprintf(s_buf,"\n\r");
		if ( ( bld = get_building(ch->x,ch->y,ch->z) ) != NULL )
		{
			if ( bld->cost > 0 )
			{
				ch->c_sn = gsn_build;
				ch->c_time = 8;
				ch->c_level = 0;
				act( "You continue building the structure.", ch, NULL, NULL, TO_CHAR );
				act( "$n continues building the structure.", ch, NULL, NULL, TO_ROOM );
				return;
			}
		}

		if ( ch->pcdata->pagelen > MAX_BUILDING /3 && MAX_BUILDING > 90 )
		{
			ch->pcdata->o_pagelen = ch->pcdata->pagelen;
			ch->pcdata->pagelen = (MAX_BUILDING / 3)-5;
		}

		buf[0] = '\0';
		send_to_char("\n\r@@WYou have met the requirements for the following structures:\n\r\n\r", ch );
		if ( IS_SET(ch->config,CONFIG_BLIND) )
		{
			char buf0[MSL];
			char buf1[MSL];
			char buf2[MSL];
			char buf3[MSL];
			char buf4[MSL];
			char buf5[MSL];
			char buf6[MSL];
			char buf_x[MSL];
			bool disp[7];
			sh_int p[7];

			for ( i=0;i<7;i++ )
			{
				disp[i] = FALSE;
				p[i] = 0;
			}

			sprintf( buf0, "@@N\n\r*%s:\n\r\n\r", building_title[0] );
			sprintf( buf1, "\n\r*%s:\n\r\n\r", building_title[1] );
			sprintf( buf2, "\n\r*%s:\n\r\n\r", building_title[2] );
			sprintf( buf3, "\n\r*%s:\n\r\n\r", building_title[3] );
			sprintf( buf4, "\n\r*%s:\n\r\n\r", building_title[4] );
			sprintf( buf5, "\n\r*%s:\n\r\n\r", building_title[5] );
			sprintf( buf6, "\n\r*%s:\n\r\n\r", building_title[6] );

			for ( i=1;i<MAX_BUILDING;i++ )
			{
				if ( build_table[i].disabled )
					continue;
				if ( is_upgrade(i) )
					continue;
				if ( build_table[i].rank > rank )
					continue;
				if ( built[build_table[i].requirements]<build_table[i].requirements_l )
					continue;
				p[build_table[i].act]++;

				sprintf(buf_x, "%s%s", build_table[i].name,(p[build_table[i].act]==3)?"\n\r":", " );
				if ( p[build_table[i].act] == 3 )
					p[build_table[i].act] = 0;

				if ( build_table[i].act == 0 )
					safe_strcat(MSL,buf0,buf_x);
				if ( build_table[i].act == 1 )
					safe_strcat(MSL,buf1,buf_x);
				if ( build_table[i].act == 2 )
					safe_strcat(MSL,buf2,buf_x);
				if ( build_table[i].act == 3 )
					safe_strcat(MSL,buf3,buf_x);
				if ( build_table[i].act == 4 )
					safe_strcat(MSL,buf4,buf_x);
				if ( build_table[i].act == 5 )
					safe_strcat(MSL,buf5,buf_x);
				if ( build_table[i].act == 6 )
					safe_strcat(MSL,buf6,buf_x);
				if ( !disp[build_table[i].act] )
					disp[build_table[i].act] = TRUE;
			}

			if ( disp[0] )
				send_to_char(buf0,ch);
			if ( disp[1] )
				send_to_char(buf1,ch);
			if ( disp[2] )
				send_to_char(buf2,ch);
			if ( disp[3] )
				send_to_char(buf3,ch);
			if ( disp[4] )
				send_to_char(buf4,ch);
			if ( disp[5] )
				send_to_char(buf5,ch);
			if ( disp[6] )
				send_to_char(buf6,ch);
			return;
		}
		if ( ch->first_building == NULL && !sysdata.killfest )
		{
			sprintf(buf,"@@cHeadquarters@@N\n\r" );
			send_to_char(buf,ch);
			return;
		}
		else
		{
			int i,j=0;
			int x,y,z,xx=0,yy=0,zz=0;
			int sect = map_table.type[ch->x][ch->y][ch->z];
			bool go = TRUE;
			buf[0] = '\0';
			strcat(buf, "@@g----------------------------------------------------------------------\n\r" );
			sprintf(buf+strlen(buf),"@@g| @@d%-20s @@g| @@d%-20s @@g| @@d%-20s @@g|\n\r",building_title[j],building_title[j+1],building_title[j+2] );
			strcat(buf, "----------------------------------------------------------------------\n\r" );
			i=0;
			while (go)
			{
				x = -1;y = -1;z = -1;
				if ( j < MAX_BUILDING_TYPES )
				{
					while (x==-1 && buildings_lists[j][xx] != -1)
					{
						x = buildings_lists[j][xx];
						xx++;
						if ( !sysdata.killfest )
						if ( build_table[x].disabled || is_upgrade(x) || build_table[x].rank > rank || built[build_table[x].requirements]<build_table[x].requirements_l || (build_table[x].act == BUILDING_LAB && lab != 0 && lab != x) || (here && !can_build(x,sect,ch->z)) )
							x = -1;
					}
				}
	
				if ( j+1 < MAX_BUILDING_TYPES )
				{
					while (y==-1 && buildings_lists[j+1][yy] != -1)
					{
						y = buildings_lists[j+1][yy];
						yy++;
						if ( !sysdata.killfest )
						if ( build_table[y].disabled || is_upgrade(y) || build_table[y].rank > rank || built[build_table[y].requirements]<build_table[y].requirements_l || (build_table[y].act == BUILDING_LAB && lab != 0 && lab != y) || (here && !can_build(y,sect,ch->z)) )
							y = -1;
					}
				}
				if ( j+2 < MAX_BUILDING_TYPES )
				{
					while (z==-1 && buildings_lists[j+2][zz] != -1)
					{
						z = buildings_lists[j+2][zz];
						zz++;
						if ( !sysdata.killfest )
						if ( build_table[z].disabled || is_upgrade(z) || build_table[z].rank > rank || built[build_table[z].requirements]<build_table[z].requirements_l || (build_table[z].act == BUILDING_LAB && lab != 0 && lab != z) || (here && !can_build(z,sect,ch->z)) )
							z = -1;
					}
				}
	
				sprintf(buf+strlen(buf),"@@g| @@W%-20s @@g| @@W%-20s @@g| @@W%-20s @@g|\n\r",
				(x>0)?build_table[x].name:"",
				(y>0)?build_table[y].name:"",
				(z>0)?build_table[z].name:"" );
				if ( x <= 0 && y <= 0 && z <= 0 )
				{
					j += 3;
//					strcat(buf, "----------------------------------------------------------------------\n\r" );
					sprintf(buf+strlen(buf),"@@g| @@d%-20s @@g| @@d%-20s @@g| @@d%-20s @@g|\n\r",(j<MAX_BUILDING_TYPES)?building_title[j]:"",(j+1<MAX_BUILDING_TYPES)?building_title[j+1]:"",(j+2<MAX_BUILDING_TYPES)?building_title[j+2]:"" );
					strcat(buf, "----------------------------------------------------------------------\n\r" );
					xx = 0; yy = 0; zz = 0;
				}
				if ( j >= MAX_BUILDING_TYPES )
					go = FALSE;
				i++;
			}
			send_to_char(buf,ch);
			sprintf( buf, "\n\r@@cThe current building limit is @@a%d@@a.@@N\n\r", ch->blimit );
			send_to_char( buf, ch );
			return;
		}
	}
	else if ( !str_cmp(argument, "list") )
	{
		char u_buf[MSL];
		char s_buf[MSL];

		u_buf[0] = '\0';
		s_buf[0] = '\0';
		buf[0] = '\0';
		if ( ch->pcdata->pagelen > MAX_BUILDING / 3 && MAX_BUILDING > 90 )
		{
			ch->pcdata->o_pagelen = ch->pcdata->pagelen;
			ch->pcdata->pagelen = (MAX_BUILDING / 3)-5;
		}

		send_to_char( "\n\rName                    Cost\n\r" , ch );
		send_to_char( "--------------------------------------------------------------------\n\r", ch );
		for ( i = 1;i<MAX_BUILDING;i++ )
		{
			if ( build_table[i].disabled )
				continue;
			if ( build_table[i].rank > 9999 )
				continue;
			if ( is_upgrade(i) )
			{
				sprintf( u_buf+strlen(u_buf), "@@e%-24s\n\r", build_table[i].name );
			}
			else if ( build_table[i].rank > 0 )
			{
			 	if ( build_table[i].rank == 9999 )
				{
					sprintf( s_buf+strlen(s_buf), "@@c%-24s                @@lUnknown\n\r", build_table[i].name );
				}
				else
				{
					sprintf( s_buf+strlen(s_buf), "@@c%-24s                @@lRank %d\n\r", build_table[i].name, build_table[i].rank );
				}
			}
			else
			{
				sprintf( buf+strlen(buf), "@@a%-24s @@y%d", build_table[i].name, build_table[i].cost );
				sprintf( buf+strlen(buf), "\n\r" );
			}
			if ( i == MAX_BUILDING / 2 )
			{
				send_to_char( buf, ch );
				buf[0] = '\0';
			}
		}
		sprintf( buf, "%s\n\r%s\n\r%s", buf, s_buf, u_buf );
		send_to_char( buf, ch );
		sprintf( buf, "\n\r@@cThe current building limit is @@a%d@@a.@@N\n\r", ch->blimit );
		send_to_char( buf, ch );
		return;
	}
	else if ( !str_cmp(argument, "report") )
	{
		char ibuf[MSL];
		bool s1=FALSE,s2=FALSE;
		char buf2[MSL];

		sprintf( buf, "@@aCompleted Buildings:\n\r" );
		sprintf( ibuf, "@@cIncomplete Buildings:\n\r" );
		i = 0;
		buf2[0] = '\0';
		for (bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( IS_SET(bld->value[1],INST_SPOOF) )
				continue;
			i++;
			if ( !complete(bld) )
			{
				sprintf( buf1, "@@c%3d.@@d %-20s [%2d] %3d/%-3d%s", i, bld->name, bld->level, bld->x, bld->y, (!s1)?"        ":"\n\r" );
				safe_strcat(MSL,ibuf,buf1);
				s1 = !s1;
			}
			else
			{
				sprintf( buf1, "@@a%3d.@@W %-20s [%d] %3d/%-3d %3s%3s%s", i, bld->name, bld->level, bld->x, bld->y, (bld->value[8] != 0) ? "@@e(H)@@N" : (bld->password==0)?"@@y(H)@@N" : "", (bld->value[3] != 0)? "@@p(V)@@N" : "", (!s2)?" ":"\n\r"  );
				safe_strcat(MSL,(i/100==0)?buf:buf2,buf1);
				s2 = !s2;
			}
		}
		send_to_char(buf,ch);
		if ( buf2 != '\0' ) send_to_char(buf2,ch);
		send_to_char("\n\r",ch);
		if ( s2 )
			send_to_char("\n\r",ch);
		send_to_char(ibuf,ch);
		if ( s1 )
			send_to_char("\n\r",ch);
		sprintf( buf, "\n\r@@gTotal: %d\n\r", buildings );
		send_to_char( buf, ch );
		return;
	}
	else if ( !str_cmp(argument, "report2") )
	{
		char ibuf[MSL];
		sprintf( buf, "Completed Buildings:\n\r" );
		sprintf( ibuf, "Incomplete Buildings:\n\r" );
		i = 0;
		for (bld = ch->first_building;bld;bld = bld->next_owned )
		{
			if ( IS_SET(bld->value[1],INST_SPOOF) )
				continue;
			i++;
			if ( !complete(bld) )
				sprintf( ibuf+strlen(ibuf), "%d. %s Level %d at %d/%d %s%s\n\r", i, bld->name, bld->level, bld->x, bld->y, ( IS_SET(bld->value[1], INST_ANTIVIRUS ) ) ? "A" : "", ( IS_SET(bld->value[1], INST_FIREWALL ) ) ? "F" : "" );
			else
				sprintf( buf+strlen(buf), "%d. %s Level %d at %d/%d %s%s %s%s\n\r", i, bld->name, bld->level, bld->x, bld->y, (bld->value[8] != 0) ? " @@e(HACKER)@@N" : (bld->password==0)?" @@y(HACKED)@@N" : "", (bld->value[3] != 0)? " @@l(VIRUS)@@N" : "" , ( IS_SET(bld->value[1], INST_ANTIVIRUS ) ) ? "A" : "", (       IS_SET(bld->value[1], INST_FIREWALL ) ) ? "F" : "" );
		}
		send_to_char(buf,ch);
		send_to_char("\n\r",ch);
		send_to_char(ibuf,ch);
		sprintf( buf, "\n\rTotal: %d\n\r", buildings );
		send_to_char( buf, ch );
		return;

	}
	else if ( !str_cmp(argument, "summary") )
	{
		for (i=0;i<MAX_BUILDING;i++)
			built[i]=0;
		for ( bld=ch->first_building;bld;bld = bld->next_owned )
			built[bld->type]++;

		sprintf(buf,"Building Summary:\n\r\n\r" );
		for (i=0;i<MAX_BUILDING;i++)
			if ( built[i]>0 )
				sprintf(buf+strlen(buf),"%s: %d\n\r", build_table[i].name,built[i] );
		send_to_char(buf,ch);
		return;
	}
	if ( map_bld[ch->x][ch->y][ch->z] )
	{
		send_to_char( "There is already a building here.\n\r", ch );
		if ( IS_NEWBIE(ch))
			send_to_char( "@@WTIP:@@N You can only build one building per room. Try leaving this room by using the @@eN@@North, @@eS@@Nouth, @@eE@@Nast and @@eW@@Nest commands.\n\r", ch );
		return;
	}
	if ( ch->z == Z_PAINTBALL )
	{
		send_to_char( "You cannot build on this surface.\n\r", ch );
		return;
	}
	if ( ch->in_vehicle )
	{
		send_to_char( "You must exit the vehicle first.\n\r", ch );
		return;
	}

	if ( sysdata.killfest && buildings >= 200 )
		mreturn("You have hit the Killfest building limit! Wow...\n\r", ch );
	if ( buildings >= ch->blimit && ch->blimit != 0 )
	{
		send_to_char( "You have hit the building limit.\n\r", ch );
		return;
	}

	if ( !str_cmp(argument,"hq") )
		sprintf(argument,"headquarters");
	else if ( !str_cmp(argument,"greenhouse") )
		sprintf(argument,"lumberyard");

	for ( i=0;i<MAX_BUILDING;i++ )
	{
		if ( !str_prefix(argument, build_table[i].name) )
		{
			if ( is_upgrade(i) )
				continue;
			if ( build_table[i].disabled )
				continue;
			if ( sysdata.killfest )
			{
				found = TRUE;
				break;
			}
			if ( build_table[i].rank > rank )
				continue;
			if ( build_table[i].act == BUILDING_LAB && lab != 0 && lab != i )
				{
					send_to_char( "You can only have one lab.\n\r", ch );
					send_to_char( "Buildings you need to demolish:\n\r", ch );
					send_to_char(buf2,ch);
					return;
				}
			if ( count[i] >= build_table[i].max )
			{
				sprintf( buf, "You may only have %d of that building type.\n\r", build_table[i].max );
				send_to_char(buf,ch);
				return;
			}
			if (build_table[i].act == BUILDING_SUPERWEAPON && (build[BUILDING_SCUD_LAUNCHER] || build[BUILDING_NUKE_LAUNCHER]) )
			{
				send_to_char( "You can only have one superweapon.\n\r", ch );
				return;
			}

			if ( built[build_table[i].requirements] < build_table[i].requirements_l )
			{
				if ( build[BUILDING_HQ] == 0 )
				{
					send_to_char( "You must have a Headquarters in order to build anything.\n\r", ch );
					return;
				}
				send_to_char( "You have not met the requirements to construct that building.\n\r", ch );
				sprintf( buf, "Building requires: Level %d %s\n\r", build_table[i].requirements_l, build_table[build_table[i].requirements].name );
				send_to_char(buf,ch);
				return;
			}
			found = TRUE;
			break;
		}
	}
	if ( !found )
	{
		send_to_char( "No such structure.\n\r", ch );

		for ( i=0;i<MAX_BUILDING_TYPES;i++ )
			if ( !str_cmp(argument,building_title[i]) )
				send_to_char( "You've typed the name of a building @@ycatagory@@N, not a building @@ename@@N. Please select an actual building to make.\n\r", ch );

		if ( IS_NEWBIE(ch) )
			send_to_char( "\n\rType 'build', with no argument, to see what you can make.\n\r", ch );
		return;
	}
	if ( build_table[i].name == NULL )
		return;
	if ( i == BUILDING_HQ && !sysdata.killfest )
	{
		int x1,y1,xx1,yy1;
		BUILDING_DATA *bld1;
		if ( ch->first_building )
		{
			  if (((bld2 = leads_to_b(ch->x,ch->y,ch->z,DIR_EAST)) == NULL || bld2->owner != ch || build_table[bld2->type].act == BUILDING_UNATTACHED )
			  && ((bld2 = leads_to_b(ch->x,ch->y,ch->z,DIR_WEST)) == NULL || bld2->owner != ch || build_table[bld2->type].act == BUILDING_UNATTACHED )
			  && ((bld2 = leads_to_b(ch->x,ch->y,ch->z,DIR_NORTH)) == NULL || bld2->owner != ch || build_table[bld2->type].act == BUILDING_UNATTACHED )
			  && ((bld2 = leads_to_b(ch->x,ch->y,ch->z,DIR_SOUTH)) == NULL || bld2->owner != ch || build_table[bld2->type].act == BUILDING_UNATTACHED ) )
			{
				send_to_char( "You can't build a new Headquarters while you still have a base.\n\rTry building it next to one of your other buildings.\n\rIf you don't want to find all of your stuff now, you can always try @@eDemolish All@@N.\n\r", ch );
				return;
			}
		}
		for ( xx1=ch->x-10;xx1<ch->x+10;xx1++ )
		for ( yy1=ch->y-10;yy1<ch->y+10;yy1++ )
		{
			x1=xx1;y1=yy1; real_coords(&x1,&y1);
			if ( (bld1=map_bld[x1][y1][ch->z] ) == NULL )
				continue;
			if ( bld1->type == BUILDING_HQ )
				mreturn("You cannot build a Headquarters within 10 rooms of another Headquarters.\n\r", ch );
		}
	}
	else if ( i == BUILDING_SHIPYARD )
	{
		if ( leads_to(ch->x,ch->y,ch->z,DIR_NORTH) != SECT_OCEAN
		  && leads_to(ch->x,ch->y,ch->z,DIR_SOUTH) != SECT_OCEAN
		  && leads_to(ch->x,ch->y,ch->z,DIR_WEST) != SECT_OCEAN
		  && leads_to(ch->x,ch->y,ch->z,DIR_EAST) != SECT_OCEAN )
		{
			send_to_char( "You must build this next to an ocean.\n\r", ch );
			return;
		}
	}
		

	bld = create_building(i);
	if ( bld == NULL )
	{
		char buff[MSL];
		sprintf( buff, "ERROR! %d", i );
		send_to_char( buff, ch );
		return;
	}
	bld->x = ch->x;
	bld->y = ch->y;
	bld->z = ch->z;
	activate_building(bld,TRUE);
	map_bld[bld->x][bld->y][bld->z] = bld;
	bld->type = i;
	bld->maxhp = build_table[i].hp;
	bld->maxshield = build_table[i].shield;
	bld->hp = 1;
	bld->shield = 0;
	x = bld->x;
	y = bld->y;
	if ( bld->owned != NULL )
		free_string(bld->owned);
	if ( bld->name != NULL )
		free_string(bld->name);
	bld->owned = str_dup(ch->name);


	bld->owner = ch;
	if ( bld->owner->first_building )
        	bld->owner->first_building->prev_owned = bld;
        bld->next_owned = bld->owner->first_building;
        bld->owner->first_building = bld;


	bld->name = str_dup(build_table[i].name);
	if ( ch->class == CLASS_ENGINEER && number_percent() < 66 )
		bld->level++;

	for ( i=0;i<ch->pcdata->skill[gsn_building];i++ )
	{
		if ( bld->level >= MAX_BUILDING_LEVEL )
			break;
		if ( number_percent() < 50 ) continue;
		bld->level++;
	}
	if ( sysdata.killfest )
	{
		bld->hp = bld->maxhp;
		bld->shield = bld->maxshield;
		bld->level = 10;
	}

	if ( !can_build(bld->type,map_table.type[bld->x][bld->y][bld->z],bld->z) )
	{
		bool ter[SECT_MAX];
		int j,k;

		for ( j=0;j<SECT_MAX;j++ )
			ter[j] = FALSE;
		buf[0] = '\0';
		for ( j=0;j<MAX_BUILDON;j++ )
		{
			k = build_table[bld->type].buildon[j];
			if ( k < 0 || k >= SECT_MAX )
				continue;
			if ( ter[k] != TRUE )
				sprintf(buf+strlen(buf), "%s%s   ", wildmap_table[k].color, wildmap_table[k].name );
			
			ter[k] = TRUE;
		}
		k = map_table.type[ch->x][ch->y][ch->z];
		sprintf( buf+strlen(buf),"\n\r\n\r@@NYou are on %s%s @@Nsector.\n\r", wildmap_table[k].color, wildmap_table[k].name );
		send_to_char("This building cannot be built in this location.\n\rPossible Building locations are:\n\r", ch );
		send_to_char(buf,ch);
		extract_building(bld,FALSE);
		return;
	}
	ok = FALSE;
	y = bld->y+1;x=bld->x;real_coords(&x,&y);
	if ( ch->pcdata->set_exit == DIR_NORTH || ch->pcdata->set_exit == -1 || map_bld[x][y][bld->z] )
	{
		bld->exit[DIR_NORTH] = TRUE;
		if ( ( bld2 = map_bld[x][y][bld->z] ) != NULL && bld2->owner && bld2->owner == ch )
		{
			bld2->exit[DIR_SOUTH] = TRUE; 
			if (complete(bld2) && build_table[bld2->type].act != BUILDING_UNATTACHED && bld->tag) ok = TRUE;
		}
	}
	y = bld->y;x=bld->x+1;real_coords(&x,&y);
	if ( ch->pcdata->set_exit == DIR_EAST || ch->pcdata->set_exit == -1 || map_bld[x][y][bld->z] )
	{
		bld->exit[DIR_EAST] = TRUE;
		if ( ( bld2 = map_bld[x][y][bld->z] ) != NULL && bld2->owner && bld2->owner == ch )
		{	bld2->exit[DIR_WEST] = TRUE; 
			if (complete(bld2) && build_table[bld2->type].act != BUILDING_UNATTACHED && bld->tag) ok = TRUE; 
		}
	}
	y = bld->y-1;x=bld->x;real_coords(&x,&y);
	if ( ch->pcdata->set_exit == DIR_SOUTH || ch->pcdata->set_exit == -1 || map_bld[x][y][bld->z] )
	{
		bld->exit[DIR_SOUTH] = TRUE;
		if ( ( bld2 = map_bld[x][y][bld->z] ) != NULL && bld2->owner && bld2->owner == ch)
		{	bld2->exit[DIR_NORTH] = TRUE; 
			if (complete(bld2) && build_table[bld2->type].act != BUILDING_UNATTACHED && bld->tag) ok = TRUE;
		}
	}
	y = bld->y;x=bld->x-1;real_coords(&x,&y);
	if ( ch->pcdata->set_exit == DIR_WEST || ch->pcdata->set_exit == -1 || map_bld[x][y][bld->z] )
	{
		bld->exit[DIR_WEST] = TRUE;
		if ( ( bld2 = map_bld[x][y][bld->z] ) != NULL && bld2->owner && bld2->owner == ch )
		{	bld2->exit[DIR_EAST] = TRUE; 
			if (complete(bld2) && build_table[bld2->type].act != BUILDING_UNATTACHED && bld->tag) ok = TRUE; 
		}
	}
	if ( !ok && bld->type != BUILDING_HQ && build_table[bld->type].act != BUILDING_UNATTACHED && !sysdata.killfest )
	{
		send_to_char( "You must attach this building to another part of your base.\n\r", ch );
		extract_building(bld,FALSE);
		return;
	}

	if ( buildings + 8 > ch->blimit && ch->blimit > 0 )
		send_to_char( "@@eYou are nearing the building limit. See BUILD REPORT to see your building list!@@N\n\r", ch );
	send_to_char( "You have begun the construction of ", ch );
	send_to_char( build_table[bld->type].name, ch );
	send_to_char( "\n\r", ch );
	act( "$n starts building a structure.", ch, NULL, NULL, TO_ROOM );
	if ( sysdata.killfest )
		bld->cost = 0;
	if ( ch->pcdata->skill[gsn_building] > 0 )
	{
		if ( bld->cost > 0 )
			bld->cost -= (bld->cost * ch->pcdata->skill[gsn_building]) /100;
	}
	ch->c_sn = gsn_build;
	ch->c_time = 8;
	ch->c_level = 0;
	return;
}

void act_build( CHAR_DATA *ch, int level )
{
	bool found = FALSE;
	BUILDING_DATA *bld;
	char buf[MSL];

	if ( ( bld = get_char_building(ch) ) != NULL )
			found = TRUE;
	if ( !found )
	{
		ch->c_sn = -1;
		return;
	}

	if ( bld->cost > 0 || sysdata.killfest )
	{
		if ( ch->pcdata->reimb <= 0 && ch->money > 0 )
		{
			ch->money -= bld->cost;
			bld->cost = 0;
			if ( ch->money < 0 ) {
				bld->cost -= ch->money;
				ch->money = 0;
			}
			ch->c_time = 0;
			sprintf( buf, "You invest some money in the construction.\n\r" );
			send_to_char(buf,ch);
		}
		if ( complete(bld) || ch->pcdata->reimb > 0 )
		{
			if ( ch->pcdata->reimb > 0 )
				ch->pcdata->reimb--;
			send_to_char( "You finish the construction!\n\r", ch );
			bld->hp = bld->maxhp;
			bld->shield = bld->maxshield;
			reset_special_building(bld);
			act( "The building construction is complete.", ch, NULL, NULL, TO_ROOM );
			ch->c_sn = -1;
			if ( bld->type == BUILDING_HQ && IS_NEWBIE(ch) )
				send_to_char("@@WTIP: It is now time to build a @@eShop@@W to help earn you some money.\n\rYou need 4 shops overall for a good cashflow. Build them around your Headquarters.\n\r@@N", ch );
			if ( build_table[bld->type].act != BUILDING_UNATTACHED ) check_hq_connection(bld);
			bld = NULL;
			return;
		}

	}
	if ( bld->cost > 0 )
	{
		sprintf( buf, "You need $%d more to complete the construction.\n\r",  bld->cost );
		send_to_char(buf,ch);
		ch->c_sn = -1;
	}
	return;
}
void do_makeexit( CHAR_DATA *ch, char *argument )
{
	int dir,x,y;
	BUILDING_DATA *bld;
	BUILDING_DATA *bld2;
	bool all = FALSE;
	if ( ch->fighttimer > 0 )
	{
		send_to_char( "Not during combat.\n\r", ch );
		return;
	}
	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "You must be in a building.\n\r", ch );
		return;
	}
	if ( !bld->owner || bld->owner != ch )
	{
		send_to_char( "In your OWN building, please.\n\r", ch);
		return;
	}
	if ( argument[0] == '\0' )
	mreturn("Which direction do you want to open an exit at?\n\rExample: make n, make south, make all\n\r", ch );
	if ( ( dir = parse_direction(ch,argument) ) < 0 || dir > 3 )
	{
		if ( str_cmp(argument,"all") )
		{
			send_to_char( "Invalid direction.\n\r", ch );
			return;
		}
		else
			all = TRUE;
	}
	if ( all )
	{
		for ( dir=0;dir<4;dir++ )
		{
			bld->exit[dir] = TRUE;
			x=bld->x;y=bld->y; move_coords(&x,&y,dir);
			if (( bld2 = map_bld[x][y][ch->z]) != NULL && bld2->owner && bld2->owner == ch )
				bld2->exit[rev_dir[dir]] = TRUE;
		}
	}
	else
	{
		bld->exit[dir] = TRUE;
		x=bld->x;y=bld->y; move_coords(&x,&y,dir);
		if (( bld2 = map_bld[x][y][ch->z]) != NULL && bld2->owner && bld2->owner == ch )
			bld2->exit[rev_dir[dir]] = TRUE;
	}

	send_to_char( "Exit formed.\n\r", ch );
	return;
}
void do_closeexit( CHAR_DATA *ch, char *argument )
{
	int dir,x,y;
	int exits = 0;
	BUILDING_DATA *bld;
	BUILDING_DATA *bld2;
	if ( ch->fighttimer > 0 )
	{
		send_to_char( "Not during combat.\n\r", ch );
		return;
	}
	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "You must be in a building.\n\r", ch );
		return;
	}
	if ( str_cmp(bld->owned,ch->name) )
	{
		send_to_char( "In your OWN building, please.\n\r", ch);
		return;
	}
	for ( dir = 0;dir < 4;dir++ )
		if ( bld->exit[dir] == TRUE )
			exits++;
	if ( ( dir = parse_direction(ch,argument) ) < 0 || dir > 3 )
	{
		send_to_char( "Invalid direction.\n\r", ch );
		return;
	}
	if ( exits <= 1 )
	{
		send_to_char( "You can't lock yourself in!\n\r", ch);
		return;
	}
	x=bld->x;y=bld->y; move_coords(&x,&y,dir);
	if (( bld2 = map_bld[x][y][ch->z]) != NULL )
	{
		int i;
		exits = 0;
		for ( i=0;i<4;i++ )
			if ( bld2->exit[i] && i != rev_dir[dir] )
				exits++;
		if ( exits == 0 )
		{
			send_to_char( "This would cause the building in that direction to become closed off. You must open up at least one more exit there.\n\r", ch );
			return;
		}
		if ( bld2->owner && bld2->owner == ch )
			bld2->exit[rev_dir[dir]] = FALSE;
	}

	bld->exit[dir] = FALSE;
	send_to_char( "Exit closed.\n\r", ch );
	return;
}

void do_throw( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	OBJ_DATA *obj;
	char arg[MSL];
	int dir=-1,rev=0;
	char buf[MSL];
	CHAR_DATA *victim=NULL;
	int x,y;

	argument = one_argument(argument,arg);
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You do not carry that object.\n\r", ch );
		return;
	}
	if ( obj->value[4] != 1 )
	{
		send_to_char( "This is not a grenade!\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "You must specify a direction or player to throw at.\n\r", ch );
		return;
	}
	if ( ( victim = get_char_world(ch,argument) ) == NULL )
	{
		if ( ( dir = parse_direction(ch,argument) ) == -1 )
		{
			send_to_char( "Where do you want to throw the grenade to?\n\r", ch );
			return;
		}
	}
	if ( victim != NULL )
	{
		int range = 2;
		if ( ch->in_building && ch->in_building->type == BUILDING_SNIPER_TOWER )
			range += 3;
		if ( victim->x == ch->x && victim->y == ch->y )
		{
			send_to_char( "You are too scared to do that!\n\r", ch );
			return;
		}
		if ( abs(ch->x-victim->x) > range || abs(ch->y-victim->y) > range )
		{
			send_to_char( "They are too far.\n\r", ch );
			return;
		}
		if ( victim->y < 3 || victim->y >= MAX_MAPS - BORDER_SIZE || victim->x < 3 || victim->x >= MAX_MAPS - BORDER_SIZE || victim->z != ch->z )
		{
			send_to_char( "They are too far.\n\r", ch );
			return;
		}
		obj_from_char(obj);
		free_string(obj->owner);
		obj->owner = str_dup(ch->name);
		rev = -1;
		x = victim->x; y = victim->y;
		if ( number_percent() > ch->pcdata->skill[gsn_grenades] )
			x = number_range(x-1,x+1);
		if ( number_percent() > ch->pcdata->skill[gsn_grenades] )
			y = number_range(y-1,y+1);
		move_obj(obj,x,y,obj->z);
		sprintf( buf, "You throw %s as hard as you can towards %s!", obj->short_descr, victim->name);
		act( buf, ch, NULL, NULL, TO_CHAR );
		sprintf( buf, "$n throws %s towards %s!", obj->short_descr, victim->name );
		act( buf, ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		obj_from_char(obj);
		free_string(obj->owner);
		obj->owner = str_dup(ch->name);
		sprintf( buf, "You throw %s as hard as you can towards the %s!", obj->short_descr, compass_name[dir] );
		act( buf, ch, NULL, NULL, TO_CHAR );
		sprintf( buf, "$n throws %s towards the %s!", obj->short_descr, compass_name[dir] );
		act( buf, ch, NULL, NULL, TO_ROOM );
		if ( dir == DIR_NORTH )
		{
			obj->y++;
			rev = DIR_SOUTH;
		}
		else if ( dir == DIR_SOUTH )
		{
			obj->y--;
			rev = DIR_NORTH;
		}
		else if ( dir == DIR_EAST )
 		{
			obj->x++;
			rev = DIR_WEST;
		}
		else
		{
			obj->x--;
			rev = DIR_EAST;
		}
	}
	obj_to_room(obj,ch->in_room);
	if ( rev <= -1 )
		rev = number_range(0,3);
	if ( ( bld = get_obj_building(obj) ) != NULL )
	{
		if ( (bld->exit[rev] && number_percent() < 10) || (!bld->exit[rev] && number_percent() < 30))
		{
			act( "$p hits the wall, and rolls back!", ch, obj, NULL, TO_CHAR );
			act( "$p hits the wall, and rolls back!", ch, obj, NULL, TO_ROOM );
			move_obj(obj,ch->x,ch->y,ch->z);
		}
		else
			send_to_loc("A grenade rolls into the room!\n\r", obj->x,obj->y,obj->z );
	}
	else
		send_to_loc("A grenade rolls into the room!\n\r", obj->x,obj->y,obj->z );
	obj->value[1] = 1;
	obj->value[0] = 3;
	WAIT_STATE(ch,5);
	return;
}

void do_heal( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
        OBJ_DATA *obj;
        int heal;

	if ( ch->class == CLASS_MEDIC )
	{
		CHAR_DATA *victim;
		if ( argument[0] == '\0' )
		{
			victim = ch;
		}
		else
		{
			if ( ( victim = get_char_room(ch,argument) ) == NULL )
			{
				send_to_char( "You can't find them here.\n\r", ch );
				return;
			}
		}
		if ( victim->hit == victim->max_hit )
		{
			send_to_char( "There's no need to heal.\n\r", ch );
			return;
		}
		heal = ch->max_hit / 10;
		if ( victim != ch )
		{
			if ( ch->hit <= heal )
			{
				send_to_char( "You do not have enough HP to heal that person.\n\r", ch );
				return;
			}
			ch->hit -= heal;
			sprintf(buf,"@@gYou heal $N! @@W(@@a%d@@W)@@N", heal );
			act( buf, ch, NULL, victim, TO_CHAR );
			sprintf(buf,"@@g%s heals you! @@W(@@a%d@@W)@@N\n\r", ch->name,heal );
			send_to_char(buf,victim);
			sprintf(buf,"@@g%s heals %s!", ch->name,victim->name );
			act( buf, ch, NULL, victim, TO_NOTVICT );
		}
		else
		{
			sprintf(buf,"@@gYou heal yourself! @@W(@@a%d@@W)@@N", heal );
			send_to_char(buf,ch);
			act( "$n heals $mself.", ch, NULL, NULL, TO_ROOM );
		}
		victim->hit += heal;
		if ( victim->hit > victim->max_hit )
			victim->hit = victim->max_hit;
		sendsound(ch,"energy1",40,1,25,"misc","energy1.wav");
		WAIT_STATE(ch,16);
		return;
	}
        if ( ( obj = get_obj_carry(ch,  argument) ) == NULL )
	{
		send_to_char( "You are not carrying this Medpack! (Try typing ""heal medpack"")\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax: heal <medpack to use>\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_MEDPACK )
	{
		send_to_char( "This isn't a Medpack!\n\r", ch );
		return;
	}
	sprintf(buf,"You use %s to heal some of your injuries.", obj->short_descr);
	heal = number_range(obj->level*obj->value[0]/2, obj->level*obj->value[0]);
	if (obj->value[1] == 2)
		sprintf(buf+strlen(buf), "It was poisonous!!\n\r" );
	else
		sprintf(buf+strlen(buf), " @@a(@@c%d@@a)@@N\n\r", heal );
	send_to_char(buf,ch);
	act( "$n uses $p to heal $s injuries.", ch,obj,NULL,TO_ROOM);
	if ( obj->value[1] == 2 )
	{
		bool ex = TRUE;
		if ( heal > ch->hit )
			ex = FALSE;
		damage(ch,ch,heal,DAMAGE_PSYCHIC);
		if ( ex )
			extract_obj(obj);
		sendsound(ch,"energy1",40,1,25,"misc","energy1.wav");
		return;
	}
	ch->hit = URANGE(0,ch->hit + heal, ch->max_hit);
	WAIT_STATE(ch,28);
	extract_obj(obj);
	sendsound(ch,"energy1",40,1,25,"misc","energy1.wav");
	return;
}

void do_demolis( CHAR_DATA *ch, char *argument )
{
	send_to_char( "If you want to DEMOLISH your building, type the full word.\n\r", ch);
	return;
}
void do_demolish( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	BUILDING_DATA *bld2;
	bool check = TRUE;
	char arg[MSL];
	int r = 50,m;
	argument = one_argument(argument,arg);

	if ( ch->fighttimer > 0 )
		mreturn("Wait until combat lag is over.\n\r", ch );

	if ( ch->next_in_room || map_ch[ch->x][ch->y][ch->z] != ch )
		mreturn ("You can't demolish the building while other people are in it.\n\r", ch );
	if ( IS_NEWBIE(ch) )
		r = 100;
	else
		r += ch->pcdata->skill[gsn_economics] * 5;

	if ( !str_cmp(arg,"all") )
	{
		BUILDING_DATA *bld2_next;
		char buf[MSL];
		int i;
		int type=0;

		if ( ch->fighttimer > 0 )
		{
			send_to_char( "Not during combat.\n\r", ch );
			return;
		}

		if ( (bld=ch->in_building) ==NULL || build_table[bld->type].act != BUILDING_LAB )
		{
			if ( argument[0] == '\0' )
			{
				type = -1;
				sprintf ( buf, "Demolishing all buildings.\n\r" );
			}
			else
			{
				for ( i=1;i<MAX_BUILDING;i++ )
				{
					if ( !str_prefix(argument,build_table[i].name) )
					{
						type = i;
						sprintf( buf, "Demolishing all \"%s\"\n\r", build_table[i].name );
						break;
					}
				}
			}
			if ( type == 0 )
			{
				send_to_char( "Syntax: Demolish All\n\r        Demolish All <building name>\n\r", ch );
				return;
			}
			send_to_char(buf,ch);
			for ( bld2 = ch->first_building;bld2;bld2 = bld2_next )
			{
				bld2_next = bld2->next_owned;
	
				if ( bld2->value[3] != 0 )
					continue;
				if ( bld2 == ch->in_building || bld2->type == BUILDING_HQ )
					continue;
				if ( type == -1 || type == bld2->type )
				{
					m = ((((build_table[bld2->type].cost * (100-ch->pcdata->skill[gsn_building])) / 100) - bld2->cost) * r)/100;
					gain_money(ch,m);
					check_building_destroyed(bld2);
					extract_building(bld2,TRUE);
				}
			}
			if ( ch->first_building ) check_hq_connection(ch->first_building);
			send_to_char( "Your base has been demolished.\n\rYou must wait 20 seconds.\n\r", ch );
			WAIT_STATE(ch,20*8);
			return;
		}
		else if ( build_table[bld->type].act == BUILDING_LAB )
		{
			for ( bld2 = ch->first_building;bld2;bld2 = bld2_next )
			{
				bld2_next = bld2->next_owned;
				type = build_table[bld2->type].requirements;
				if ( bld2->value[3] != 0 ) continue;
				m = ((((build_table[bld2->type].cost * (100-ch->pcdata->skill[gsn_building])) / 100) - bld2->cost) * r)/100;
				if ( type > 0 )
				{
					if ( type == bld->type || build_table[type].requirements == bld->type )
					{
						gain_money(ch,m);
						check_building_destroyed(bld2);
						extract_building(bld2,TRUE);
						continue;
					}
				}
			}
			if ( ch->first_building ) check_hq_connection(ch->first_building);
			send_to_char( "Your lab buildings have been demolished. You must wait 20 seconds.\n\r", ch );
			WAIT_STATE(ch,20*8);
			return;
		}
		return;
	}
	if ( ( bld = get_char_building(ch) ) == NULL )
	{
		send_to_char( "You must be in a building.\n\r", ch );
		return;
	}
	if ( !bld->owner || bld->owner != ch )
	{
		send_to_char( "This isn't your building!\n\r", ch );
		return;
	}
	if ( !complete(bld) )
		check = FALSE;

        if ( bld->value[3] != 0 && bld->type != BUILDING_DUMMY )
        {
                CHAR_DATA *vch;
                if ( (vch= get_ch(bld->attacker) ) != NULL && vch != ch )
                {
			damage_building(vch,bld,bld->maxhp*2);
			if ( ch->first_building ) check_hq_connection(ch->first_building);
			return;
                } 
        }

	m = ((((build_table[bld->type].cost * (100-ch->pcdata->skill[gsn_building])) / 100) - bld->cost) * r)/100;
	gain_money(ch,m);
	act( "You enter a code, and the building collapses!", ch, NULL, NULL, TO_CHAR );
	act( "$n enters a code, and the building collapses!", ch, NULL, NULL, TO_ROOM );
	if ( check )
		check_building_destroyed(bld);
	extract_building(bld,TRUE);
	if ( ch->first_building ) check_hq_connection(ch->first_building);
	return;
}
void do_securit( CHAR_DATA *ch, char *argument )
{
	send_to_char( "You must type SECURITY to turn it on or off.\n\r", ch );
	return;
}
void do_security( CHAR_DATA *ch, char *argument )
{
	if ( ch->security )
	{
		send_to_char( "Security turned OFF!!!\n\r", ch);
		ch->security = FALSE;
	}
	else
	{
		send_to_char( "Security turned back on.\n\r", ch );
		ch->security = TRUE;
	}
	return;
}

void do_install( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	OBJ_DATA *obj;
	char buf[MSL];

	if ( ( bld = get_char_building(ch) ) == NULL )
	{
		send_to_char( "You must install something in a building.\n\r", ch);
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "This building has been installed with:\n\r\n\r", ch );
		if ( IS_SET(bld->value[1], INST_DEPLEATED_URANIUM ) ) 	send_to_char( "Depleated Uranium\n\r", ch );
		if ( IS_SET(bld->value[1], INST_ANTIVIRUS ) ) 		send_to_char( "Antivirus\n\r", ch );
		if ( IS_SET(bld->value[1], INST_FIREWALL ) ) 		send_to_char( "Firewall\n\r", ch );
		if ( IS_SET(bld->value[1], INST_ORGANIC_CORE ) ) 	send_to_char( "Organic Core\n\r", ch );
		if ( IS_SET(bld->value[1], INST_VIRAL_ENHANCER ) ) 	send_to_char( "Viral Enhancer\n\r", ch );
		if ( IS_SET(bld->value[1], INST_ACID_DEFENSE ) ) 	send_to_char( "Acid Defenses\n\r", ch );
		if ( IS_SET(bld->value[1], INST_ALIEN_HIDES ) ) 	send_to_char( "Spy Uplink\n\r", ch );
		
		sprintf(buf,"\n\r\n\rSecurity System:\n\r");
		if ( bld->security == 0 )
			sprintf(buf+strlen(buf),"Not Installed\n\r");
		else
			sprintf(buf+strlen(buf),"Level: %d %s\n\r", bld->security, (bld->security>5)?"(Stun)":"" );
		send_to_char(buf,ch);
		return;
	}
	if ( ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "You do not have that installation part.\n\r", ch );
		return;
	}

	if ( obj->item_type != ITEM_INSTALLATION )
	{
		send_to_char( "You can't install this.\n\r", ch );
		return;
	}
	if ( obj->value[1] != bld->type
	  && obj->value[2] != bld->type
	  && obj->value[3] != bld->type
	  && obj->value[4] != bld->type
	  && obj->value[1] != -1 )
	{
		send_to_char( "This upgrade cannot be installed here.\n\r", ch );
		return;
	}

	if ( !bld->owner ) return;
	if ( obj->value[0] == -1  ) // Internal Defenses
	{
		if ( obj->value[5] < 1 ) return;

		if ( bld->type == BUILDING_HQ )
		{
			send_to_char( "You cannot install security systems inside a Headquarters.\n\r", ch );
			return;
		}
		if (!build_table[bld->type].military ) mreturn("You can't install security kits in this building.\n\r",ch);
		if ( obj->value[5] <= bld->security )
		{
			send_to_char( "The security system installed here is better.\n\r", ch );
			return;
		}
		bld->security = obj->value[5];
		sprintf(buf, "You install %s in the building.\n\r", obj->short_descr);
		send_to_char(buf,ch);
		extract_obj(obj);
		return;
	}
	else if ( obj->value[0] == -2  ) // Ammo Change
	{
		if ( obj->value[5] < 1 || obj->value[5] > 10 )
			return;
		if ( build_table[bld->type].act != BUILDING_DEFENSE )
		{
			send_to_char( "This can only be installed in defensive turrets.\n\r", ch );
			return;
		}
		if (bld->value[10] == obj->value[5])
		{
			send_to_char( "This damage type is already installed.\n\r", ch );
			return;
		}
		if ( obj->value[5] == DAMAGE_PSYCHIC )
		{
			BUILDING_DATA *bld2;
			bool ok = FALSE;
			for ( bld2=bld->owner->first_building;bld2;bld2 = bld2->next_owned )
				if ( bld2->type == BUILDING_PSYCHIC_LAB )
				{
					ok = TRUE;
					break;
				}
			if ( !ok )
				mreturn("You must have a psychic lab to use this,\n\r", ch );
		}
		bld->value[10] = obj->value[5];
		sprintf(buf,"You attach the %s's systems to the %s.\n\rThis building will now do %s damage.\n\r", bld->name,obj->short_descr,dam_type[bld->value[10]]);
		send_to_char(buf,ch);
		extract_obj(obj);
		return;
	}
	else if ( obj->pIndexData->vnum == 1173 ) // Dr Norton
	{
		CHAR_DATA *bch = bld->owner;
		if ( !bch ) return;
		for ( bld = bch->first_building;bld;bld = bld->next_owned )
			bld->value[3] = 0;
		send_to_char( "Dr. Norton fixed your base right up!\n\r", ch );
		extract_obj(obj);
		return;
	}
	else if ( IS_SET(bld->value[1], obj->value[0]) )
	{
		send_to_char( "You have already installed this here.\n\r", ch );
		return;
	}

	sprintf( buf, "You install %s into %s's system!\n\r", obj->short_descr, bld->name );
	send_to_char( buf, ch );
	sprintf( buf, "$n installs %s into %s's system!\n\r", obj->short_descr, bld->name );
	act( buf, ch, NULL, NULL, TO_ROOM );
	SET_BIT( bld->value[1], obj->value[0] );
	extract_obj(obj);
	return;
}


void do_doom( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	CHAR_DATA *vch = NULL;
	char buf[MSL];
	int i = 0;

	if ( !ch->in_building )
		return;
	if ( ( bld = get_char_building(ch) ) == NULL || str_cmp(ch->name,bld->owned) || !complete(bld) || bld->type != BUILDING_HACKPORT )
	{
		send_to_char( "You must be in your hackport!\n\r", ch );
		return;
	}
	if ( bld->value[0] != 0 )
	{
		send_to_char( "It's not ready yet!\n\r", ch );
		return;
	}
	if ( bld->type == BUILDING_HACKPORT )
	{
		OBJ_DATA *obj;
		for ( obj = first_obj;obj;obj = obj->next )
			if ( obj->item_type == ITEM_COMPUTER )
			{
				if ( obj->in_building && obj->in_building->active == FALSE )
					continue;
				if ( obj->in_building && (obj->owner == NULL || str_cmp(obj->owner,obj->in_building->owned) ) )
				{
					free_string(obj->owner);
					obj->owner = str_dup(bld->owned);
				}
				if ( vch == NULL || str_cmp(vch->name,obj->owner) )
					vch = get_ch(obj->owner);
				if ( !vch || vch == NULL || IS_IMMORTAL(vch) )
					continue;
				if ( vch == ch || ( ch->pcdata->alliance != -1 && ch->pcdata->alliance == vch->pcdata->alliance ) )
					continue;
				obj->value[3] = 1;
				i++;
			}
		if ( i == 0 )
		{
			send_to_char( "Here's a tip... wait until some players with computers log on.\n\r", ch );
			return;
		}
		act( "You press a button, and begin transmitting the virus!", ch, NULL, NULL, TO_CHAR );
	}
	sprintf( buf, "Reports indicate of %d infections!\n\r", i );
	send_to_char( buf, ch );
	bld->value[0] = 1800;
	return;
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
	VEHICLE_DATA *vhc;
	char buf[MSL];

	if ( ( vhc = ch->in_vehicle ) == NULL )
	{
		send_to_char( "You must be in the vehicle you'd like to destroy.\n\r", ch );
		return;
	}
	if ( map_table.type[ch->x][ch->y][ch->z] == SECT_OCEAN )
	{
		send_to_char( "And what, be thrown into the ocean?\n\r", ch );
		return;
	}
	if ( ch->z == Z_AIR )
	{
		send_to_char( "Umm, that might not be such a good idea... You can try ejecting with the Exit command, though!\n\r", ch );
		return;
	}
	ch->in_vehicle = NULL;

	sprintf( buf, "%s bursts into flames!\n\r", vhc->desc );
	send_to_loc(buf,vhc->x,vhc->y,vhc->z);
	extract_vehicle(vhc,FALSE);
	return;
}

void do_winstall( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *weapon;
	char buf[MSL];

	if ( paintball(ch) )
		return;
	if ( ( weapon = get_eq_char(ch, WEAR_HOLD_HAND_L) ) == NULL )
	{
		send_to_char( "You must be holding the weapon in your left hand.\n\r", ch );
		return;
	}
	if ( weapon->item_type != ITEM_WEAPON )
	{
		send_to_char( "You are not holding a weapon in your left hand.\n\r", ch );
		return;
	}
	if ( weapon->pIndexData->vnum == 1152 )
	{
		send_to_char( "You can't install stuff on that.\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' || ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "Weapon-Install what?\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_WEAPON_UP )
	{
		send_to_char( "This is not a weapon upgrade.\n\r", ch );
		return;
	}
	if ( obj->value[0] == 0 )
	{
		if ( weapon->value[4] - 2 >= weapon->pIndexData->value[4] )
		{
			send_to_char( "This won't do anything.\n\r", ch );
			return;
		}
		weapon->value[4] += obj->value[1];
	}
	else if ( obj->value[0] == 1 )
	{
		if ( weapon->value[6] > obj->value[1] )
		{
			send_to_char( "You already have a better one installed.\n\r", ch );
			return;
		}
		weapon->value[6] = obj->value[1];
	}
	else if ( obj->value[0] == 2 )
	{
		if ( obj->value[2] == 0 )
		{
			if ( weapon->value[2] != 3 )
			{
				send_to_char( "This weapon does not fire rockets!\n\r", ch );
				return;
			}
			weapon->value[2] = 18;
			weapon->value[0] = 5;
			weapon->value[1] = 5;
			weapon->value[4] += 3;
			free_string(weapon->short_descr);
			free_string(weapon->description);
			free_string(weapon->name);
			weapon->short_descr = str_dup("@@WA @@RF@@ei@@Rr@@ee@@g-@@WRocket Launcher@@N");
			weapon->description = str_dup("@@WA @@RF@@ei@@Rr@@ee@@g-@@WRocket Launcher@@N");
			weapon->name = str_dup("fire rocket launcher");
		}
		else
		{
			if ( weapon->value[2] != 8 )
			{
				send_to_char( "You must install it on a short-ranged cookie launcher.\n\r", ch );
				return;
			}
			weapon->value[1] = 25;
			weapon->value[0] = 25;
			weapon->value[4] = 4;
			free_string(weapon->short_descr);
			free_string(weapon->description);
			free_string(weapon->name);
			weapon->short_descr = str_dup( "@@bA @@WG@@bi@@yng@@We@@yrb@@br@@We@@bad @@yB@@Wom@@bbe@@yr@@N" );
			weapon->description = str_dup( "@@bA @@WG@@bi@@yng@@We@@yrb@@br@@We@@bad @@yB@@Wom@@bbe@@yr@@N" );
			weapon->name = str_dup("gingerbread bomber ginger bread");
		}
	}
	else if ( obj->value[0] == 3 )
	{
		int bit = get_bit_value(obj->value[1]);
		if ( IS_SET(weapon->value[3],bit) )
		{
			send_to_char( "The weapon already has this effect.\n\r", ch );
			return;
		}
		SET_BIT(weapon->value[3],bit);
	}
	else if ( obj->value[0] == 4 )
	{
		if ( weapon->pIndexData && weapon->pIndexData->vnum < MAX_QP_OBJ )
			mreturn ("You can't install this on QP-bought objects.\n\r", ch );
		if ( IS_SET(weapon->extra_flags,ITEM_STICKY) )
		{
			send_to_char( "The weapon already has that effect.\n\r", ch );
			return;
		}
		SET_BIT(weapon->extra_flags,ITEM_STICKY);
		if ( !IS_SET(weapon->extra_flags,ITEM_NODROP) )
			SET_BIT(weapon->extra_flags,ITEM_NODROP);
	}
	else if ( obj->value[0] == 5 )
	{
		if ( weapon->value[7]+clip_table[weapon->value[2]].dam >= (clip_table[weapon->value[2]].dam+weapon->pIndexData->value[7]) * 1.1 )
		{
			send_to_char( "This won't do anything.\n\r", ch );
			return;
		}
		weapon->value[7] += (weapon->pIndexData->value[7]+clip_table[weapon->value[2]].dam) * 0.05;
	}
	else if ( obj->value[0] == 6 )
	{
		if ( weapon->value[11] > 0 )
			mreturn("You can only have one type of poison coating your weapon.\n\r", ch );
		weapon->value[11] = (obj->value[1]*100)+obj->value[2];
	}
	else
	{
		send_to_char( "Unknown installation value. Please contact an administrator.\n\r", ch );
		return;
	}

	sprintf( buf, "You install %s in %s.\n\r", obj->short_descr, weapon->short_descr );
	send_to_char( buf, ch );
	sprintf( buf, "$n installs %s in %s.", obj->short_descr, weapon->short_descr );
	act( buf, ch, NULL, NULL, TO_ROOM );
	extract_obj(obj);
	return;
}
void do_vinstall( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	VEHICLE_DATA *vhc;
	char buf[MSL];

	if ( ( vhc = ch->in_vehicle ) == NULL )
	{
		send_to_char( "You must be inside the vehicle you want to improve.\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' || ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		sprintf( buf, "Installations:\n\rWeapons: %d\n\r", vhc->range );
		send_to_char(buf,ch);
		return;
	}
	if ( obj->item_type != ITEM_VEHICLE_UP )
	{
		send_to_char( "This is not a vehicle addon.\n\r", ch );
		return;
	}
	if ( obj->value[0] == 1 )
	{
		if ( vhc->range >= obj->value[1] )
		{
			sprintf( buf, "Your ship already has a combat range of %d. This upgrade won't change anything.\n\r", vhc->range );
			send_to_char(buf,ch);
			return;
		}
		vhc->range = obj->value[1];
	}
	else if ( obj->value[0] == 2 )
	{
		int val = get_bit_value(obj->value[9]);
		if ( IS_SET(vhc->flags,val) )
		{
			send_to_char( "The craft is already implanted with the upgrade.\n\r", ch );
			return;
		}
		SET_BIT(vhc->flags,val);
	}
	else
	{
		send_to_char( "Unknown installation value. Please contact an administrator.\n\r", ch );
		return;
	}

	sprintf( buf, "You install %s in %s.\n\r", obj->short_descr, vhc->desc );
	send_to_char( buf, ch );
	sprintf( buf, "$n installs %s in %s.", obj->short_descr, vhc->desc );
	act( buf, ch, NULL, NULL, TO_ROOM );
	extract_obj(obj);
	return;
}


void do_connect( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];
	char buf[MSL];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	OBJ_DATA *obj3;
	int item = -1;
	bool ok = FALSE;

	argument = one_argument(argument,arg);
	if ( argument[0] == '\0' || arg[0] == '\0' )
	{
		send_to_char( "Syntax: connect <item1> <item2>\n\r", ch );
		return;
	}
	if ( ( obj1 = get_obj_carry(ch,arg) ) == NULL )
	{
		sprintf( buf, "You are not carrying %s.\n\r", arg );
		send_to_char( buf, ch );
		return;
	}
	if ( ( obj2 = get_obj_carry(ch,argument) ) == NULL )
	{
		sprintf( buf, "You are not carrying %s.\n\r", argument );
		send_to_char( buf, ch );
		return;
	}
	if ( obj1->item_type != ITEM_PIECE || obj2->item_type != ITEM_PIECE )
	{
		send_to_char( "Both items must be pieces of another.\n\r", ch );
		return;
	}
	if ( obj1->value[0] == obj2->pIndexData->vnum )
	{
		ok = TRUE;
		item = obj1->value[1];
	}
	else if ( obj1->value[2] == obj2->pIndexData->vnum )
	{
		ok = TRUE;
		item = obj1->value[3];
	}
	if ( !ok )
	{
		send_to_char( "These pieces won't connect in that order.\n\r", ch );
		return;
	}
	if ( ( obj3 = create_object(get_obj_index(item),0) ) == NULL )
	{
		send_to_char( "System error! Target object not found!\n\r", ch );
		return;
	}
	sprintf( buf, "You connect %s and %s - They become %s!\n\r", obj1->short_descr, obj2->short_descr, obj3->short_descr );
	send_to_char( buf, ch );
	obj_to_char(obj3,ch);
	extract_obj(obj1);
	extract_obj(obj2);
	return;
}
void do_track( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	OBJ_DATA *obj;
	CHAR_DATA *wch;
	char buf[MSL];

	if ( ( bld = ch->in_building ) == NULL || bld->type != BUILDING_HACKERS_HIDEOUT )
	{
		send_to_char( "You must be at a hackers hideout!\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		bool found = FALSE;
//		send_to_char( "Track whom?\n\r", ch );
		sprintf( buf, "Tracable Targets:\n\r\n\r" );
		for ( wch = first_char;wch;wch = wch->next )
		{
                        for ( obj = wch->first_carry;obj;obj = obj->next_in_carry_list )
                        {
                                if ( obj->item_type == ITEM_COMPUTER && obj->value[3] != 0 )
                                {
                                        found = TRUE;
					sprintf( buf+strlen(buf), "%s\n\r", wch->name );
					break;
                                }
                        }
		}
		send_to_char( buf, ch );
		if ( !found )
			send_to_char( "No tracable targets detected.\n\r", ch );

		return;
	}
	if ( ( wch = get_char_world(ch,argument) ) == NULL )
	{
		send_to_char( "You can't find that target.\n\r", ch );
		return;
	}
	for ( obj = wch->first_carry;obj;obj = obj->next_in_carry_list )
	{
		if ( obj->item_type == ITEM_COMPUTER && obj->value[3] != 0 )
		{
			if ( IS_SET(wch->effect,EFFECT_ENCRYPTION) )
			{
				send_to_char( "You just can't seem to lock on that target...\n\r", ch );
				return;
			}
			sprintf( buf, "\n\r@@cYou detect your target at @@a%d@@c,@@a%d!@@N\n\r", wch->x, wch->y );
			send_to_char( buf, ch );
			return;
		}
	}
	send_to_char( "Your target was not carrying an infected computer.\n\r", ch );
	return;
}


void do_spy( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];
	BUILDING_DATA *bld;

	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "You must be at a ready spy quarters or satellite.\n\r", ch );
		return;
	}
	if ( (bld->type != BUILDING_SPY_QUARTERS && bld->type != BUILDING_SPY_SATELLITE && bld->type != BUILDING_SHOCKWAVE ) || bld->value[0] > 0 || !complete(bld) )
	{
		send_to_char( "You must be at a ready spy quarters, satellite or shockwave.\n\r", ch );
		return;
	}
	if ( str_cmp(bld->owned,ch->name) )
	{
		send_to_char( "This isn't your building!\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "Spy after whom, or where?\n\r", ch );
		return;
	}
	if ( bld->type == BUILDING_SPY_QUARTERS )
	{
		CHAR_DATA *wch;
		int x=0;
		int b=0;
		int i;
		BUILDING_DATA *bld2;
		char buf[MSL];
		int buildings[MAX_BUILDING];
		OBJ_DATA *obj;

		if ( ( wch = get_char_world(ch,argument) ) == NULL || IS_NEWBIE(wch) || IS_IMMORTAL(wch) )
		{
			send_to_char( "You can't find that target.\n\r", ch );
			return;
		}
		if ( IS_SET(wch->effect,EFFECT_EVADE) )
		{
			i = number_range(0,5);
			for ( ;i>0;i-- )
			{
				if ( wch->next )
					wch = wch->next;
				else
					wch = first_char;
				if ( IS_NEWBIE(wch) || IS_IMMORTAL(wch) )
					i++;
				if ( ++x > 20 ) mreturn("You can't find that target.\n\r", ch );
			}
		}
		x = 0;
		for ( i = 0;i<MAX_BUILDING;i++ )
			buildings[i] = 0;
		act( "You send your spies on a mission.\n\r", ch, NULL, NULL, TO_CHAR );
		for ( bld2=first_active_building;bld2;bld2 = bld2->next_active )
			if ( bld2->owner == wch )
			{
				buildings[bld2->type]++;
				if ( defense_building(bld2) )
					x += bld2->level;
				if ( bld->level >= 4 && bld2->type == BUILDING_HQ )
				{
					obj = create_object(get_obj_index(1002),0);
					obj->x = bld2->x;
					obj->y = bld2->y;
					obj->z = bld2->z;
					obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
					obj->value[1] = 1;
					obj->value[0] = 120;
					free_string(obj->owner);
					obj->owner = str_dup(ch->name);
					b++;
				}
			}
		x = number_range(x/1.1,x*1.1);
		sprintf( buf, "Your spies report: We estimate %s's defense rating is at %d.\n\r", wch->name, x );
		if ( bld->level >= 4 )
			sprintf( buf+strlen(buf), "%d bombs have been planted in headquarters.\n\r", b );
		act( buf, ch, NULL, NULL, TO_CHAR );
		if ( bld->level >= 2 && wch->z != Z_PAINTBALL && !wch->fake )
		{
			sprintf( buf, "They have also discovered %s residing at %d/%d.\n\r", wch->name,wch->x,wch->y );
			send_to_char(buf,ch);
		}

		if ( bld->level >= 5 )
		{
			send_to_char( "\n\rOur spies have delivered a more detailed report of the enemy's base:\n\r", ch );
			for ( i = 0;i<MAX_BUILDING;i++ )
			{
				if ( buildings[i] == 0 )
					continue;
				sprintf( buf, "@@G%d @@r%s\n\r", buildings[i], build_table[i].name );
				send_to_char(buf,ch);
			}
		}
	}
	else if ( bld->type == BUILDING_SHOCKWAVE )
	{
		CHAR_DATA *wch;
		BUILDING_DATA *bld2;
		char buf[MSL];
		int x = 0;

		if ( ( wch = get_char_world(ch,argument) ) == NULL || IS_NEWBIE(wch) )
		{
			send_to_char( "You can't find that target.\n\r", ch );
			return;
		}
		act( "You transmit a shockwave at your target location!\n\r", ch, NULL, NULL, TO_CHAR );
		for ( bld2=wch->first_building;bld2;bld2 = bld2->next_owned )
		{
			if ( bld2->type != BUILDING_DUMMY )
				continue;
			if ( !str_cmp(bld2->owned,wch->name) )
			{
				bld2->value[5] = 1;
				x++;
			}
		}
		sprintf( buf, "@@gReports indicate that @@W%d@@g dummies were disabled.\n\r", x );
		send_to_char(buf,ch);
		bld->value[0] = 540;
		return;
	}
	else if ( bld->type == BUILDING_SPY_SATELLITE )
	{
		int x,y,zz,xx,yy,map;
		argument = one_argument(argument,arg);
		if ( !is_number(arg) || !is_number(argument) || INVALID_COORDS(atoi(arg),atoi(argument)) )
		{
			send_to_char( "Invalid coords.\n\r", ch );
			return;
		}
		xx = ch->x;
		yy = ch->y;
		zz = ch->z;
		map = ch->map;
		x = atoi(arg);
		y = atoi(argument);
		move(ch,x,y,zz);
		ch->map = (bld->level/3)+5;
               	ShowWMap( ch, IS_SET( ch->config, CONFIG_SMALLMAP)?2:IS_SET(ch->config,CONFIG_TINYMAP)?1:4, 997 );
		move(ch,xx,yy,zz);
		ch->map = map;
	}
	bld->value[0] = 360;
	if ( ch->fighttimer < 40 ) ch->fighttimer = 40;
	return;
}

void do_torment( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	BUILDING_DATA *bld;
	char arg[MSL];
	CHAR_DATA *victim;
	int action;
	int x,y,xx,yy,range;

	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "You must be at a psychic tormentor!\n\r", ch );
		return;
	}
	if ( !complete(bld) || str_cmp(bld->owned,ch->name) || bld->value[0] != 0 || bld->type != BUILDING_PSYCHIC_TORMENTOR )
	{
		send_to_char( "You can't use this building.\n\r", ch );
		return;
	}
	argument = one_argument(argument,arg);
	if ( argument[0] == '\0' || arg[0] == '\0')
	{
		send_to_char( "Torment <player> <demolish/arm/fire/destroy,run x direction>\n\r", ch );
		return;
	}
	action = (!str_cmp(argument,"demolish") ) ? 1 : (!str_cmp(argument,"arm") ) ? 2 : (!str_cmp(argument,"fire") ) ? 3 : (!str_prefix("run ",argument)) ? 4 : (!str_cmp(argument,"destroy"))?5:-1;
	if ( action == -1 )
	{
		send_to_char( "Invalid action (demolish/arm/fire/destroy/run x direction).\n\r", ch );
		return;
	}
	if ( ( victim = get_char_world(ch,arg) ) == NULL )
	{
		send_to_char( "You can't find that target.\n\r", ch );
		return;
	}
	if ( IS_NEWBIE(victim) || IS_IMMORTAL(victim) )
	{
		send_to_char( "You can't target that player.\n\r", ch );
		return;
	}
	if ( victim == ch )
	{
		send_to_char( "Ummm... can't you just type the command yourself?\n\r", ch );
		return;
	}
	if ( action == 1 && victim->in_building && victim->in_building->type == BUILDING_HQ )
	{
		send_to_char("Nothing seems to happen!\n\r", ch );
		return;
	}
	bld->value[0] = 360;
/*	if ( ( bld = victim->in_building) != NULL && bld->type == BUILDING_PSYCHIC_SHIELD )
	{
		send_to_char( "The psychic shield begins to glow!\n\r", victim );
		victim = ch;
	}*/
	range = 4;
	for ( xx = victim->x-range;xx<=victim->x + range;xx++ )
		for ( yy = victim->y-range;yy<=victim->y+range;yy++ )
		{
			x=xx;y=yy;real_coords(&x,&y);
			if ( map_bld[x][y][victim->z] && map_bld[x][y][victim->z]->type == BUILDING_PSYCHIC_SHIELD )
			{
				victim = ch;
			}
		}
	if ( (obj=get_eq_char(victim,WEAR_HEAD))!= NULL && victim != ch )
	{
		if ( obj->pIndexData->vnum == 32659 ) //Psicap
		{
			char buf[MSL];
			sprintf( buf, "%s begins to glow!\n\r", obj->short_descr );
			send_to_loc(buf,victim->x,victim->y,victim->z);
			victim = ch;
		}
	}
	send_to_char( "@@eYou suddenly feel compelled to do something... You just can't control it!@@N\n\r", victim );
	if ( action == 1 )
	{
		do_demolish(victim,"");
	}
	else if ( action == 2 )
	{
		do_set(victim,"bomb 3");
		do_arm(victim,"bomb");
	}
	else if ( action == 3 )
	{
		do_shoot(victim,"");
	}
	else if ( action == 5 )
	{
		do_destroy(victim,"");
	}
	else
		interpret(victim,argument);
	act( "You force $N to do your bidding!", ch, NULL, victim, TO_CHAR );
	return;
}

void do_paradrop( CHAR_DATA *ch, char *argument )
{
	int x,y,z,xx,yy,xxx,yyy;
	char arg[MSL];
	BUILDING_DATA *bld;

	if ( ( bld = ch->in_building ) == NULL || !complete(bld) || bld->type != BUILDING_PARADROP || str_cmp(bld->owned,ch->name) )
	{
		send_to_char( "You can only do that in a completed paradrop building.\n\r", ch );
		return;
	}
	if ( ch->in_vehicle )
	{
		send_to_char( "Not while inside a vehicle!\n\r", ch );
		return;
	}
	argument = one_argument(argument,arg);
	if ( !is_number(arg) || !is_number(argument) )
	{
		send_to_char( "Syntax: paradrop <x> <y>\n\r", ch );
		return;
	}
	x = atoi(arg);
	y = atoi(argument);
	z = ch->z;
	if ( x >= MAX_MAPS || y >= MAX_MAPS || x < 0 || y < 0 )
	{
		send_to_char( "Invalid coords.\n\r", ch );
		return;
	}
	if ( map_bld[x][y][ch->z] != NULL || map_table.type[x][y][ch->z] == SECT_NULL || map_table.type[x][y][ch->z] == SECT_WATER || map_table.type[x][y][ch->z] == SECT_NULL || map_table.type[x][y][ch->z] == SECT_OCEAN )
	{
		send_to_char( "Reports sugget that paradropping to that location is a bad idea.\n\rYou decide to listen to the reports.\n\r", ch );
		return;
	}
	for ( xxx=x-3;xxx<=x+3;xxx++ )
	{
		for ( yyy=y-3;yyy<=y+3;yyy++ )
		{
			xx = xxx;yy = yyy; real_coords(&xx,&yy);
			if ( map_bld[xx][yy][ch->z] != NULL && map_bld[xx][yy][ch->z]->active == FALSE )
			{
				send_to_char( "You can't go to that location.\n\r", ch );
				return;
			}
		}
	}
	act( "You jump on board a transport plane and begin the flight.", ch, NULL, NULL, TO_CHAR );
	move(ch,490,490,Z_PAINTBALL);
	ch->x = x;
	ch->y = y;
	ch->z = z;
	ch->in_building = NULL;
	ch->c_sn = gsn_paradrop;
	ch->c_time = 8;
	ch->c_level = 0;
	return;
}

void act_paradrop( CHAR_DATA *ch, int level )
{
	int x,y;
	if ( level < 20 )
	{
		if ( !blind_player(ch) )
			send_to_char( "You continue the flight.\n\r", ch );
	}
	else if ( level == 20 )
	{
		x = ch->x;
		y = ch->y;
		ch->x = 1;
		ch->y = 1;
		move(ch,x,y,ch->z);
		if ( !blind_player(ch) )
			do_look(ch,"");
		send_to_char( "You reach the destination, and jump off the plane!\n\r", ch );
	}
	else if ( level > 20 && level < 40 )
	{
		if ( !blind_player(ch) )
			send_to_char( "You glide down quietly.\n\r", ch );
	}
	else
	{
		BUILDING_DATA *bld;
		bld = map_bld[ch->x][ch->y][ch->z];
		if ( bld && !ch->in_vehicle )
		{
			send_to_char( "You CRASH into a building, falling through the glass windows!\n\r", ch );
			act( "$n CRASHES into the building, falling through the glass windows!\n\r", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
			send_to_char( "You hit the ground!\n\r", ch );
			act( "$n lands on the ground.", ch, NULL, NULL, TO_ROOM );
		}
		ch->c_sn = -1;
		ch->c_level = 0;
		ch->c_time = 0;
	}
	ch->c_time = 8;
	ch->c_level++;
	return;
}

void do_reset( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	if ( ch->in_building == NULL || str_cmp(ch->name,ch->in_building->owned))
		send_to_char( "You must be in a building to reset its password.\n\r", ch );
	else
	{
		if ( IS_SET(ch->in_building->value[1],INST_SPOOF) )
			REMOVE_BIT(ch->in_building->value[1],INST_SPOOF);
		ch->in_building->password = number_range(10000,99999);
		ch->in_building->real_dir = number_range(1,ch->in_building->directories);
		sprintf( buf, "Your new building password is %d\n\r", ch->in_building->password );
		send_to_char(buf,ch);
	}
	return;
}

void warp(CHAR_DATA *ch, int range)
{
	int x,y;
	if ( ch->z == Z_PAINTBALL )
	{
		send_to_char( "The warp failed.\n\r", ch );
		return;
	}
	x = number_range(ch->x - (range), ch->x + (range) );
	y = number_range(ch->y - (range), ch->y + (range) );
	if ( x < 5 )
		x = 5;
	if ( y < 5 )
		y = 5;
	if ( x > MAX_MAPS-5 )
		x = MAX_MAPS - 5;
	if ( y > MAX_MAPS-5 )
		y = MAX_MAPS - 5;

	if ( get_building(x,y,ch->z) != NULL )
	{
		send_to_char( "The warp failed!\n\r", ch );
		act( "Nothing happens!", ch, NULL, NULL, TO_ROOM );
		return;
	}
	act( "You shimmer away!", ch, NULL, NULL, TO_CHAR );
	if ( ch->in_vehicle != NULL )
		act( "$t shimmers away!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
	else
		act( "$n shimmers away!", ch, NULL, NULL, TO_ROOM );
	move ( ch, x, y, ch->z );
	do_look(ch,"");
	if ( ch->in_vehicle != NULL )
		act( "$t shimmers into the room!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
	else
		act( "$n shimmers into the room!", ch, NULL, NULL, TO_ROOM );
	return;
}

void do_backup_building( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	int i,f;
	char buf[MSL];
	char arg[MSL];
	BUILDING_DATA *bld;

	argument = one_argument(argument,arg);
	if ( !str_cmp(arg,"save") )
	{
		f = 1;
	}
	else if ( !str_cmp(arg,"load") )
	{
		f = 2;
	}
	else
	{
		send_to_char( "Syntax: backup save/load <disk>\n\r", ch );
		return;
	}
	if ( ( bld = ch->in_building ) == NULL )
	{
		send_to_char( "You must be in a building.\n\r", ch );
		return;
	}
	if ( ( obj = get_obj_carry(ch,argument) ) == NULL || obj->item_type != ITEM_BACKUP_DISK )
	{
		send_to_char( "You must specify a backup disk to save or load the data on.\n\r", ch );
		return;
	}
	if ( f == 1 )
		for ( i=0;i<10;i++ )
			obj->value[i] = bld->value[i];
	if ( f == 2 )
	{
		if ( bld->type != obj->value[2] )
		{
			send_to_char( "This disk doesn't contain data for this building.\n\r", ch );
			return;
		}
		for ( i=0;i<10;i++ )
			if ( i != 2 )
				bld->value[i] = obj->value[i];
		send_to_char( "Data Loaded.\n\r", ch );
		return;
	}
	obj->level = bld->level;
	obj->value[2] = bld->type;
	sprintf( buf, "A L%d %s Backup Disk\n\r", obj->level, build_table[obj->value[2]].name );
	free_string(obj->short_descr);
	free_string(obj->description);
	obj->short_descr = str_dup(buf);
	obj->description = str_dup(buf);
	send_to_char( "Backup Made.\n\r", ch );
	return;
}

void do_run( CHAR_DATA * ch, char *argument )
{
	char arg[MSL];
	int dir, loop;
	argument = one_argument(argument,arg);
	if ( ch->fighttimer > 0 )
	{
		send_to_char( "Not during combat.\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' || arg[0] == '\0' )
	{
		send_to_char( "Syntax: run x dir  (run 10 west)\n\r", ch );
		return;
	}
	if ( ch->z != Z_GROUND && ch->z != Z_PAINTBALL )
	{
		send_to_char( "You can't walk here, how do you expect to run?\n\r", ch );
		return;
	}
	if ( IS_BUSY(ch) || ch->pcdata->queue )
	{
		send_to_char( "You must not be busy, and have an empty command queue to use this.\n\r", ch );
		return;
	}
	dir = parse_direction(ch,argument);
	loop = atoi(arg);
	if ( loop <= 1 || loop > 50 )
	{
		send_to_char( "Running amount must be between 2 and 50.\n\r", ch );
		return;
	}
	if ( dir == -1 )
	{
		send_to_char( "Valid directions are: North, South, East, West.\n\r", ch );
		return;
	}
	send_to_char( "You begin running...\n\r", ch );
	WAIT_STATE(ch,loop);
	if ( !IS_SET(ch->effect,EFFECT_RUNNING) )
		SET_BIT(ch->effect,EFFECT_RUNNING);
	for ( ;loop > 0;loop-- )
		add_to_queue(ch,argument);
	check_queue(ch);
	return;
}
void do_cloneflag( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	if ( !str_cmp(argument,"off") )
	{
		free_string(ch->pcdata->who_name);
		ch->pcdata->who_name = str_dup("off");
		send_to_char("Done.\n\r", ch );
		return;
	}
	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You can only copy flags from your alliance members.\n\r", ch );
		return;
	}
	if ( ( victim = get_char_world(ch,argument) ) == NULL )
	{
		send_to_char( "You can't find that person.\n\r", ch );
		return;
	}
	if ( victim->pcdata->alliance != ch->pcdata->alliance )
	{
		send_to_char( "Target must be in the same alliance as you are.\n\r", ch );
		return;
	}
	free_string(ch->pcdata->who_name);
	ch->pcdata->who_name = str_dup(victim->pcdata->who_name);
	send_to_char("Done.\n\r", ch );
	return;
}
void do_blindupdate( CHAR_DATA *ch, char *argument )
{
	int i;
	if ( !IS_SET(ch->config,CONFIG_BLIND) )
	{
		send_to_char( "This option is meant for people who are blind in RL.\n\r", ch );
		return;
	}
	i = atoi(argument);
	if ( !is_number(argument) || i <= 0 || i > 100 )
	{
		send_to_char( "Syntax: blindupdate <interval>\n\rWhere the second interval between updates is <interval> X 3 seconds.\n\r", ch );
		return;
	}
	ch->pcdata->spec_init = i;
	send_to_char( "Blind update interval set.\n\r", ch );
	return;
}

void do_oresearch( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char buf[MSL];

	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax: research <ore>\n\rYou must specify an ore item to research!\n\r", ch );
		return;
	}
	if ( ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "You do not carry that ore.\n\r", ch );
		return;
	}
	if ( obj->item_type != ITEM_ORE )
	{
		sprintf(buf, "%s is not a researchable ore.\n\r", obj->short_descr );
		send_to_char(buf,ch);
		return;
	}
	ch->c_level = obj->value[0];
	ch->c_sn = gsn_oreresearch;
	ch->c_time = 40;
	ch->c_obj = obj;
	act( "You begin studying $p.", ch, obj, NULL, TO_CHAR );
	act( "$n begins studying $p.", ch, obj, NULL, TO_ROOM );
	return;
}

void act_oresearch(CHAR_DATA *ch, int level)
{
	OBJ_DATA *obj = ch->c_obj;
	char buf[MSL];
	int i,m=5;

	if ( !obj || obj->item_type != ITEM_ORE )
	{
		send_to_char( "For some reason, you have lost the ore you were researching!\n\r", ch );
		ch->c_obj = NULL;
		ch->c_level = -1;
		ch->c_sn = -1;
		return;
	}
	ch->c_level -= m;
	ch->c_obj->value[0] -= m;
	if ( ch->c_level <= 0 )
	{
		OBJ_DATA *n_obj;
		bool stop = FALSE;


		act( "You have completed your research!", ch, NULL, NULL, TO_CHAR );
		act( "$n finishes researching the ore.", ch, NULL, NULL, TO_ROOM );
		i=0;
		while ( !stop )
		{
			i++;
			if ( number_percent() < 80 || obj->value[i+1] <= 0 )
//				stop = TRUE;
				break;
		}
		if ( ( n_obj = create_object(get_obj_index(obj->value[i]),0) ) == NULL )
		{
			send_to_char( "There was an error with your new item, please contact an imm to check your ore's values.\n\r", ch );
			return;
		}
		obj_to_char(n_obj,ch);
		act( "You managed to extract $p!", ch, n_obj, NULL, TO_CHAR );
		act( "$n managed to extract $p!", ch, n_obj, NULL, TO_ROOM );
		ch->c_sn = -1;
		ch->c_obj = NULL;
		ch->c_level = -1;
		extract_obj(obj);
		return;
	}
	act( "You continue researching $p.", ch, obj, NULL, TO_CHAR );
	act( "$n continues researching the ore.", ch, NULL, NULL, TO_ROOM );
	if ( number_percent() < 25 )
	{
		sprintf( buf, "You estimate it would last another %d seconds.\n\r", ch->c_level );
		send_to_char(buf,ch);
	}
	ch->c_time = 40;
	return;
}


void do_use( CHAR_DATA *ch, char *argument )
{
	BUILDING_DATA *bld;
	OBJ_DATA *obj;
	char arg[MSL];
	char *arg2;
	char cmd[MSL];
	char buf[MSL];
	cmd[0] = '\0';

	arg2 = one_argument(argument,arg);
	if ( ( obj = get_obj_carry(ch,arg) ) != NULL )
	{
		if ( obj->item_type == ITEM_AMMO )
			sprintf(cmd,"load %s",argument);
		else if ( obj->item_type == ITEM_BOMB )
			sprintf(cmd," %s",argument);
		else if ( obj->item_type == ITEM_BLUEPRINT )
			sprintf(cmd,"upgrade %s",argument);
		else if ( obj->item_type == ITEM_INSTALLATION )
			sprintf(cmd,"install %s",argument);
		else if ( obj->item_type == ITEM_WEAPON )
			sprintf(cmd,"wear %s",argument);
		else if ( obj->item_type == ITEM_ARMOR )
			sprintf(cmd,"wear %s",argument);
		else if ( obj->item_type == ITEM_SUIT )
			sprintf(cmd,"wear %s",argument);
		else if ( obj->item_type == ITEM_MEDPACK )
			sprintf(cmd,"heal %s",arg);
		else if ( obj->item_type == ITEM_TELEPORTER )
			sprintf(cmd,"teleport %s",arg);
		else if ( obj->item_type == ITEM_LOCATOR )
			sprintf(cmd,"locate");
		else if ( obj->item_type == ITEM_ORE )
			sprintf(cmd,"research %s",arg);
		else if ( obj->item_type == ITEM_PIECE )
			sprintf(cmd,"connect %s",argument);
		else if ( obj->item_type == ITEM_VEHICLE_UP )
			sprintf(cmd,"vinstall %s",arg);
		else if ( obj->item_type == ITEM_WEAPON_UP )
			sprintf(cmd,"winstall %s",arg);
		else if ( obj->item_type == ITEM_BIOTUNNEL )
			sprintf(cmd,"settunnel %s",argument);
		else
		{
			send_to_char( "There is nothing you can do with it using this command. Try using the IDENTIFY command.\n\r", ch );
			return;
		}
	}
	else if ( ( bld = ch->in_building ) != NULL )
	{
		if ( bld->type == BUILDING_ARMORY )
			sprintf(cmd,"buy %s",argument);
		else if ( bld->type == BUILDING_ARMORER )
			sprintf(cmd,"buy %s",argument);
		else if ( bld->type == BUILDING_STORAGE )
			sprintf(cmd,"buy %s",argument);
		else if ( bld->type == BUILDING_SNIPER_TOWER )
			sprintf(cmd,"target %s",argument);
		else if ( bld->type == BUILDING_AIRFIELD )
			sprintf(cmd,"buy %s",argument);
		else if ( bld->type == BUILDING_GARAGE )
			sprintf(cmd,"buy %s",argument);
		else if ( bld->type == BUILDING_PARADROP )
			sprintf(cmd,"paradrop %s",argument);
		else if ( bld->type == BUILDING_SCUD_LAUNCHER )
			sprintf(cmd,"blast %s",argument);
		else if ( bld->type == BUILDING_NUKE_LAUNCHER )
			sprintf(cmd,"blast %s",argument);
		else if ( bld->type == BUILDING_HACKERS_HIDEOUT )
			sprintf(cmd,"trace %s",argument);
		else if ( bld->type == BUILDING_HACKPORT )
			sprintf(cmd,"doom %s",argument);
		else if ( bld->type == BUILDING_SPY_QUARTERS || bld->type == BUILDING_SPY_SATELLITE || bld->type == BUILDING_SHOCKWAVE )
			sprintf(cmd,"spy %s",argument);
		else if ( bld->type == BUILDING_PSYCHIC_TORMENTOR )
			sprintf(cmd,"torment %s",argument);
		else
		{
			send_to_char( "There is nothing you can do here.\n\r", ch );
			return;
		}
	}
	else
	{
		send_to_char( "You must either be in a building, or specify an object to use.\n\r", ch );
		return;
	}
	sprintf(buf, "Using command: %s\n\r", cmd );
	send_to_char(buf,ch);
	interpret(ch,cmd);
	return;
}
void do_setexit( CHAR_DATA *ch, char *argument )
{
	int dir;
	if ( !str_cmp(argument,"all") )
	{
		dir = -1;
	}
	else
	{
		if ( ( dir = parse_direction(ch,argument) ) == -1 )
		{
			send_to_char( "Valid default building exits are: North, West, East, South, All.\n\r", ch );
			return;
		}
	}
	ch->pcdata->set_exit = dir;
	send_to_char( "Done.\n\r", ch );
	do_save(ch,"");
	return;
}
void do_mute( CHAR_DATA *ch, char *argument )
{
	send_to_char("!!MUSIC(off)\n\r", ch );
	return;
}
void do_settunnel( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	int x,y;
	char arg[MSL];
	char arg2[MSL];
	char buf[MSL];
	bool found = FALSE;

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if ( arg[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Syntax: set <item> <x> <y>\n\r", ch );
		return;
	}

	if ( ( obj = get_obj_here(ch,arg) ) == NULL )
	{
		sprintf( buf,"You can't find a %s here.\n\r", arg );
		send_to_char( buf,ch);
		return;
	}
	if ( obj->item_type != ITEM_BIOTUNNEL )
	{
		send_to_char( "That is not a bio tunnel.\n\r", ch );
		return;
	}
	
	if ( obj->value[0] != 1 )
	{
		send_to_char( "That is not a transmitting biotunnel.\n\r", ch );
		return;
	}
	x = atoi(arg2);
	y = atoi(argument);

	if ( x < BORDER_SIZE || y < BORDER_SIZE || x > MAX_MAPS - BORDER_SIZE || y > MAX_MAPS - BORDER_SIZE )
	{
		send_to_char( "Invalid coords.\n\r", ch );
		return;
	}
	if ( abs(obj->x - x) > obj->value[1] || abs(obj->y - y) > obj->value[1] )
	{
		send_to_char( "That is out of range of the tunnel.\n\r", ch );
		return;
	}
	for ( obj2 = map_obj[x][y];obj2;obj2 = obj2->next_in_room )
	{
		if ( obj2->z != ch->z )
			continue;
		if ( obj2->item_type != ITEM_BIOTUNNEL || obj2->value[0] != 0 )
			continue;

		found = TRUE;
		break;
	}
	if ( !found )
	{
		send_to_char( "There is no receiving bio tunnel there.\n\r", ch );
		return;
	}
	obj->value[2] = x;
	obj->value[3] = y;
	sprintf( buf, "@@gYou set %s @@gto @@y%d@@g/@@y%d@@g.@@N\n\r", obj->short_descr, x, y );
	send_to_char( buf,ch);
	return;
}

void do_buy(CHAR_DATA *ch, char *argument)
{
	long cost=0,money = ch->money;
	CHAR_DATA *vch;
	VEHICLE_DATA *vhc=NULL;
	OBJ_DATA *obj;
	BUILDING_DATA *bld = ch->in_building;
	char buf[MSL];

	if ( !bld || (vch=bld->owner) == NULL)
	{
		if ( bld && is_evil(bld) )
		{
			vch = ch;
		}
		else
		{
			send_to_char( "You can't buy anything in here.\n\r", ch );
			return;
		}
	}
	if ( !complete(bld) )
	{
		send_to_char( "You can't buy anything in here.\n\r", ch );
		return;
	}
	if ( vch != ch && vch->fighttimer > 0 && !allied(ch,vch) )
	{
		send_to_char( "You can't buy from people in combat.\n\r", ch );
		return;
	}
	if ( bld->value[3] < 0 )
	{
		send_to_char( "The building has been disabled by a virus!\n\r", ch );
		return;
	}
	else if ( bld->type == BUILDING_SHIPYARD || bld->type == BUILDING_GARAGE || bld->type == BUILDING_AIRFIELD )
	{
		if ( argument[0] == '\0' )
		{
			int i,x=-1;
			char buf[MSL];
			shop_title(ch);
			for (i=0;shop_table[i].building != -1;i++)
			{
				if ( shop_table[i].level > bld->level ) 
				{
					if (( x == -1 || shop_table[x].level > shop_table[i].level ) && shop_table[i].building == bld->type )
						x = i;
					continue;
				}
				sprintf(buf,"%-4d   %-4d   %-4d (%-7s)  %-2d     %s", shop_table[i].hp,shop_table[i].fuel,shop_table[i].ammo,(shop_table[i].ammo_type<0||shop_table[i].ammo_type>=MAX_AMMO)?"None":clip_table[shop_table[i].ammo_type].name,shop_table[i].range,(IS_SET(shop_table[i].flags,VEHICLE_FLOATS))?"(Floats)":"None");
				if ( shop_table[i].building == bld->type )
					shop_item(ch,shop_table[i].name,shop_table[i].cost,buf);
			}
			if ( x > -1 )
			{
				sprintf(buf,"\n\r@@g[%s] will be available to you at level @@e%d@@g.\n\r", shop_table[x].name, shop_table[x].level );
				send_to_char(buf,ch);
			}
			return;
		}
		else
		{
			int i;
			for (i=0;shop_table[i].building != -1;i++)
			{
				if ( shop_table[i].building != bld->type ) continue;
				if ( shop_table[i].level > bld->level ) continue;
				if ( is_name(argument,shop_table[i].name) )
				{
					cost = shop_table[i].cost;
					if ( ch->money >= cost )
					{
						vhc = create_vehicle(shop_table[i].type);
						move_vehicle(vhc,ch->x,ch->y,ch->z);
						vhc->fuel = shop_table[i].fuel; vhc->max_fuel = vhc->fuel;
						vhc->hit = shop_table[i].hp; vhc->max_hit = vhc->hit;
						vhc->ammo_type = shop_table[i].ammo_type;
						vhc->ammo = shop_table[i].ammo; vhc->max_ammo = vhc->ammo;
						vhc->range = shop_table[i].range;
						vhc->speed = 2;
						free_string(vhc->desc); free_string(vhc->name);
						vhc->desc = str_dup(shop_table[i].name); vhc->name = str_dup(shop_table[i].name);
						if ( shop_table[i].flags > 0 )
							vhc->flags = shop_table[i].flags;
						sprintf(buf, "You buy the %s.\n\r", shop_table[i].name );
						send_to_char(buf,ch);
						sprintf(buf,"$n buys the %s", shop_table[i].name );
						act(buf,ch,NULL,NULL,TO_ROOM);
						ch->money -= cost;
					}
					else
						send_to_char( "You don't have enough money.\n\r", ch );
					return;
				}
			}
			send_to_char( "You can't buy this vehicle here.\n\r", ch );
			return;
		}
	}
	else if ( bld->type == BUILDING_PAWN_SHOP )
	{
		int i,x=0;
		OBJ_DATA *obj;
		if ( argument[0] == '\0' )
		{
			shop_title(ch);
			for (i=0;i<MAX_PAWN;i++)
			{
				if ((obj= pawn_obj[i]) == NULL )
					continue;
				shop_item(ch,obj->short_descr,obj->pIndexData->cost * 0.5,"");
				x++;
			}
			if ( x == 0 )
				shop_item(ch,"Nothing",0,"There is nothing here you can buy.");
		}
		else
		{
			char arg[MSL];
			long cost;
			x = number_argument(argument,arg);

			if ( ch->carry_number >= can_carry_n(ch) ) mreturn("You cannot carry any more items.\n\r", ch );
			for (i=0;i<MAX_PAWN;i++)
			{
				if ((obj= pawn_obj[i]) == NULL )
					continue;
				if ( is_name(argument,obj->name) && --x <= 0 )
				{
					cost = obj->pIndexData->cost * 0.5;
					if ( ch->money < cost ) {
						sprintf(buf, "%s costs %ld. You only have %ld.\n\r", obj->short_descr,cost,ch->money );
						send_to_char(buf,ch);
						break;
					}
					else if ( ch->carry_weight + obj->weight > can_carry_w(ch) )  {
						send_to_char("You cannot carry any more weight.\n\r", ch );
						return;
					}
					else
					{
						ch->money -= cost;
						obj_to_char(obj,ch);
						act( "You buy $p.", ch, obj,NULL,TO_CHAR);
						act( "$n buys $p.", ch, obj,NULL,TO_ROOM);
						REMOVE_BIT(obj->extra_flags,ITEM_PAWN);
						pawn_obj[i] = NULL;
					}
					return;
				}
			}
			send_to_char("There is no such object on sale.\n\r", ch );
			return;
		}
	}
	else if ( bld->type == BUILDING_QP_MEGASTORE )
	{
		if ( argument[0] == '\0' )
		{
			int i,n=0;
			OBJ_INDEX_DATA *pObj;
			shop_title(ch);
			for (i=1;i<=MAX_QP_OBJ;i++ )
			{
				if ( (pObj=get_obj_index(i))==NULL )
					continue;
				if ( pObj->building != bld->type )
					continue;
				sprintf(buf, "Type: %s    ",(pObj->item_type==ITEM_TOKEN)?"Misc.":capitalize(tab_item_types[ (pObj->item_type)-1 ].text));

				shop_item(ch,pObj->short_descr,pObj->cost,buf);
				n++;
			}
			if ( n==0 )
				shop_item(ch,"Nothing",0,"There is nothing here you can buy.");
		}
		else
		{
			int i,x=1;
			OBJ_INDEX_DATA *pObj;
			char arg[MSL];
			if ( ch->carry_number >= can_carry_n(ch) ) mreturn("You cannot carry any more items.\n\r", ch );
			x = number_argument(argument,arg);
			sprintf(argument,"%s",arg);
			for (i=1;i<=MAX_QP_OBJ;i++ )
			{
				if ( (pObj=get_obj_index(i))==NULL )
					continue;
				if ( pObj->building != bld->type )
					continue;

				if ( is_name(argument,pObj->name) && --x <= 0 )
				{
					cost = pObj->cost;
					if ( ch->quest_points < cost ) {
						sprintf(buf, "%s costs %ld QPs. You only have %d.\n\r", pObj->short_descr,cost,ch->quest_points );
						send_to_char(buf,ch);
						break;
					}
					else
					{
						ch->quest_points -= cost;
						obj=create_object(pObj,0);
						obj_to_char(obj,ch);
						act( "You buy $p.", ch, obj,NULL,TO_CHAR);
						act( "$n buys $p.", ch, obj,NULL,TO_ROOM);
						break;
					}
				}
			}
		}
		send_to_char("There is no such object on sale here.\n\r", ch );
		return;
	}
	else
	{
		if ( bld->type == BUILDING_BAR && bld->value[0] > 0 )
			mreturn("The barkeep tells you, \"Sorry, we're closed right now. Come back later.\"\n\r", ch );
		if ( argument[0] == '\0' )
		{
			int i,n=0;
			OBJ_INDEX_DATA *pObj;
			shop_title(ch);
			for (i=MIN_LOAD_OBJ;i<=MAX_LOAD_OBJ;i++ )
			{
				if ( (pObj=get_obj_index(i))==NULL )
					continue;
				if ( pObj->building != bld->type )
					continue;
				if ( pObj->value[14] > 0 && bld->value[5] != pObj->value[14] )
					continue;
				else if ( bld->value[5] < abs(pObj->value[14]) )
					continue;
				sprintf(buf, "Type: %s    ",(pObj->item_type==ITEM_TOKEN)?"Misc.":capitalize(tab_item_types[ (pObj->item_type)-1 ].text));

				if ( pObj->item_type == ITEM_WEAPON && pObj->value[2]>=0 && pObj->value[2] < MAX_AMMO )
			             sprintf( buf+strlen(buf), "Damage: %-4d  Delay: %-3d %s",
                			clip_table[pObj->value[2]].dam+pObj->value[7],                
			                clip_table[pObj->value[2]].speed+pObj->value[9],
			                clip_table[pObj->value[2]].explode?"(X)":"" );
				else if ( pObj->item_type == ITEM_BOMB )
				     sprintf(buf+strlen(buf), "Max Damage: %d", pObj->weight * pObj->value[2] );
				else if ( pObj->item_type == ITEM_DISK && bld->type == BUILDING_WEB_RESEARCH )
				     sprintf(buf+strlen(buf), "Version: %d.%d", bld->level / 2, (bld->level%2)*5);
				else if ( pObj->item_type == ITEM_DISK && bld->type != BUILDING_WEB_RESEARCH )
				     sprintf(buf+strlen(buf), "Version: %d.%d", pObj->value[1] / 10, pObj->value[1] % 10);
				else if ( pObj->item_type == ITEM_COMPUTER )
				     sprintf(buf+strlen(buf), "Range: %d", (bld->type == BUILDING_HP_OFFICES)?bld->level:pObj->value[0]);
				else if ( pObj->item_type == ITEM_BATTERY )
				     sprintf(buf+strlen(buf), "Energy: %d", pObj->value[0]);
				else if ( pObj->item_type == ITEM_DRONE && pObj->value[0] == 3 )
				     sprintf(buf+strlen(buf), "Distance: %d  Vision: %d", pObj->value[4], pObj->value[1]);

	
				shop_item(ch,pObj->short_descr,pObj->cost*(1-((float)(bld->level-1)/100)),buf);
				n++;
			}
			if ( n==0 )
				shop_item(ch,"Nothing",0,"There is nothing here you can buy.");
			return;
		}
		else
		{
			int i,x=1;
			OBJ_INDEX_DATA *pObj;
			char arg[MSL];
			if ( ch->carry_number >= can_carry_n(ch) ) mreturn("You cannot carry any more items.\n\r", ch );
			x = number_argument(argument,arg);
			sprintf(argument,"%s",arg);
			for (i=MIN_LOAD_OBJ;i<=MAX_LOAD_OBJ;i++ )
			{
				if ( (pObj=get_obj_index(i))==NULL )
					continue;
				if ( pObj->building != bld->type )
					continue;
				if ( pObj->value[14] > 0 && bld->value[5] != pObj->value[14] )
					continue;
				else if ( bld->value[5] < abs(pObj->value[14]) )
					continue;

				if ( is_name(argument,pObj->name) && --x <= 0 )
				{
					cost = pObj->cost * (1-((float)(bld->level-1)/100));
					if ( ch->money < cost ) {
						sprintf(buf, "%s costs %ld. You only have %ld.\n\r", pObj->short_descr,cost,ch->money );
						send_to_char(buf,ch);
						return;
					}
					else if ( ch->carry_weight + pObj->weight > can_carry_w(ch) )  {
						send_to_char("You cannot carry any more weight.\n\r", ch );
						return;
					}
					else
					{
						ch->money -= cost;
						if ( bld->type == BUILDING_WARP_SCANNER )
						{
	                        			int y,m=ch->map;
            						char buf[MSL];
							i=30;
	                        			while ( i>0 )
	                       				{
	                       				         i--;
	                       				         x = number_range(1,MAX_MAPS-1);
	                       				         y = number_range(1,MAX_MAPS-1);
	                       				         if ( map_bld[x][y][ch->z] ) continue;
	                       				         if ( map_table.type[x][y][ch->z] == SECT_OCEAN ) continue;
	                       				         ch->map = bld->level + 5;
								 if ( IS_SET(ch->config,CONFIG_BLIND) )
	                       				         	sprintf(buf,"%d %d buildings", x,y );
								 else
	                       				         	sprintf(buf,"%d %d map", x,y );
	                       				         do_at(ch,buf);
	                       				         ch->map = m;
								 sprintf(buf,"\n\rCoordinates: %d/%d\n\r", x,y);
								 send_to_char(buf,ch);
	                        			         return;
	                        			}
	                        			send_to_char( "The drone has not returned any feedback.\n\r", ch );
							return;
						}
						else if ( bld->type == BUILDING_TELEPORTER )
						{
							if ( bld->value[10] < 30000 )
							{
								bld->value[10]++;
								send_to_char( "You buy a new battery pack for this teleporter.\n\r", ch );
							}
							return;
						}
						obj=create_object(pObj,0);
						obj_to_char(obj,ch);
						act( "You buy $p.", ch, obj,NULL,TO_CHAR);
						act( "$n buys $p.", ch, obj,NULL,TO_ROOM);
						if ( obj->item_type == ITEM_DISK && bld->type == BUILDING_WEB_RESEARCH )
							obj->value[1] = ((bld->level/2)*10)+((bld->level%2)*5);
						else if ( obj->item_type == ITEM_ARMOR && IS_SET(bld->value[1],INST_ALIEN_HIDES) )
							for ( i=2;i<9;i++ ) { if (obj->value[i]<obj->level/2)obj->value[i]=obj->level/2; }
						else if ( obj->item_type == ITEM_COMPUTER && bld->type == BUILDING_HP_OFFICES )
							obj->value[0] = bld->level;
						else if ( obj->item_type == ITEM_DRONE && obj->value[0] == 3 )
							obj->value[2] = number_range(0,3);
						break;
					}
				}
			}
		}
	}
	if ( cost > 0 && money > ch->money )
	{
		if ( ch->pcdata->skill[gsn_economics] > 0 )
		{
			long dis;
			dis = cost * ((float)(ch->pcdata->skill[gsn_economics])*3)/100;
			cost -= dis;
			ch->money += dis;
			sprintf(buf, "You haggle your way out of $%ld.\n\r", dis );
			send_to_char(buf,ch);
		}
		if ( vch != ch )
		{
			char buf[MSL];
			cost /= 2;
			sprintf(buf, "@@a[@@c%s:%d/%d@@a]@@c has generated @@a$%ld@@c for selling @@a%s@@c to @@a%s@@N\n\r", 	bld->name, bld->x,bld->y, cost, (vhc!=NULL)?vhc->desc:"something", ch->name );
			send_to_char(buf,vch);
			gain_money(vch,cost);
		}
	}
	else
		send_to_char("There is no such object on sale here.\n\r", ch );
	return;
}
void shop_title(CHAR_DATA *ch)
{
	if ( IS_SET(ch->config,CONFIG_BLIND) ) return;
	if ( ch->in_building && (ch->in_building->type == BUILDING_GARAGE || ch->in_building->type == BUILDING_AIRFIELD || ch->in_building->type == BUILDING_SHIPYARD) )
		send_to_char("\n\r@@WVehicle                     Cost       Armor  Fuel   Ammo            Range  Flags\n\r@@g------------------------------------------------------------------------------------\\\n\r", ch );
	else
		send_to_char("\n\r@@WItem Name                   Cost       Description\n\r@@g------------------------------------------------------------------------------------\\\n\r", ch );
	return;
}
void shop_item(CHAR_DATA *ch,char *name,int cost,char *desc)
{
	char buf[MSL];

	if ( IS_SET(ch->config,CONFIG_BLIND) )
		sprintf( buf, "Item: %s. Cost: $%d. Desc: %s\n\r", name,cost,desc);
	else
		sprintf( buf, "@@b%-*s@@g|@@y %7d @@g|@@y %-45s@@g|\n\r------------------------------------------------------------------------------------<\n\r",27+(strlen(name)-nocol_strlen(name)),name,cost,desc);
	send_to_char(buf,ch);
	return;
}
void do_pay(CHAR_DATA *ch, char *argument)
{
	int v;
	CHAR_DATA *vch;
	char arg[MSL];
	char buf[MSL];

	argument = one_argument(argument,arg);

	if (arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char("Syntax: pay (amount) (player)\n\r", ch );
		return;
	}
	if ( (vch = get_char_room(ch,argument) ) == NULL)
	{
		if ( (vch = get_char_room(ch,arg) ) == NULL)
		{
			send_to_char( "That player isn't here.\n\r", ch );
			return;
		}
	}
	if ( vch == ch )
	{
		send_to_char("Aren't you the generous type?\n\r",ch);
		return;
	}
	if ( (v = atoi(arg)) <= 0 )
	{
		if ( (v = atoi(argument)) <= 0 )
		{
			send_to_char("How much do you want to pay exactly?\n\r", ch );
			return;
		}
	}

	if ( v > ch->money )
	{
		send_to_char( "You don't have that much money.\n\r", ch );
		return;
	}
	ch->money -= v;
	gain_money(vch,v);
	sprintf(buf,"You pay %s $%d.\n\r", vch->name,v);
	send_to_char(buf,ch);
	sprintf(buf,"%s pays you $%d.\n\r", ch->name, v);
	send_to_char(buf,vch);
	sprintf(buf,"$n pays $N $%d.", v );
	act(buf,ch,NULL,vch,TO_NOTVICT);
	if ( IS_IMMORTAL(ch) )
	{
		sprintf(buf,"%s pays $%d to %s.", ch->name,v,vch->name);
		log_f(buf);
	}
	return;
}
