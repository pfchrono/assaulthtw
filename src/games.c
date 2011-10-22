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

#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

DECLARE_DO_FUN(do_gcommands);
DECLARE_DO_FUN(do_nuke_start);
DECLARE_DO_FUN(do_nuke_pass);
const   struct  cmd_type       game_cmd_table   [] =
{ 
    { "commands",	do_gcommands, 	0,    0,  LOG_NORMAL },
    { "help",		do_gcommands, 	0,    0,  LOG_NORMAL },
    { "exit",  		do_nukem,   	POS_NUKEM,    0,  LOG_NORMAL },
    { "look", 		do_look,   	POS_NUKEM,    0,  LOG_NORMAL },
    { "nukem", 		do_nukem,   	POS_NUKEM,    0,  LOG_NORMAL },
    { "ooc", 		do_ooc,   	0,    0,  LOG_NORMAL },
    { "pass", 		do_nuke_pass, 	POS_NUKEM,    0,  LOG_NORMAL },
    { "say", 		do_say,   	0,    0,  LOG_NORMAL },
    { "start", 		do_nuke_start, 	POS_NUKEM,    0,  LOG_NORMAL },
    { "tell", 		do_tell,   	0,    0,  LOG_NORMAL },
    { "who", 		do_who,   	0,    0,  LOG_NORMAL },

    { "",               0,              POS_DEAD,        0,  LOG_NORMAL }
};

