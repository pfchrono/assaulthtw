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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ack.h"
#include "config.h"

void do_alliances( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	int i,x=0,j;
	bool all = FALSE;
	bool sort = TRUE;
	int ord[MAX_ALLIANCE];

	if ( !str_cmp(argument,"all") )
		all = TRUE;

	for ( i=0;i<MAX_ALLIANCE;i++ )
		ord[i] = i;
	if ( sort )
	{
		for ( i=0;i<MAX_ALLIANCE;i++ )
		{
			for ( x=0;x<i;x++ )
			{
				if ( alliance_table[ord[i]].kills > alliance_table[ord[x]].kills )
				{
					j = ord[x];
					ord[x] = ord[i];
					ord[i] = j;
					i--;
					break;
				}
			}
		}
	}

	x = 1;
	send_to_char( "\n\r@@W   Leader       @@g|@@W  Members  @@g|@@W  Kills  @@g|@@W  Name@@g\n\r", ch );
	for ( j = 0;j < MAX_ALLIANCE;j++ )
	{
		i = ord[j];
		if ( alliance_table[i].name == NULL )
			break;
		if ( alliance_table[i].members <= 1 && !all )
			continue;
		buf[0] = '\0';
		if ( IS_IMMORTAL(ch) )
			sprintf(buf,"%2d. ", i );
		sprintf( buf+strlen(buf), "@@g[@@W%-10s@@g] |   [@@W%3d@@g]   |  [@@W%3d@@g]  @@W|@@N  %s@@g\n\r", 
			alliance_table[i].leader, 
			alliance_table[i].members, 
			alliance_table[i].kills,
			alliance_table[i].name );
		x++;
		send_to_char( buf, ch );
	}
	send_to_char("Proposed alliances:\n\r #  |Name           Leader          Supporters\n\r", ch);
        buf[0] = '\0';
	for(i=0;i<MAX_ALLIANCE;i++)
	{
		if(!strcmp(palliance_table[i].name, ""))
			break;
    		sprintf(buf, "@@g[@@W%2d@@g][@@W%-13s@@g][@@W%-13s@@g][@@W%s, %s@@g]\n\r", i, palliance_table[i].name, palliance_table[i].leader, palliance_table[i].support[0], palliance_table[i].support[1]);
		send_to_char(buf, ch);
	}
	return;
}

void do_awhere( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *wch;
	char buf[MSL];
	char loc[MSL];

	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You are not a member of an alliance.\n\r", ch );
		return;
	}
	sprintf( buf, "@@WMembers of@@N %s@@N:\n\r", alliance_table[ch->pcdata->alliance].name );
	for ( wch = first_char;wch;wch = wch->next )
	{
		if ( ch->pcdata->alliance == wch->pcdata->alliance )
		{
			sprintf( loc, "%3d/%-3d", wch->x, wch->y );

			sprintf( buf+strlen(buf), "@@W%-15s @@d- @@a%s @@R[@@e%s@@R] %s%s%s @@e%3d@@c/@@R%-3d HP   @@bFighting@@c: @@y%3s@@N\n\r", wch->name, loc, planet_table[wch->z].name, (wch->in_building)?"@@c(@@a":"", (wch->in_building)?wch->in_building->name:"", (wch->in_building)?"@@c)":"", wch->hit, wch->max_hit, (wch->fighttimer>0)?"Yes":"No" );
		}
	}
	send_to_char( buf, ch );
	return;
}
void do_accept( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MSL];

	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax: accept <person>\n\r        accept emergency\n\r", ch );
		return;
	}
	if ( !str_cmp(argument,"emergency") )
	{
		int i;

		if ( ch->pcdata->alliance != -1 )
		{
			send_to_char( "You are already a member of an alliance.\n\r", ch );
			return;
		}
		for ( i = 0;i < 99999;i++ )
		{
			if ( alliance_table[i].name == NULL )
				break;
			if ( !str_cmp(ch->name,alliance_table[i].leader) )
			{
				ch->pcdata->alliance = i;
				send_to_char( "You have regained membership of your alliance.\n\r", ch );
				alliance_table[i].members++;
				return;
			}
		}
		send_to_char( "You are not the leader of any alliance!\n\r", ch );
		return;
	}

	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You are not a part of an alliance.\n\r", ch );
		return;
	}
	if ( ( victim = get_char_room(ch,argument) ) == NULL )
	{
		send_to_char( "You can't find that person.\n\r", ch );
		return;
	}
	if ( victim->pcdata->alliance != -1 )
	{
		send_to_char( "They are already a member of another alliance.\n\r", ch );
		return;
	}
	sprintf( buf, "You insert $N's data to %s's member database.\n\r", alliance_table[ch->pcdata->alliance].name );
	act( buf, ch, NULL, victim, TO_CHAR );
	sprintf( buf, "$n grabs a small computer, and inserts your data to %s's member database.\n\r", alliance_table[ch->pcdata->alliance].name );
	act( buf, ch, NULL, victim, TO_VICT );
	sprintf( buf, "$n grabs a small computer, and inserts $N's data to %s's member database.\n\r", alliance_table[ch->pcdata->alliance].name );
	act( buf, ch, NULL, victim, TO_NOTVICT );
	victim->pcdata->alliance = ch->pcdata->alliance;
	alliance_table[ch->pcdata->alliance].members++;
	save_alliances();
	return;
}

