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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ack.h"
#include <signal.h>

//If it's related to buildings, it's here.
void check_alien_hide args((OBJ_DATA *obj));
bool hits_air(BUILDING_DATA *bld);
long get_upgrade_cost(BUILDING_DATA *bld);
long get_upgrade2_cost(BUILDING_DATA *bld);

void building_update( void )
{
	int i,range,xx,yy,x,y,z;
	OBJ_DATA *obj = NULL;
	BUILDING_DATA *bld;
	BUILDING_DATA *bld_next;
	CHAR_DATA *bch = NULL;
	CHAR_DATA *ch = NULL;
	bool qb = FALSE;
	char buf[MSL];
	extern int active_building_count;
	extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];

	if ( first_building == NULL )
		return;

	active_building_count = 0;
	for ( bld = first_active_building;bld;bld = bld_next )
	{
		bld_next = bld->next_active;
		qb = FALSE;
		active_building_count++;
		if ( bld->timer > 0 )
		{
			bld->timer--;
			if ( bld->timer == 0 )
			{
				if ( is_evil(bld) )
				{
					extract_building(bld,TRUE);
					continue;
				}
				activate_building(bld,FALSE);
			}
			continue;
		}
		if ( --bld->tick > 0 )
			continue;
		bld->tick = number_range(build_table[bld->type].tick-1,build_table[bld->type].tick+1);
//		if ( !bld->tag && build_table[bld->type].act != BUILDING_UNATTACHED ) bld->tick *= 10;
		if ( bld->z == Z_PAINTBALL && bld->type == BUILDING_SNIPER_TOWER )
			continue;
		if ( number_percent() < 25 )
			bld->visible = TRUE;
		if ( bld->protection > 0 )
			bld->protection--;
		if ( !bld->active )
			continue;
		if ( bld->value[4] > 0 )
			bld->value[4]--;
		if ( bld->value[9] > 0 )
			bld->value[9]--;
		if ( is_evil(bld) )
			qb = TRUE;
		z = bld->z;
		if ( !qb )
		{
			if ( bld->owner )
			{
				bch = bld->owner;
			}
			else
			{
				bch = get_ch(bld->owned);
				bld->owner = bch;
			}
			ch = bch;
			if ( bch == NULL )
			{
				activate_building(bld,FALSE);
				continue;
			}
			if ( complete(bld) )
			{
				if ( bld->shield < bld->maxshield )
					bld->shield+=bld->level;
				if ( bld->hp < bld->maxhp / 2 && IS_SET(bld->value[1],INST_ORGANIC_CORE))
				{
					if ( number_percent() < 20 )
						bld->hp++;
				}
			}
	

			if ( bld->value[9] > 0 )
			{
				if ( has_ability(ch,1) ) //Nuclear containment
				{
					bld->value[9] -= number_range(1,3);
					if ( bld->value[9] < 0 )
					bld->value[9] = 0;
				}
			}
			if ( bld->value[3] > 0 && ( !IS_SET(bld->value[1],INST_FIREWALL) || number_percent() < 55 ) )
			{
				bld->value[3]--;
				if ( bld->value[3] == 0 )
				{
					bld->value[3] = -1;
					send_to_char( "@@yOne of your buildings has been taken over by a virus!@@N\n\rDon't know what to do? See: Help Virus\n\r", bch );
					if ( ch->fighttimer < 360 ) ch->fighttimer = 360;
				}
			}
			if ( bld->value[3] < 0 )
			{
				if ( bld->type == BUILDING_VIRUS_ANALYZER && bld->value[0] == 0 && bld->attacker )
				{
//					char buf[MSL];
					int chance=bld->level * 20;

					sprintf( buf,"@@yOne of your Virus Analyzers has come up with the following report:\n\r\n\r@@cThe virus was originated by @@a%s@@c.@@N\n\r", bld->attacker );
					if ( number_percent() < chance )
					{
						if ( ( ch = get_ch(bld->attacker) ) != NULL )
						{
							if ( ch->first_building )
								sprintf( buf+strlen(buf), "@@cIt has been determined that they have a building in the vicinity of@@a %d/%d@@c.@@N\n\r", ch->first_building->x,ch->first_building->y);
						}
					}
					else
						sprintf( buf+strlen(buf), "@@cIt couldn't trace the location of the attacker.@@N\n\r");
					bld->value[0] = 1;
					send_to_char(buf,bch);
				}
				if ( IS_SET(bld->value[1],INST_ANTIVIRUS) && number_percent() < 30 )
				{
					bld->value[3] = 0;
					bld->value[4] = 12;
					if ( number_percent() < 33 )
						REMOVE_BIT(bld->value[1],INST_ANTIVIRUS);
				}
				else if ( number_percent() < 30 )
				{
					BUILDING_DATA *bld2;
					bool got = FALSE;
					range = bld->level * 2;
	
					y = bld->y;
					for ( x = bld->x - range;x < bld->x + range + 1;x++ )
					{
						if ( INVALID_COORDS(x,y) )
							continue;
						bld2 = map_bld[x][y][z];
						if ( bld2 == NULL )
							continue;
						if ( number_percent() < 10 - bld->value[3] && bld2->value[4] == 0 )
						{
							if ( bld2 && bld2->value[3] == 0 && !str_cmp(bld->owned,bld2->owned) )
							{
								bld2->value[3] = bld->value[3];
								if ( bld->attacker )
								{
									if ( bld2->attacker != NULL && bld->attacker )
										free_string(bld2->attacker);
									bld2->attacker = str_dup(bld->attacker);
								}
								send_to_char( "@@eReports indicate the virus is spreading through your network!@@N\n\r", bch );
								if ( bch->fighttimer < 360 ) bch->fighttimer = 360;
								got = TRUE;
								break;
							}
						}
					}
					x = bld->x;
					if ( !got )
					{
						for ( y = bld->y - range;y < bld->y + range + 1;y++ )
						{
							if ( INVALID_COORDS(x,y) )
								continue;
							bld2 = map_bld[x][y][z];
							if ( bld2 == NULL )
								continue;
							if ( number_percent() < 10 - bld->value[3] && bld2->value[4] == 0 )
							{
								if ( bld2 && bld2->value[3] == 0 && !str_cmp(bld->owned,bld2->owned) )
								{
									bld2->value[3] = bld->value[3];
									if ( bld->attacker )
									{
										if ( bld2->attacker != NULL )
											free_string(bld2->attacker);
										bld2->attacker = str_dup(bld->attacker);
									}
									send_to_char( "@@eReports indicate the virus is spreading through your network!@@N\n\r", bch );
									got = TRUE;
									break;
								}
							}
						}
					}
					continue;
				}
				else
					continue;
			}
			if ( !complete(bld) )
				continue;
	
	    		if ( IS_SET( bch->pcdata->pflags, PFLAG_AFK ) && number_percent() < 75 )
				continue;
	
			if ( ( bch->security == FALSE || bld->protection > 0 ) && defense_building(bld) )
				continue;
		}
		
		obj = NULL;

		if ( !complete(bld) )
			continue;

		if ( build_table[bld->type].act == BUILDING_DEFENSE )
		{
			int dam,acc;
			range = bld->value[6];
			for ( xx = bld->x - range;xx < bld->x + range + 1;xx++ )
			for ( yy = bld->y - range;yy < bld->y + range + 1;yy++ )
			{
				x = xx; y = yy;
				real_coords(&x,&y);
				if ( ( ch = get_rand_char(x,y,bld->z) ) == NULL )
				{
					if (hits_air(bld))
					{
						if ( bld->z != Z_GROUND || ( ch = get_rand_char(x,y,Z_AIR) ) == NULL )
							continue;
					}
					else
						continue;
				}
				if ( is_evil(bld) )
					bch = ch;
				else
				{
					if ( bch->pcdata->alliance != -1 && bch->pcdata->alliance == ch->pcdata->alliance )
						continue;
				}
				if ( !building_can_shoot(bld,ch,range) )
					continue;
				sprintf(buf, "@@e[@@R%s@@e]@@R fires at you! ", bld->name );
				send_to_char(buf,ch);
				sprintf(buf, "@@G[@@r%s:%d/%d@@G]@@r fires at %s! ", bld->name,bld->x,bld->y,ch->name );
				send_to_char(buf,bch);
				acc = (40+(bld->level * 5)-bld->value[6]) + bld->value[8];
				if ( bld->value[0] == TURRET_MAGNET ) acc -= 30;
				if ( number_percent() < acc)
				{
					if ( bld->value[0] == TURRET_MAGNET )
					{
						OBJ_DATA *obj;
						if ( ( obj = ch->first_carry ) != NULL )
						{
		                                        while ( obj && ( IS_SET(obj->extra_flags,ITEM_NODROP) || IS_SET(obj->extra_flags,ITEM_STICKY) ) )
                		                                obj = obj->next_in_carry_list;

                                		        if ( !obj )
                                        		        continue;
		                                        set_fighting(bch,ch);
        		                                act( "You discover $p floating towards the magnet tower!", ch, obj, NULL, TO_CHAR );
                		                        act( "$n discovers $p floating towards the magnet tower!", ch, obj, NULL, TO_ROOM );
                        		                obj_from_char(obj);
                        		                obj->x = bld->x; 
                                		        obj->y = bld->y;
                                		        obj->z = bld->z;
                                		        free_string(obj->owner);
                                		        obj->owner = str_dup(bld->owned); 
                                		        obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
						}
					}
					else if ( bld->value[0] == TURRET_PROJECTOR )
					{
						if ( !IS_SET(ch->effect,EFFECT_VISION) )
							SET_BIT(ch->effect,EFFECT_VISION);
						send_to_char( "Your eyes feel blurry...\n\r", ch );
						send_to_char("\n\r",bch);
					}
					else if ( bld->value[0] == TURRET_PROJECTOR )
					{
						if ( !IS_SET(ch->effect,EFFECT_VISION) )
							SET_BIT(ch->effect,EFFECT_VISION);
						send_to_char( "Your eyes feel blurry...\n\r", ch );
						send_to_char("\n\r",bch);
					}
					else
					{
						dam = bld->value[5] - (bld->value[6] * bld->value[7]);
						sprintf(buf,"@@G(@@r%d:%s@@G)@@N\n\r",dam,dam_type[bld->value[10]]);
						send_to_char(buf,bch);
						sprintf(buf,"@@e(@@R%d:%s@@e)@@N\n\r",dam,dam_type[bld->value[10]]);
						send_to_char(buf,ch);
						damage( bch, ch, dam, bld->value[10] );
						if ( IS_SET(bld->value[1],INST_ACID_DEFENSE) && ch && ch->position != POS_DEAD )
							damage(bch,ch,bld->level*10,DAMAGE_ACID);
					}
				}
				else
				{
					sprintf(buf,"@@p(@@mMiss@@p)@@N\n\r" );
					send_to_char(buf,ch);
					send_to_char(buf,bch);
				}

				break;
			}

		}
		else if ( build_table[bld->type].act == BUILDING_MONEY )
		{
			int r = 1;
			gain_money(bch,r);
			if ( bld->level < MAX_BUILDING_LEVEL )
			{
				if ( --bld->value[0] <= 0 )
				{
					bld->level++;
					sprintf(buf, "@@a[@@c%s:%d/%d@@a]@@c has been upgraded to level @@a%d@@c!\n\r", bld->name,bld->x,bld->y,bld->level );
					send_to_char(buf,bch);
					bld->value[0] = build_table[bld->type].value[0];
				}
			}
		}
		else
		switch( UPPER(bld->name[0]) )
		{
/**/		case 'A':
		if ( bld->type == BUILDING_ARMOR_FACTORY )
		{
			for ( obj = map_obj[bld->x][bld->y];obj;obj = obj->next_in_room )
			{
				if ( obj->z == bld->z && obj->item_type == ITEM_ARMOR && obj->level < 99 )
				{
					int j,am;
					am = (obj->level < 95 ) ? 5 : 99 - obj->level;
					for ( j=2;j<9;j++ )
						obj->value[j] += am;
					obj->level += am;
					break;
				}
			}
		}

		break;
/**/		case 'B':
		if ( bld->type == BUILDING_BANK )
		{
			int t;
			t = build_table[bld->type].value[0] - bld->value[0];
			if ( t < 0 ) t = 0;
			bld->value[6] = ((bld->value[6] * t) + bld->value[5]) / (t+1);
			if ( --bld->value[0] <= 0 )
			{
				bld->value[0] = build_table[bld->type].value[0];
				bld->value[5] += (0+(((float)(bld->level)/100))) * bld->value[6];
				if ( bld->value[5] > 30000 ) bld->value[5] = 30000;
				bld->value[6] = 0;
			}
		}
		else if ( bld->type == BUILDING_BAR )
		{
			if ( bld->value[0] > 0 ) bld->value[0]--;
		}
		break;
/**/		case 'C':
		if ( bld->type == BUILDING_COUNTERSPY_AGENCY )
		{
			range = bld->level;

			for ( xx = bld->x-range;xx < bld->x+range;xx++ )
			for ( yy = bld->y-range;yy < bld->y+range;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				for ( ch = map_ch[x][y][bld->z];ch;ch = ch->next_in_room )
					if ( !IS_SET(ch->effect,EFFECT_EVADE) )
						SET_BIT(ch->effect,EFFECT_EVADE);
			}
		}
		break;
/**/		case 'D':
		break;
/**/		case 'E':
		if ( bld->type == BUILDING_ENCRYPTION_POD )
		{
			if ( bch->in_building == bld && bch->x == bld->x && bch->y == bld->y && bch->z == bld->z )
			{
				if ( !IS_SET(ch->effect,EFFECT_ENCRYPTION) )
				{
					SET_BIT(ch->effect,EFFECT_ENCRYPTION);
					send_to_char( "You are protected by an encryption barrier.\n\r", ch );
				}
			}
		}
		break;
/**/		case 'F':
		break;
/**/		case 'G':
		if ( bld->type == BUILDING_GOVERNMENT_HALL && bch->pcdata->alliance != -1 )
		{
			i = 0;
                        for ( obj = map_obj[bld->x][bld->y];obj;obj = obj->next_in_room )
                                if ( obj->x == bld->x && obj->y == bld->y && obj->z == bld->z && obj->item_type == ITEM_BOARD )
				{
					i++;
					break;
				}
			if ( i <= 0 )
			{
        	                obj = create_object( get_obj_index(OBJ_VNUM_ALLI_BOARD), 0 );
				if ( obj )
				{
                        		obj->x = bld->x;
	                        	obj->y = bld->y;
					obj->z = bld->z;
					obj->value[4] = bch->pcdata->alliance;
					obj->value[3] = OBJ_VNUM_ALLI_BOARD + bch->pcdata->alliance;
                	        	obj_to_room( obj, get_room_index(ROOM_VNUM_WMAP) );
				}
			}
		}
		break;
/**/		case 'H':
		if ( bld->type == BUILDING_HQ )
		{
			i = 0;
			if ( bch )
			gain_money(bch,5);
                        for ( obj = map_obj[bld->x][bld->y];obj;obj = obj->next_in_room )
                                if ( obj->x == bld->x && obj->y == bld->y && obj->z == bld->z && obj->item_type == ITEM_BOARD )
				{
					i++;
					break;
				}
			if ( i <= 0 )
			{
        	                obj = create_object( get_obj_index(OBJ_VNUM_MAIN_BOARD), 0 );
                        	obj->x = bld->x;
	                        obj->y = bld->y;
				obj->z = bld->z;
                	        obj_to_room( obj, get_room_index(ROOM_VNUM_WMAP) );
			}
		}
		else if ( bld->type == BUILDING_HACKPORT )
		{
			if ( bch->timer > 5 )
				continue;
			if ( bld->value[0] > 0 )
			{
				bld->value[0]--;
				if ( bld->value[0] == 0 )
					send_to_char( "@@eYour hackport is ready!@@N\n\r", ch);
			}
		}

		break;
/**/		case 'I':
		break;
/**/		case 'J':
		if ( bld->type == BUILDING_JAMMER )
		{
			BUILDING_DATA *jam;
			range = bld->level * 2;

			if ( number_percent() < 75 )
				bld->visible = FALSE;

			for ( xx = bld->x-range;xx < bld->x+range;xx++ )
			for ( yy = bld->y-range;yy < bld->y+range;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				jam = map_bld[x][y][z];
				if ( jam == NULL )
					continue;
				if ( !jam->owner || jam->owner != bld->owner )
					continue;
				if ( number_percent() < bld->level * 5 )
					continue;
				jam->visible = FALSE;
			}
		}
		break;
/**/		case 'K':
		break;
/**/		case 'L':
		break;
/**/		case 'M':
		break;
/**/		case 'N':
		if ( bld->type == BUILDING_NUKE_LAUNCHER )
		{
			if ( bch->timer > 5 )
				continue;
			if ( bld->value[0] > 0 )
			{
				bld->value[0]--;
				if ( bld->value[0] == 0 )
					send_to_char( "@@eYour Nuke Launcher has produced a Nuke!@@N\n\r", ch);
				continue;
			}
			if ( bld->value[0] == 0 && number_percent() <= 5 )
				send_to_char( "@@eA reminder-> One of your launchers has a Nuke ready for use.@@N\n\r", ch );
		}
		break;
/**/		case 'O':
		break;
/**/		case 'P':
		if ( bld->type == BUILDING_PSYCHIC_TORMENTOR )
		{
			if ( bch->timer > 5 )
				continue;
			if ( bld->value[0] > 0 )
			{
				bld->value[0]--;
				if ( bld->value[0] == 0 )
					send_to_char( "@@eYour Psychic Tormentor is ready!@@N\n\r", ch);
				continue;
			}
		}
		else if ( bld->type == BUILDING_PSYCHIC_RADAR )
		{
			range = 30;
			for ( xx = bld->x - range;xx < bld->x + range + 1;xx++ )
			for ( yy = bld->y - range;yy < bld->y + range + 1;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				if ( ( ch = get_rand_char(x,y,bld->z) ) == NULL || ch == bch || IS_IMMORTAL(ch) )
					if ( bld->z != Z_GROUND || ( ch = get_rand_char(x,y,Z_AIR) ) == NULL || ch == bch || IS_IMMORTAL(ch) )
					continue;
				if ( bch->pcdata->alliance != -1 && bch->pcdata->alliance == ch->pcdata->alliance )
					continue;
				sprintf( buf, "%s has been detected nearby at %d/%d!\n\r", ch->name, ch->x, ch->y );
				send_to_char(buf,bch);
				break;
			}
		}
		else if ( bld->type == BUILDING_PARTICLE_EMITTER )
		{
			range = 4;
			OBJ_DATA *obj2;
			for ( xx = bld->x - range;xx < bld->x + range + 1;xx++ )
			for ( yy = bld->y - range;yy < bld->y + range + 1;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				for ( obj2 = map_obj[x][y];obj2;obj2 = obj2->next_in_room )
				{
					if ( !obj2 )
						continue;
					if ( obj2->z != bld->z )
						continue;
					if ( obj2->item_type != ITEM_BOMB || obj2->value[1] <= 0 )
						continue;
					obj2->value[1] = 0;
				}
			}
		}
		break;
/**/		case 'Q':
		if ( bld->type == BUILDING_QP_MEGASTORE )
		{
			int s=0;
			bool bad;
			while (s<30)
			{
				s++;
				x = number_range(0,MAX_MAPS-1);
				y = number_range(0,MAX_MAPS-1);
				bad = FALSE;
				for ( xx=x-5;xx<x+5;xx++ )
				for ( yy=y-5;yy<y+5;yy++ )
				if (map_bld[xx][yy][bld->z])
				bad = TRUE;
				if ( !bad )
				{
					for ( ch = map_ch[bld->x][bld->y][bld->z];ch;ch = ch->next_in_room )
					{
						ch->in_building = NULL;
						send_to_char( "The QP Megastore vanishes into the air!\n\r", ch );
					}
					map_bld[bld->x][bld->y][bld->z] = NULL;
					map_bld[x][y][bld->z] = bld;
					bld->x = x; bld->y = y;
					return;
				}
			}
		}
		break;
/**/		case 'R':
		break;
/**/		case 'S':
		if ( bld->type == BUILDING_SHIELD_GENERATOR )
		{
			BUILDING_DATA *jam;
			int s;

			if ( bld->shield <= 0 )
				continue;

			range = bld->level * 2;

			for ( xx = bld->x-range;xx < bld->x+range;xx++ )
			for ( yy = bld->y-range;yy < bld->y+range;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				jam = map_bld[x][y][z];

				if ( jam == NULL || !map_bld[x][y][z] )
					continue;
				if ( jam == bld )
					continue;
				if ( bch )
				{
					if ( jam->owner )
					{
						if ( jam->owner != bch )
							continue;
					}
				}
				else
				{
					if ( str_cmp(jam->owned,bld->owned) )
						continue;
				}
				if ( !complete(jam) )
					continue;
				if ( jam->shield >= jam->maxshield )
					continue;
				if ( jam->value[9] > 0 )
					continue;
				s = bld->shield;
				if ( jam->maxshield - jam->shield < s )
					s = jam->maxshield - jam->shield;
				bld->shield -= s;
				jam->shield += s;
			}
		}
		else if ( bld->type == BUILDING_SOLAR_PANEL )
		{
			BUILDING_DATA *jam;
			range = 1;

			for ( xx = bld->x-range;xx < bld->x+range;xx++ )
			for ( yy = bld->y-range;yy < bld->y+range;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				jam = map_bld[x][y][z];
				if ( jam == NULL || jam == bld )
					continue;
				jam->tick--;
			}
		}
		else if ( bld->type == BUILDING_SPY_SATELLITE || bld->type == BUILDING_SPY_QUARTERS || bld->type == BUILDING_SHOCKWAVE )
		{
			if ( bch && bch->timer > 5 )
				continue;
			if ( bld->value[0] > 0 )
				bld->value[0]--;
		}
		else if ( bld->type == BUILDING_SCUD_LAUNCHER )
		{
			if ( bch->timer > 5 )
				continue;
			if ( bld->value[0] > 0 )
			{
				bld->value[0]--;
				if ( bld->value[0] == 0 )
					send_to_char( "@@eYour SCUD Launcher has produced a SCUD!@@N\n\r", ch);
				continue;
			}
			if ( bld->value[0] == 0 && number_percent() <= 5 )
				send_to_char( "@@eA reminder-> One of your launchers has a SCUD ready for use.@@N\n\r", ch );
		}
		break;
/**/		case 'T':
		if ( bld->type == BUILDING_TEMPLE )
		{
			int t;
			if ( --bld->value[0] <= 0 )
			{
				bld->value[0] = build_table[bld->type].value[0];
				t = (ch->max_hit - ch->hit) / (7-(bld->level/2));
				if ( t > 0 )
				{
					sprintf(buf,"@@aA Divine blessing falls upon you. (@@c%d@@a)@@N\n\r", t );
					send_to_char(buf,ch);
					ch->hit += t;
				}
			}
		}
		break;
/**/		case 'U':
		break;
/**/		case 'V':
		if ( bld->type == BUILDING_VIRUS_ANALYZER )
		{
			bld->value[0] = 0;
		}
		break;
/**/		case 'W':
		if ( bld->type == BUILDING_WEATHER_MACHINE )
		{
			range = bld->level;

			for ( xx = bld->x-range;xx < bld->x+range;xx++ )
			for ( yy = bld->y-range;yy < bld->y+range;yy++ )
			{
				x=xx;y=yy;real_coords(&x,&y);
				if ( map_table.type[x][y][bld->z] != SECT_WATER ) continue;
				if ( number_percent() < bld->level * 5 )
					continue;
				map_table.type[x][y][bld->z] = SECT_ICE;
			}
		}
		break;
/**/		case 'X':
		break;
/**/		case 'Y':
		break;
/**/		case 'Z':
		break;
		}

	}
	return;
}