void do_darts( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *board;
	BUILDING_DATA *bld;
	bool bboard = FALSE;
	extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];
	char arg[MSL];

	argument = one_argument(argument,arg);

	if ( ( bld = ch->in_building ) == NULL || !complete(bld) || bld->type != BUILDING_CASINO )
	{
		send_to_char( "You must be in a casino to play.\n\r", ch );
		return;
	}
	for ( board = map_obj[ch->x][ch->y];board != NULL; board = board->next_in_room )
	{
		if ( board->z == bld->z && board->item_type == ITEM_DART_BOARD )
		{
			bboard = TRUE;
			break;
		}
	}
	if ( arg[0] == '\0' )
		send_to_char( "\n\rSyntax: Darts begin - Begins a new dart game (Costs $30)\n\r        Darts throw <1-100> - Throw a dart.\n\r", ch );
	else if ( !str_prefix(arg,"begin") )
	{
		if ( bld->value[5] != 0 ) mreturn("There is already a game in progress.\n\r", ch );
		if ( bboard )
		{
			send_to_char( "There is already an active board here.\n\r", ch );
			return;
		}
		if ( ch->money < 30 )
			mreturn("You do not have the $30 to start the game.\n\r", ch );
		ch->money -= 30;
		bld->value[5] = 1;
		board = create_object(get_obj_index(OBJ_VNUM_DART_BOARD),0);
		board->level = number_range(1,100);
		board->x = ch->x;
		board->y = ch->y;
		board->z = ch->z;
		obj_to_room(board,get_room_index(ROOM_VNUM_WMAP));
		board->in_building = bld;
		board->value[0] = 0;
		board->value[1] = 0;
		board->value[2] = 0;
		send_to_loc( "A new game of Darts has started!\n\r", ch->x, ch->y, ch->z );
		return;
	}
	else if ( !str_prefix(arg,"throw") )
	{
		int hit;
		int score;
		char buf[MSL];

		if ( !bboard )
		{
			send_to_char( "You didn't start a game of darts!\n\r", ch );
			return;
		}
		if ( !is_number(argument) || (hit=atoi(argument)) <= 0 || hit > 100 )
		{
			send_to_char( "Syntax: darts throw <1-100>\n\r", ch );
			return;
		}
		score = (board->level>hit) ? board->level - hit : hit - board->level;
		if ( board->value[0] == 0 )
			board->value[0] = hit;
		else if ( board->value[1] == 0 )
			board->value[1] = hit;
		else if ( board->value[2] == 0 )
			board->value[2] = hit;

		board->value[3] += score;
		if ( score != 0 )
		{
			score = (score / 10) + 1;
			if ( score > 4 )
			{
				act( "You throw a dart, and completely miss the board!", ch, NULL, NULL, TO_CHAR );
				act( "$n throwd a dart, and completely misses the board!", ch, NULL, NULL, TO_ROOM );
			}
			else
			{
				sprintf( buf, "You throw a dart, hitting %d ring%s away from the center!", score, score == 1 ? "" : "s" );
				act( buf, ch, NULL, NULL, TO_CHAR );
				sprintf( buf, "$n throws a dart, hitting %d ring%s away from the center!", score, score == 1 ? "" : "s" );
				act( buf, ch, NULL, NULL, TO_ROOM );
			}
		}
		else
		{
			act( "You throw a dart, hitting the bullseye!", ch, NULL, NULL, TO_CHAR );
			act( "$n throws a dart, hitting the bullseye!", ch, NULL, NULL, TO_ROOM );
		}
		if ( board->value[2] != 0 )
		{
			CHAR_DATA *wch;
			int reward=0;
			if ( board->value[3] < 75 ) reward = 100 - (board->value[3]*1.3);
			sprintf( buf, "Game over! The Results:\n\r\n\rRound 1: %d\n\rRound 2: %d\n\rRound 3: %d\n\rMiss Score: %d\n\r\n\rThe target was: %d\n\rReward Money: $%d.\n\r", board->value[0], board->value[1], board->value[2], board->value[3], board->level,reward );
			for ( wch= map_ch[board->x][board->y][board->z];wch;wch = wch->next_in_room )
			{
				send_to_char(buf,wch);
				if ( reward > 0 )
					gain_money(wch,reward);
			}
			bld->value[5] = 0;
			extract_obj(board);
		}		
	}
	return;
}
void do_where ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	bool world = FALSE;
	char buf[MSL];
	char buf2[MSL];
	char wbuf[MSL];
	int range = 10;
	int lap=0;
	int bombs;

	if ( IS_IMMORTAL(ch) )
		lap = 999;
	else if ( paintball(ch) )
		lap = 1;
	else
	{
		if ( (obj = get_best_laptop(ch))!= NULL )
				lap = obj->value[0];
	}

	buf2[0] = '\0';
	if ( !paintball(ch) && !sysdata.killfest )
	{
		world = TRUE;
	}
	for ( wch = first_char;wch;wch = wch->next )
	{
		if ( wch == ch ) continue;
		if ( !world && !paintball(wch) && !sysdata.killfest )
			continue;
		if ( world && IS_IMMORTAL(wch) )
			continue;
		if ( wch->z != ch->z && !sysdata.killfest )
			continue;
		else if ( world && (wch->x + range < ch->x || wch->x - range > ch->x || wch->y + range < ch->y || wch->y - range > ch->y ) )
		{
			if ( IS_SET(wch->effect,EFFECT_TRACER) && wch != ch )
				sprintf( buf2, "@@W%s @@gat @@a%d@@g/@@a%d@@N\n\r", wch->name, wch->x, wch->y );
			continue;
		}
		wbuf[0] = '\0';
		if ( ( obj = get_eq_char(wch,WEAR_HOLD_HAND_L) ) == NULL || obj->item_type != ITEM_WEAPON ) 
			if ( (obj = get_eq_char(wch,WEAR_HOLD_HAND_R)) == NULL || obj->item_type != ITEM_WEAPON )
				obj = NULL;
		if ( lap >= 6 )
			sprintf(wbuf+strlen(wbuf),"  HP: %d/%d", wch->hit,wch->max_hit );
		if ( obj != NULL && lap >= 4 )
			sprintf(wbuf+strlen(wbuf),"  Weapon: %s@@g", obj->short_descr );
		if ( lap >= 7 )
		{
			bombs = 0;
			for ( obj=wch->first_carry;obj;obj = obj->next_in_carry_list ) if ( obj->item_type == ITEM_BOMB ) bombs++;
			sprintf(wbuf+strlen(wbuf), "  Bombs: %d", bombs );
		}
		
		
		sprintf( buf, "@@W%s @@gat @@a%d@@g/@@a%d%s%s%s\n\r", (lap>0)?wch->name:"Unknown Enemy", wch->x, wch->y, (wbuf[0]!='\0')?"  @@W[@@g":"",wbuf,(wbuf[0]!='\0')?"  @@W]":"" );
		send_to_char( buf, ch );
	}

	if ( lap == 0 )
	{
		send_to_char("@@eTip:@@W Carrying laptops with you may give you more information about the enemies nearby!\n\r", ch );
	}
	if ( buf2[0] != '\0' )
	{
		send_to_char( "@@eYou could also trace the following:\n\r", ch );
		send_to_char(buf2,ch);
	}
	return;
}

