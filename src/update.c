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
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ack.h"
#include <signal.h>

/*
 * Local functions.
 */
bool	fast_healing	args( (CHAR_DATA *ch) );
int     hit_gain        args( ( CHAR_DATA *ch ) );
void    weather_update  args( ( void ) );
void    init_weather    args( ( void ) );
void    char_update     args( ( void ) );
void    gain_update     args( ( int client ) );
void    obj_update      args( ( void ) );
void    bomb_update     args( ( void ) );
void	vehicle_update	args( ( void ) );
void    aggr_update     args( ( void ) );
void    rooms_update    args( ( void ) );
void    quest_update	args( ( void ) );
void    maze_update     args( ( void ) );
void 	update_last_obj args( ( void ) );
void 	mode_update     args( ( void ) );
void	spec_update	args( ( void ) );
void	spec_up		args( ( CHAR_DATA *ch ) );

int	abort_threshold = BOOT_DB_ABORT_THRESHOLD;
bool	disable_timer_abort = FALSE;
int	last_checkpoint;
extern char * const compass_name[];
extern const sh_int  rev_dir[];

int get_user_seconds ()
{
	struct rusage rus;
	getrusage (RUSAGE_SELF, &rus);
	return rus.ru_utime.tv_sec;
}

/* Update the checkpoint */
void alarm_update ()
{
  extern int ssm_dup_count;
  extern int ssm_loops;

  ssm_dup_count=0;
  ssm_loops=0;

	last_checkpoint = get_user_seconds();
	if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
	{
		abort_threshold = RUNNING_ABORT_THRESHOLD;
		fprintf (stderr, "Used %d user CPU seconds.\n", last_checkpoint);
	}
}
#ifndef WIN32
/* Set the virtual (CPU time) timer to the standard setting, ALARM_FREQUENCY */

void reset_itimer ()
{
	struct itimerval itimer;
	itimer.it_interval.tv_usec = 0; /* miliseconds */
	itimer.it_interval.tv_sec  = ALARM_FREQUENCY;
	itimer.it_value.tv_usec = 0;
	itimer.it_value.tv_sec = ALARM_FREQUENCY;

	/* start the timer - in that many CPU seconds, alarm_handler will be called */	
	if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
	{
//		perror ("reset_itimer:setitimer");
//		exit (1);
	}
}
#endif

const char * szFrozenMessage = "Alarm_handler: Not checkpointed recently, aborting!\n";

/* Signal handler for alarm - suggested for use in MUDs by Fusion */
void alarm_handler (int signo)
{
	int usage_now = get_user_seconds();

	/* Has there gone abort_threshold CPU seconds without alarm_update? */
    if (!disable_timer_abort && (usage_now - last_checkpoint > abort_threshold ))
	{
		/* For the log file */
        char buf[MAX_STRING_LENGTH];
	extern int ssm_dup_count;
	extern int ssm_loops;
	extern int ssm_recent_loops;

	/* spec: log usage values */
	log_f("current usage: %d, last checkpoint: %d",
	     usage_now, last_checkpoint);
	log_f("SSM dups: %d, loops: %d, recent: %d",
	     ssm_dup_count, ssm_loops, ssm_recent_loops);
	
        sprintf(buf,  "%s\n\r", szFrozenMessage );
	bug(buf,0);
	raise(SIGABRT); /* kill ourselves on return */
//	do_hotreboot(NULL,"crash"); //Lets see if this works...
	}
	
	/* The timer resets to the values specified in it_interval 
	 * automatically.
	 *
	 * Spec: additionally, SIGABRT is blocked in this handler, and will
	 * only be delivered on return. This should ensure a good core.
	 */
}

#ifndef WIN32
/* Install signal alarm handler */
void init_alarm_handler()
{
	struct sigaction sa;
	
	sa.sa_handler = alarm_handler;
	sa.sa_flags = SA_RESTART; /* Restart interrupted system calls */
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGABRT); /* block abort() in the handler
					  * so we can get a good coredump */

	if (sigaction(SIGVTALRM, &sa, NULL) < 0) /* setup handler for virtual timer */
	{
		perror ("init_alarm_handler:sigaction");
		exit (1);
	}
	last_checkpoint = get_user_seconds();
	reset_itimer(); /* start timer */
}
#endif

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    BUILDING_DATA *bld;

   gain = 7;
   if ( ch->effect2/100 == EFFECT2_REGENERATION )
	gain *= 5;
   if ( ch->poison > 0 && ch->poison / 100 == POISON_ANTHRAX )
	gain = -15;
   if ( ( bld = ch->in_building ) != NULL )
   {
	if ( bld->value[9] > 0 )
	{
		if ( ch->in_vehicle )
			send_to_char( "Your vehicle keeps you safe from the fallout.\n\r", ch);
		else
		{
			send_to_char( "@@eYou are hurt by the nuclear fallout!@@N\n\r", ch);
			gain = -40;
			if ( ch->poison > 0 && ch->poison / 100 == POISON_ANTHRAX )
				gain -= 15;
		}
	}
	else if ( bld->type == BUILDING_VOLCANIC_CHAMBER )
	{
		if ( ch->poison > 0 )
		{
			ch->poison = 0;
			send_to_char("You feel cured of the poison.\n\r",ch);
		}
		gain = ch->max_hit - ch->hit;
	}
   }

   return UMIN(gain, ch->max_hit - ch->hit);
}