void do_mimic( CHAR_DATA *ch, char *argument )
{
	int i;
	BUILDING_DATA *bld;
	if ( ( bld = get_char_building(ch) ) == NULL )
	{
		send_to_char( "You must be inside a dummy building.\n\r", ch );
		return;
	}
	if ( bld->type != BUILDING_DUMMY || str_cmp(bld->owned,ch->name) || !complete(bld) )
	{
		send_to_char( "You can't do that here.\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "Please provide a building to mimic.\n\r", ch );
		return;
	}
	for ( i = 1;i<MAX_BUILDING;i++ )
	{
		if ( !str_cmp(build_table[i].name,argument) )
		{
			bld->value[0] = i;
			send_to_char( "Ok.", ch );
			return;
		}
	}
	send_to_char( "No such building.\n\r", ch );
	return;
}

bool check_missile_defense(OBJ_DATA *obj)
{
	bool ex = FALSE;
	int x,y,z,xx,yy,y1;
	BUILDING_DATA *bld;
	CHAR_DATA *bch;

	x = obj->x - 5;
	y1 = obj->y - 5;
	if ( x < 0 )
		x = 0;
	if ( y1 < 0 )
		y1 = 0;
	xx = obj->x + 5;
	yy = obj->y + 5;
	if ( xx >= MAX_MAPS )
		xx = MAX_MAPS-1;
	if ( yy >= MAX_MAPS )
		yy = MAX_MAPS-1;
	z = obj->z;
	for ( ;x < xx;x++ )
	{
		for ( y=y1;y < yy;y++ )
		{
			if ( INVALID_COORDS(x,y) )
				continue;

			bld = map_bld[x][y][z];

			if ( !bld || bld->type != BUILDING_MISSILE_DEFENSE || !complete(bld) )
				continue;
			bch = bld->owner;
			if ( !bch )
				if ( ( bch = get_ch(bld->owned) ) == NULL )
					continue;
			ex = TRUE;
			send_to_char( "@@eYour missile defense system has intercepted a bomb!@@N\n\r", bch );
			break;
		}
		if ( ex )
			break;
	}
	return ex;
}

void check_alien_hide(OBJ_DATA *obj)
{
	int i,x,t;
	x = obj->level;
	t = (obj->value[0]==DAMAGE_BULLETS)?3:(obj->value[0]==DAMAGE_GENERAL)?2:(obj->value[0]==DAMAGE_BLAST)?4:(obj->value[0]==DAMAGE_ACID)?5:(obj->value[0]==DAMAGE_FLAME)?6:(obj->value[0]==DAMAGE_LASER)?7:(obj->value[0]==DAMAGE_SOUND)?8:-1;
	if ( t == -1 )
		return;
	for ( i=2;i<9;i++ )
	{
		if ( i == t )
			continue;
		if ( obj->value[i] < obj->level / 2 )
			obj->value[i] = obj->level / 2;
	}
	return;
}
bool hits_air(BUILDING_DATA *bld)
{
	switch (bld->value[10])
	{
		case DAMAGE_BULLETS:
		case DAMAGE_LASER:
		case DAMAGE_BLAST:
			return TRUE;
		default:
			return FALSE;
	}
	return TRUE;
}

void show_building_cust(BUILDING_DATA *bld,CHAR_DATA *ch)
{
	char buf[MSL];
	int type;


	if ( !bld || bld == NULL ) return;
	if ( build_table[bld->type].act == BUILDING_DEFENSE )
		type = 1;
	else if ( bld->type == BUILDING_BANK )
		type = 2;
	else if ( bld->type == BUILDING_ARMORY || bld->type == BUILDING_ARMORER || bld->type == BUILDING_EXPLOSIVES_SUPPLIER || bld->type == BUILDING_STORAGE )
		type = 3;
	else
		type = 0;

	buf[0] = '\0';
	sprintf(buf+strlen(buf),"\n\r@@W+-----@@a %25s @@W-----+\n\r",center_text(bld->name,25));

	switch(type)
	{
		case 1:
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@aLevel:@@c  %-2d ($%-6ld)                @@W|\n\r",bld->level,get_upgrade_cost(bld));
		sprintf(buf+strlen(buf),"@@W| @@aRange:@@c  %-2d                          @@W|\n\r",bld->value[6]);
		sprintf(buf+strlen(buf),"@@W| @@dDamage:@@c %-4d                        @@W|\n\r",bld->value[5]-(bld->value[6]*bld->value[7]));
		sprintf(buf+strlen(buf),"@@W| @@dHit %%:@@c  %-4d                        @@W|\n\r",(40+(bld->level * 5)-bld->value[6]) + bld->value[8]);
		sprintf(buf+strlen(buf),"@@W| @@dAmmo:@@c   %-7s                     @@W|\n\r",dam_type[bld->value[10]]);
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		break;
		case 2:
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@aLevel:@@c     %-2d ($%-6ld)             @@W|\n\r",bld->level,get_upgrade_cost(bld));
		sprintf(buf+strlen(buf),"@@W| @@dDeposited:@@c %-14d           @@W|\n\r",bld->value[5]);
		sprintf(buf+strlen(buf),"@@W| @@dInterest:@@c  %2.2f                     @@W|\n\r",1+((float)(bld->level)/100));
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@dExtra Commands: Withdraw, Deposit   @@W|\n\r");
		break;
		case 3:
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@aLevel:@@c      %-2d ($%-6ld)            @@W|\n\r",bld->level,get_upgrade_cost(bld));
		sprintf(buf+strlen(buf),"@@W| @@aEquipment:@@c  %-2d ($%-6ld)            @@W|\n\r",bld->value[5],get_upgrade2_cost(bld));
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@dExtra Commands: Revert              @@W|\n\r");
		break;
		default:
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		sprintf(buf+strlen(buf),"@@W| @@aLevel:@@c  %-2d ($%-6ld)                @@W|\n\r",bld->level,get_upgrade_cost(bld));
		sprintf(buf+strlen(buf),"@@W|                                     |\n\r");
		break;
	}


	sprintf(buf+strlen(buf),"@@W+----------  @@aCustomization@@W  ----------+\n\r" );
	send_to_char(buf,ch);
	return;
}
void cust_interpret(CHAR_DATA *ch, char *argument)
{
	char arg[MSL];
	int val,type;
	BUILDING_DATA *bld = ch->in_building;

        if ( !ch->in_building )
        {
                send_to_char( "You are no longer inside a building.\n\r", ch );
                ch->position = POS_STANDING;
                return;
        }
	if ( argument[0] == '\0' )
		return;
	if ( build_table[bld->type].act == BUILDING_DEFENSE )
		type = 1;
	else if ( bld->type == BUILDING_BANK )
		type = 2;
	else if ( bld->type == BUILDING_ARMORY || bld->type == BUILDING_ARMORER || bld->type == BUILDING_EXPLOSIVES_SUPPLIER || bld->type == BUILDING_STORAGE )
		type = 3;
	else
		type = 0;
	argument = one_argument(argument,arg);
	if ( !str_cmp(argument,"all"))
		val = bld->value[5];
	else
		val = atoi(argument);
	if ( arg[0] == '\0' || !str_prefix(arg,"Show") )
	{
		show_building_cust(ch->in_building,ch); 
		return;
	}
	else if (!str_prefix(arg,"Done"))
	{
		ch->position = POS_STANDING;
		return;
	}
	else if ( !str_prefix(arg,"look") )
	{
		do_look(ch,argument);
		return;
	}
	else if ( !str_prefix(arg,"level") )
	{
		int c = get_upgrade_cost(bld);
		if ( c == 0 )
		{
			send_to_char( "This building cannot be upgraded anymore.\n\r", ch );
			return;
		}
		if ( ch->money < c )
		{
			send_to_char("You don't have enough money to purchase an upgrade.\n\r", ch );
			return;
		}
		send_to_char("\n\r\n\rLevel up!\n\r", ch );
		bld->level++;
		ch->money -= c;
		return;
	}
	else if ( !str_cmp(arg,"revert") && type == 3 )
	{
		int c;
		if ( bld->value[5] <= 1 ) mreturn("There is nothing to revert to.\n\r", ch );
		bld->value[5]--;
		c = get_upgrade2_cost(bld);
		if ( c == 0 )
		{
			send_to_char( "This building cannot be reverted anymore.\n\r", ch );
			return;
		}
		gain_money(ch,c);
		return;
	}
	else if ( !str_prefix(arg,"equipment") && type == 3 )
	{
		int c = get_upgrade2_cost(bld);
		if ( c == 0 )
		{
			send_to_char( "This property cannot be upgraded anymore.\n\r", ch );
			return;
		}
		if ( ch->money < c )
		{
			send_to_char("You don't have enough money to purchase an upgrade.\n\r", ch );
			return;
		}
		send_to_char("\n\r\n\rEquipment up!\n\r", ch );
		bld->value[5]++;
		ch->money -= c;
		return;
	}
	if ( val <= 0 || val > 30000 )
	{
		send_to_char( "Invalid value.\n\rExample: range 6\n\r", ch );
		return;
	}
	if ( type == 1 )
	{
		if ( !str_prefix(arg,"range") )
		{
			if ( bld->value[5]-(val*bld->value[7])>0 )
				bld->value[6] = val;
			else
				send_to_char( "That's too far.\n\r", ch );
		}
		else
			send_to_char( "This property cannot be modified.\n\r", ch );
	}
	else if ( type == 2 )
	{
		if ( !str_prefix(arg,"withdraw") )
		{
			if ( val > bld->value[5] )
				send_to_char( "You don't have that much money in the account.\n\r", ch );
			else
			{
				if ( ch->money + val >= MAX_CASH ) val = MAX_CASH - ch->money;
				gain_money(ch,val);
				bld->value[5] -= val;
				send_to_char( "You withdraw the money.\n\r", ch );
			}
		}
		else if ( !str_prefix(arg,"deposit") )
		{
			if ( val > ch->money )
				send_to_char( "You don't have that much to deposit.\n\r", ch );
			else if ( bld->value[5] + val > 30000 )
				send_to_char( "You can't have more than $30,000 in one bank account.\n\r", ch );
			else
			{
				ch->money -= val;
				bld->value[5] += val;
				send_to_char( "Money deposited.\n\r", ch );
			}
		}
	}
	else if ( type == 3 )
	{
	}
	else
		send_to_char( "@@cYou cannot change anything in this building. Please type @@aDone@@c.\n\r", ch );
	return;

}
void do_customize(CHAR_DATA *ch, char *argument)
{
	BUILDING_DATA *bld = ch->in_building;
	if ( !ch->in_building || ch->in_building == NULL )
	{
		send_to_char( "You must be inside a building to customize it.\n\r", ch );
		return;
	}
	if ( !bld->owner || bld->owner != ch )
		if ( !IS_IMMORTAL(ch) )
			mreturn( "You are not allowed to upgrade this building.\n\r", ch );
	ch->position = POS_CUSTOMIZE;
	show_building_cust(ch->in_building,ch); 
	return;
}
long get_upgrade_cost(BUILDING_DATA *bld)
{
	int c,i;

	if ( bld->level >= MAX_BUILDING_LEVEL )
		return 0;
	if ( sysdata.killfest ) return 1;
	c = build_table[bld->type].cost;
	switch(bld->type)
	{
		case BUILDING_HQ: c = 3000; break;
		case BUILDING_PAWN_SHOP: c = 3000; break;
	}
	if ( c == 0 ) c = 10000;
	for(i=0;i<bld->level;i++ )
		c *= 1.05;
	if ( bld->owner && has_ability(bld->owner,7) ) //Databank
		c *= 0.7;
	return c;
}
long get_upgrade2_cost(BUILDING_DATA *bld)
{
	int c = 0;
	if ( bld->value[5] >= bld->value[0] )
		return c;
	if ( sysdata.killfest ) return 1;
	c = 300*(bld->value[5]+1);
	if ( bld->owner && has_ability(bld->owner,7) ) //Databank
		c *= 0.7;
	return c;
}