void do_nukem( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *wch;
	int p=0;
	char buf[MSL];
	bool in = FALSE;
	bool start = FALSE;

	for ( wch = map_ch[2][2][Z_PAINTBALL];wch;wch = wch->next_in_room )
	{
		if ( wch->z != Z_PAINTBALL )
			continue;
		p++;
		if ( wch == ch )
			in = TRUE;
		if ( wch->first_carry )
			start = TRUE;
	}
	if ( in )
	{
		BUILDING_DATA * bld;
		if ( ch->first_carry )
		{
			send_to_char( "Pass the bomb first!\n\r", ch );
			return;
		}
		if ( p == 2 )
		{
			send_to_char( "Come on, finish this last round!\n\r", ch );
			return;
		}
		sprintf( buf, "%s has left the Nukem Arena.", ch->name );
		info(buf,0);
		ch->fighttimer += 200;
		for ( bld = first_building;bld;bld = bld->next )
		{
			if ( bld->type != BUILDING_HQ )
				continue;
			if ( str_cmp(bld->owned,ch->name) )
				continue;
			move(ch,bld->x,bld->y,bld->z);
			do_look(ch,"");
			return;
		}
		move(ch,number_range(100,400),number_range(100,400),Z_GROUND);
		do_look(ch,"");
		return;
	}
	else
	{
		if ( start )
		{
			send_to_char( "Sorry, the game has already started.\n\r", ch );
			return;
		}
		if ( p >= 10 )
		{
			send_to_char( "There can only be 10 people inside.\n\r", ch );
			return;
		}
		if ( ch->z == Z_PAINTBALL || IN_PIT(ch) )
		{
			send_to_char( "Not from here.\n\r", ch );
			return;
		}
		if ( !ch->in_building || ch->in_building->type != BUILDING_HQ )
		{
			send_to_char( "You can only go there from your HQ.\n\r", ch );
			return;
		}
		if ( ch->in_vehicle )
		{
			send_to_char( "Exit your vehicle first.\n\r", ch );
			return;
		}
		if ( ch->first_carry )
		{
			send_to_char( "You may not carry any items into the game arena.\n\r", ch );
			return;
		}
		move(ch,2,2,Z_PAINTBALL);
		sprintf( buf, "%s has entered the Nukem Arena. There are %d people inside.", ch->name, p+1 );
		info(buf,0);
		ch->position = POS_NUKEM;
		return;
	}
	return;
}

void game_interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    bool found;
      
    /*  
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
        argument++;
    if ( argument[0] == '\0' )
        return;
     
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) ) 
    {
        command[0] = argument[0];
        command[1] = '\0';
        argument++;
        while ( isspace(*argument) )
            argument++;
    } 
    else
    {  
        argument = one_argument( argument, command );
    }
     
    
    /*
     * Look for command in command table.
     */
    found = FALSE;
    for ( cmd = 0; game_cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( command[0] == game_cmd_table[cmd].name[0]
        &&   !str_prefix( command, game_cmd_table[cmd].name )
	&&   (game_cmd_table[cmd].position == ch->position || game_cmd_table[cmd].position == 0)
        &&   get_trust(ch) >= game_cmd_table[cmd].level )
        {
            found = TRUE;
            break;
        }
    }

   if ( IS_BUSY(ch) && str_cmp(game_cmd_table[cmd].name,"stop") )
   {
        send_to_char( "Finish or 'stop' your other process, first.\n\r", ch );
        return;
   }
            
    /*
     * Log and snoop.
     */
    if ( game_cmd_table[cmd].log == LOG_NEVER )
        strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );
     
    if ( ( IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||  game_cmd_table[cmd].log == LOG_ALWAYS )
    {
        sprintf( log_buf, "Log %s: %s", ch->name, logline );
        log_string( log_buf );
    }
    if ( ch->desc != NULL && ch->desc->snoop_by != NULL ) 
    {
        write_to_buffer( ch->desc->snoop_by, "% ",    2 );
        write_to_buffer( ch->desc->snoop_by, logline, 0 );
        write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }   
     
    if ( !found )
    {
	send_to_char( "Invalid command.\n\r", ch );
        return;
    }
    
    
    (*game_cmd_table[cmd].do_fun) ( ch, argument );
        
    tail_chain( );
    return;
}

