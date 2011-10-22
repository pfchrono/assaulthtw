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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "ack.h"
#include "tables.h"
#include "cursor.h"
#include <math.h>
#include "mapper.h"

char *  const   where_name      [] =
{
    "@@GH@@re@@Wa@@rd@@e:               ",
    "@@GE@@ry@@We@@rs@@e:               ",
    "@@GF@@ra@@Wc@@re@@e:               ",
    "@@GS@@rho@@Wuld@@rer@@Gs@@e:          ",
    "@@GA@@rr@@Wm@@rs@@e:               ",  /* 15 */
    "@@GH@@re@@Wl@@rd@@d-> @@aLeft@@e:        ",
    "@@GH@@re@@Wl@@rd@@d-> @@aRight@@e:       ",
    "@@GW@@ra@@Wi@@rs@@Gt@@e:              ",  /* 25 */
    "@@GB@@ro@@Wd@@ry@@e:               ",
    "@@GL@@re@@Wg@@rs@@e:               ",  /* 30 */
    "@@GF@@re@@We@@rt@@e:               ",
};




/*
 * Local functions.
 */
char *  format_obj_to_char      args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void    show_char_to_char_0     args( ( CHAR_DATA *victim, CHAR_DATA *ch, bool truncate ) );
void    show_char_to_char_1     args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
void	r_help			args( ( CHAR_DATA *ch, int i ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s", color_string( ch, "objects" ) );

    if ( obj->pIndexData->image && ch->desc->mxp && IS_SET(ch->config,CONFIG_IMAGE) )
    {
	    char buf1[MAX_STRING_LENGTH];
	    sprintf( buf1, "\e[1z" );
	    sprintf( buf1+strlen(buf1), MXPTAG(ch->desc,"IMAGE %s ALIGN=Bottom"), obj->pIndexData->image );
	    sprintf( buf1+strlen(buf1), "  " );
	    safe_strcat(MAX_STRING_LENGTH, buf, buf1 );
    }
    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    safe_strcat(MAX_STRING_LENGTH, buf, capitalize(obj->short_descr) );
    }
    else
    {
	if ( obj->description != NULL )
	    safe_strcat(MAX_STRING_LENGTH, buf, obj->description );
    }

    safe_strcat( MAX_STRING_LENGTH, buf, color_string( ch, "normal" ) );
    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;
    buf[0] = '\0';


    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_in_carry_list )
	count++;
    prgpstrShow = qgetmem( count * sizeof(char *) );
    prgnShow    = qgetmem( count * sizeof(int)    );
    nShow       = 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_in_carry_list )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_SET(ch->config, CONFIG_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_SET(ch->config, CONFIG_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->config, CONFIG_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    qdispose( prgpstrShow, count * sizeof(char *) );
    qdispose( prgnShow,    count * sizeof(int)    );

    return;
}

void show_room_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    VEHICLE_DATA *vhc;
    int nShow;
    int iShow;
    int count;
    bool fCombine;
    bool blank = TRUE;
    extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];

    if ( ch->desc == NULL )
	return;
    buf[0] = '\0';


    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_in_room )
	count++;
    prgpstrShow = qgetmem( count * sizeof(char *) );
    prgnShow    = qgetmem( count * sizeof(int)    );
    nShow       = 0;

    /* First, show vehicles */
    	for ( vhc = map_vhc[ch->x][ch->y][ch->z];vhc;vhc = vhc->next_in_room )
    	{
		if ( !vhc )
			continue;
		if ( (vhc->driving && vhc->driving == ch) || vhc->in_vehicle )
			continue;
		buf[0] = '\0';
		if ( IS_SET(vhc->flags,VEHICLE_FLOATS) )
			sprintf( buf+strlen(buf), "@@c(@@aFLO@@WAT@@aING@@c)@@N " );
		sprintf( buf+strlen(buf), "@@l(@@aInside: @@c%s@@l)@@N %s\n\r", (vhc->driving) ? vhc->driving->name : "EMPTY", vhc->desc );
		send_to_char( buf, ch );
	}
	if ( ch->in_vehicle )
		return;
    /*
     * Format the list of objects.
     */

    for ( obj = map_obj[ch->x][ch->y]; obj != NULL; obj = obj->next_in_room )
    {
  	if ( (NOT_IN_ROOM(ch,obj) || obj->carried_by ) )
		continue;

	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) && str_cmp( obj->description, "" ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;
	    blank = FALSE;

	    if ( IS_SET(ch->config, CONFIG_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( ( IS_NPC(ch) || IS_SET(ch->config, CONFIG_COMBINE) ) && str_cmp( prgpstrShow[iShow], "" ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_SET(ch->config, CONFIG_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    if ( blank && !IS_SET(ch->config,CONFIG_CLIENT) )
	send_to_char("\n\r", ch );
    /*
     * Clean up.
     */
    qdispose( prgpstrShow, count * sizeof(char *));
    qdispose( prgnShow,    count * sizeof(int)   );

    return;
}




void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch, bool truncate)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    sprintf( buf, "%s", color_string( ch, "mobiles" ) );
    buf2[0] = '\0';

	if ( IS_BUSY( victim ) )
	{
	  sprintf( buf2, " @@d(@@eBUSY@@d)@@N" );
	  safe_strcat(MSL, buf, buf2 );
	}
	if ( victim->position == POS_BUILDING )
	{
	  sprintf( buf2, " @@d(@@eBUILDING@@d)@@N" );
	  safe_strcat(MSL, buf, buf2 );
	}

	/* strcat to buf *linkdead* if not connected? */
      if ( (victim->desc) == NULL && !victim->fake )
       safe_strcat( MAX_STRING_LENGTH, buf, "@@g(@@dL@@WINK@@dD@@WEAD@@g)@@N " );

	if ( IS_SET(victim->pcdata->pflags, PFLAG_AFK) )
		safe_strcat( MSL, buf, "@@p(@@mA@@pF@@mK@@p)@@N " );


    if ( sysdata.pikamod )
    {
	char nbuf[MSL];
	sprintf( nbuf, "@@yA Pikachu, looking like @@W%s@@y, is here.@@N", victim->name );
	safe_strcat(MAX_STRING_LENGTH, buf, nbuf );
    }
    else
    {
	bool nukem = (victim->z == Z_PAINTBALL && victim->x == 2 && victim->y == 2 );
	safe_strcat(MAX_STRING_LENGTH, buf, victim->name );
	if ( victim->position == POS_HACKING )
		safe_strcat(MSL,buf, " is here, messing with a computer." );
	else if ( nukem && victim->first_carry )
		safe_strcat(MSL,buf," has the @@ynuke@@N!!" );
	else
	        safe_strcat(MAX_STRING_LENGTH, buf, " is here." );
    }
    /* buf[0] = UPPER(buf[0]); */
    safe_strcat( MAX_STRING_LENGTH, buf, color_string( ch, "normal" ) );
    safe_strcat( MAX_STRING_LENGTH, buf, "\n\r" );

    send_to_char( buf, ch );
    return;
}


void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int pct;
    bool found;
    buf[0] = '\0';


    if ( can_see( victim, ch ) )
    {
     
	act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }
    act( "Isn't 'You see nothing special about $M' a bit boring?.", ch, NULL, victim, TO_CHAR );
    
    if ( victim->max_hit > 0 )
	pct = ( 100 * victim->hit ) / victim->max_hit;
    else
	pct = -1;

    strcpy( buf, victim->name );
    sprintf( buf, "%s", victim->name );

	 if ( pct >= 100 ) safe_strcat(MAX_STRING_LENGTH, buf, " is ready for battle!\n\r" );
    else if ( pct >=  90 ) safe_strcat(MAX_STRING_LENGTH, buf, " has some wounds here and there.\n\r" );
    else if ( pct >=  80 ) safe_strcat(MAX_STRING_LENGTH, buf, " has a few bullet holes.\n\r" );
    else if ( pct >=  70 ) safe_strcat(MAX_STRING_LENGTH, buf, " has a face full of blood.\n\r" );
    else if ( pct >=  60 ) safe_strcat(MAX_STRING_LENGTH, buf, " is covered in sweat and blood.\n\r" );
    else if ( pct >=  50 ) safe_strcat(MAX_STRING_LENGTH, buf, " has one leg in the grave.\n\r" );
    else if ( pct >=  40 ) safe_strcat(MAX_STRING_LENGTH, buf, " is falling apart!\n\r" );
    else if ( pct >=  30 ) safe_strcat(MAX_STRING_LENGTH, buf, " is missing an arm or two.\n\r" );
    else if ( pct >=  20 ) safe_strcat(MAX_STRING_LENGTH, buf, " is having trouble living.\n\r" );
    else if ( pct >=  10 ) safe_strcat(MAX_STRING_LENGTH, buf, " is ready to drop!\n\r"        );
    else                   safe_strcat(MAX_STRING_LENGTH, buf, " is DYING.\n\r" );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;

      for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
      {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
      }

      if ( victim != ch )
      {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	show_list_to_char( victim->first_carry, ch, TRUE, TRUE );
      }
    return;
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    if ( list == NULL )
	return;
    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->next_in_room == rch )
		rch->next_in_room = NULL;
	if ( rch == ch )
	    continue;
	if ( rch->next_in_room && rch->next_in_room == rch )
		extract_char(rch->next_in_room , TRUE);

	if ( NOT_IN_ROOM(ch, rch) )
		continue;

	if ( !can_see(ch,rch) )
		continue;

	if ( rch->in_vehicle != NULL )
		continue;
        show_char_to_char_0( rch, ch, FALSE );
    }

    return;
} 

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char out  [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    extern OBJ_DATA *map_obj[MAX_MAPS][MAX_MAPS];
    VEHICLE_DATA *vhc;
    bool base = FALSE;
    buf[0] = '\0';
    out[0] = '\0';

    if ( ch->desc == NULL ) 
	return;

    if ( IS_SET(ch->effect,EFFECT_BLIND) )
    {
	if ( number_percent() < 20 || !blind_combat_check(ch) )
	{
		send_to_char( "You can't see a thing!\n\r", ch );
		return;
	}
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp(arg1,"outside"))
	base = TRUE;
    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) || base )
    {
	/* 'look' or 'look auto' */

	if ( sysdata.pikamod )
		send_to_char( "\n\r@@yPika Mod @@bO@@yN@@bL@@yI@@bN@@yE@@b!@@N\n\r", ch );
	if ( sysdata.killfest )
		send_to_char( "\n\r@@2@@aKILLFEST!@@N mode on!\n\r", ch  );
	if ( ch->in_vehicle != NULL && !IS_SET(ch->config,CONFIG_BLIND) )
	{
		char buf[MSL];
		VEHICLE_DATA *vhc;
		sprintf( buf, "\n@@ySTATUS:   @@RHEALTH: @@e%d        @@cFUEL: %s%d@@N        @@dAMMO: @@g%d@@N\n\r", ch->in_vehicle->hit, ( ch->in_vehicle->fuel <= 10 ) ? "@@e@@f" : "@@a", ch->in_vehicle->fuel, ch->in_vehicle->ammo );
		send_to_char( buf, ch );
		if ( ( vhc = get_vehicle_from_vehicle(ch->in_vehicle) ) != NULL )
		{
			sprintf( buf, "Inside: %s\n\r", vhc->desc );
			send_to_char( buf, ch );
		}
	}
	if ( IS_SET(ch->config,CONFIG_NOMAP) ) return;
	if ( IS_SET(ch->config, CONFIG_NOCOLORS) && !IS_SET(ch->config, CONFIG_COLOR) )
		SET_BIT(ch->config, CONFIG_COLOR);
	if ( !base )
		do_mapper( ch, "" );
	else
		do_mapper( ch, "996" );

	if ( IS_SET(ch->config, CONFIG_NOCOLORS) && IS_SET(ch->config, CONFIG_COLOR) )
		REMOVE_BIT(ch->config, CONFIG_COLOR);
	show_char_to_char( map_ch[ch->x][ch->y][ch->z],   ch );
	show_room_list_to_char( map_obj[ch->x][ch->y], ch, FALSE, FALSE );
	return;

    }
    else if ( !str_prefix(arg1,"ground") && ch->z == Z_AIR )
    {
	move(ch,ch->x,ch->y,Z_GROUND);
	do_mapper(ch,"");
	move(ch,ch->x,ch->y,Z_AIR);
	return;
    }
    else if ( !str_cmp( arg1, "board" ) || !str_cmp( arg2, "board" ) )
    {
	/*int bnum;*/
	
	if ( ( obj = get_obj_here( ch, "board" ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;

	}
	
	do_show_contents( ch, obj );
	return;
    }
 
    if (( vhc = get_vehicle_char( ch, arg1 ) ) != NULL )
    {
	VEHICLE_DATA *vhc2;

	sprintf( buf, "%s is driven by %s.\n\r", vhc->desc, ( vhc->driving == NULL ) ? "nobody" : ( vhc->driving == ch ) ? "you" : vhc->driving->name );
	send_to_char( buf, ch );
	sprintf( buf, "%s has: %d%% HP, %d %s-type Ammunition and %d gallons of Fuel. Its speed is %d.\n\r", vhc->desc, (((100000 / vhc->max_hit) * vhc->hit) / 1000), vhc->ammo, (vhc->ammo_type>1&&vhc->ammo_type<MAX_AMMO)?clip_table[vhc->ammo_type].name:"None", vhc->fuel, vhc->speed );
	send_to_char( buf, ch );
	if ( vhc && ( vhc2 = get_vehicle_from_vehicle(vhc) ) != NULL )
	{
		sprintf( buf, "\n\rIt's carrying %s inside.\n\r", vhc2->desc );
		send_to_char( buf, ch );
	}
	act( "$n looks at $T.", ch, NULL, vhc->desc, TO_ROOM );
	return;
    }    
    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	if ( victim->in_vehicle )
	{
		VEHICLE_DATA *vhc2;

		vhc = victim->in_vehicle;
		sprintf( buf, "%s is driven by %s.\n\r", vhc->desc, ( !vhc->driving ) ? "nobody" : ( vhc->driving == ch ) ? "you" : vhc->driving->name );
		send_to_char( buf, ch );
		sprintf( buf, "%s has: %d%% HP, %d %s-type Ammunition and %d gallons of Fuel. Its speed is %d.\n\r", vhc->desc, (((100000 / vhc->max_hit) * vhc->hit) / 1000), vhc->ammo, (vhc->ammo_type<0)?"null":clip_table[vhc->ammo_type].name, vhc->fuel, vhc->speed );
		send_to_char( buf, ch );
		if (  ( vhc2 = get_vehicle_from_vehicle(vhc) ) != NULL )
		{
			sprintf( buf, "\nIt's carrying %s inside.\n\r", vhc2->desc );
			send_to_char( buf, ch );
		}
		act( "$n looks at $T.", ch, NULL, vhc->desc, TO_ROOM );
		return;
	}
	show_char_to_char_1( victim, ch );
	return;
    }

    if ( (obj=get_obj_carry(ch,arg1) ) == NULL )
    	if ( (obj=get_obj_room(ch,arg1,map_obj[ch->x][ch->y]) ) == NULL )
	{
		send_to_char( "You don't see that here.\n\r", ch );
		return;
	}

    if ( obj->item_type == ITEM_TOKEN && (obj->value[0] == 2 || obj->value[0] == 3))
    {
	int m =ch->map;
	ch->map = obj->value[1];
	if ( obj->value[0] == 2)
		do_mapper(ch,"995");
	else
		do_mapper(ch,"994");
	ch->map = m;
	return;
    }
    send_to_char( obj->description, ch );
    return;
}