void do_leave( CHAR_DATA *ch, char *argument )
{
	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You are not in alliance, how can you leave it?\n\r", ch );
		return;
	}
	if ( !ch->in_building || ch->in_building->type != BUILDING_HQ )
	{
		send_to_char( "You must be in your headquarters to leave an alliance.\n\r", ch );
		return;
	}
	if (!str_cmp(alliance_table[ch->pcdata->alliance].leader,ch->name) )
	{
		send_to_char( "You can't leave your own alliance. Use Setowner to give it to someone else.\n\r", ch );
		return;
	}
	act( "You use your Headquarters' computer to remove your data from your alliance's database.", ch, NULL, NULL, TO_CHAR );
	act( "$n uses $s Headquarters' computer to remove $mself from $s alliance's database.", ch, NULL, NULL, TO_ROOM );
	alliance_table[ch->pcdata->alliance].members--;
	save_alliances();
	ch->pcdata->alliance = -1;
	return;
}

void do_aban( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MSL];

	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You are not a part of an alliance.\n\r", ch );
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "Ban whom?\n\r", ch );
		return;
	}
	if ( ( victim = get_char_world(ch,argument) ) == NULL )
	{
		send_to_char( "You can't find that person.\n\r", ch );
		return;
	}
	if ( victim->pcdata->alliance != ch->pcdata->alliance )
	{
		send_to_char( "They are not a member of your alliance.\n\r", ch );
		return;
	}
	alliance_table[ch->pcdata->alliance].members--;
	save_alliances();
	sprintf( buf, "You remove $N's data from %s's member database.\n\r", alliance_table[ch->pcdata->alliance].name );
	act( buf, ch, NULL, victim, TO_CHAR );
	victim->pcdata->alliance = -1;
	return;
}

void do_amem( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	int alliance;

	if ( ch->pcdata->alliance == -1 )
	{
		send_to_char( "You are not a part of an alliance.\n\r", ch );
		return;
	}
	if ( is_number(argument) && IS_IMMORTAL(ch) )
		alliance = atoi(argument) - 1;
	else
		alliance = ch->pcdata->alliance;
	sprintf(buf,"fgrep -lx 'Alliance     %d' %s*/*", alliance, PLAYER_DIR); /**/
	do_pipe(ch, buf);
	return;
}

char *fgetf( char *s, int n, register FILE *iop )
{
    register int c;
    register char *cs;
    
    c = '\0';
    cs = s;
    while( --n > 0 && (c = getc(iop)) != EOF)
    {
        if ((*cs++ = c) == '\0')
            break;
    }

    *cs = '\0';
    return((c == EOF && cs == s) ? NULL : s);
}
void do_pipe( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    FILE *fp;

#if defined (unix)
    fp = popen( argument, "r" );
#endif

#if defined ( WIN32 )
	fp = _popen(argument, "r" );
#endif

    fgetf( buf, MSL, fp );

    if ( ch && ch != NULL )
        send_to_char( buf, ch );


#if defined (unix)
    pclose( fp );
#endif
#if defined ( WIN32 )
	_pclose (fp);
#endif

    return;
}