void do_gcommands( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];   
   char out[MAX_STRING_LENGTH];
   int cmd;
   int col = 0;
    
   sprintf( out, "Game Commands Available to You:\n\r" );
    
   for ( cmd = 0; game_cmd_table[cmd].name[0] != '\0'; cmd++ )
   {
      if ( game_cmd_table[cmd].level > get_trust( ch ) )
         continue; 
      sprintf( buf, "%-20s    ", game_cmd_table[cmd].name );
      safe_strcat( MSL, out, buf );
      if ( ++col % 3 == 0 )
         safe_strcat( MSL, out, "\n\r" );
   }  
   safe_strcat( MSL, out, "\n\r" );
   send_to_char( out, ch );
   return;
}   

void do_nuke_start( CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *wch;
	int p = 0;
	bool in = FALSE;
	bool start = FALSE;
	char buf[MSL];

	for ( wch = map_ch[2][2][Z_PAINTBALL];wch;wch = wch->next_in_room )
	{
		if ( wch->z != Z_PAINTBALL )
			continue;
		p++;
		if ( wch == ch )
			in = TRUE;
		if ( wch->first_carry )
			start = TRUE;
	}
	if ( !in )
	{
		send_to_char( "You're not in the Nukem arena!\n\r", ch );
		move(ch,number_range(100,400),number_range(100,400),Z_GROUND);
		return;
	}
	if ( p == 1 )
	{
		send_to_char( "Playing with yourself? Bah...\n\r", ch );
		return;
	}
	if ( start )
	{
		send_to_char( "The game has already started!\n\r", ch );
		return;
	}
	obj = create_object(get_obj_index(1028),0);
	obj->value[0] = number_range(10,30);
	obj->value[1] = 1;
	obj->x = 2;
	obj->y = 2;
	obj->z = Z_PAINTBALL;
	obj_to_char(obj,ch);
	sprintf( buf, "@@WThe game has started, and @@a%s@@W has the nuke!@@N\n\r", ch->name );
	send_to_loc(buf,2,2,Z_PAINTBALL);
	send_to_char( "@@WYou have the nuke, you must @@aPASS@@W it on to another player!@@N\n\r", ch );
	return;
}
void do_nuke_pass(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	if ( !ch->first_carry )
	{
		send_to_char( "You don't carry the nuke!\n\r", ch );
		return;
	}
	if ( ( victim = get_char_room(ch,argument) ) == NULL )
	{
		send_to_char( "They are not playing here!\n\r", ch );
		return;
	}
	obj = ch->first_carry;
	obj_from_char(obj);
	obj_to_char(obj,victim);
	act( "You pass the nuke to $N!", ch, NULL, victim, TO_CHAR );
	act( "$n passes the nuke to $N!", ch, NULL, victim, TO_NOTVICT );
	act( "$n passes the nuke to you!", ch, NULL, victim, TO_VICT );
	send_to_char( "@@WYou have the nuke, you must @@aPASS@@W it on to another player!@@N\n\r", victim );
	WAIT_STATE(victim,16);
	return;
}
void nuke_blow(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	CHAR_DATA *wch;
	CHAR_DATA *last = ch;
	int p = 0;
	BUILDING_DATA * bld;
	char buf[MSL];
	bool found = FALSE;

	sprintf(buf,"@@a%s @@dgot @@eNUKEM'd@@d!", ch->name );
	info(buf,0);
	for ( bld = first_building;bld;bld = bld->next )
	{
		if ( bld->type != BUILDING_HQ )
			continue;
		if ( str_cmp(bld->owned,ch->name) )
			continue;
		found = TRUE;
		move(ch,bld->x,bld->y,bld->z);
		do_look(ch,"");
	}
	if ( !found )
	{
		move(ch,number_range(100,400),number_range(100,400),Z_GROUND);
		do_look(ch,"");
	}

	for ( wch = map_ch[2][2][Z_PAINTBALL];wch;wch = wch->next_in_room )
	{
		if ( wch->z != Z_PAINTBALL )
			continue;
		last = wch;
		p++;
	}
	if ( p == 1 )
	{
		send_to_char("@@WYou are the winner!@@N\n\r", last);
		sprintf(buf,"@@a%s@@d got everyone @@eNUKEM'd@@d, and is the winner!", last->name );
		last->pcdata->nukemwins++;
		found = FALSE;
		for ( bld = first_building;bld;bld = bld->next )
		{
			if ( bld->type != BUILDING_HQ )
				continue;
			if ( str_cmp(bld->owned,last->name) )
				continue;
			found = TRUE;
			move(last,bld->x,bld->y,bld->z);
			do_look(last,"");
		}
		if ( !found )
		{
			move(last,number_range(100,400),number_range(100,400),Z_GROUND);
			do_look(last,"");
		}
		save_char_obj(last);
		info(buf,0);
		return;
	}
	obj = create_object(get_obj_index(1028),0);
	obj->value[0] = number_range(10,30);
	obj->value[1] = 1;
	obj_to_char(obj,last);
	sprintf( buf, "@@WThe game continues, and @@a%s@@W has the nuke!@@N\n\r", last->name );
	send_to_loc(buf,2,2,Z_PAINTBALL);
	send_to_char( "@@WYou have the nuke, you must @@aPASS@@W it on to another player!@@N\n\r", last );
	return;
}
void do_blackjack(CHAR_DATA *ch, char *argument)
{
	BUILDING_DATA *bld = ch->in_building;
	int card,i;
	char buf[MSL];
	char arg[MSL];
	argument = one_argument(argument,arg);

	if ( !bld || !complete(bld) || bld->type != BUILDING_CASINO )
		mreturn("You must be in a casino to play this.\n\r", ch );
	if ( arg[0] == '\0' )
		mreturn("\n\rSyntax: blackjack bet (amount)\n\r        blackjack hit\n\r        blackjack stand\n\r", ch );
	if ( !str_prefix(arg,"bet") )
	{
		char crd[MSL];
		int bet;

		bet = atoi(argument);
		if ( bet <= 0 || bet > ch->money || bet > 1000 )
		{
			sprintf(buf,"You may bet anywhere between 1 and %ld dollars. You tried betting %d.\n\r", (ch->money>1000)?1000:ch->money, bet );
			send_to_char(buf,ch);
			send_to_char("Syntax: blackjack bet (amount)\n\r", ch );
			return;
		}
		if ( bld->value[5] != 0 )
			mreturn(" There is already a game in progress.\n\r", ch );
		bld->value[0] = 0;
		bld->value[5] = 2;
		bld->value[6] = 0;
		bld->value[7] = 0;
		bld->value[2] = bet;
		ch->money -= bet;
		for (i=0;i<2;i++)
		{
			card = number_range(2,14);
			sprintf(crd,"%d",card);
			sprintf(buf,"@@WYou are dealt a @@a%s@@W.@@N\n\r",  (card<11)?crd:(card==11)?"Jack":(card==12)?"Queen":(card==13)?"King":"Ace");
			send_to_char(buf,ch);
			if ( card==14 )
			{
				bld->value[0]++;
				card = 11;
			}
			else if ( card > 10 )
			{
				card = 10;
			}
			bld->value[6] += card;
			if ( bld->value[6] > 21 && bld->value[0] > 0 )
			{
				bld->value[0]--;
				bld->value[6] -= 10;
			}
		}
		do_blackjack(ch,"show");

		return;
	}
	else if ( bld->value[5] != 2 )
	{
		send_to_char("There isn't a game of blackjack going on. Use Blackjack Bet (amount).\n\r", ch );
		return;
	}
	else if ( !str_prefix(arg,"hit") )
	{
		char crd[MSL];
		card = number_range(2,13);
		sprintf(crd,"%d",card);
		sprintf(buf,"@@WYou are dealt a @@a%s@@W.@@N\n\r",  (card<11)?crd:(card==11)?"Jack":(card==12)?"Queen":(card==13)?"King":"Ace");
		send_to_char(buf,ch);
		if ( card==14 )
		{
			bld->value[0]++;
			card = 11;
		}
		else if ( card > 10 )
		{
			card = 10;
		}
		bld->value[6] += card;
		if ( bld->value[6] > 21 && bld->value[0] > 0 )
		{
			bld->value[0]--;
			bld->value[6] -= 10;
		}
		do_blackjack(ch,"show");
		if ( bld->value[6] >= 21 )
			do_blackjack(ch,"stay");
		return;
	}
	else if ( !str_prefix(arg,"show") )
	{
		sprintf(buf,"@@WYour Cards sum at: @@a%d@@N\n\r", bld->value[6] );
		if ( bld->value[0] > 0 )
			sprintf(buf+strlen(buf),"@@WYou have @@a%d@@W ace(s) counting as 11.\n\r", bld->value[0] );
		send_to_char(buf,ch);
		return;
	}
	else if ( !str_prefix(arg,"stay") || !str_prefix(arg,"stand") )
	{
		int i;
		if ( bld->value[6] > 21 )
		{
			sprintf(buf,"@@WYou hit@@d %d@@W, you bust.\n\r", bld->value[6] );
			send_to_char(buf,ch);
			bld->value[5] = 0;
			bld->value[2] = 0;
			return;
		}
		bld->value[7] = 0;
//		if ( bld->value[6] != 21 )
		{
			for ( i=1;i<10;i++ )
			{
				card = number_range(2,10);
				bld->value[7] += card;
				if ( bld->value[7] > bld->value[6] )
					break;
				if ( bld->value[6] > 19 && bld->value[6] <= 21 && bld->value[6] == bld->value[7] )
					break;
				if ( bld->value[7] > 21 && number_percent() < 33 ) bld->value[7] = number_range(17,21);
			}
			sprintf(buf,"@@WThe Dealer draws @@a%d@@W cards for himself.@@N\n\r", i );
			send_to_char(buf,ch);
		}

		if ( bld->value[7] == bld->value[6] )
		{
			int p,d;
			sprintf(buf,"@@WBoth you and the dealer got @@a%d@@W. Dealer draws a card for each of you, lower card wins.\n\r", bld->value[6] );
			send_to_char(buf,ch);
			p = number_range(2,9);
			d = number_range(2,9);
			if ( p == d && p == 2 ) p++;
			if ( p == d ) p--;
			sprintf(buf,"@@WDealer draws @@a%d@@W, you draw @@a%d@@W.@@N\n\r", d, p );
			send_to_char(buf,ch);
			if ( p > d )
			{
				send_to_char("@@dYou lose.@@N\n\r", ch );
			}
			else
			{
				sprintf(buf,"@@WYou win! @@c(@@a$%d@@c)@@N\n\r", bld->value[2] * 2);
				send_to_char(buf,ch);
				gain_money(ch,bld->value[2]*2);
			}
			bld->value[5] = 0;
			bld->value[2] = 0;
			return;
				
		}
		else if ( bld->value[7] > bld->value[6] && bld->value[7] <= 21 )
		{
			sprintf(buf,"@@WDealer has @@a%d@@W. Your @@d%d@@W loses.@@N\n\r", bld->value[7],bld->value[6] );
			send_to_char(buf,ch);
		}
		else
		{
			int win = bld->value[2] * 2;
			if ( bld->value[6] == 21 ) win += bld->value[2] * 0.5;
			if ( bld->value[7] > 21 )
				sprintf(buf,"@@WDealer busts with @@d%d@@W. @@c(@@a$%d@@c)@@N\n\r", bld->value[7],win );
			else
				sprintf(buf,"@@WDealer has @@d%d@@W. Your @@a%d@@W wins! @@c(@@a$%d@@c)@@N\n\r", bld->value[7],bld->value[6],win );
			send_to_char(buf,ch);
			gain_money(ch,win);
		}
		bld->value[5] = 0;
		bld->value[2] = 0;
		return;
		
	}

	
	
}