void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    buf[0] = '\0';

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_BOMB:
	    if ( obj->value[1] != 0 )
	    	sprintf( buf, "It is set to explode in %d seconds!\n\r", obj->value[0] );
	    else
		sprintf( buf, "It is not armed.\n\r" );
	    send_to_char( buf, ch );
		break;

	case ITEM_ARMOR:
		sprintf( buf, "%s has %d damage. It appears to be good against %s.", capitalize(obj->short_descr), obj->value[1], ( obj->value[0] == DAMAGE_BULLETS ) ? "bullet damage" : (obj->value[0] == DAMAGE_BLAST) ? "blast damage" : (obj->value[0] == DAMAGE_ACID) ? "acid damage" : (obj->value[0] == DAMAGE_LASER) ? "laser damage" : (obj->value[0] == DAMAGE_FLAME) ? "Fire damage" : "general damage" );
		send_to_char( buf, ch);
		break;

	case ITEM_COMPUTER:
		sprintf( buf, "It has %d more uses of battery life.\n\r", obj->value[1] );
		send_to_char( buf, ch);
		break;
	}

    }

    return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->config,CONFIG_BLIND) || !str_cmp(argument,"old"))
	display_details_old(ch,ch);
    else
    	display_details(ch,ch);       
    return;
}

void display_details( CHAR_DATA * viewer, CHAR_DATA *ch )
{
	char buf[MSL];
	char namebuf[MSL];
	char mccp_buf[MSL];
	bool mccp = FALSE;
	int hours,minutes;
	extern char * const poison_name[];

	buf[0] = '\0';
	sprintf(namebuf, "%s %s", ch->name, ch->pcdata->title );
	if ( ch->desc && ch->desc->out_compress )
	{
		sprintf(mccp_buf, " MCCP%d Support", (ch->desc->compressing==85)?1:2 );
		mccp = TRUE;
	}
	else
	{
		sprintf(mccp_buf, "No MCCP Support" );
		mccp = FALSE;
	}
	hours = my_get_hours(ch,FALSE);
	minutes = my_get_minutes(ch,FALSE) - (hours * 60);
	sprintf( buf+strlen(buf),"\n\r%s\n\r", center_text(namebuf,75) );

	sprintf(buf+strlen(buf),"@@d                                        _                              __\n\r" );
	sprintf(buf+strlen(buf),"@@d /\\   _      ___   /\\       _       ___/ \\_         ___/\\           __/  \\ \n\r" );
	sprintf(buf+strlen(buf),"@@d|  \\_/ \\  __/   \\_/  \\     / \\_  __/       \\__   __/     \\__     __/      \\ \n\r" );
	sprintf(buf+strlen(buf),"@@d|       \\/            \\___/    \\/             \\_/           \\___/         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@g  Play Time:  @@W%5d.%-2d Hours @@g( @@W%4d@@g Total)            (%s%-15s@@g)  @@d|\n\r", hours, minutes, my_get_hours(ch,TRUE), mccp?"@@a":"@@e", mccp_buf );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	if ( ch->pcdata->alliance >= 0 )
		sprintf(buf+strlen(buf),"@@d|                 @@W%*s                @@d|\n\r", 
(strlen(alliance_table[ch->pcdata->alliance].name) - nocol_strlen(alliance_table[ch->pcdata->alliance].name) ) + 40, 
center_text(alliance_table[ch->pcdata->alliance].name,40) );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	if ( !IS_IMMORTAL(ch) )
		sprintf(buf+strlen(buf),"@@d|@@g                     You are currently at rank @@W%4d@@g.                     @@d|\n\r",get_rank(ch) );
	else
		sprintf(buf+strlen(buf),"@@d|@@g                       You are trusted at level @@W%2d@@g.                      @@d|\n\r", get_trust(ch) );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@a  Player Statistics                      PK Statistics                   @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@g  Health:@@W %5d/%-5d                   @@g Kills:@@W    %-6d @@g(@@W%-6d@@g Total) @@d|\n\r", ch->hit, ch->max_hit, ch->pcdata->pkills, ch->pcdata->tpkills );
	sprintf(buf+strlen(buf),"@@d|@@g  Class:@@W  %-10s                    @@g B. Kills:@@W %-6d @@g(@@W%-6d@@g Total) @@d|\n\r", class_table[ch->class].name, ch->pcdata->bkills, ch->pcdata->tbkills );
	sprintf(buf+strlen(buf),"@@d|@@g  Money:@@W  $%-7ld                      @@g Deaths:@@W   %-6d                @@d|\n\r", ch->money, ch->pcdata->deaths );
	sprintf(buf+strlen(buf),"@@d|@@g  Exp:@@W    %-11d                   @@g B. Lost:@@W  %-6d                @@d|\n\r", ch->pcdata->experience, ch->pcdata->blost );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@a  Points                                 Paintball Statistics            @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@g  Quest:@@W  %-5d                          @@gKills:@@W    %-6d                @@d|\n\r", ch->quest_points, ch->pcdata->pbhits );
	sprintf(buf+strlen(buf),"@@d|@@g  Game:@@W   %-5d                          @@gLosses:@@W   %-6d                @@d|\n\r", ch->game_points, ch->pcdata->pbdeaths );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@a  Timers                                 Nukem Statistics                @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@g  Combat:@@W %-5d Seconds                  @@gWins:@@W     %-6d                @@d|\n\r", ch->fighttimer / PULSE_PER_SECOND, ch->pcdata->nukemwins );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@a  Items                                                                  @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|@@g  Carry:@@W  %6d/%-4d                                                    @@d|\n\r",ch->carry_number, can_carry_n(ch) );
	sprintf(buf+strlen(buf),"@@d|@@g  Weight:@@W %6.2f/%-4d	                                                  @@d|\n\r",ch->carry_weight, can_carry_w(ch) );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_CONFUSE) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Confusion                                                  @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_BLIND) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Blindness                                                  @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_SLOW) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Slow                                                       @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_ENCRYPTION) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Encryption                                                 @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_RESOURCEFUL) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Resourceful                                                @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_BOMBER) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Bomber                                                     @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_POSTAL) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Postal                                                     @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_WULFSKIN) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Wulfskin                                                   @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_DRUNK) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Drunk                                                      @@d|\n\r" );
	if ( IS_SET(ch->effect,EFFECT_EVADE) )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W Evade                                                      @@d|\n\r" );
	if ( ch->effect2 > 0 )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W %-12s  @@gFor:@@W %2d Minute(s)                            @@d|\n\r", (ch->effect2/100==EFFECT2_CONSTITUTION)?"Constitution":(ch->effect2/100==EFFECT2_SPEED)?"Speed":(ch->effect2/100==EFFECT2_SMART)?"Brilliance":(ch->effect2/100==EFFECT2_RAGE)?"Rage":(ch->effect2/100==EFFECT2_REGENERATION)?"Regeneration":"Unknown", ch->effect2 % 100 );
	if ( ch->poison > 0 )
		sprintf( buf+strlen(buf),"| @@gAffected by:@@W %-12s  @@gFor:@@W %2d Minute(s)                            @@d|\n\r", poison_name[ch->poison/100], ch->poison % 100 );
	sprintf(buf+strlen(buf),"@@d|                                                                         @@d|\n\r" );
	sprintf(buf+strlen(buf),"@@d|_________________________________________________________________________|\n\r" );

	send_to_char(buf,viewer);
	return;
}

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf, "@@cIt is @@a%d @@co'clock @@a%s@@c Game Time.@@N\n\r@@WAssault @@cstarted up at @@a%s@@N\r@@cThe system time is @@a%s@@N\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	str_boot_time,
	(char *) ctime( &current_time )
	);

    if ( sysdata.qpmode > 0 )
	sprintf(buf+strlen(buf),"@@cQPMode is enabled for @@a%d@@c more minutes!@@N\n\r", sysdata.qpmode / 4 );
    if ( sysdata.xpmode > 0 )
	sprintf(buf+strlen(buf),"@@cXPMode is enabled for @@a%d@@c more minutes!@@N\n\r", sysdata.xpmode / 4 );
    send_to_char( buf, ch );
    return;
}

void do_bhelp( CHAR_DATA *ch, char *argument )
{
	if ( !IS_SET(ch->config,CONFIG_CLIENT) )
	{
		send_to_char("Huh?\n\r", ch );
		return;
	}
	show_building_info(ch,atoi(argument));
	return;
}
void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    bool building = FALSE;
    bool sep = FALSE;
    char buf2[MSL];
    buf2[0] = '\0';

    if ( argument[0] == '\0' )
	argument = "help";

    {
	int i;

	if ( !str_cmp(argument,"hq") )
	{
		show_building_info(ch,BUILDING_HQ);
		return;
	}
	for ( i = 1;i<MAX_BUILDING;i++ )
	{
		if ( build_table[i].rank > 9999 || build_table[i].disabled )
			continue;
		if ( !str_cmp(build_table[i].name,argument) || !str_cmp(build_table[i].symbol,argument))
		{
			show_building_info(ch,i);
			building = TRUE;
			return;
		}
	}
    }

    for ( pHelp = first_help; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level-1 > get_trust( ch ) )
	    continue;

	if ( is_name( argument, pHelp->keyword ) || !str_cmp(argument,pHelp->keyword) )
	{

	    if ( sep )
	    {
		sprintf( buf2+strlen(buf2), "@@C%s, ", pHelp->keyword );
//		send_to_char("\n\r---------------------------------------------------------------\n\r\n\r", ch );
		continue;
	    }
/*	    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) && !building )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\n\r", ch );
	    }*/

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		send_to_char( pHelp->text+1, ch );
	    else
		send_to_char( pHelp->text  , ch );
	    sep = TRUE;
	    return;
	}
	else
	{
/*		if ( !str_infix(argument,pHelp->keyword) )
		{
			one_argument(pHelp->keyword,help);
			sprintf( buf2+strlen(buf2), "@@C%s, ", help );
		}*/
	}
    }
    if ( !sep )
    {
    	send_to_char( "No help on that word.\n\r", ch );
    	append_file( ch, HELP_FILE, argument );
    	if ( buf2[0] != '\0' )
    	{
		send_to_char( "\n\r\n\rMaybe you are looking for one of these:\n\r",ch);
		send_to_char(buf2,ch);
		send_to_char( "@@N\n\r", ch );
    	}
	return;
    }
    if ( buf2[0] != '\0' )
    {
	send_to_char( "Maybe you are looking for one of these:\n\r",ch);
	send_to_char(buf2,ch);
    }
    {
	OBJ_DATA *obj;
	char buf[MSL];
	if ( ( obj = get_obj_carry(ch,argument) ) != NULL )
	{
		sprintf( buf, "\n\r@@NIf you're looking for help about %s, you can try to ""identify"" it (Not all objects can be identified)", obj->short_descr );
		send_to_char( buf, ch );
	}
    }
    return;
}