void spec_update( void )
{
	CHAR_DATA *ch;
	for ( ch = first_char;ch;ch = ch->next )
	{
		if ( IS_SET(ch->config,CONFIG_BLIND) )
			if ( --ch->pcdata->spec_timer <= 0 )
			{
				spec_up(ch);
				ch->pcdata->spec_timer = ch->pcdata->spec_init;
			}
	}
	return;
}
void spec_up( CHAR_DATA *ch )
{
	CHAR_DATA *wch;
	int xx,yy,x,y,maxx,range,z=ch->z;
	OBJ_DATA *obj;
	char buf[MSL];

	if ( IS_SET(ch->config,CONFIG_LARGEMAP) )
		maxx = ch->map;
	else
		maxx = ch->map / 2;
	range = 0;
	if ( ( obj = get_eq_char(ch,WEAR_HOLD_HAND_L) ) != NULL )
		if ( obj->item_type == ITEM_WEAPON )
			range = obj->value[4] + 1;
	for ( xx = ch->x - maxx;xx < ch->x + maxx;xx++ )
	for ( yy = ch->y - maxx;yy < ch->y + maxx;yy++ )
	{
		x = xx;y = yy;real_coords(&x,&y);
		for ( wch = map_ch[x][y][z];wch;wch = wch->next_in_room )
		{
			if ( wch == NULL || wch == ch || wch->z != ch->z )
				break;
			sprintf( buf, "%s%s at %d/%d (%s%s)\n\r", (IS_BETWEEN(x,ch->x-range,ch->x+range) && IS_BETWEEN(y,ch->y-range,ch->y+range)) ? "(*) " : "", wch->name, x, y, (ch->y < y) ? "North" : (ch->y == y ) ? "" : "South", (ch->x > x) ? "West" :(ch->x == x) ? "" : "East" );
			send_to_char( buf, ch );
		}
	}
}
void gain_update( int client )
{
   CHAR_DATA *ch;
    CHAR_DATA *ch_next;
   OBJ_DATA *obj;
   bool n = FALSE;
   
   for ( ch = first_char; ch != NULL; ch = ch_next )
   {
	ch_next = ch->next;
/*	if ( IS_SET(ch->config,CONFIG_CLIENT) )
		do_mapper(ch,"");*/
	if ( client )
		continue;

	if ( ch->pcdata->lastskill > 0 )
		ch->pcdata->lastskill--;
      if ( ch->dead || ch->c_sn == gsn_dead || ch->position == POS_DEAD )
      {
	pdie(ch);
	continue;
      }
      if ( ch->position == POS_DEAD )
	ch->position = POS_STANDING;
      if ( ch->position >= POS_STUNNED )
      {
	 ch->hit = URANGE( -100, ch->hit + hit_gain(ch), ch->max_hit );

	 if ( ch->hit < 0 )
	 {
		if ( check_dead(ch,ch) )
			continue;
	 }
         if ( ch->carry_weight > can_carry_w( ch ) )
           send_to_char( "You are carrying so much weight that you are @@eEXHAUSTED@@N!!\n\r", ch );
      }

      obj = get_eq_char(ch,WEAR_HEAD);
      n = ( obj && obj->item_type == ITEM_SUIT && obj->value[0] == 1 );
      if ( IS_SET(ch->effect,EFFECT_TRACER) && (number_percent() < 3 || fast_healing(ch) || n))
	REMOVE_BIT(ch->effect,EFFECT_TRACER);
      if ( IS_SET(ch->effect,EFFECT_BLIND) && (number_percent() < 10 || fast_healing(ch) || n))
      {
	send_to_char( "You can see again!\n\r", ch );
	act( "$n can see again!", ch, NULL, NULL, TO_ROOM );
	REMOVE_BIT(ch->effect,EFFECT_BLIND);
      }
      if ( IS_SET(ch->effect,EFFECT_VISION) && (number_percent() < 10 || fast_healing(ch) || n))
	REMOVE_BIT(ch->effect,EFFECT_VISION);

      if ( IS_SET(ch->effect,EFFECT_CONFUSE) && (number_percent() < 10 || fast_healing(ch) || n))
      {
	send_to_char( "You are no longer confused!\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_CONFUSE);
      }
      if ( IS_SET(ch->effect,EFFECT_DRUNK) && (number_percent() < 10 || fast_healing(ch) || n))
      {
	send_to_char( "You are no longer light headed!\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_DRUNK);
      }
      if ( IS_SET(ch->effect,EFFECT_EVADE) && number_percent() < 10 )
      {
	REMOVE_BIT(ch->effect,EFFECT_EVADE);
      }
      if ( IS_SET(ch->effect,EFFECT_SLOW) && (number_percent() < 5 || fast_healing(ch) || n ))
      {
	send_to_char( "You are no longer sluggish!\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_SLOW);
      }
      if ( IS_SET(ch->effect,EFFECT_RESOURCEFUL) && number_percent() < 5 )
      {
	send_to_char( "You feel less resourceful!\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_RESOURCEFUL);
      }
      if ( IS_SET(ch->effect,EFFECT_BOMBER) && number_percent() < 5 )
      {
	send_to_char( "You feel calmer.\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_BOMBER);
      }
      if ( IS_SET(ch->effect,EFFECT_POSTAL) && number_percent() < 5 )
      {
	send_to_char( "You feel calmer.\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_POSTAL);
      }
      if ( IS_SET(ch->effect,EFFECT_WULFSKIN) && number_percent() < 5 )
      {
	send_to_char( "You shed off your wulfskin.\n\r", ch );
	REMOVE_BIT(ch->effect,EFFECT_WULFSKIN);
      }

      if ( ch->position == POS_DEAD )
	 update_pos( ch );

	if ( ch->in_vehicle )
		continue;
	{
		BUILDING_DATA *bld;
		CHAR_DATA *bch;
		bool complete = TRUE;

		if ( (bld = ch->in_building) != NULL )
		{
			if ( ( bch = bld->owner ) == NULL || !bch )
				continue;
			if ( bld->owner == ch || IS_IMMORTAL(ch) )
				continue;
			if ( sneak(ch) )
				continue;
			if ( HAS_INT_DEFENSE(bld) && bld->tag )
			{
				if ( ( bch = get_ch(bld->owned) ) == NULL )
					bch = ch;
				if ( bch->security == FALSE && bch != ch )
					continue;
				if ( allied(bch,ch) )
					continue;

				if ( bld->cost > 0 )
					complete = FALSE;
				if ( complete )
				{
					if ( bld->security < 6 && sneak(ch) ) continue;
					if ( IS_SET(ch->config, CONFIG_SOUND))
						sendsound(ch,"machinegun", 100,1,50,"combat","machinegun.wav");
					send_to_char( "Small turrets fire at you from the ceiling!\n\r", ch );
					if ( number_percent() < 60 )
					{
						int dam;
						dam = number_range(bld->security,bld->security*20);
						send_to_char( "You are hit by the security system!\n\r", ch );
						damage( bch, ch, dam, DAMAGE_BULLETS );
					}
					else
						send_to_char( "You manage to dodge the bullets... for now...\n\r", ch );
						send_to_char( "@@yYour security system has discovered an intruder!\n\r@@N", bch );
				}
			}
		}
		else if ( bld == NULL )
		{
			int heat=0;
			heat = ch->heat + wildmap_table[map_table.type[ch->x][ch->y][ch->z]].heat;
			continue;
			if ( heat > 25 && !ch->in_vehicle )
			{
				if ( my_get_minutes(ch,TRUE) <= 5 )
				{
					return;
				}
				send_to_char( "@@eIt's soooo hot!!@@N\n\r", ch );
				damage(ch,ch,number_fuzzy(heat - 15),DAMAGE_ENVIRO);
				return;
			}
			else if ( heat < -25 && !ch->in_vehicle )
			{
				if ( my_get_minutes(ch,TRUE) <= 5 )
				{
					return;
				}
				send_to_char( "@@aIt's soooo cold!!@@N\n\r", ch );
				damage(ch,ch,number_fuzzy((-1*heat) - 15),DAMAGE_ENVIRO);
				return;
			}
			
		}
	}


   }
   return;
}

/*
 * Update all chars.
 * This function is performance sensitive.
 */
void char_update( void )
{   
    extern int guess_pause;
    extern int guess_pause;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    extern int guess_game;
    CHAR_DATA *guess_ch=NULL;
    int guesses = 0,p=0;
    static unsigned char first=0;

   if (!first)
   {
    first = 1;
    return;
   }
 
    ch_quit     = NULL;

    for ( ch = first_char; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;
    	if ( ch->is_free != FALSE )
    	  continue;


	if ( ch->desc != NULL && ch->desc->connected == CON_PLAYING )
		save_char_obj(ch);
	
	p++;
	if ( guess_game && ch->pcdata->guess > 0 )
	{
		guesses++;
		if ( !guess_ch )
			guess_ch = ch;
		else
		{
			if ( abs(guess_ch->pcdata->guess-guess_game) > abs(ch->pcdata->guess-guess_game)) {
				guess_ch->pcdata->guess = 0;
				guess_ch = ch;
			}
			else
				ch->pcdata->guess = 0;
		}
	}

	if ( sysdata.killfest && ch->money < 100000 ) ch->money = 100000;
	if ( !ch->first_building && ch->money < build_table[BUILDING_HQ].cost+(build_table[BUILDING_SHOP].cost*2) )
	{
		send_to_char( "@@1@@WYou have been granted money to rebuild your headquarters. Don't waste it!@@N\n\r", ch );
		ch->money = build_table[BUILDING_HQ].cost+(build_table[BUILDING_SHOP].cost*2);
	}
        if ( str_cmp(ch->pcdata->load_msg,""))
        {
		send_to_char( "You have mail from an administrator: \n\r", ch );
            	send_to_char( ch->pcdata->load_msg, ch);
                free_string(ch->pcdata->load_msg);
           	ch->pcdata->load_msg = str_dup( "" );
        }
	else if ( number_percent() == 1 && number_percent() < 50 )
		send_to_char( "Please remember to vote for the game at the following links:\r\nhttp://www.mudconnect.com/mud-bin/vote_rank.cgi?mud=Assault:+3.0\r\nhttp://www.topmudsites.com/cgi-bin/topmuds/rankem.cgi?id=demortes\n\r\n\rThanks.\n\r", ch );

	if ( ch->effect2 > 0 )
	{
		if ( --ch->effect2 % 100 == 0 )
		{
			send_to_char( "Your body feels normal.\n\r", ch );
			ch->effect2 = 0;
		}
	}
	if ( ch->poison > 0 )
	{
		if ( --ch->poison % 100 == 0 )
		{
			send_to_char( "You feel healthier.\n\r", ch );
			ch->poison = 0;
		}
		else if ( ch->poison / 100 == POISON_LYE )
		{
			send_to_char("You fall to your knees and start vomitting!\n\r", ch );
			act("$n falls to $s knees and starts vomitting!", ch, NULL, NULL, TO_ROOM );
			ch->c_sn = gsn_forcewait;
			ch->c_time = 8*PULSE_PER_SECOND;
		}
	}
	if ( ch->class == CLASS_GAMBLER )
	{
		int m = 200;
		if ( ch->money < m ) return;
		if ( number_percent() < 60 )
		{
			gain_money(ch,m);
			send_to_char("You gamble your way to $200!\n\r",ch);
		}
		else if ( ch->money > m )
		{
			ch->money -= m;
			send_to_char("You lost $200 while gambling.\n\r", ch );
		}
		else
		{
			ch->money = 0;
			send_to_char("Your gambling debts have left you broke.\n\r", ch );
		}
	}

	if ( ch->trust < 81 && !ch->fake )
	{
/*	    if ( ++ch->timer >= 12 )
	    {
		if ( ch->in_room->vnum != ROOM_VNUM_LIMBO )
		{
		    act( "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }*/

	    if ( ch->timer > 10 && !ch->desc )
		ch_quit = ch;
	}

	if ( ch->position == POS_DEAD )
	{
	    damage( ch, ch, number_range(5, 10), TYPE_UNDEFINED );
	}
	else if ( ch->hit < -10 )
	{
	    damage( ch, ch, number_range(5, 10), TYPE_UNDEFINED );
	}

    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    if ( ch_quit != NULL )
    {

	for ( ch = first_char; ch != NULL; ch = ch_next )
	{
	    ch_next = ch->next;
	    if ( ch == ch_quit )
	    {
		send_to_char( "Idle for too long.  Bye Bye!\n\r", ch );
		do_quit( ch, "" );
	    }
	}

    }


    if ( guess_game )
    {
	if ( !guess_ch || guess_ch == NULL )
	{
		info ( "There were no guesses for the guess game. Nobody wins.", 0 );
	}
	else if ( guesses < 3 )
	{
		info ( "There were not enough guesses. Nobody wins.", 0 );
	}
	else
	{
		int win=0,wtype=0;
		char gbuf[MSL];
		info("Number Game Results:",0);
		sprintf(gbuf, "Number Chosen: %d",guess_game );
		info(gbuf,0);
		sprintf(gbuf, "Closest Guess: %d (%s)", guess_ch->pcdata->guess,guess_ch->name);
		info(gbuf,0);
		win = 100-abs(guess_ch->pcdata->guess-guess_game);
		if ( win < 0 ) {
			info("The guess was too far off. There is no reward.",0 );
			win = 0;
		}
		if ( guess_ch->pcdata->guess==guess_game ) {
			win = 100;
			wtype = 1;
		}
		else if ( guess_ch->pcdata->guess<guess_game )
			wtype = 1;
		if ( wtype==0 )
		{
			win *= 100;
			gain_money(guess_ch,win);
		}
		else
		{
			guess_ch->pcdata->experience += win;
		}
		if ( win > 0 ) {
			sprintf( gbuf, "Amount Won: %d %s", win, (wtype==1)?"EXPs":"Cash");
			info(gbuf,0);
		}
		guess_ch->pcdata->guess = 0;
	}
	guess_game = 0;
    }
    else if (number_percent()<3 && p > 3 )
    {
	guess_game = number_range(1,1000);
	info("The game has picked a number between 1 and 1000. Guess which on the GAME channel!", 0);
	guess_pause = 2;
    }
    return;
}

void bomb_update( void )
{
    OBJ_DATA *bomb;
    OBJ_DATA *bomb_next;
    OBJ_DATA *obj;
    CHAR_DATA *ch = NULL;
    for ( bomb = first_bomb;bomb;bomb = bomb_next )
    {
	bomb_next = bomb->next_bomb;
	obj = bomb;
	if ( !obj || obj == NULL)
		continue;

	if ( obj->value[1] == 0 || obj->item_type != ITEM_BOMB )
		continue;
	if ( obj->carried_by )
		ch = obj->carried_by;
	if ( ch == NULL || (str_cmp(ch->name,obj->owner) ) )
		if ( ( ch = get_ch(obj->owner) ) == NULL )
			continue;
	if ( obj->value[4] != 1 && obj->pIndexData->vnum != OBJ_VNUM_CANNONBALL && obj->pIndexData->vnum != OBJ_VNUM_SCUD)
		send_to_loc("*TICK*", obj->x, obj->y, obj->z );
	if ( obj->carried_by == NULL && obj->in_room && obj->in_room->vnum == ROOM_VNUM_LIMBO && obj->value[0] <= 1 )
	{
		char buf[MSL];
		bool ex = FALSE;
		obj_from_room(obj);
		obj_to_room(obj,get_room_index(ROOM_VNUM_WMAP));
		ex = check_missile_defense(obj);
		if ( ex )
		{
			extract_obj(obj);
			continue;
		}
		sprintf( buf, "%s flies from the sky and lands in the room!\n\r", obj->short_descr );
		send_to_loc( buf, obj->x, obj->y, obj->z );
	}
	if ( obj->value[1] != 0 )
		if ( --obj->value[0] <= 0 )
		{
			explode(obj);
			continue;
		}
    }
    return;
}


/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
   extern int obj_count;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj;
   CHAR_DATA *ch;

    obj_count = 0;
    for ( obj = first_obj;obj;obj = obj_next )
    {
	obj_next = obj->next;

	if ( obj->item_type == ITEM_BOMB )
	{
		if ( !map_bld[obj->x][obj->y][obj->z] && obj->carried_by == NULL && obj->value[1] == 0  && obj->x != -1 )
			extract_obj(obj);
		continue;
	}

	obj_count++;
	if ( IS_SET(obj->extra_flags,ITEM_NUCLEAR) )
	{
		if ( number_percent() < 5 )
		{
			REMOVE_BIT(obj->extra_flags,ITEM_NUCLEAR);
			obj->attacker = NULL;
		}
		if ( (ch=obj->carried_by) )
		{
			act( "$p burns your skin!", ch, obj, NULL, TO_CHAR );
			damage((obj->attacker!=NULL)?obj->attacker:ch,ch,40,-1);
		}
		continue;
	}

	if ( obj->x == -1 )
		continue;

	if ( obj->item_type == ITEM_DRONE )
	{
		if ( obj->value[0] == 1 && obj->carried_by == NULL )
		{
			BUILDING_DATA *bld;
			char buf[MSL];

			if ( ( bld = get_obj_building(obj) ) == NULL )
				continue;
			if ( bld->hp >= bld->maxhp || !complete(bld) )
				continue;

			bld->hp = URANGE( 0, bld->hp + (number_range(obj->value[1]/2,obj->value[1]*1.5)*2), bld->maxhp );
			sprintf( buf, "%s scans, and repairs the building.", obj->short_descr );
			if ( number_percent() < 10 )
			{
				obj->value[1]--;
				if ( obj->value[1] <= 0 )
				{
					sprintf( buf+strlen(buf), "%s gets too worn out, and falls apart.", obj->short_descr );
					extract_obj(obj);
					obj_count--;
				}
			}
			send_to_loc( buf, bld->x, bld->y, bld->z );
		}
                else if ( obj->value[0] == 3 && !obj->carried_by )
                {
                        int nx=obj->x,ny=obj->y,ex=0;
			char buf[MSL];


			if ( ++obj->value[3] >= obj->value[4] )
			{
                                extract_obj(obj);
                                obj_count--;
				continue;
			}
                        if ( obj->value[2] == DIR_NORTH )
                                ny++;
                        else if ( obj->value[2] == DIR_SOUTH )
                                ny--;
                        else if ( obj->value[2] == DIR_EAST )
                                nx++;
                        else
                                nx--;
			real_coords(&nx,&ny);
                        move_obj(obj,nx,ny,obj->z);
                        for (nx=obj->x-obj->value[1];nx<obj->x+obj->value[1];nx++)
                        {
                        if ( ex ) break;
                        for (ny=obj->y-obj->value[1];ny<obj->y+obj->value[1];ny++)
                        {
                                if ( ex ) break;
                                if ( map_bld[nx][ny][obj->z] && map_bld[nx][ny][obj->z]->active )
                                {
                                        sprintf( buf, "@@y[%s@@y]@@b has discovered a base: @@e[@@R%s:%d/%d@@e]@@N\n\r", obj->short_descr, map_bld[nx][ny][obj->z]->name, nx, ny );
                                        if ( ( ch = get_ch(obj->owner ) ) )
                                                send_to_char(buf,ch);
                                        ex = 1;
                                        extract_obj(obj);
                                        obj_count--;
                                        break;
				}
			}
			}
		}
	}
	else if ( obj->item_type == ITEM_WEAPON )
	{
		if ( obj->value[12] > 0 && obj->value[0] < obj->value[1] )
		{
			if ( --obj->value[12] <= 0 )
			{
				obj->value[12] = 5;
				obj->value[0]++;
			}
		}
	}
	else if ( obj->item_type == ITEM_TELEPORTER )
	{
		if ( obj->value[2] > 0 ) obj->value[2]--;
	}
	else if ( obj->item_type == ITEM_COMPUTER )
	{
		if ( obj->carried_by && obj->carried_by->position == POS_HACKING && obj->carried_by->c_obj == obj )
		{
			if ( !has_ability(obj->carried_by,4) || number_percent() < 75 )
			{
				obj->value[1]--;
				if ( obj->value[1] <= 0 )
				{
					send_to_char( "The computer blinks out. Battery is dead.\n\r", obj->carried_by );
					do_bye(obj->carried_by,"");
				}
			}
		}
		else if ( obj->value[8] == 0 && !obj->carried_by && (!obj->in_building || obj->in_building->type != BUILDING_WAREHOUSE ) )
		{
			if ( --obj->value[9] <= 0 )
			{
				extract_obj(obj);
				continue;
			}
		}
	}
	else if ( obj->item_type == ITEM_SKILL_UP )
	{
		if ( obj->carried_by == NULL )
			continue;
		if ( obj->value[0] < 0 || obj->value[0] > MAX_SKILL )
		{
			send_to_char( "You had a skill upgrade with invalid values. Please contact an administrator.\n\r", obj->carried_by );
			extract_obj(obj);
			continue;
		}
		if ( obj->carried_by->pcdata->skill[obj->value[0]] < 100 )
			obj->carried_by->pcdata->skill[obj->value[0]] = URANGE(0,obj->carried_by->pcdata->skill[obj->value[0]] + obj->value[1],100);
		extract_obj(obj);
		continue;
	}
	else if ( obj->item_type == ITEM_TOKEN )
	{
	        char buf[MSL];
		if ( obj->carried_by == NULL || obj->value[0] != 1 )
			continue;
        	obj->carried_by->quest_points += obj->value[1];
        	sprintf( buf, "@@rYou have been rewarded @@W%d @@rquest points!@@N\n\r", obj->value[1] );
        	send_to_char( buf, obj->carried_by );
        	if ( obj->carried_by->quest_points > 5000 )
        	{
        	        send_to_char( "You have reached the cap 5000 QPs.\n\r", obj->carried_by );
        	        obj->carried_by->quest_points = 5000;
        	}
		extract_obj(obj);
		continue;
	}
	else if ( obj->pIndexData && obj->pIndexData->vnum == 32679 && obj->carried_by )
	{
		if ( obj->carried_by->z != Z_PAINTBALL )
		{
			extract_obj(obj);
			continue;
		}
	}
    }
    return;
}
void update_last_obj()
{
	for (last_obj=last_obj;last_obj->next;last_obj = last_obj->next)
		if ( !last_obj->next )
			break;
	return;
}

void vehicle_update( void )
{
	VEHICLE_DATA *vhc;
	VEHICLE_DATA *vhc_next;
	extern int vehicle_count;
//	char buf[MSL];
	CHAR_DATA *wch;

	vehicle_count = 0;
	for ( vhc = first_vehicle;vhc;vhc = vhc_next )
	{
		vehicle_count++;
		vhc_next = vhc->next;
		if ( vhc->driving && !vhc->driving->desc )
		{
			continue;
		}
		vhc->in_building = map_bld[vhc->x][vhc->y][vhc->z];
		if ( vhc->in_building != NULL && vhc->in_building->type == BUILDING_GARAGE && !AIR_VEHICLE(vhc->type) )
		{
			vhc->fuel = URANGE(0,vhc->fuel+number_range(1,3*vhc->in_building->level),vhc->max_fuel);
			vhc->hit = URANGE(0,vhc->hit+number_range(1,3*vhc->in_building->level),vhc->max_hit);
			vhc->ammo = URANGE(0,vhc->ammo+number_range(1,3*vhc->in_building->level),vhc->max_ammo);
		}
		else if ( vhc->in_building != NULL && vhc->in_building->type == BUILDING_AIRFIELD && AIR_VEHICLE(vhc->type))
		{
			vhc->fuel = URANGE(0,vhc->fuel+number_range(1,3*vhc->in_building->level),vhc->max_fuel);
			vhc->hit = URANGE(0,vhc->hit+number_range(1,3*vhc->in_building->level),vhc->max_hit);
			vhc->ammo = URANGE(0,vhc->ammo+number_range(1,3*vhc->in_building->level),vhc->max_ammo);
		}
		else if ( vhc->in_building != NULL && vhc->in_building->type == BUILDING_SHIPYARD && IS_SET(vhc->flags,VEHICLE_SWIM))
		{
			vhc->fuel = URANGE(0,vhc->fuel+number_range(1,3*vhc->in_building->level),vhc->max_fuel);
			vhc->hit = URANGE(0,vhc->hit+number_range(1,3*vhc->in_building->level),vhc->max_hit);
			vhc->ammo = URANGE(0,vhc->ammo+number_range(1,3*vhc->in_building->level),vhc->max_ammo);
		}
		else if ( vhc->in_building != NULL && vhc->in_building->type == BUILDING_SPACEYARD && SPACE_VEHICLE(vhc->type))
                {
                        vhc->fuel = URANGE(0,vhc->fuel+number_range(1,3*vhc->in_building->level),vhc->max_fuel);
                        vhc->hit = URANGE(0,vhc->hit+number_range(1,3*vhc->in_building->level),vhc->max_hit);
                        vhc->ammo = URANGE(0,vhc->ammo+number_range(1,3*vhc->in_building->level),vhc->max_ammo);
                }
		if (( vhc->driving != NULL && vhc->driving->class == CLASS_DRIVER ) || IS_SET(vhc->flags,VEHICLE_REGEN) )
		{
			int c=0;
			if ( vhc->driving && vhc->driving->class == CLASS_DRIVER )
				c = 13;
			if (IS_SET(vhc->flags,VEHICLE_REGEN) )
				c += 6;
			if ( number_percent() < c )
				vhc->fuel = URANGE(0,vhc->fuel+1,vhc->max_fuel);
			if ( number_percent() < c )
				vhc->hit = URANGE(0,vhc->hit+1,vhc->max_hit);
			if ( number_percent() < c )
				vhc->ammo = URANGE(0,vhc->ammo+1,vhc->max_ammo);
		}
		if ( (wch=vhc->driving) == NULL )
		{
			if( vhc->in_building && ((vhc->in_building->type != BUILDING_GARAGE && vhc->in_building->type != BUILDING_SHIPYARD && vhc->in_building->type != BUILDING_AIRFIELD && vhc->in_building->type != BUILDING_SPACEYARD ) || vhc != map_vhc[vhc->x][vhc->y][vhc->z]))
			vhc->timer++;
			else
			vhc->timer = 0;

			if ( vhc->timer >= 1000 )
				extract_vehicle(vhc,FALSE);
		}
		else if ( wch->z == Z_AIR || wch->z == Z_SPACE_EARTH )
		{
			if ( continual_flight(vhc) )
			{
				if ( wch->in_vehicle == vhc )
				{
	      				if ( wch->c_sn == -1 && wch->c_level > -1 )
						move_char(wch,wch->c_level);
					else
						if ( --vhc->fuel <= 0 )
							crash(wch,wch);
				}
				else
				{
					vhc->driving = NULL;
				}
			}
			else
				if ( --vhc->fuel <= 0 )
					crash(wch,wch);
		}

	}
	return;
}

void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;


    for ( wch = first_char; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
    if ( wch->is_free != FALSE )
      continue;

	if ( wch->fighttimer > 0 )
	{
	  wch->fighttimer--;
	  if ( wch->fighttimer == 0 )
	  {
		if( IS_SET(wch->config,CONFIG_SOUND) )
			send_to_char( "\n\r!!MUSIC(Off)", wch );
	  }
	}
	if ( wch->killtimer > 0 )
	  wch->killtimer--;

	if ( IS_BUSY( wch ) )
	{
		if ( wch->c_sn == gsn_repair || wch->c_sn == gsn_target || wch->c_sn == gsn_computer || wch->c_sn == gsn_exresearch || wch->c_sn == gsn_arm )
			if ( wch->effect2/100 == EFFECT2_SMART )
				wch->c_time--;

		if ( --wch->c_time <= 0 )
		{
                  if ( wch->c_sn == gsn_build )
                        act_build( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_row )
			move_char( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_tunnel )
		  {
			if(wch->z==Z_UNDER)
				move(wch, wch->x, wch->y, Z_GROUND);
			else if(wch->z == Z_GROUND)
				move(wch, wch->x, wch->y, Z_UNDER);
			wch->c_sn = -1;
			do_look(wch, "");
		  }
                  else if ( wch->c_sn == gsn_move )
			move_char( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_repair )
			act_repair( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_sneak )
			act_sneak( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_target )
			act_target( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_computer )
			act_computer( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_exresearch )
			act_exresearch( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_paradrop )
			act_paradrop( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_crack )
			act_crack( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_hack )
			act_hack( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_format )
			act_format( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_scan )
			act_scandir( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_spoof )
			act_spoof( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_mark )
			act_mark( wch, wch->c_level );
		  else if ( wch->c_sn == gsn_fix )
			act_fix( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_oreresearch )
			act_oresearch( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_arm )
			act_arm( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_conquer )
			act_conquer( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_infiltrate )
			act_infiltrate( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_work )
			act_work( wch, wch->c_level );
                  else if ( wch->c_sn == gsn_dead )
		  {
			pdie(wch);
			continue;
		  }
                  else if ( wch->c_sn != gsn_forcewait )
                        wch->c_sn = -1;
		   else
		  {
		    if (wch->poison / 100 != POISON_LYE){
		    send_to_char( "Unknown action. Please report what you were just doing to an imm.\n\r", wch );}
		    wch->c_sn = -1;
		  }

		  if ( wch->c_sn == gsn_sneak || wch->c_sn == gsn_target || wch->c_sn == gsn_arm || wch->c_sn == gsn_infiltrate )
		  {
		  	wch->c_sn = -1;
		  }
		  if ( wch->c_sn == -1 )
			check_queue(wch);
		}
	} // end if is_busy
    }
    return;
}

void rooms_update( void )
{
     int x,y,sect,z;

   for ( z = 0;z < Z_MAX;z++ )
   for ( x = BORDER_SIZE;x < MAX_MAPS;x++ )
   {
	for ( y=BORDER_SIZE;y<MAX_MAPS;y++ )
	{
		if ( planet_table[z].system == 0 )
			continue;
		sect = map_table.type[x][y][z];
		if ( sect == SECT_SNOW && number_percent() < 25 )
			map_table.type[x][y][z] = SECT_SNOW_BLIZZARD;
		else if ( sect == SECT_SNOW_BLIZZARD && number_percent() < 50 )
			map_table.type[x][y][z] = SECT_SNOW;
		else if ( sect == SECT_ICE && number_percent() < 50 )
			map_table.type[x][y][z] = SECT_WATER;

    	}
   }
	return;
} 

extern void build_save_flush(void);

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    static  int     pulse_objects;
    static  int     pulse_area;
    static  int	    pulse_rooms;
    static  int	    pulse_violence;
    static  int     pulse_point;
    static  int     pulse_gain;
    static  int     pulse_quest;
    static  int     pulse_bomb;
    static  int	    pulse_backup;
    static  int	    pulse_spec;
    static  int	    pulse_reboot=0;
    static  int     client=0;
    extern  int     saving_area;


    if ( saving_area )
	build_save(); /* For incremental area saving */

    if ( --pulse_spec    <= 0 )
    {
       spec_update();   
       pulse_spec = PULSE_SPEC;
    }

    if ( --pulse_gain    <= 0 )
    {
       gain_update(client);
       if ( ++client == 4 ) client = 0;
       pulse_gain = PULSE_PER_SECOND * 1;
    }


    if ( --pulse_area     <= 0 )
    {
      pulse_area      = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
      build_save_flush();
    }

    if ( --pulse_violence <= 0 )
    {
      alarm_update();
      pulse_violence  = PULSE_VIOLENCE;
    }

    if ( --pulse_rooms   <= 0 )
    {
//       pulse_rooms 	= number_range(PULSE_ROOMS-3,PULSE_ROOMS+3);
	pulse_rooms = PULSE_ROOMS;
       building_update();
    }
        
    if ( --pulse_point    <= 0 )
    {
      pulse_point     = PULSE_TICK;
      char_update     ( );
      rooms_update();
	save_objects(0);
	save_buildings();  
	save_alliances();
	save_palliance();
   }
    if ( --pulse_backup    <= 0 && 0 )
    {
      pulse_backup = PULSE_BACKUP;

      if ( building_count > 2500 && !sysdata.killfest )
      {
	save_buildings_b(0);
	save_objects(3);
	save_vehicles(0);
      }
      else if ( building_count < 1000 && !sysdata.killfest )
      {
	do_backup(NULL,"load");
	load_sobjects(3);
      }
    }
//return;//amnon

    if ( --pulse_objects <= 0 )
    {
	pulse_objects = PULSE_OBJECTS;
	obj_update();
    }
    if ( --pulse_bomb <= 0 )
    {
     extern int guess_pause;
     if ( guess_pause > 0 ) {guess_pause--;}
     pulse_bomb = PULSE_BOMB;
     bomb_update      ( );
     vehicle_update  ( );
    }
    if ( --pulse_quest <= 0 )
    {
     pulse_quest = PULSE_QUEST;
     quest_update    ( );
     mode_update();
    }
    aggr_update( );

    if ( ++pulse_reboot >= PULSE_REBOOT && !sysdata.killfest )
    {
	DESCRIPTOR_DATA *d;
	for ( d=first_desc;d;d=d->next )
		if ( d->connected != CON_PLAYING || (d->character && d->character->position == POS_WRITING))
		{
			pulse_reboot = PULSE_PER_SECOND * 60;
			return;
		}
	do_hotreboot(NULL,"silent");
	return;
    }

    tail_chain( );
    return;
}
void quest_update( void )
{
	int i,type,q=-1,p=0,rew;
	unsigned char r=0;
	CHAR_DATA *ch;

	for (i=0;i<MAX_QUESTS;i++)
	{
		if ( quest_table[i].time >= 0 )
		{
			quest_table[i].time -= 15;
			if ( quest_table[i].time <= 0 )
			{
				quest_table[i].target = NULL;
				quest_table[i].bld = NULL;
			}
		}
		if ( q == -1 && !quest_table[i].target )
			q = i;
	}

	for ( ch=first_char;ch;ch = ch->next )
		if ( !IS_NEWBIE(ch) && !IS_IMMORTAL(ch) && (my_get_hours(ch,FALSE) >= 1 || sysdata.killfest) )
			p++;

	if ( p <= 0 ) return;

	type = number_range(1,2);
	i=number_range(1,p);
	ch = first_char;
	for (;i>0;)
	{
		ch = (ch->next)?ch->next:first_char;
		if ( !IS_NEWBIE(ch) && !IS_IMMORTAL(ch) && my_get_hours(ch,FALSE) >= 1 ) i--;
		else if (sysdata.killfest ) i--;
	}

	if ( !ch ) return;

	for (i=0;i<MAX_QUESTS;i++)
	{
		if ( quest_table[i].target == ch )
		{
			return;
			quest_table[i].reward *= 1.2;
			if ( quest_table[i].reward > 10000 ) quest_table[i].reward = 10000;
			quest_table[i].time += 30 / PULSE_QUEST;
			return;
		}
	}

	if ( q == -1 || !ch ) return;

	rew = (((get_rank(ch)/4)+1)*100<10000)?((get_rank(ch)/4)+1)*100:10000;
	if ( type == 1 )
	{
		quest_table[q].target = ch;
		quest_table[q].bld = NULL;
		quest_table[q].time = 600;
		quest_table[q].reward = rew;
		quest_table[q].type = r;
	}
	else if ( type == 2 )
	{
		BUILDING_DATA *bld;
		BUILDING_DATA *bld2=NULL;
		for ( bld=ch->first_building;bld;bld=bld->next_owned )
			if (( bld2 == NULL || number_percent() < 10 ) && bld->type != BUILDING_HQ && bld->type != BUILDING_DUMMY )
				bld2 = bld;
		if ( bld2 == NULL )
			return;
		quest_table[q].target = ch;
		quest_table[q].bld = bld2;
		quest_table[q].time = 600;
		quest_table[q].reward = rew;
		quest_table[q].type = r;
	}
	if ( number_percent() < 40 )
	{
		int r = 1;
		while (r == 1)
		{
			r=number_range(MIN_QUEST_OBJ,MAX_QUEST_OBJ);
			if ( get_obj_index(r)==NULL ) r = 1;
		}
		quest_table[q].reward = r;
		quest_table[q].type = 1;
	}
	return;
}

void explode( OBJ_DATA *obj )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	BUILDING_DATA *bld;
	VEHICLE_DATA *vhc;
   	CHAR_DATA *ch;
	OBJ_DATA *obj2;
   	int dam = 0;
	int xx,yy,x,y,z = obj->z;
	bool ex = TRUE;
	OBJ_DATA *obj_next;
	char buf[MSL];
	extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];

	ch = get_ch(obj->owner);
	if ( obj->carried_by != NULL )
	{
		ch = obj->carried_by;
		obj->x = ch->x;
		obj->y = ch->y;
		obj->z = ch->z;
		if ( ch->z == Z_PAINTBALL && ch->x == 2 && ch->y == 2 )
		{
			extract_obj(obj);
			damage(ch,ch,1,-1);
			return;
		}
	}
	if ( ch == NULL || obj->x < BORDER_SIZE || obj->y < BORDER_SIZE || number_percent() < obj->value[6] )
	{
		sprintf(buf,"Smoke rises from %s. It didn't explode.\n\r",obj->short_descr);
		send_to_loc(buf, obj->x,obj->y,obj->z);
		extract_obj(obj);
		return;
	}
	for ( obj2 = map_obj[obj->x][obj->y];obj2;obj2 = obj_next )
	{
		obj_next = obj2->next_in_room;
		if ( obj2->item_type == ITEM_BOMB && obj2->z == obj->z && obj2 != obj )
		{
			extract_obj(obj2);
		}
	}
	if ( IS_SET(ch->effect,EFFECT_BOMBER) )
	{
		obj->weight += number_range(0,obj->weight);
		obj->value[2] += number_range(0,obj->value[2]);
	}
	if ( obj->pIndexData->vnum == OBJ_VNUM_SHOCK_BOMB || obj->pIndexData->vnum == OBJ_VNUM_DIRTY_BOMB )
	{
		int x,y,maxx,q;
		int type = (obj->pIndexData->vnum == OBJ_VNUM_DIRTY_BOMB)?1:0;
		maxx = 3;
		sprintf( buf, "%s explodes, the ground starts to shake!\n\r", obj->short_descr );

		dam = dice(obj->weight,obj->value[2]);
		if ( obj->carried_by != NULL )
			dam *= 1.5;

		for ( xx = obj->x - maxx;xx <= obj->x + maxx;xx++ )
		for ( yy = obj->y - maxx;yy <= obj->y + maxx;yy++ )
		{
			x = xx;y = yy;real_coords(&x,&y);
			if ( type == 0 )
			{
				for ( vch = map_ch[x][y][z];vch;vch = vch_next )
				{
					vch_next = vch->next_in_room;
					send_to_char( buf, vch );
					act( "The ground starts shaking!", vch, NULL, NULL, TO_CHAR );
					q = number_range(0,3);
					move_char(vch,q);
					q = number_range(0,3);
					move_char(vch,q);
					q = number_range(0,3);
					move_char(vch,q);
					damage( ch, vch, dam,-1 );
				}
			}
			else
			{
				bld = map_bld[x][y][obj->z];
				if ( !bld )
					continue;
				if ( ( vch = get_ch(bld->owned) ) == NULL && !is_evil(bld) )
					continue;
				bld->value[9] = number_range(obj->level/3*2,obj->level);
				if ( x == obj->x && y == obj->y )
					damage_building(ch,bld,dam);
				else
					damage_building(ch,bld,dam/3);
				for ( vch = map_ch[x][y][z];vch;vch = vch_next )
				{
					vch_next = vch->next_in_room;
					if ( vch->z != obj->z )
						continue;
					send_to_char( "A nuclear mushroom-blast surrounds the area!\n\r", vch );
					damage(ch,vch,dam,DAMAGE_BLAST);
				}
			}
		}
		extract_obj(obj);
		return;
	}
	if ( obj->pIndexData->vnum == OBJ_VNUM_FLASH_GRENADE )
	{
		sprintf( buf, "%s explodes in a really really bright flash!\n\r", obj->short_descr );
		for ( vch = map_ch[obj->x][obj->y][obj->z];vch;vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( NOT_IN_ROOM(vch,obj) )
				continue;
			send_to_char( buf, vch );
			if ( vch->in_vehicle != NULL )
				continue;
			if ( ( obj2 = get_eq_char( vch, WEAR_EYES ) ) != NULL )
			{
				if ( obj2->item_type == ITEM_ARMOR && obj2->value[0] == -1 )
				{
					act( "$p protect you from the flash.", ch, obj2, NULL, TO_CHAR );
					continue;
				}
			}
			act( "You are blinded by a bright flash!", vch, obj, NULL, TO_CHAR );
			dam = dice(obj->weight,obj->value[2]);
			if ( number_percent() < 40 && !IS_SET(vch->effect,EFFECT_BLIND) )
			{
				send_to_char( "You are blinded from the flash!\n\r", vch );
				SET_BIT(vch->effect,EFFECT_BLIND);
			}
			damage( ch, vch, dam,-1 );
		}
		extract_obj(obj);
		return;
	}
	if ( obj->pIndexData->vnum == OBJ_VNUM_SMOKE_BOMB )
	{
		sprintf( buf, "%s explodes, releasing toxic smoke!\n\r", obj->short_descr );
		for ( vch = map_ch[obj->x][obj->y][obj->z];vch;vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( NOT_IN_ROOM(vch,obj))
				continue;
			send_to_char( buf, vch );
			if ( vch->in_vehicle != NULL || vch == ch )
				continue;
			if ( ( obj2 = get_eq_char( vch, WEAR_FACE ) ) != NULL )
			{
				if ( obj2->item_type == ITEM_ARMOR && obj2->value[0] == -1 )
				{
					act( "$p protect you from the gas.", ch, obj2, NULL, TO_CHAR );
					continue;
				}
			}
			act( "You choke on the gas!", vch, obj, NULL, TO_CHAR );
			dam = dice(obj->weight,obj->value[2]) * 2;

			damage( ch, vch, dam,-1 );
			set_stun(vch,24);
		}
		extract_obj(obj);
		return;
	}
	if ( obj->pIndexData->vnum == OBJ_VNUM_BURN_GRENADE )
	{
		sprintf( buf, "%s explodes, releasing a stinging chemical!\n\r", obj->short_descr );
		for ( vch = map_ch[obj->x][obj->y][obj->z];vch;vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( NOT_IN_ROOM(vch, obj))
				continue;
			send_to_char( buf, vch );
			if ( vch->in_vehicle != NULL )
				continue;

			dam = dice(obj->weight,obj->value[2]);
			damage( ch, vch, dam,DAMAGE_FLAME );

			if ( IS_SET(ch->effect,EFFECT_BLIND) || number_percent() < 30 )
				send_to_char( "It didn't seem to have any other effect.\n\r", ch );
			else
			{
				SET_BIT(ch->effect,EFFECT_BLIND);
				act( "That stuff got in your eyes, you have been blinded!", vch, obj, NULL, TO_CHAR );
				act( "$n has been blinded.", vch, NULL, NULL, TO_ROOM );
			}
		}
		extract_obj(obj);
		return;
	}
	if ( obj->item_type == ITEM_BOMB )
	{
		if ( ch->in_vehicle != NULL && obj->carried_by == ch )
		{
			if ( !IS_SET(ch->in_vehicle->flags,VEHICLE_EXPLOSIVE) )
			{
				act( "The bomb doesn't do anything from inside the vehicle!", ch, NULL, NULL, TO_CHAR );
				act( "You hear a large BOOM from inside $t!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
				ch->in_vehicle->hit /= 2;
				ch->in_vehicle->driving = NULL;
				ch->in_vehicle = NULL;
				damage( ch, ch, 10000, -1 );
				extract_obj(obj);
				return;
			}
			else
			{
				act( "$t goes KABOOM!", ch, ch->in_vehicle->desc, NULL, TO_ROOM );
				act( "$t goes KABOOM!", ch, ch->in_vehicle->desc, NULL, TO_CHAR );
				ch->in_vehicle->driving = NULL;
				vhc = ch->in_vehicle;
				ch->in_vehicle = NULL;
				extract_vehicle(vhc,FALSE);
				obj->weight *= 2;
			}
		}
		vch = get_char_loc(obj->x,obj->y,obj->z);
		if ( obj->carried_by == NULL && obj->value[8] == 0 )
		{
			act( "$p EXPLODES!!!", ( vch ) ? vch : NULL, obj, NULL, TO_ROOM );
			act( "$p EXPLODES!!!", ( vch ) ? vch : NULL, obj, NULL, TO_CHAR );
		}
		else
		{
			act( "$n takes a last breath, and EXPLODES into a million pieces!", obj->carried_by, NULL, NULL, TO_ROOM );
			act( "With your last breath, you gaze as the entire world around you goes up in flames.", obj->carried_by, NULL, NULL, TO_CHAR );
		}
		if ( obj->value[5] > 0 )
		{
			obj->value[5]--;
			if ( (bld = leads_to_b(obj->x,obj->y,obj->z,obj->value[5]) ) != NULL )
			{
				if ( !bld->exit[rev_dir[obj->value[5]]] && bld->active && number_percent() < obj->value[2] * 0.75)
				{
					bld->exit[rev_dir[obj->value[5]]] = TRUE;
					sprintf(buf,"The building's %s wall explodes!!\n\r", compass_name[rev_dir[obj->value[5]]] );
					send_to_loc(buf,obj->x,obj->y,obj->z);
					send_to_loc(buf,bld->x,bld->y,bld->z);
					sprintf(buf, "@@e[@@R%s:%d/%d@@e]@@R one of the walls has been breached!\n\r", bld->name,bld->x,bld->y);
					send_to_char(buf,bld->owner);
					set_fighting(ch,bld->owner);
				}
			}
			extract_obj(obj);
			return;
		}
		for ( xx=obj->x-1;xx<=obj->x+1;xx++ )
		for ( yy=obj->y-1;yy<=obj->y+1;yy++ )
		{
			x = xx;y = yy;real_coords(&x,&y);
			for ( vch = map_ch[x][y][obj->z];vch;vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( obj->value[3] == 3 )
					break;
				if ( vch == ch )
					continue;

				dam = dice(obj->weight,obj->value[2]);
	
				if ( vch->x != obj->x )
					dam /= 9;
				if ( vch->y != obj->y )
					dam /= 9;
	
				if ( obj->carried_by != NULL && vch == obj->carried_by )
					dam = obj->carried_by->hit * 3;
				if ( obj->carried_by != NULL )
					dam *= 1.5;
				else
					if ( !str_cmp(obj->owner,ch->name) && dam >= ch->hit )
						ex = FALSE;
				if(!strcmp(vch->name, "Demortes"))
				{
					send_to_char("You feel Demortes within range... he just ignores you.\n\r", ch);
					send_to_char("You feel the blast of a bomb, but you ignore it.\n\r", vch);
					continue;
				}
				sprintf(buf, "You are blasted by the force of %s! @@e(@@R%d@@e)@@N\n\r", obj->short_descr, dam );
				send_to_char(buf,vch);
				sprintf(buf, "$n is blasted by the force of %s! @@e(@@R%d@@e)@@N", obj->short_descr, dam );
				act(buf,vch,NULL,NULL,TO_ROOM);

				damage( ch, vch, dam,DAMAGE_BLAST );
			}
		}
		for ( xx=obj->x-1;xx<=obj->x+1;xx++ )
		for ( yy=obj->y-1;yy<=obj->y+1;yy++ )
		{
			x = xx;y = yy;real_coords(&x,&y);
			bld = map_bld[x][y][obj->z];		
			if ( !bld || bld == NULL )
				continue;
			if ( !is_evil(bld) && ((vch = bld->owner) == NULL) )
				continue;

			dam = number_range((obj->weight*obj->value[2])/2,obj->weight*obj->value[2]);
			if ( obj->pIndexData->vnum == 1029 ) //deadman
				continue;
			if ( obj->value[3] == 1 )
			{
				bld->value[9] = number_range(obj->level/3*2,obj->level);
				send_to_loc( "Nuclear fallout spreads everywhere!", bld->x, bld->y, bld->z );
			}
			if ( x != obj->x )
				dam /= 1.5;
			if ( y != obj->y )
				dam /= 1.5;
			if ( obj->value[3] == 2 && bld->type == BUILDING_DUMMY )
			{
				dam = 99999;
				bld->type = BUILDING_TURRET;
			}
			if ( obj->carried_by != NULL )
				dam *= 1.5;
			if ( dam >= bld->hp + bld->shield )
				ex = FALSE;
			damage_building(ch,bld,dam);
		}
		if ( ch->position != POS_DEAD  )
		{
			vch = ch;
			if ( ( abs(ch->x-obj->x) <= 1 && abs(ch->y-obj->y) <= 1 && obj->z == ch->z )
			|| obj->carried_by == vch)
			{
				act( "You are blasted by the force of $p!", vch, obj, NULL, TO_CHAR );
				act( "$n is blasted by the force of $p!", vch, obj, NULL, TO_ROOM );
				dam = dice(obj->weight,obj->value[2]);
				if ( obj->carried_by != NULL && vch == obj->carried_by )
					dam = obj->carried_by->hit * 3;
				if ( vch->x != obj->x || vch->y != obj->y )
					dam /= 10;
				if ( obj->carried_by != NULL && vch == obj->carried_by )
				{
					obj_from_char(obj);
					obj_to_room(obj,vch->in_room);
				}
				damage( ch, vch, dam,DAMAGE_BLAST );
				if ( vch->position == POS_DEAD )
					return;
			}
		}
	}
	extract_obj(obj);
	return;
}

bool fast_healing(CHAR_DATA *ch)
{
	if ( has_ability(ch,3) && number_percent() < 7 )
		return TRUE;
	return FALSE;
}

void mode_update()
{
	char buf[MSL];

	if ( sysdata.qpmode > 0 )
	{
		buf[0] = '\0';
		if ( --sysdata.qpmode % 20 == 0 )
		{
			if ( sysdata.qpmode / 4 == 0 )
				sprintf( buf, "QPMode is almost OVER.@@N\n\r" );
			else
				sprintf( buf, "QPMode is ENABLED for %d more minutes!@@N\n\r", sysdata.qpmode / 4 );
		}
		if ( buf[0] != '\0' )
			info(buf,0);
	}
	if ( sysdata.xpmode > 0 )
	{
		buf[0] = '\0';
		if ( --sysdata.xpmode % 20 == 0 )
		{
			if ( sysdata.xpmode / 4 == 0 )
				sprintf( buf, "XPMode is almost OVER.@@N\n\r" );
			else
				sprintf( buf, "XPMode is ENABLED for %d more minutes!@@N\n\r", sysdata.xpmode / 4 );
		}

		if ( buf[0] != '\0' )
			info(buf,0);
	}

	if ( number_percent() == 1 && number_percent() <= 50 )
	{
		if ( number_percent() < 50 )
		{
			if ( sysdata.qpmode <= 0 )
				do_qpmode(NULL,"10");
		}
		else
		{
			if ( sysdata.xpmode <= 0 )
				do_xpmode(NULL,"10");
		}
	}
	return;
}