void do_createalliance(char leader[20], char *argument)
{
    int i;
	char *name_nocol, *name_nocol2;
	CHAR_DATA *ch;
	DESCRIPTOR_DATA d;
	bool found;

	smash_tilde(argument);
	smash_swear(argument);
	name_nocol = str_dup(strip_color(argument,"@@"));
	strcat(argument,"@@N");
	for ( i = 0;i< MAX_ALLIANCE;i++ )
	{
		if ( alliance_table[i].name == NULL || alliance_table[i].members == 0)
		{
			if ( alliance_table[i].members == 0)
			{
				char file[MSL]; //Erase board data
				BOARD_DATA *board;
				sprintf( file, "boards/board.%d", OBJ_VNUM_ALLI_BOARD + i );
				for ( board = first_board;board;board = board->next )
				{
					if ( board->vnum == OBJ_VNUM_ALLI_BOARD + i )
					{
						MESSAGE_DATA *msg;
						MESSAGE_DATA *msg_next;
						for ( msg = board->first_message;msg;msg = msg_next )
						{
							msg_next = msg->next;
							UNLINK(msg, board->first_message, board->last_message, next, prev);
							PUT_FREE(msg, message_free);
						}
					break;
					}
				}
				unlink(file);
			}
			if ( alliance_table[i].name != NULL )
				free_string(alliance_table[i].name);
			if ( alliance_table[i].leader != NULL )
				free_string(alliance_table[i].leader);
			alliance_table[i].leader = str_dup(leader);
			alliance_table[i].name = str_dup(argument);
			alliance_table[i].members = 1;
			alliance_table[i].kills = 0;
			leader[0] = UPPER(leader[0]);
			if((ch = get_ch(leader)) != NULL && !strcmp(leader, ch->name))
				ch->pcdata->alliance = i;
			else {
				found = load_char_obj( &d, leader, FALSE );
				if(!found)
				{
					log_f("Creating alliance for non-existant leader.\n\r");
					free_string(name_nocol);
					save_alliances();
					free_char(d.character);
					return;
				}
				ch = d.character;
				ch->desc == NULL;
				d.character = NULL;
				ch->fake = TRUE;
				ch->pcdata->alliance = i;
				save_char_obj(ch);
				free_char(ch);
			}
			free_string(name_nocol);
			save_alliances();
			return;
		} else {
			name_nocol2 = str_dup(strip_color(alliance_table[i].name,"@@"));
			if ( !str_cmp(name_nocol2,name_nocol) )
			{
				log_f("The alliance already exists.\n\r");
				free_string(name_nocol);
				free_string(name_nocol2);
				return;
			}
			if ( !str_cmp(alliance_table[i].leader,ch->name) )
			{
				log_f("A leader of one alliance tried to create a new.\n\r");
				free_string(name_nocol);
				free_string(name_nocol2);
				return;
			}
		}
		free_string(name_nocol2);
	}
	free_string(name_nocol);
	free_string(name_nocol2);
	log_f("No alliance slot found.\n\r");
	return;
}

void do_setowner( CHAR_DATA *ch, char *argument )
{
	int i = ch->pcdata->alliance;
	DESCRIPTOR_DATA d;
	CHAR_DATA *wch;
	bool load = FALSE;

	if ( i == -1 )
	{
		send_to_char( "You are not a member of an alliance, not to mention leader.\n\r", ch );
		return;
	}
	if ( str_cmp(alliance_table[i].leader,ch->name) )
	{
		send_to_char( "You are not the leader of this alliance.\n\r", ch );
		return;
	}
	if ( ( wch = get_char_world(ch,argument) ) == NULL )
	{
		bool found = FALSE;

		found = load_char_obj( &d, argument, FALSE );
		if ( !found )
		{
			send_to_char( "No such player.\n\r", ch );
			free_char(d.character);
			return;
		}
		wch = d.character;
		d.character = NULL;
		wch->desc = NULL;
		load = TRUE;
//		send_to_char( "That person is not online.\n\r", ch );
//		return;
	}
	if ( wch->pcdata->alliance != i )
	{
		send_to_char( "The person must be a member of your alliance.\n\r", ch );
		if ( load )
		{
			free_char(wch);
			wch = NULL;
		}
		return;
	}
	free_string(alliance_table[i].leader);
	alliance_table[i].leader = str_dup(capitalize(wch->name));
	if ( load )
	{
		free_char(wch);
		wch = NULL;
		send_to_char( "Ok.\n\r", ch );
		return;
	}
	send_to_char( "Ok.\n\r", ch );
	send_to_char( "You have been set as the leader of your alliance.\n\r", wch );
	return;
}
void do_alliprop( CHAR_DATA *ch, char *argument )
{
    int i = ch->pcdata->alliance;
    int x;
    char buf[MSL];
    if(i != -1)
    {
	send_to_char("You have to leave your current alliance before proposing another one.\n", ch);
	return;
    }
    
    if(!strcmp(argument, ""))
    {
	send_to_char("Syntax: alliprop <title>\n\r", ch);
    	return;
    }

    if(strcmp(palliance_table[MAX_ALLIANCE-1].name, ""))
    {
	send_to_char("Another alliance can't be proposed at this time. Try again later.\n\r", ch);
        return;
    }

    for(x=0;x < MAX_ALLIANCE;x++)
    {
	if(!strcmp(palliance_table[x].name, ""))
	    break;
	if(!strcmp(palliance_table[x].leader, ch->name))
	{
	    send_to_char("You can not propose more than one alliance at a time.\n\r", ch);
	    return;
	}
    }
    
    strcpy(palliance_table[x].name, argument);
    strcpy(palliance_table[x].leader, ch->name);
    strcpy(palliance_table[x].support[0], "None");
    strcpy(palliance_table[x].support[1], "None");
    sprintf(buf, "The alliance %s has been proposed in slot %d\n", argument, x);
    send_to_char(buf, ch);
    return;
}