void do_helplist( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
	char buf[MSL];
	char buf2[MSL];

    if ( argument[0] == '\0' )
	argument = "summary";

	buf[0] = '\0';
	send_to_char( "A list of all the helps:\n\r", ch );
    for ( pHelp = first_help; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	sprintf( buf2, "%s\n\r", pHelp->keyword );
	safe_strcat( MSL, buf, buf2 );
    }
	send_to_char( buf, ch );
    return;
}

void do_who( CHAR_DATA *ch, char *argument )
{
//    DESCRIPTOR_DATA *d; 
    char buf[MSL];
    int true_cnt;
    int hidden_cnt = 0;
    int g_rank=0,rank,type;
    extern int max_players;
    extern int max_players_ever;
    char n_buf[MSL];
    char s_buf[MSL];
    char h_buf[MSL];
    CHAR_DATA *wch;
    bool blind = FALSE;
    bool blind_all = FALSE;
    bool tmc = FALSE;

    if ( IS_SET(ch->config,CONFIG_BLIND) )
	blind = TRUE;

    if ( argument[0] != '\0' )
    {
	char arg[MSL];

	argument = one_argument(argument,arg);
	if ( !str_cmp(arg,"setmax") && is_number(argument) && (!ch || IS_IMMORTAL(ch) ) )
	{
		max_players = atoi(argument);
		if ( ch ) send_to_char("Max players set.\n\r", ch );
		set_max_ever(1);
		return;
	}
	else if ( !str_cmp(arg,"tmc_wholist") )
	{
		tmc = TRUE;
	}
	else if ( !str_cmp(arg,"rank") && is_number(argument) )
	{
		g_rank = atoi(argument);
	}
	else if ( blind && !str_prefix(arg,"all") )
		blind_all = TRUE;
    }
    send_to_char( "\n\r", ch );
    if ( blind )
	sprintf( buf, "Visible players online:\n\r" );
    else if ( !IS_SET(ch->config,CONFIG_MINCOLORS) )
	sprintf( buf, "@@c|@@g===============================================================================@@N\n\r" );
//    	sprintf( buf, "@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@G*@@r~@@N\n\r"  );
    else
	sprintf( buf, "@@g|===============================================================================@@N\n\r" );
//    	sprintf( buf, "@@G~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~@@N\n\r"  );

    send_to_char( buf, ch );
    if ( !blind )
    {

  	send_to_char("@@c|@@W Rank @@d|  @@WFlags  @@d| @@WS @@d| @@WPlayer@@N\n\r", ch );
	sprintf( h_buf, "@@c|@@g    ? |         | ? | Hidden Player\n\r" );
    }



    if ( !IS_SET(ch->config,CONFIG_MINCOLORS) )
    {
	sprintf( n_buf, "@@y N@@be@@yw@@yb@@bi@@ye@@N  " );
	sprintf( s_buf, "@@b (@@yS@@bT@@yA@@bF@@yF@@b)@@N " );
    }
    else
    {
	sprintf( n_buf, "@@y Newbie @@N " );
	sprintf( s_buf, "@@b (STAFF) @@N" );
    }
    true_cnt = 0;
    for ( type=1;type<=2;type++ )
    {
	if ( !blind )
	{
		if ( type == 1 )
		{
			send_to_char( "@@c|@@g===================================== @@aStaff @@g===================================\n\r", ch );
		}
		else if ( type == 2 )
		{
			send_to_char( "@@c|@@g==================================== @@aPlayers @@g==================================\n\r", ch );
			if ( tmc )
			{
				send_to_char( "@@c|@@g   30 |         | M | @ Qiller\n\r", ch );
				send_to_char( "@@c|@@g   38 |         | M | @ Noma\n\r", ch );
			}
		}
	}

    	for ( wch = first_char;wch;wch = wch->next )
    	{
	
		if ( (type == 1 && !IS_IMMORTAL(wch) )
		  || (type == 2 && IS_IMMORTAL(wch) ) )
			continue;

/*		if ( tmc && wch->next && wch->next == ch )
		{
			send_to_char( "@@c|@@g    1 |         | M | @ Godlike\n\r", ch );
		}*/

		true_cnt++;
		if ( !can_see(ch,wch) || (IS_LINKDEAD(wch) && ch->trust < 84) )
		{
			true_cnt--;
			hidden_cnt++;
			continue;
		}
	
		rank = get_rank(wch) ;
		if ( rank < g_rank )
			continue;
	
		if ( blind )
		{
			if ( blind_all )
				sprintf( buf, "%s, %s, Rank %d%s\n\r", wch->name, wch->pcdata->title, rank, IS_NEWBIE(wch)?" (Newbie)":"" );
			else
				sprintf( buf, "%s%s\n\r", wch->name , IS_NEWBIE(wch)?" (Newbie)" : "");
		}
		else
		{
			sprintf( buf, "@@c| %s%4d@@g |%-9s@@g| %s |",
			   (get_trust(wch) <= 1)?"@@g":(get_trust(wch)==79)?"@@W":"@@y",
			   rank,
			   IS_SET(wch->act,PLR_WIZINVIS)	      ? "@@m(@@p(I@@mN@@pV@@mI@@pS)@@m)@@N" :
			   IS_SET( wch->pcdata->pflags, PFLAG_AFK  )  ? "AFK" : 
			   wch->timer > 5                             ? "IDLE" : 
			   !wch->fake && wch->desc && wch->desc->connected != CON_PLAYING        ? "DEAD" : 
			   ( wch->trust == 90 ) ? "@@B(@@l(@@aO@@cw@@an@@ce@@ar@@l)@@B)@@N" :
			   ( wch->trust >= LEVEL_IMMORTAL ) ? s_buf :
			   (IS_NEWBIE(wch)) ? n_buf :
			   ( str_cmp(wch->pcdata->who_name,"off") ) ? wch->pcdata->who_name : " ",
			   (wch->sex == 1) ? "M" : "F" );
		}
		   
		if ( wch->in_building && wch->in_building->type == BUILDING_CLUB && complete(wch->in_building) && wch != ch )
			sprintf( buf+strlen(buf)," *" );
		if ( IS_LINKDEAD(wch) )
			sprintf(buf+strlen(buf)," @" );
	
		if ( !blind )
			sprintf( buf+strlen(buf), " %s%s%s \n\r", wch->name, (wch->pcdata->title[0] != '@') ? "" : " ", wch->pcdata->title ); 
		
	    	send_to_char( buf, ch );
	}
    } 

    hidden_cnt += sysdata.showhidden;

    if ( IS_SET(ch->pcdata->pflags,PLR_ASS) )
	hidden_cnt = 0;

    for ( type=0;type<hidden_cnt;type++)
	send_to_char(h_buf,ch);

    if ( true_cnt + hidden_cnt > max_players )
       max_players = true_cnt + hidden_cnt;
    if ( max_players > max_players_ever ) {
	max_players_ever=max_players;
	set_max_ever(1);
    }
    if ( true_cnt + hidden_cnt != web_data.num_players )
    {
	char plr[4];
	sprintf( plr, "%d", true_cnt+hidden_cnt );
	update_web_data(WEB_DATA_NUM_PLAYERS,plr);
    }

    if ( blind )
    {
	sprintf( buf, "\n\rTotal Players: %d\n\rHidden: %d\n\rMax this Reboot: %d\n\rMax Ever: %d\n\r", true_cnt+hidden_cnt, 
hidden_cnt, 
max_players, max_players_ever );
	send_to_char(buf,ch);
	return;
    }

    sprintf( buf,
"@@c|@@g===============================================================================@@N\n\r" );
    sprintf( buf+strlen(buf), "@@c|      @@c(@@W%d T@@gotal Player%s@@c)  (@@W%d H@@gidden@@c)  (@@W%d M@@gax This Reboot@@c)  (@@W%d M@@gax Ever@@c)\n\r@@N", true_cnt+hidden_cnt, ( true_cnt+hidden_cnt > 1 ) ? "s" : "", hidden_cnt, 
max_players,max_players_ever );
    sprintf( buf+strlen(buf), "@@c|                              @@eASSAULT: 3.0\n\r" );

    safe_strcat( MAX_STRING_LENGTH, buf, 
"@@c|@@g===============================================================================@@N\n\r" );

  
    send_to_char( buf, ch );
    send_to_char( color_string( ch, "normal" ), ch );

    return; 
}
void do_inventory( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
	if ( IS_SET(ch->effect,EFFECT_BLIND) ) {
		send_to_char( "You can't see a thing!\n\r", ch );
		return;
	}
 
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->first_carry, ch, TRUE, TRUE );
    sprintf(buf,"\n\rYou have $%ld in cash.\n\r", ch->money );
    send_to_char(buf,ch);
    return;
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
    do_wear( ch, argument );
    return;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "credits" );
    return;
}


void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    /* Changed this to limit title length, and to remove and brackets. -S- */
    
    char buf[MAX_STRING_LENGTH];
    int cnt;
    bool changed;
    buf[0] = '\0';

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }
    changed = FALSE;

    for ( cnt = 0; cnt < strlen(argument); cnt++ )
    {
       if ( argument[cnt] == '[' || argument[cnt] == ']' )
       {
	  changed = TRUE;
	  argument[cnt] = ( argument[cnt] == ']' ) ? '>' : '<';
       }
    }

    if ( changed )
       send_to_char( "You used either [ or ] in your title.  They have been removed!\n\r", ch );

    /* my_strlen handles color codes as zero length */
    if ( my_strlen(argument) > 49 )
    {
	send_to_char( "Title too long.  Please try again.\n\r", ch );
	return;
    }

    smash_tilde( argument );
    set_title( ch, argument );
    sprintf( buf, "You are now: %s%s.\n\r", ch->name, ch->pcdata->title );
    send_to_char( buf, ch );
}

void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;
    arg1[0] = '\0';
    arg2[0] = '\0';

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if (  ( ch->pcdata->pwd != '\0' )
       && ( arg1[0] == '\0' || arg2[0] == '\0' )  )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if (  ( ch->pcdata->pwd != '\0' )
       && ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )  )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}
void do_socials( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char out[MAX_STRING_LENGTH*2];
    int iSocial;
    int col;
    buf[0] = '\0';
 
    col = 0;
    out[0] = '\0';

    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
    {
	sprintf( buf, "%-12s", social_table[iSocial].name );
	safe_strcat(MAX_STRING_LENGTH, out, buf );
	if ( ++col % 6 == 0 )
	    safe_strcat(MAX_STRING_LENGTH, out, "\n\r" );
    }
 
    if ( col % 6 != 0 )
	safe_strcat(MAX_STRING_LENGTH, out, "\n\r" );
    send_to_char( out, ch );
    return;
}

struct show_cmds
{
  char buf[MSL];
  sh_int col;
};

static char * const cmd_group_names  [] =
{
  "@@WMisc", "@@aCommunication", "@@mConfiguration", "@@eInformation", "@@rActions", 
  "@@dItem Manipulation" , "@@cAlliance Commands", "@@yImm", ""
};

void do_commands( CHAR_DATA *ch, char *argument )
{
    static struct show_cmds show_table[8];
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg1[MSL];
    int cmd;
    int col = 0;
    sh_int      show_only = -1;


    buf[0] = '\0';
    buf1[0] = '\0';

/* NOTE: This is better coded via a build_tab.c style lookup, but this is
   quicker to code right now :) Zen */

    argument = one_argument( argument, arg1 );
    if ( arg1[0] != '\0' )
    {
      if ( !str_prefix( arg1, "miscellaneous" ))
        show_only = 0;
      else if ( !str_prefix( arg1, "communication" ))
        show_only = 1;
      else if ( !str_prefix( arg1, "configuration" ))
        show_only = 2;
      else if ( !str_prefix( arg1, "information" ))
        show_only = 3;
      else if ( !str_prefix( arg1, "actions" ))
        show_only = 4;
      else if ( !str_prefix( arg1, "objects" )
              || !str_prefix( arg1, "manipulation") )
        show_only = 5;
      else if ( !str_prefix( arg1, "alliance" ))
        show_only = C_TYPE_ALLI;
      else if ( !str_prefix( arg1, "imm" ))
        show_only = C_TYPE_IMM;     
    }

  for ( col = 0; col < 8; col++ )
  {
    sprintf( show_table[col].buf, "%s:\n\r",  cmd_group_names[col] );
    show_table[col].col = 0;
  }

  col = 0;


    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( cmd_table[cmd].level <=  L_GOD
	&&   cmd_table[cmd].level <= get_trust( ch ) )
	{
	     if ( cmd_table[cmd].show == C_SHOW_NEVER )
               continue;

	    sprintf( buf, "%-12s", cmd_table[cmd].name );
            safe_strcat( MSL, show_table[cmd_table[cmd].type ].buf, buf );
            if ( ++show_table[cmd_table[cmd].type].col % 6 == 0 )
              safe_strcat(MSL, show_table[cmd_table[cmd].type].buf, "\n\r" );  
	}
    }
 
   send_to_char( buf1, ch );
  for ( col = 0; col < 8; col++ )
  {
    if (  ( show_only > -1 )
       && ( show_only != col ) )
      continue;
    safe_strcat(MAX_STRING_LENGTH, show_table[col].buf, "@@N\n\r" );
    send_to_char( show_table[col].buf, ch );
  }

  return;
}
	     
struct chan_type channels[] = {
	    { CHANNEL_GOSSIP,  0, "gossip",
	      "@@r[ @@a+GOSSIP   @@r] @@GYou hear general gossip.\n\r",
	      "@@d[ @@c-gossip   @@d] @@GYou do not hear general gossip.\n\r" },
	    { CHANNEL_MUSIC,   0, "music", 
	      "@@r[ @@a+MUSIC    @@r] @@GYou hear people's music quotes.\n\r",
	      "@@d[ @@c-music    @@d] @@GYou don't hear people's music quotes.\n\r" },
	    { CHANNEL_IMMTALK, LEVEL_HERO, "immtalk",
	      "@@r[ @@a+IMMTALK  @@r] @@GYou hear what other immortals have to say.\n\r",
	      "@@d[ @@a-immtalk  @@d] @@GYou don't hear what other immortals have to say.\n\r" },
	    { CHANNEL_LOG,0, "log",
	    "",
	    ""},
	
	    { CHANNEL_NEWBIE, 0, "newbie",
	     "@@r[ @@a+NEWBIE   @@r] @@GYou hear newbie's chit-chat.\n\r",
	     "@@d[ @@c-newbie   @@d] @@GYou don't hear newbie's chit-chat.\n\r" },
	    

	    { CHANNEL_QUESTION, 0, "question",
	    "@@r[ @@a+QUESTION @@r] @@GYou hear player's questions & answers.\n\r",
	    "@@d[ @@c-question @@d] @@GYou don't hear player's questions & answers.\n\r"},
	    
            { CHANNEL_GAME,0, "game",
            "@@r[ @@a+GAME     @@r] @@GYou hear players participating in games.\n\r",   
            "@@d[ @@c-game     @@d] @@GYou no longer hear people playing games.\n\r"},  

	    { CHANNEL_FLAME,0, "flame",
	    "@@r[ @@a+FLAME    @@r] @@GYou hear players flaming each other.\n\r",
	    "@@d[ @@c-flame    @@d] @@GYou don't hear players flaming each other.\n\r"},
	
	    { CHANNEL_INFO,0, "info",
	    "@@r[ @@a+INFO     @@r] @@GYou hear information about deaths, etc.\n\r",
	    "@@d[ @@c-info     @@d] @@GYou don't hear information about deaths, etc.\n\r"},
	    
	    { CHANNEL_CREATOR, 85, "creator",
	    "@@r[ @@a+CREATOR  @@r] @@GYou hear Creators discussing Mud matters.\n\r",
	    "@@d[ @@c-creator  @@d] @@GYou don't hear Creators discussing Mud matters\n\r" },
	    
	    { CHANNEL_HERMIT, 0, "hermit",
	    "@@r[ @@a+HERMIT   @@r] @@GYou are NOT ignoring all channels.\n\r",
	    "@@d[ @@c-hermit   @@d] @@GYOU ARE IGNORING ALL CHANNELS!\n\r" },
	    
	    { CHANNEL_BEEP,   0, "beep",
	    "@@r[ @@a+BEEP     @@r] @@GYou accept 'beeps' from other players.\n\r",
	    "@@d[ @@c-beep     @@d] @@GYou are ignoring 'beeps' from other players.\n\r" },

	    { CHANNEL_ALLALLI, 84, "allalli",
	    "@@r[ @@a+ALLALLI  @@r] @@GYou hear ALL alliance channels.\n\r",
	    "@@d[ @@c-allalli  @@d] @@GYou don't hear ALL alliance channels.\n\r" },

	    { CHANNEL_OOC,   0, "ooc",
	    "@@r[ @@a+OOC      @@r] @@GYou hear non-roleplaying chit-chat.\n\r",
	    "@@d[ @@c-ooc      @@d] @@GYou ignore those boring non-roleplaying fools!\n\r" },

	    { CHANNEL_POLITICS,   0, "politics",
	    "@@r[ @@a+POLITICS @@r] @@GYou hear people talk about RL politics stuff... booring!.\n\r",
	    "@@d[ @@c-POLITICS @@d] @@GYou don't care about RL politics!\n\r" },
	    
	    { CHANNEL_CODE, 0, "code",
	    "@@r[ @@a+CODE     @@r] @@GYou hear people talking about coding issues.\r\n",
	    "@@d[ @@a-code     @@d] @@GYou ignore talk about coding issues.\r\n" },
			    
	    { CHANNEL_ALLIANCE,  0, "alliance",
	      "@@r[ @@a+ALLIANCE @@r] @@GYou hear alliance talk.\n\r",
	      "@@d[ @@c-alliance @@d] @@GYou do not hear alliance talk.\n\r" },

	    { 0,0, NULL, NULL }
};
void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    int a,trust,i=1;
    bool tog = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
      if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
      {
	send_to_char( "You are silenced.\n\r", ch );
	return;
      }

      trust=get_trust(ch);
      buffer[0]='\0';
      safe_strcat( MAX_STRING_LENGTH, buffer, "@@aChannels@@c:\n\r");
	
      for (a=0; channels[a].bit != 0; a++)
      {
	if (trust >= channels[a].min_level)
	{
          char  colbuf[10];
          colbuf[0] = '\0';

	  if ( IS_SET(ch->deaf, channels[a].bit) )
          {
            if ( !IS_NPC( ch ) )
            {
              sprintf( colbuf, "@@%c", ch->pcdata->dimcol );
              safe_strcat( MSL, buffer, colbuf );
            }
	    safe_strcat( MAX_STRING_LENGTH, buffer, channels[a].off_string);
          }
	  else
          {
            if ( !IS_NPC( ch ) )
            {
              sprintf( colbuf, "@@%c", ch->pcdata->hicol );
              safe_strcat( MSL, buffer, colbuf );
            }
	    safe_strcat( MAX_STRING_LENGTH, buffer, channels[a].on_string);
          }
	}
	
 	safe_strcat( MAX_STRING_LENGTH, buffer,"@@N");
      }
      send_to_char( buffer, ch );
    }
    else
    {
	bool fClear=TRUE;
	int bit;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
//	    send_to_char( "Channels -channel or +channel?\n\r", ch );
//	    return;
		tog = TRUE;
		i=0;
	}

	bit = 0;
	for ( a = 0; channels[a].bit != 0; a++ )
	{
	   if ( channels[a].min_level > get_trust( ch ) )
	      continue;
	   if ( !str_prefix( arg+i, channels[a].name ) )
	   {
	      bit = channels[a].bit;
	      break;
	   }
	}
	if ( ( bit == 0 ) )
	{
	   send_to_char( "Set or clear which channel?\n\r", ch );
	   return;
	}

	if ( tog )
		fClear = IS_SET(ch->deaf,bit)?TRUE:FALSE;

	if ( fClear ) 
	{
	    REMOVE_BIT (ch->deaf, bit);
	} else {
	    SET_BIT    (ch->deaf, bit);
	}

	send_to_char( "Ok.\n\r", ch );
    }

    return;
}
/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MSL];
    bool rev = FALSE;
    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "@@r[ @@aKeyword  @@r] @@GOption\n\r", ch );

   
	send_to_char( "\n\r@@W[@@R--------@@eGeneral Options@@R--------@@W]\n\r", ch );
	send_to_char(  IS_SET(ch->config, CONFIG_PUBMAIL)
	    ? "@@r[@@a+PUBMAIL  @@r] @@GOther people can see your email address.\n\r"
	    : "@@d[@@c-pubmail  @@d] @@GOther people can NOT see your email address.\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_TELNET_GA)
	    ? "@@r[@@a+TELNETGA @@r] @@GYou receive a telnet GA sequence.\n\r"
	    : "@@d[@@c-telnetga @@d] @@GYou don't receive a telnet GA sequence.\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_FULL_ANSI)
	    ? "@@r[@@a+FULLANSI @@r] @@GYour client supports FULL ANSI.\n\r"
	    : "@@d[@@c-fullansi @@d] @@GYour client does not support full ANSI (GMUD).\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_ECHAN )
	    ? "@@r[@@a+ECHAN    @@r] @@GYour title shows when you use channels.\r\n"
	    : "@@d[@@c-echan    @@d] @@GYour title does NOT show when you use channels.\r\n"
	    , ch );

	send_to_char( "\n\r@@W[@@R-------@@eGame-play Options@@R-------@@W]\n\r", ch );
	send_to_char(  IS_SET(ch->config, CONFIG_NOFOLLOW )
	    ? "@@r[@@a+NOFOLLOW @@r] @@GYou do not allow people to follow you.\r\n"
	    : "@@d[@@c-nofollow @@d] @@GYou allow people to follow you.\r\n"
	    , ch );

	send_to_char( "\n\r@@W[@@R--------@@eDisplay Options@@R--------@@W]\n\r", ch );
	send_to_char(  IS_SET(ch->config, CONFIG_COMBINE)
	    ? "@@r[@@a+COMBINE  @@r] @@GYou see object lists in combined format.\n\r"
	    : "@@d[@@c-combine  @@d] @@GYou see object lists in single format.\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_PROMPT)
	    ? "@@r[@@a+PROMPT   @@r] @@GYou have a prompt.\n\r"
	    : "@@d[@@c-prompt   @@d] @@GYou don't have a prompt.\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_EXITS )
	    ? "@@r[@@a+EXITS    @@r] @@GYou see building exits at the bottom.\r\n"
	    : "@@d[@@c-exits    @@d] @@GYou do not see building exits at the bottom.\r\n"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_BRIEF )
	    ? "@@r[@@a+BRIEF    @@r] @@GYou don't see buildings' descriptions.\r\n"
	    : "@@d[@@c-brief    @@d] @@GYou see buildings' descriptions.\r\n"
	    , ch );

	send_to_char( "\n\r@@W[@@R---------@@eColor Options@@R---------@@W]\n\r", ch );
	send_to_char( IS_SET( ch->config, CONFIG_COLOR )
	   ? "@@r[@@a+COLOR    @@r] @@GYou recieve 'ANSI' color.\n\r"
	   : "@@d[@@c-color    @@d] @@GYou don't receive 'ANSI' color.\n\r"
	   , ch );
		send_to_char(  IS_SET(ch->config, CONFIG_NOCOLORS )
	    ? "@@r[@@a+NOCOLORS @@r] @@GYou see only map colors.\r\n"
	    : "@@d[@@c-nocolors @@d] @@GYou see everything in colors (Turn this on to only see the map in color).\r\n"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_MINCOLORS )
	    ? "@@r[@@a+MINCOLORS@@r] @@GYou see minimal colors.\r\n"
	    : "@@d[@@c-mincolors@@d] @@GYou see full colors (Turn this on to see less colors in the game).\r\n"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_NOBLACK )
	    ? "@@r[@@a+NOBLACK  @@r] @@GYou do not see the color Black.\r\n"
	    : "@@d[@@c-noblack  @@d] @@GYou can see black text.\r\n"
	    , ch );

	send_to_char( "\n\r@@W[@@R----------@@eMap Options@@R----------@@W]\n\r", ch );
	send_to_char(  IS_SET(ch->config, CONFIG_INVERSE )
	    ? "@@r[@@a+INVERSE  @@r] @@GYou see inversed map colors.\r\n"
	    : "@@d[@@c-inverse  @@d] @@GYou do not see inversed map colors.\r\n"
	    , ch );

	send_to_char( "\n\r@@W[@@R---------@@eEnhancements@@R----------@@W]\n\r", ch );
	send_to_char( IS_SET(ch->config, CONFIG_SOUND)
	    ? "@@r[@@a+SOUND    @@r] @@GYou hear sounds sent by the mud.\n\r"
	    : "@@d[@@c-sound    @@d] @@GYou do not hear sounds sent by the mud.\n\r"
	    , ch );
	send_to_char(  IS_SET(ch->config, CONFIG_IMAGE )
	    ? "@@r[@@a+IMAGE    @@r] @@GYour client supports the IMAGE MXP tag.\n\r"
	    : "@@d[@@c-image    @@d] @@GYour client does not support the IMAGE MXP tag.\n\r"
	    , ch );

	send_to_char( "\n\r@@W[@@R--------@@eMisc. Settings@@R---------@@W]\n\r", ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "@@r[@@a+SILENCE  @@r] @@GYou are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_EMOTE)
	    ? ""
	    : "@@r[@@a+NOEMOTE  @@r] @@GYou can't emote.\n\r"
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "@@r[@@a+NOTELL   @@r] @@GYou can't use 'tell'.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->config, CONFIG_CLIENT)
	    ? "@@r[@@a+CLIENT  @@r] @@GYou are using the ASSAULT Client.\n\r"
	    : ""
	    , ch );

  sprintf( buf, "@@aTerminal set to@@d  @@c%i @@aRows@@d, @@c%i @@aColumns@@d.@@N\n\r", 
    ch->pcdata->term_rows, ch->pcdata->term_columns );
  send_to_char( buf, ch );
    }
    else
    {
	bool fSet = FALSE;
	int bit,i=1;
  	bool config_var= TRUE;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    rev = TRUE;
	    i=0;
//	    send_to_char( "Config -option or +option?\n\r", ch );
//	    return;
	}

	if ( !str_cmp( arg+i, "combine"  ) ) bit = CONFIG_COMBINE;
	else if ( !str_cmp( arg+i, "prompt"   ) ) bit = CONFIG_PROMPT;
	else if ( !str_cmp( arg+i, "telnetga" ) ) bit = CONFIG_TELNET_GA;
	else if ( !str_cmp( arg+i, "color"   ) ) bit = CONFIG_COLOR;
	else if ( !str_cmp( arg+i, "fullansi"   ) ) bit = CONFIG_FULL_ANSI;
	else if ( !str_cmp( arg+i, "pubmail" ) ) bit = CONFIG_PUBMAIL;
	else if ( !str_cmp( arg+i, "sound" ) ) bit = CONFIG_SOUND;
	else if ( !str_cmp( arg+i, "echan" ) ) bit = CONFIG_ECHAN;
	else if ( !str_cmp( arg+i, "blind" ) ) bit = CONFIG_BLIND;
	else if ( !str_cmp( arg+i, "exits" ) ) bit = CONFIG_EXITS;
	else if ( !str_cmp( arg+i, "nocolors" ) ) bit = CONFIG_NOCOLORS;
	else if ( !str_cmp( arg+i, "mincolors" ) ) bit = CONFIG_MINCOLORS;
	else if ( !str_cmp( arg+i, "nofollow" ) ) bit = CONFIG_NOFOLLOW;
	else if ( !str_cmp( arg+i, "nomap" ) ) bit = CONFIG_NOMAP;
	else if ( !str_cmp( arg+i, "client" ) ) bit = CONFIG_CLIENT;
	else if ( !str_cmp( arg+i, "brief" ) ) bit = CONFIG_BRIEF;
	else if ( !str_cmp( arg+i, "image" ) ) bit = CONFIG_IMAGE;
	else if ( !str_cmp( arg+i, "noblack" ) ) bit = CONFIG_NOBLACK;
	else if ( !str_cmp( arg+i, "inverse" ) ) bit = CONFIG_INVERSE;
	else if ( !str_cmp( arg+i, "NOLEGEND" ) ) bit = CONFIG_NOLEGEND;


	else if ( !str_cmp(arg+i,"skipnewbie") )
	{
		if ( IS_NEWBIE(ch) )
			ch->played_tot = 5*3600;
		return;
	}
  else if ( !str_prefix("rows", arg+i ) )
  {
    char arg1[MSL];
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg1 );
    if ( !is_number( arg1 ) )
    {
      send_to_char( "That is not a number.\n\r", ch );
      return;
    }
    ch->pcdata->term_rows = URANGE( 10, atoi( arg1 ), 80 );
    return;
  }
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}

	if ( rev )
	{
		fSet = !IS_SET(ch->config,bit);
	}
	if ( fSet )
  {
	    SET_BIT    (config_var?ch->config:ch->act, bit);
	    if ( bit != CONFIG_NOMAP )
	    	send_to_char( "@@dEnabled.@@N\n\r", ch );
      if ( bit == CONFIG_FULL_ANSI )
      {
        char scrollbuf[MSL];
        sprintf( scrollbuf, "%s%s%s%i;%ir%s%i;%iH",
          CRS_RESET,
          CRS_CLS,
          CRS_CMD,
          0,
          ch->pcdata->term_rows - 12,
          CRS_CMD,
          ch->pcdata->term_rows - 13,
          0 );
        send_to_char( scrollbuf, ch );
      }
  }
	else
  {
	    REMOVE_BIT (config_var?ch->config:ch->act, bit);
	    if ( bit != CONFIG_NOMAP )
	    	send_to_char( "@@dDisabled.@@N\n\r", ch );
      if ( bit == CONFIG_FULL_ANSI )
      {

          
        send_to_char( CRS_RESET, ch );
        send_to_char( "Done.\n\r", ch );

      }
      if ( bit == CONFIG_SOUND )
	send_to_char("!!MUSIC(off)\n\r", ch );

  }

    if ( bit == CONFIG_NOMAP && !IS_SET(ch->config,CONFIG_NOMAP) )
	do_look(ch,"");

      if ( IS_SET(ch->config,CONFIG_BLIND) )
	{
		send_to_char( "Note: To see the details of the buildings around you, type \"buildings\". You may also specify the specific player you want to display.\n\r", ch );
		sprintf(buf,"%s turned CONFIG_BLIND on.",ch->name);
		log_f(buf);
	}
	else if (bit == CONFIG_BLIND)
	{
		sprintf(buf,"%s turned CONFIG_BLIND off.",ch->name);
		log_f(buf);
	}
    }

    save_char_obj(ch);
    return;
}