void save_palliance()
{
    FILE *fp;
    int i;

    if((fp = fopen(PALLIANCES_FILE, "w")) == NULL)
    {
	printf("Can't open PALLIANCES_FILE, %s", PALLIANCES_FILE);
	exit(0);
    }
    
    for(i=0;i<MAX_ALLIANCE;i++)
    {
	if(!strcmp(palliance_table[i].name, ""))
	   break;
	fputs(palliance_table[i].name, fp); fputs("\n", fp);
	fputs(palliance_table[i].leader, fp); fputs("\n", fp);
	fputs(palliance_table[i].support[0], fp); fputs("\n", fp);
	fputs(palliance_table[i].support[1], fp); fputs("\n", fp);
    }
    
    fprintf(fp, "#DONE");
    fclose(fp);
    return;
}

void load_palliance()
{
	FILE *fp;
	int i;

	log_f("Loading %s\n\n", PALLIANCES_FILE);

	if((fp = fopen(PALLIANCES_FILE, "r")) == NULL)
	{
		log_f("Can't open PALLIANCES_FILE, %s\n", PALLIANCES_FILE);
		exit(0);
	}

	for(i=0;i<MAX_ALLIANCE;i++)
	{
		fgets(palliance_table[i].name, MSL, fp);
		if(!strncmp(palliance_table[i].name, "#DONE", 5))
		{
			strcpy(palliance_table[i].name, "");
			break;
		}
                palliance_table[i].name[strlen(palliance_table[i].name)-1] = '\0';

		fgets(palliance_table[i].leader, MSL, fp);
		if(!strncmp(palliance_table[i].leader, "#DONE", 5))
		{
			log_f("Incomplete PALLIANCE data. Please correct.");
			exit(0);
		}
                palliance_table[i].leader[strlen(palliance_table[i].leader)-1]= '\0';

                fgets(palliance_table[i].support[0], MSL, fp);
                if(!strncmp(palliance_table[i].support[0], "#DONE", 5))
                {
                        log_f("Incomplete PALLIANCE data. Please correct.");
                        exit(0);
                }
                palliance_table[i].support[0][strlen(palliance_table[i].support[0])-1] = '\0';

		fgets(palliance_table[i].support[1], MSL, fp);
		if(!strncmp(palliance_table[i].support[1], "#DONE", 5))
		{
			log_f("Incomplete PALLIANCE data. Please correct.");
			exit(0);
		}
                palliance_table[i].support[1][strlen(palliance_table[i].support[1])-1] = '\0';

	}
	return;
}	

void do_allisupport(CHAR_DATA *ch, char *argument)
{
	int x;
	
	if(!strcmp(argument, ""))
	{
		send_to_char("Syntax: allisupport ##  (ex: allisupport 25)\n\r", ch);
		return;
	}

	if(!(isdigit((int) argument[0]) && isdigit((int) argument[1])))
	{
		send_to_char("Syntax: allisupport ##  (ex: allisupport 25)\n\r", ch);
		return;
	}
	
	x = atoi(argument);
	if(!strcmp(palliance_table[x].leader, ch->name))
	{
		send_to_char("You are the leader, you already lent your support.\n\r", ch);
		return;
	}
	
	if(strcmp(palliance_table[x].support[0], "None")) // If someone is already supporting this alliance
	if(strcmp(palliance_table[x].support[1], "None"))
	{
		send_to_char("No more support needed. If you see this, contact an immortal.", ch);
		return;
	} else 
	if(strcmp(palliance_table[x].support[0], ch->name))
	{
		strcpy(palliance_table[x].support[1], ch->name);
		do_createalliance(palliance_table[x].leader, palliance_table[x].name);
		for( ;x < MAX_ALLIANCE-1;x++)
		{
			strcpy(palliance_table[x].name, palliance_table[x+1].name);
			strcpy(palliance_table[x].leader, palliance_table[x+1].leader);
			strcpy(palliance_table[x].support[0], palliance_table[x+1].support[0]);
			strcpy(palliance_table[x].support[1], palliance_table[x+1].support[1]);
		}
		palliance_table[MAX_ALLIANCE].name[0] = '\0';
		palliance_table[MAX_ALLIANCE].leader[0] = '\0';
		strcpy(palliance_table[MAX_ALLIANCE].support[0], "None");
		strcpy(palliance_table[MAX_ALLIANCE].support[1], "None");
	} else {
		send_to_char("You already support this alliance!\n\r", ch);
		return;
	}
	else
		strcpy(palliance_table[atoi(argument)].support[0], ch->name);

	send_to_char("You have lent your support to that alliance.\n\r", ch);
	return;
}