void do_wizlist ( CHAR_DATA *ch, char *argument )
{

    do_help ( ch, "wizlist" );
    return;
}

void do_pubmail ( CHAR_DATA *ch, char *argument )
{
    ( IS_SET ( ch->config, CONFIG_PUBMAIL )
     ? do_config( ch, "-pubmail" )
     : do_config( ch, "+pubmail" ) );
}

void do_sound ( CHAR_DATA *ch, char *argument )
{
	( IS_SET ( ch->config, CONFIG_SOUND )
	 ? do_config( ch, "-sound" )
	 : do_config( ch, "+sound") );
}

void do_combine ( CHAR_DATA *ch, char *argument )
{

    ( IS_SET ( ch->config, CONFIG_COMBINE )
     ? do_config( ch, "-combine" )
     : do_config( ch, "+combine" ) );

}
 
void do_pagelen ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int lines;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	sprintf( buf, "Your page length is %d.\n\r", ch->pcdata->pagelen );
	send_to_char(buf,ch);
	return;
    }
    else
	lines = atoi( arg );

    if ( lines < 1 )
    {
	send_to_char(
		"Negative or Zero values for a page pause is not legal.\n\r",
		     ch );
	return;
    }

    ch->pcdata->pagelen = lines;
    sprintf( buf, "Page pause set to %d lines.\n\r", lines );
    send_to_char( buf, ch );
    return;
}
void do_prompt( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

   if ( argument[0] == '\0' || !str_cmp(argument,"off") )
   {
       ( IS_SET ( ch->config, CONFIG_PROMPT )
	? do_config( ch, "-prompt" )
	: do_config( ch, "+prompt" ) );
       return;
   }

    if ( !str_cmp(argument,"all") )
	strcpy( buf, "@@e%h/%Hhp @@G>@@N " );
    else if ( !str_cmp(argument,"newbie") )
	strcpy( buf, "@@W(@@c%C-%s@@W) @@W[@@gExits:%E@@W] @@W[@@GCash:@@r %$@@W] @@e%h/%H@@Rhp @@N> ");
    else if ( !str_cmp(argument,"cool") )
	strcpy( buf, "@@c[%q] @@e[@@r%h@@G/@@r%H@@Ghp @@r%T@@Gft @@y%Q@@Gqp@@e] @@cV: @@a%v@@c/@@a%V@@chp @@a%f@@c/@@a%F@@cmv @@a%m@@c/@@a%M@@cam (%C)@@N " );
    else if ( !str_cmp(argument,"riune") )
	strcpy( buf, "@@W[@@G%C@@W|@@N%s@@W] [HP: @@e%h@@W] [T: @@e%T@@W] [@@e%E@@W] [Cash: @@G%$@@W]@@N " );
    else
   {
      if ( strlen(argument) > 200 )
	 argument[200] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   send_to_char( "Ok.\n\r", ch );
   return;
} 

void do_auto( CHAR_DATA *ch, char *argument )
{
    do_config( ch, argument );
    return;

}
void do_diagnose( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int pct;

    
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Diagnose whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }
    
    
    if ( can_see( ch, victim ) )
    {
	act( "$n gives you the once-over.", ch, NULL, victim, TO_VICT    );
	act( "$n gives $N the once-over.",  ch, NULL, victim, TO_NOTVICT );
    }
    else
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }
    
    if ( victim->max_hit > 0 )
	pct = ( 100 * victim->hit ) / victim->max_hit;
    else
	pct = -1;

    strcpy( buf, PERS(victim, ch) );

	 if ( pct >= 100 ) safe_strcat(MAX_STRING_LENGTH, buf, " --  [5] 4  3  2  1   \n\r" );
    else if ( pct >=  80 ) safe_strcat(MAX_STRING_LENGTH, buf, " --   5 [4] 3  2  1   \n\r" );
    else if ( pct >=  60 ) safe_strcat(MAX_STRING_LENGTH, buf, " --   5  4 [3] 2  1   \n\r" );
    else if ( pct >=  40 ) safe_strcat(MAX_STRING_LENGTH, buf, " --   5  4  3 [2] 1   \n\r" );
    else                   safe_strcat(MAX_STRING_LENGTH, buf, " --   5  4  3  2 [1]  \n\r" );
    
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );


    return;
}

void do_color( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->config,CONFIG_COLOR) )
	do_config(ch,"-color");
    else
	do_config(ch,"+color");

    return;
}

/* A simple, return the char sequence, function -S- */

char * color_string( CHAR_DATA *ch, char *argument )
{
   int cnt;
   int num;
   
   
   /* if we don't want to send the string, return "" */
   /* argument should be the string to find, eg "say" */
   
   if ( IS_NPC( ch ) || argument[0] == '\0' )
      return( "" );
   
   if ( !IS_SET( ch->config, CONFIG_COLOR ) )
      return( "" );
      
   if ( !str_cmp( argument, "normal" ) )
      return( "\x1b[0m" );
   
   /* By here, ch is a PC and wants color */
   
   num = -1;
   for ( cnt = 0; cnt < MAX_color; cnt ++ )
     if ( !str_cmp( argument, color_table[cnt].name ) )
	num = cnt;
	
   if ( num == -1 ) /* bug report? */
   
      return( "" );
   
   return( ansi_table[ch->pcdata->color[num]].value );

} 

void do_whois( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   if ( ( victim = get_char_world( ch, argument ) ) == NULL )
   {
      send_to_char( "No such player found.\n\r", ch );
      return;
   }
 
   if ( !can_see( ch, victim ) )
   {
      send_to_char( "No such player found.\n\r", ch );
      return;
   }

   /* Ok, so now show the details! */
   sprintf( buf, "@@r+@@G-=-=-=-=-=-=-=-=-=-=- @@c%11s @@G-=-=-=-=-=-=-=-=-=-=-@@r+@@N\n\r", victim->name );
   if ( IS_IMMORTAL( victim ) )
   {
      sprintf( buf+strlen(buf), " @@r[ %3s @@r]@@N\n\r", victim->pcdata->who_name );
   }
   if ( IS_GUIDE ( victim ) )
   {
      sprintf( buf+strlen(buf), " @@r[ NEWBIE HELPER @@r]@@N\n\r" );
   }

   sprintf( buf+strlen(buf), "@@GRank@@d: @@r%d@@G   Play Time@@d: @@r%d Hours (%d Total)\n\r", get_rank(victim), my_get_hours(victim,FALSE), my_get_hours(victim,TRUE) );
   sprintf( buf+strlen(buf), "@@ePK Statistics\n\r" );
   sprintf( buf+strlen(buf), "@@bPlayers Killed@@d: @@y%d@@b (@@y%d@@b).\n\r", victim->pcdata->pkills, victim->pcdata->tpkills  );
   sprintf( buf+strlen(buf), "@@bBuildings Destroyed@@d: @@y%d@@b (@@y%d@@b).\n\r", victim->pcdata->bkills, victim->pcdata->tbkills );
   sprintf( buf+strlen(buf), "@@bDeaths@@d: @@y%d@@b.\n\r", victim->pcdata->deaths );
   if ( victim->pcdata->blost <= 30000 )
   	sprintf( buf+strlen(buf), "@@bBuildings Lost@@d: @@y%d@@b.\n\r\n\r", victim->pcdata->blost );
   else
   	sprintf( buf+strlen(buf), "@@bBuildings Lost@@d: @@yOver 30,000@@b.\n\r\n\r" );


   sprintf( buf+strlen(buf), "@@eMinigame Statistics\n\r" );
   sprintf( buf+strlen(buf), "@@cPaintball:   @@dHits: @@W%d   @@dLosses: @@W%d\n\r", victim->pcdata->pbhits, victim->pcdata->pbdeaths );
   sprintf( buf+strlen(buf), "@@cNUKEM:       @@dWins: @@W%d\n\r", victim->pcdata->nukemwins );

   if ( IS_SET ( victim->config, CONFIG_PUBMAIL ) )
    sprintf(buf+strlen(buf), "\n\r@@WEmail: %s@@N\n\r", victim->pcdata->email_address );

   sprintf( buf+strlen(buf), "@@r+@@G-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-@@r+@@N\n\r");
   send_to_char(buf,ch);
   return;
}
void do_afk( CHAR_DATA *ch, char *argument )
{
    int value;
    
    if ( ch->fighttimer > 0 )
    {
	send_to_char( "Excuse me? While in combat? I don't think so.\n\r", ch );
	return;
    }
    value = PFLAG_AFK;
    
    if ( IS_SET( ch->pcdata->pflags, value ) )
    {
       REMOVE_BIT( ch->pcdata->pflags, value );
       send_to_char( "AFK flag turned off.\n\r", ch );
    }
    else
    {
       SET_BIT( ch->pcdata->pflags, value );
       send_to_char( "AFK flag turned on. Remember, you are AFK at your own risk - People CAN come and attack you!\n\r", ch );
    }
    return;
}  


void do_colist( CHAR_DATA *ch, char *argument )
{
   int col, n;
   char buf[MAX_STRING_LENGTH];
   
   if IS_NPC( ch )
      return;
   
   send_to_char( "@@WColor Codes: @@yTo use color codes within a string, use the following\n\r", ch );
   send_to_char( "characters in sequence: @@@@<letter>.@@N\n\r\n\r", ch );
   
   n = 0;
   
   for ( col = 0; col < MAX_ANSI; col++ )
   {
      if ( col >= 29 )
	continue;
      sprintf( buf, "%c - %s%-14s@@N    ", 
	 ansi_table[col].letter, 
	 IS_SET( ch->config, CONFIG_COLOR ) ? ansi_table[col].value : "", 
	 ansi_table[col].name );
      send_to_char( buf, ch );
      if ( ++n % 3 == 0 )
	 send_to_char( "\n\r", ch );
   }
   if ( n % 3 != 0 )
      send_to_char( "\n\r", ch );
   
   send_to_char( "\n\r", ch );
   return;
}
void do_highscores( CHAR_DATA *ch, char *argument )
{
	int i,max,j, leng;
	char buf[MSL];
	int order[100];
	bool appears[100];

	for ( i = 0;i<100;i++ )
	{
		order[i] = -1;
		appears[i] = FALSE;
	}
//	order[0] = 0;
	for ( j = 0;j < 100;j++ )
	{
		max = -1;
		for ( i=0;i<100;i++ )
		{
			if ( score_table[i].kills == 0 && score_table[i].buildings == 0 && score_table[i].time == 0 )
				continue;
			if ( appears[i] )
				continue;
			if ( j >= 99 )
				break;
			if ( (( score_table[max].kills < score_table[i].kills || ( score_table[i].kills == score_table[max].kills && score_table[i].buildings > score_table[max].buildings ) || ( score_table[max].kills == score_table[i].kills && score_table[max].buildings == score_table[i].buildings  && score_table[max].time < score_table[i].time ) ) && !appears[i] ))
			{
				max = i;
			}
		}
		if ( !appears[max] )
		{
			order[j] = max;
			appears[max] = TRUE;
		}
	}

	leng = 30;
	if ( argument[0] != '\0' && is_number(argument) )
		leng = atoi(argument);

	if ( leng > 100 || leng <= 0 )
	{
		send_to_char( "Length must be from 1 to 100.\n\r", ch );
		return;
	} 

	send_to_char( "\n\r@@G  @@d| @@GName         @@d| @@GKills @@d| @@GBuildings @@d| @@GHours @@d| @@GKilled By\n\r", ch );
	sprintf( buf, "@@r------------------------------------------------------------------------------\n\r" );
	for ( i=0;i<leng;i++ )
	{
		if ( order[i] == -1 || score_table[i].name == NULL )
			break;
		sprintf( buf+strlen(buf), "@@W%2d@@d| @@a%-12s @@d| @@e%4d  @@d|    @@b%4d   @@d|   @@c%2d  @@d| @@y%-13s\n\r", i+1, score_table[order[i]].name, score_table[order[i]].kills, score_table[order[i]].buildings, score_table[order[i]].time, score_table[order[i]].killedby );
		if ( i == leng / 2 )
		{
			send_to_char( buf, ch );
			buf[0] = '\0';
		}
	}
	send_to_char( buf, ch );
	send_to_char( "@@r------------------------------------------------------------------------------@@N\n\r", ch );
	return;
}
void do_highranks( CHAR_DATA *ch, char *argument )
{
	int i,max,j, leng;
	char buf[MSL];
	int order[30];
	bool appears[30];

	for ( i = 0;i<30;i++ )
	{
		order[i] = -1;
		appears[i] = FALSE;
	}
	for ( j = 0;j < 30;j++ )
	{
		max = -1;
		for ( i=0;i<30;i++ )
		{
			if ( j >= 30 )
				break;
			if ( rank_table[i].rank <= 1 || rank_table[i].name == NULL || appears[i] )
				continue;
			if ( max == -1 || rank_table[i].rank > rank_table[max].rank )
				max = i;
		}
		if ( !appears[max] )
		{
			order[j] = max;
			appears[max] = TRUE;
		}
	}

	leng = 10;
	if ( argument[0] != '\0' && is_number(argument) )
		leng = atoi(argument);

	if ( leng > 30 || leng <= 0 )
	{
		send_to_char( "Length must be from 1 to 30.\n\r", ch );
		return;
	} 

	send_to_char( "\n\r@@G  @@d| @@GName         @@d| @@GRank\n\r", ch );
	sprintf( buf, "@@r----------------------------------------\n\r" );
	for ( i=0;i<leng;i++ )
	{
		if ( order[i] == -1 || rank_table[order[i]].name == NULL )
		{
			sprintf( buf+strlen(buf), "End of list.\n\r" );
			break;
		}
		sprintf( buf+strlen(buf), "@@W%2d@@d| @@a%-12s @@d| @@e%4d\n\r", i+1, rank_table[order[i]].name, rank_table[order[i]].rank );
		if ( i == leng / 2 )
		{
			send_to_char( buf, ch );
			buf[0] = '\0';
		}
	}
	send_to_char( buf, ch );
	sprintf( buf, "@@r----------------------------------------\n\r" );
	return;
}

void do_map( CHAR_DATA *ch, char *argument )
{
	if ( !str_cmp(argument,"legend") || !str_cmp(argument,"legand"))
	{
		int i;
		float s;
		char buf[MSL];
		for ( i=0;i<SECT_MAX;i++ )
		{
			s = (float)(wildmap_table[i].speed)/PULSE_PER_SECOND;
			sprintf( buf, "%s%s @@N- %-20s    @@dRun Time: @@g%5.2f Seconds,  @@cFuel: @@a%-3d\n\r",wildmap_table[i].color,wildmap_table[i].mark, wildmap_table[i].name, s, wildmap_table[i].fuel  );
			send_to_char(buf,ch);
		}
		return;
	}
	else if ( is_number(argument) )
	{
		if ( atoi(argument) < 3 || atoi(argument) > 25 )
		{
			send_to_char( "Invalid value.\n\r", ch );
			return;
		}
		ch->map = atoi(argument);
		send_to_char( "Map size set.\n\r", ch);
		save_char_obj(ch);
		return;
	}
	else
	{
		do_look(ch,"outside");
		return;
	}
	return;
}
void do_alias( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];
	char arg2[MSL];
	int i;

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if ( arg[0] == '\0' || arg2[0] == '\0' || !is_number(arg) || is_number(arg2) )
	{
		char buf[MSL];
		for (i=0;i<5;i++)
		{
			if ( str_cmp(ch->alias[i],"") )
			{
				sprintf( buf, "%d. %s   ->  %s\n\r", i+1,ch->alias[i],ch->alias_command[i]);
				send_to_char( buf, ch);
			}
		}
		return;
	}

	i = atoi(arg);
	if ( i < 1 || i > 5 )
	{
		send_to_char( "1 - 5 Please.\n\r", ch);
		return;
	}
	i--;
	free_string(ch->alias[i]);
	free_string(ch->alias_command[i]);
	if ( !str_cmp(arg2,"remove") )
	{
		ch->alias[i] = str_dup("");
		ch->alias_command[i] = str_dup("");
	}
	else
	{
		ch->alias[i] = str_dup(arg2);
		ch->alias_command[i] = str_dup(argument);
	}
	send_to_char( "Ok.\n\r", ch);
	return;
}
void do_quest( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	char rbuf[MSL];
	char borderbuf[MSL];
	int i = 0,x;
	bool planet = FALSE;
	OBJ_INDEX_DATA *pObj;

	if ( !str_prefix(argument,"planet") && argument[0] != 0 )
		planet = TRUE;
	if ( argument[0] == '\0' || planet )
	{
		x = 0;
		send_to_char("\n\r", ch );
		sprintf( borderbuf, "@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@W-@@g-@@N\n\r" );
		if ( !IS_SET(ch->config,CONFIG_BLIND) )
			send_to_char( borderbuf, ch );
		for ( i=0;i<MAX_QUESTS;i++ )
		{
			if ( quest_table[i].type == 1 )
			{
				pObj = get_obj_index(quest_table[i].reward);
				if (!pObj)
					continue;
				sprintf(rbuf,"%s", pObj->short_descr );
			}
			else
				sprintf(rbuf,"$%d", quest_table[i].reward );
			if ( quest_table[i].target && !can_see(ch,quest_table[i].target) ) continue;
			if ( quest_table[i].bld )
			{
				if ( IS_SET(ch->config,CONFIG_BLIND) )
					sprintf(buf,"%2d. Target: %-8s's %-10s. Reward: %s. Time: %d Seconds\n\r", i+1,quest_table[i].target->name, quest_table[i].bld->name, rbuf, quest_table[i].time );
				else
					sprintf(buf,"@@W| %2d. @@N%-8s's %-10s [%s] [Time: %d Seconds]\n\r", i+1,quest_table[i].target->name, quest_table[i].bld->name, rbuf, quest_table[i].time );
				send_to_char(buf,ch);
				x++;
			}
			else if ( quest_table[i].target )
			{
				if ( IS_SET(ch->config,CONFIG_BLIND) )
					sprintf(buf,"%2d. Target: %-20s. Reward: %s. Time: %d Seconds\n\r", i+1,quest_table[i].target->name,rbuf, quest_table[i].time );
				else

					sprintf(buf,"@@W| %2d. @@N%-20s  [%s] [Time: %d Seconds]\n\r", i+1,quest_table[i].target->name,rbuf, quest_table[i].time );
				send_to_char(buf,ch);
				x++;
			}
		}
		if ( x == 0 )
		{
			send_to_char( "@@W| @@NNo quests found.\n\r", ch );
		}
		if ( !IS_SET(ch->config,CONFIG_BLIND) )
			send_to_char( borderbuf, ch );
//		send_to_char( "\n\r@@eType Quest <quest number>  to see the clue. (Quest 1, for example)\n\r", ch );
		return;
	}
	else if (is_number(argument) )
	{
		bool found = FALSE;

		i = atoi(argument);
		if ( i < 1 || i > MAX_QUESTS )
		{
			send_to_char( "That is not a valid quest number.\n\r", ch);
			return;
		}
		i--;

		if ( !found )
		{
			send_to_char( "That is not a valid quest number.\n\r", ch);
			return;
		}
		
		send_to_char( "Invalid selection.\n\r", ch );
	}
	return;
}

void do_identify( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char buf[MSL];
	int val;

	if ( ( obj = get_obj_carry(ch,argument) ) == NULL )
	{
		send_to_char( "You do not possess that.\n\r", ch );
		return;
	}
	if ( obj->item_type == ITEM_ARMOR )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		send_to_char( buf, ch );
		sprintf( buf, "Rarity/Quality: %d%%\n\r", obj->level );
		send_to_char( buf, ch );
		sprintf( buf, "Heat Addition: %d\n\r", obj->heat );
		send_to_char(buf,ch);
		sprintf( buf, "Equipment Damage: %d\n\r\n\r", obj->value[1] );
		send_to_char(buf,ch);

		sprintf( buf, "\n\rDamage Absorption:\n\r" );
		val = 2;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. General Damage: %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val]);
		val = 3;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Bullet Damage:  %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val]);
		val = 4;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Blast Damage:   %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val] );
		val = 5;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Acid Damage:    %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val] );
		val = 6;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Fire Damage:    %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val] );
		val = 7;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Laser Damage:   %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val]);
		val = 8;
		if ( obj->value[val] != 0 )
			sprintf(buf+strlen(buf), "%s vs. Sound Damage:   %d%%\n\r", (obj->value[val]<0)?"Weak":"Strong", obj->value[val]);

		send_to_char( buf, ch );
		if ( obj->pIndexData->building != -1 )
		{
			sprintf( buf, "Generates in: %s\n\r", build_table[obj->pIndexData->building].name );
			send_to_char( buf, ch );
		}
	}
	else if ( obj->item_type == ITEM_WEAPON )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		send_to_char( buf, ch );
		sprintf( buf, "Rarity: %d%%\n\r", obj->level );
		send_to_char( buf, ch );
		sprintf( buf, "Ammo: %d/%d\n\r", obj->value[0], obj->value[1] );
		send_to_char( buf, ch );
		sprintf( buf, "Range: %d Rooms\n\r", obj->value[4] + 1 );
		send_to_char( buf, ch );
		sprintf( buf, "Scope: %d\n\r", obj->value[6] );
		send_to_char( buf, ch );
		sprintf( buf, "Ammo Type: %s\n\r", clip_table[obj->value[2]].name );
		send_to_char( buf, ch );
		sprintf( buf, "Damage: %d\n\r", clip_table[obj->value[2]].dam + obj->value[7] );
		send_to_char( buf, ch );
		sprintf( buf, "Building Damage: %d\n\r", clip_table[obj->value[2]].builddam + obj->value[8] );
		send_to_char( buf, ch );
		sprintf( buf, "Hit Chance: %d%%\n\r", clip_table[obj->value[2]].miss + obj->value[10] );
		send_to_char( buf, ch );
		sprintf( buf, "Explodes: %s\n\r", ( clip_table[obj->value[2]].explode == TRUE ) ? "Yes (50% of double building damage)" : "No" );
		send_to_char( buf, ch );
		sprintf( buf, "Delay: %d\n\r\n\r", clip_table[obj->value[2]].speed + obj->value[9] );
		if ( obj->pIndexData->building != -1 )
		{
			sprintf( buf+strlen(buf), "Generates in: %s\n\r", build_table[obj->pIndexData->building].name );
			send_to_char( buf, ch );
		}
		if ( IS_SET(obj->value[3],WEAPON_CONFUSING) )
			send_to_char( "* Special Effect: Confusion\n\r", ch );
		if ( IS_SET(obj->value[3],WEAPON_ALCOHOL) )
			send_to_char( "* Special Effect: Alcohol\n\r", ch );
		if ( IS_SET(obj->value[3],WEAPON_BLINDING) )
			send_to_char( "* Special Effect: Blindness\n\r", ch );
		if ( IS_SET(obj->value[3],WEAPON_HITS_AIR) )
			send_to_char( "\n\r* This weapon can hit airborn targets.\n\r", ch );
	}
	else if ( obj->item_type == ITEM_INSTALLATION )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		if ( obj->value[1] != 0 )
			sprintf( buf+strlen(buf), "Installs in: %s\n\r", ( obj->value[1] == -1 ) ? "All Buildings" : build_table[obj->value[1]].name );
		if ( obj->value[2] > 0 )
			sprintf( buf+strlen(buf), "Installs in: %s\n\r", build_table[obj->value[2]].name );
		if ( obj->value[3] > 0 )
			sprintf( buf+strlen(buf), "Installs in: %s\n\r", build_table[obj->value[3]].name );
		if ( obj->value[4] > 0 )
			sprintf( buf+strlen(buf), "Installs in: %s\n\r", build_table[obj->value[4]].name );
		sprintf( buf+strlen(buf),"\n\r" );
		if ( obj->pIndexData->building != -1 )
		{
			sprintf( buf+strlen(buf), "Generates in: %s\n\r", build_table[obj->pIndexData->building].name );
			send_to_char( buf, ch );
		}
	}
	else if ( obj->item_type == ITEM_WEAPON_UP )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Weapon installation: Install using the command @@eWInstall@@N.\n\r" );
		sprintf( buf+strlen(buf), "No further info...\n\r" );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_VEHICLE_UP )
	{
		int i;
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Vehicle Addon: Install using the command @@eVInstall@@N.\n\r" );
		for ( i=2;i<10;i++ )
		{
			if ( i > 0 && i < MAX_VEHICLE )
				sprintf( buf+strlen(buf), "Installs on: %s.\n\r", vehicle_desc[obj->value[i]] );
		}
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_BOMB )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Timer Set to: %d\n\r", obj->value[0] );
		sprintf( buf+strlen(buf), "Armed: %s\n\r", (obj->value[1] != 0)?"Yes":"No");
		sprintf( buf+strlen(buf), "Weight: %d\n\r", get_obj_weight(obj) );
		sprintf( buf+strlen(buf), "Damage Modifier: %d\n\r", obj->value[2] );
		sprintf( buf+strlen(buf), "Damage Range: %d - %d\n\r", (obj->weight*obj->value[2]/2), obj->weight * obj->value[2] );
		sprintf( buf+strlen(buf), "Dud Chance: %d%%\n\r", obj->value[4] );
		if ( obj->value[3] == 1 )
			sprintf( buf+strlen(buf), "\n\r* Nuclear\n\r" );
		if ( obj->pIndexData->vnum == OBJ_VNUM_GRANADE )
			sprintf( buf+strlen(buf), "\n\r* Grenade\n\r" );
		if ( obj->pIndexData->vnum == OBJ_VNUM_FLASH_GRENADE )
			sprintf( buf+strlen(buf), "\n\r* Flash Grenade - Only damages people\n\r" );
		if ( obj->pIndexData->vnum == OBJ_VNUM_SMOKE_BOMB )
			sprintf( buf+strlen(buf), "\n\r* Smoke Bomb - Only damages people\n\r" );
		if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
			sprintf( buf+strlen(buf), "\n\r* Deadman Switch - Explodes when you die if worn\n\r" );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_BLUEPRINT )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Upgrades: %s\n\r", build_table[obj->value[0]].name );
		if ( obj->value[1] == 0 )
			sprintf( buf+strlen(buf), "Upgrades to level: %d\n\r", obj->level );
		else
			sprintf( buf+strlen(buf), "Upgrades into: %s\n\r", build_table[obj->value[1]].name );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_MEDPACK )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Quality: %d\n\r", obj->value[0] );
		sprintf( buf+strlen(buf), "\n\rNote: Use the @@eHeal@@N command to use. Some types of medpacks can be poisonous.\n\r" );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_COMPUTER )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Range: %d\n\rBattery: %d\n\r", obj->value[0], obj->value[1] );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_TOOLKIT )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf),"Quality: %d\n\r", obj->level );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_DISK )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Version: %d.%d\n\r", obj->value[1]/10,obj->value[1]%10 );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_SCAFFOLD )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
		sprintf( buf+strlen(buf), "Scaffold Item - Drop outside to make it open into a building.\n\r" );
		sprintf(buf+strlen(buf), "Building: %s\n\r", build_table[obj->value[0]].name );
		sprintf(buf+strlen(buf), "Level: %d\n\r", obj->level );
		send_to_char( buf, ch );
	}
        else if ( obj->item_type == ITEM_SUIT )    
        {
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
                sprintf( buf+strlen(buf), "Type: %s\n\r", (obj->value[0]==0)?"Jammer - Causes invisiblity against players and defenses":(obj->value[0]==0)?"Nullification - Removes bad effects instantly":"Unknown" );
		sprintf( buf+strlen(buf), "Quality: %d\n\r", obj->value[1] );
                send_to_char( buf, ch );
        }	
	else if ( obj->item_type == ITEM_ORE )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
                sprintf( buf+strlen(buf), "Unprocessed Ore - @@eResearch@@N in space to create an item!\n\r" ); 
		sprintf( buf+strlen(buf), "Research Time: %d Seconds (About %d minutes)\n\r", obj->value[0], obj->value[0]/60 );
		send_to_char( buf, ch );
	}
	else if ( obj->item_type == ITEM_BIOTUNNEL )
	{
		sprintf( buf, "Statistics for: %s\n\r", capitalize(obj->short_descr) );
                sprintf( buf+strlen(buf), "BioTunnel - Transports items from a Transmitting tunnel to Receiving using the SETTUNNEL command.\n\r" ); 
		sprintf( buf+strlen(buf), "Type: %s\n\r", (obj->value[0]==1)?"Transmitting":"Receiving");
		if ( obj->value[0] == 1 )
			sprintf( buf+strlen(buf), "Range: %d\n\r", obj->value[1]);
		send_to_char( buf, ch );
	}
	else
		send_to_char( "You cannot identify this type of item.\n\r", ch );
	sprintf( buf, "\n\rItem type: %s\n\r", item_type_name(obj) );
	send_to_char(buf,ch);

	return;
}
void do_skills( CHAR_DATA *ch, char *argument ) {
	char buf[MSL];
	char cbuf[MSL];
	int i;

	sprintf( cbuf, "\n\r                                   @@rProficiencies:@@N\n\r@@g-------------------------------------------------------------------------------\n\r" );
	send_to_char( "\n\r                                   @@rNormal Skills:@@N\n\r@@g-------------------------------------------------------------------------------\n\r", ch );
	for ( i = 0;skill_table[i].gsn != -1;i++ )
	{
		if ( skill_table[i].prof )
		{
			sprintf( cbuf+strlen(cbuf), "@@g| @@c%25s @@g| @@c%-40s @@g| %3d  @@g|\n\r", skill_table[i].name, skill_table[i].desc, ch->pcdata->skill[skill_table[i].gsn] );
			continue;
		}
		sprintf( buf, "@@g| @@c%25s @@g| @@c%-40s @@g| %3d%% @@g|\n\r", skill_table[i].name, skill_table[i].desc, ch->pcdata->skill[skill_table[i].gsn] );
		send_to_char( buf, ch );
	}
	send_to_char("-------------------------------------------------------------------------------\n\r", ch );
	send_to_char(cbuf,ch);
	send_to_char("-------------------------------------------------------------------------------\n\r", ch );
	send_to_char("\n\r",ch);
	for ( i=0;ability_table[i].economics != -1;i++ )
	{
		if ( has_ability(ch,i) )
		{
			sprintf(buf,"@@gYou have unlocked: @@x%s@@g (%s)!\n\r",ability_table[i].name,ability_table[i].desc);
			send_to_char(buf,ch);
		}
	}
	return;
}

void do_classes( CHAR_DATA *ch, char *argument )
{
   int i;
   char buf[MSL];
   char buf2[MSL];
   int rank = get_rank(ch);
 
   sprintf( buf, "\n\r@@NClasses available to you:\n\r\n\r" );
   sprintf( buf2, "\n\r@@dClasses not available to you yet (At rank %d):\n\r\n\r", rank );
   for ( i=0;i<MAX_CLASS;i++ )
   {
        if ( class_table[i].rank <= rank )
                sprintf( buf+strlen(buf), "%s%s - %s: %s\n\r", ( class_table[i].rec ) ? "@@a" : "@@c", class_table[i].who_name, class_table[i].name, class_table[i].desc );
        else
                sprintf( buf2+strlen(buf2), "%s (Requires rank %d)\n\r", class_table[i].name, class_table[i].rank );
   }
   send_to_char( buf2, ch );
   send_to_char( buf, ch );
   send_to_char( "\n\rClasses in @@alight cyan@@c (For people with color) are recommended for new players.\n\r", ch );
   return;	 
}

void do_status( CHAR_DATA *ch, char *argument )
{
	if ( IS_SET(ch->config,CONFIG_BLIND) )
	{
		do_vehicle_status(ch,argument);
		return;
	}
	do_a_build(ch,"report");
	return;
}

void do_info( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];
	int x,y,hp,shield;
	BUILDING_DATA *bld;
	char buf[MSL];

        argument = one_argument(argument,arg);
        if ( !is_number(arg) || !is_number(argument) )
        {
                send_to_char( "Syntax: info <x> <y>\n\r", ch );
                return;
        }
        x = atoi(arg);
        y = atoi(argument);
        if ( INVALID_COORDS(x,y) )
        {
                send_to_char( "Invalid coords.\n\r", ch );
                return;
        }
	if (( !IS_BETWEEN(x,ch->x-ch->map,ch->x+ch->map) || !IS_BETWEEN(y,ch->y-ch->map,ch->y+ch->map)) && !IS_IMMORTAL(ch) )
	{
		send_to_char( "You can't see that far.\n\r", ch );
		return;
	}
	if ( ( bld = get_building(x,y,ch->z) ) == NULL )
	{
		send_to_char( "You don't see a building there.\n\r", ch );
		return;
	}
	if ( bld->type == BUILDING_DUMMY && (bld->value[0] < 1 || bld->value[0] >= MAX_BUILDING ))
		bld->value[0] = BUILDING_TURRET;
	
	shield = 0;
	hp = 0;
	if ( bld->maxhp > 0 )
		hp = ((10000/(bld->maxhp))*bld->hp)/100;
	if ( bld->maxshield > 0 )
		shield = ((10000/(bld->maxshield))*bld->shield)/100;
	sprintf( buf, "Building: %s.\n\rOwned By: %s.\n\rHP: %d%%   Shield: %d%%\n\r", (bld->type != BUILDING_DUMMY) ? bld->name : build_table[bld->value[0]].name , bld->owned,hp,shield ); 
	if ( !complete(bld) )
		sprintf(buf+strlen(buf),"\n\r@@eINCOMPLETE BUILDING@@N\n\r" );
	send_to_char(buf,ch);
	if ( ch->class == CLASS_SPY )
	{
		buf[0] = '\0';
		send_to_char("\n\rYour spies report the following:\n\r", ch );
		sprintf( buf+strlen(buf), "\n\rHP: @@e%d/@@R%d@@N     Shield: @@y%d@@b/%d@@N     Level: @@c%d@@N\n\r", bld->hp, bld->maxhp, bld->shield, bld->maxshield, bld->level );
		sprintf( buf+strlen(buf), "\n\rYou can enter the building from the:" );
		if ( bld->exit[DIR_NORTH] )
			safe_strcat(MSL,buf,"  @@gNorth");
		if ( bld->exit[DIR_SOUTH] )
			safe_strcat(MSL,buf,"  @@bSouth");
		if ( bld->exit[DIR_EAST] )
			safe_strcat(MSL,buf,"  @@cEast");
		if ( bld->exit[DIR_WEST] )
			safe_strcat(MSL,buf,"  @@RWest");
		safe_strcat(MSL,buf,"@@N.\n\r");
		if ( bld->type == BUILDING_DUMMY )
			safe_strcat(MSL,buf,"\n\r* This building is a dummy trap!\n\r" );
		send_to_char(buf,ch);
	}
	return;
}


void set_max_ever( int max )
{
	FILE * fp;
	extern int max_players;
	extern int max_players_ever;

	fclose( fpReserve );
//	max_players_ever = max;
	 if ( ( fp = fopen( MAX_PLAYERS_FILE, "w" ) ) == NULL )
    {
        bug( "Set Max Ever: fopen", 0 );
        perror( "failed open of max_players_file in set_max_ever" );
    }   
    else
    {
	fprintf(fp,"%d\n%d",max_players_ever,(max)?0:max_players);
    }
	fflush(fp);
	fclose(fp);
	fpReserve = fopen( NULL_FILE, "r" );
	return;
}

void show_building_info(CHAR_DATA *ch, int i)
{
	int j;
	bool ter[SECT_MAX];
	char buf[MSL];
	extern char *building_title[MAX_BUILDING_TYPES];

	if ( i<0 || i >= MAX_BUILDING )
		return;

	for ( j=0;j<SECT_MAX;j++ )
		ter[j] = FALSE;

	if ( IS_SET(ch->config,CONFIG_CLIENT) ) 
	{
		REMOVE_BIT(ch->config,CONFIG_COLOR);
		send_to_char("\4",ch);
	}
	sprintf( buf, "@@dBuilding Information:\n\r@@aName:@@c %s    %s\n\r@@aHP:@@c %d    @@aShield:@@c %d\n\r@@aType:@@c %s\n\r@@aMax Allowed: @@c%d\n\r@@aCost: @@c%d.\n\r",
	build_table[i].name, build_table[i].symbol, build_table[i].hp, build_table[i].shield, building_title[build_table[i].act], build_table[i].max, build_table[i].cost);
	if ( !IS_SET(ch->config,CONFIG_CLIENT) )
	{
		if ( build_table[i].requirements > 0 )
		{
			sprintf(buf+strlen(buf),"@@aRequires: @@c%s", build_table[build_table[i].requirements].name );
			if ( build_table[i].requirements_l > 1 )
				sprintf(buf+strlen(buf), "  Level %d", build_table[i].requirements_l );
		}
		send_to_char( buf, ch );
	
		if ( build_table[i].rank > 0 )
			sprintf( buf, " and Rank %d.\n\r", build_table[i].rank );
		else
			sprintf( buf, "\n\r" );
		send_to_char(buf,ch);
	
		for ( j = 0;j<MAX_BUILDING;j++ )
			if ( build_table[j].requirements == i && build_table[j].rank <= 0 && !build_table[j].disabled )
			{
				sprintf( buf, "@@aRequired for:@@c %-20s", build_table[j].name );
				if ( build_table[j].requirements_l > 1 )
					sprintf(buf+strlen(buf)," (Level %d)", build_table[j].requirements_l );
				sprintf(buf+strlen(buf),"\n\r" );
				send_to_char( buf, ch );
			}
		for ( j=0;j<MAX_BUILDON;j++ )
			if (build_table[i].buildon[j] < SECT_MAX)
				ter[build_table[i].buildon[j]] = TRUE;
		sprintf( buf, "@@aBuild on:@@c " );
		if ( ter[0] )
		{
			sprintf(buf+strlen(buf), "Anywhere (Scaffold)" );
		}
		else
		{
			for ( j=0;j<SECT_MAX;j++ )
				if ( ter[j] )
					sprintf( buf+strlen(buf), "%s%s ", wildmap_table[j].color, wildmap_table[j].name );
		}
		sprintf( buf+strlen(buf), "\n\r\n\r@@c" );
		send_to_char( buf, ch );
		if ( ch->build_vnum == i )
		{
			int j;
			sprintf(buf,"\n\r@@aTick: @@c%d (%d Seconds)\n\r", build_table[i].tick, build_table[i].tick * (PULSE_ROOMS / PULSE_PER_SECOND) );
			send_to_char(buf,ch);
			send_to_char( "@@aDesc:@@N\n\r", ch );
			send_to_char( build_table[i].desc, ch );
			sprintf(buf,"\n\r\n\r" );
			for (j=0;j<11;j++ )
				sprintf(buf+strlen(buf),"@@aV%-2d:@@c %-5d%s", j,build_table[i].value[j],((j+1)%5==0)?"\n\r":"  ");
			send_to_char(buf,ch);
			send_to_char( "\n\r\n\r@@aHelp:@@c\n\r", ch );
		}
	}
	else
		send_to_char(buf,ch);
	send_to_char( build_help_table[i].help, ch );
	if (build_table[i].military == FALSE) send_to_char("\n\r@@yThe building has no internal defenses.\n\r", ch );
	if ( IS_SET(ch->config,CONFIG_CLIENT) ) {
		send_to_char("\1",ch);
		SET_BIT(ch->config,CONFIG_COLOR);
	}
	else
		send_to_char( "\n\r@@N", ch );
	return;
}
void do_ammo( CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *weapon;
	char buf[MSL];

	if (  ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_L ) ) == NULL || weapon->item_type != ITEM_WEAPON )
        {
              	if (  ( weapon = get_eq_char( ch, WEAR_HOLD_HAND_R ) ) == NULL || weapon->item_type != ITEM_WEAPON  )
		{
			send_to_char( "You are not holding a weapon!\n\r", ch );
			return;
		}
	}
	sprintf( buf, "You have %d ammo (out of %d).\n\r", weapon->value[0], weapon->value[1] );
	send_to_char(buf,ch);
	return;
}
void do_vehicle_status( CHAR_DATA *ch, char *argument )
{
	VEHICLE_DATA *vhc = ch->in_vehicle;
	char buf[MSL];

	if ( !vhc )
	{
		send_to_char( "You are not in a vehicle.\n\r", ch );
		return;
	}
	sprintf( buf, "Vehicle Status:\n\rHealth: %d/%d, Fuel: %d/%d, Ammo: %d/%d\n\r", vhc->hit, vhc->max_hit, vhc->fuel, vhc->max_fuel, vhc->ammo, vhc->max_ammo );
	send_to_char(buf,ch);
	return;
}

void display_details_old( CHAR_DATA * viewer, CHAR_DATA *ch ){	char buf[MSL];	char c_buf[MSL];	c_buf[0] = '\0';	if ( ch->desc && ch->desc->out_compress )		sprintf(c_buf,"%d",(ch->desc->compressing==85)?1:2);	sprintf( buf, "@@g%11s%-29s\n\r", ch->name, ch->pcdata->title );	sprintf( buf+strlen(buf), "@@dPlay Time: @@c%5d Hours @@a(@@c%d Total@@a) (%sMCCP%s support@@a)@@N\n\r", my_get_hours( ch, FALSE ), my_get_hours( ch, TRUE ), !ch->desc ? "Unknown" : ch->desc->out_compress ? "" : "@@eNO ", c_buf);	sprintf( buf+strlen(buf), "@@eHP: @@W%-5d/%-5d      @@aQPs: @@W%d\n\r", ch->hit, ch->max_hit, ch->quest_points );	sprintf( buf+strlen(buf), "@@dClass: @@c%s\n\r", class_table[ch->class].name );	sprintf( buf+strlen(buf), "@@dAlliance: @@c%s\n\r", (ch->pcdata->alliance == -1) ? "None" : alliance_table[ch->pcdata->alliance].name );	if ( get_trust( ch ) != ch->level )      		sprintf( buf+strlen(buf), "@@aYou are trusted at level @@W%2d@@a.@@N\n\r", get_trust(ch) );    	send_to_char( buf, viewer );    	sprintf( buf, "@@ePK Statistics\n\r" );    	send_to_char( buf, viewer );    	sprintf( buf, "@@dKills: @@W%d (%d)     ", ch->pcdata->pkills, ch->pcdata->tpkills );    	send_to_char( buf, viewer );    	sprintf( buf, "@@dDeaths: @@W%d     ", ch->pcdata->deaths );    	send_to_char( buf, viewer );    	sprintf( buf, "@@dBuildings: @@W%d (%d)\n\r", ch->pcdata->bkills, ch->pcdata->tbkills );    	send_to_char( buf, viewer );    	sprintf( buf, "@@dExperience Points: @@W%d     \n\r", ch->pcdata->experience );    	send_to_char( buf, viewer );    	send_to_char( "@@eMinigame Statistics@@N\n\r", viewer );   	sprintf( buf, "@@cPaintball: @@dHits: @@W%d   @@dLosses: @@W%d\n\r", ch->pcdata->pbhits, ch->pcdata->pbdeaths );    	send_to_char( buf, viewer );   	sprintf( buf, "@@cNUKEM:     @@dWins: @@W%d\n\r", ch->pcdata->nukemwins );    	send_to_char( buf, viewer );    	sprintf( buf, "@@W%d@@d/@@W%d @@ditems @@a<->@@W%6.2f@@d/@@W%d @@dweight\n\r@@N",	ch->carry_number, can_carry_n(ch), ch->carry_weight, can_carry_w(ch) );    	send_to_char( buf, viewer );    	if ( IS_SET(ch->effect,EFFECT_BLIND) )		send_to_char( "Affected by: Blindness\n\r", viewer );	if ( IS_SET(ch->effect,EFFECT_CONFUSE) )		send_to_char( "Affected by: Confusion\n\r", viewer );	if ( IS_SET(ch->effect,EFFECT_BARIN) )		send_to_char( "Affected by: Bar Invisibility\n\r", viewer );    	if ( IS_SET(ch->effect,EFFECT_SLOW) )		send_to_char( "Affected by: Slow\n\r", viewer );    	if ( IS_SET(ch->effect,EFFECT_ENCRYPTION) )		send_to_char( "Affected by: Encryption\n\r", viewer );	return;}
