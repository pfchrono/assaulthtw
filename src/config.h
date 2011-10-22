/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Zepp 1998    *
 *    _/_/_/_/      _/          _/  _/             Version #: 4.3          *
 *   _/      _/      _/_/_/     _/    _/     _/                            *
 *                                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include "buildings.h"

#define DEC_CONFIG_H		1

/*
 *  Your mud info here :) Zen
 */

#define mudnamecolor  "@@aAssault: 3.0@@N"
#define mudnamenocolor  "Assault: 3.0"
#define UPGRADE_REVISION 16
#define WEBSITE	"http://www.assaultmud.com"
#define admin  "Demortes"
#define admin_email "demortes@assaultmud.com"
/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             2048
#define MAX_STRING_LENGTH        12288
#define MSL			MAX_STRING_LENGTH
#define MAX_INPUT_LENGTH          1280
#define MAX_AREAS                 2000
#define MAX_VNUM 		  32767

#define BOOT_DB_ABORT_THRESHOLD			  25
#define RUNNING_ABORT_THRESHOLD			  10
#define ALARM_FREQUENCY               20

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */

#define DAMAGE_REDUCTION_PER_LEVEL	3
#define MAX_CASH			500000
#define STARTING_MONEY			75000
#define STARTING_HP		    1000
#define MAX_BUILDING_LEVEL	    10
#define MAX_color		    10  /* eg look, prompt, shout */
#define MAX_ANSI		    32  /* eg red, black, etc */
#define MAX_ALIASES		     5
#define MAX_ALLIANCE		     25
#define MAX_IGNORES		     3
#define MAX_CLASS		    12
#define MAX_OBJECT_VALUES	     15
#define MAX_BUILDON		     6
#define MAX_SKILL		     14
#define MAX_LEVEL		     90
#define LEVEL_HERO                 (80)
#define LEVEL_GUIDE                  79
#define LEVEL_IMMORTAL               80
#define MAX_QUOTE		200
#define MAX_MAPS		1500
#define MIN_LOAD_OBJ		1000
#define MAX_LOAD_OBJ		2000
#define BORDER_SIZE		0
#define MAX_HQS_ALLOWED		5


#define TERRAIN_NONE		0
#define TERRAIN_BALANCED	1
#define TERRAIN_PAINTBALL	2
#define TERRAIN_UG		3

#define Z_GROUND	0
#define Z_AIR		1
#define Z_PAINTBALL	2
#define Z_UNDER		3
#define Z_SPACE_EARTH   4
#define Z_SUPER_SPACE   5
#define Z_MAX		6

#define MAX_AMMO	45
#define DAMAGE_ENVIRO   -3
#define DAMAGE_GENERAL	-1
#define DAMAGE_BULLETS	1
#define DAMAGE_BLAST	2
#define DAMAGE_ACID	3
#define DAMAGE_FLAME	4
#define DAMAGE_LASER	5
#define DAMAGE_PAINT	6
#define DAMAGE_SOUND	7
#define DAMAGE_PSYCHIC	8
#define DAMAGE_EMP	9
#define DAMAGE_WATER	10
#define DAMAGE_MINING   11

#define VEHICLE_JEEP			0
#define VEHICLE_TANK			1
#define VEHICLE_AIRCRAFT		2
#define VEHICLE_TRUCK			3
#define VEHICLE_CHINOOK			4
#define VEHICLE_BOMBER			5
#define VEHICLE_MISSILE_SHIP		6
#define VEHICLE_LASER			7
#define VEHICLE_RECON			8
#define VEHICLE_CARRIER			9
#define VEHICLE_BIO_FLOATER		10
#define VEHICLE_CREEPER			11
#define VEHICLE_BOAT			12
#define VEHICLE_GUNSHIP			13
#define VEHICLE_DRAGON_TYRANT           14
#define VEHICLE_SPACESHIP		15
#define MAX_VEHICLE			19

#define VEHICLE_FIRE_RESISTANT		BIT_1
#define VEHICLE_EXPLOSIVE		BIT_2
#define VEHICLE_FLOATS			BIT_3
#define VEHICLE_EATS_FUEL		BIT_4
#define VEHICLE_REGEN			BIT_5
#define VEHICLE_SWIM			BIT_6
#define VEHICLE_SPACE			BIT_7

#define EFFECT_BLIND		BIT_1
#define EFFECT_BARIN		BIT_2
#define EFFECT_SLOW		BIT_3
#define EFFECT_RESOURCEFUL	BIT_4
#define EFFECT_BOMBER		BIT_5
#define EFFECT_CONFUSE		BIT_6
#define EFFECT_POSTAL		BIT_7
#define EFFECT_ENCRYPTION	BIT_8
#define EFFECT_VISION		BIT_9
#define EFFECT_RUNNING		BIT_10
#define EFFECT_TRACER		BIT_11
#define EFFECT_WULFSKIN		BIT_12
#define EFFECT_DRUNK		BIT_13
#define EFFECT_EVADE		BIT_14
#define EFFECT2_CONSTITUTION	1
#define EFFECT2_SPEED		2
#define EFFECT2_SMART		3
#define EFFECT2_REGENERATION	4
#define EFFECT2_RAGE		5
#define POISON_ANTHRAX		1
#define POISON_TRANQ		2
#define POISON_LYE		3

#define SUIT_NONE			0
#define SUIT_WARP			1
#define SUIT_JUMP			2

#define WEAPON_BLINDING			BIT_4
#define WEAPON_HITS_AIR			BIT_6
#define WEAPON_CONFUSING		BIT_7
#define WEAPON_ALCOHOL			BIT_8

#define INST_NONE			0
#define INST_ONLINE			BIT_1
#define INST_NOCHECK			BIT_2
#define INST_DEPLEATED_URANIUM		BIT_8
#define INST_ANTIVIRUS			BIT_10
#define INST_FIREWALL			BIT_11
#define INST_STUN_GUN			BIT_12
#define INST_SPOOF			BIT_14
#define INST_QP				BIT_15
#define INST_ORGANIC_CORE		BIT_16
#define INST_VIRAL_ENHANCER		BIT_17
#define INST_ACID_DEFENSE		BIT_19
#define INST_ALIEN_HIDES		BIT_20

#define TURRET_MAGNET			1
#define TURRET_PROJECTOR		2

#define PIT_BORDER_X			(MAX_MAPS-10)
#define PIT_BORDER_Y			(MAX_MAPS-10)

/*
 * Extended bitvector stuff.
 */
#ifndef INT_BITS
// #define INT_BITS                   32
#define INT_BITS                   64
#endif
#define	XBM                        31
#define	RSV                         5	/* log2( INT_BITS )     */
#define XBI                         2	/* int's in a bitvector */
#define	MAX_BITS                  ( XBI * INT_BITS )

#define TYPE_UNDEFINED	-1

#define C_TYPE_MISC     0
#define C_TYPE_COMM     1
#define C_TYPE_CONFIG   2
#define C_TYPE_INFO     3
#define C_TYPE_ACTION   4
#define C_TYPE_OBJECT   5
#define C_TYPE_ALLI	6
#define C_TYPE_IMM	7

#define C_SHOW_NEVER    -1
#define C_SHOW_ALWAYS     0
#define C_SHOW_SKILL      1

#define PULSE_PER_SECOND     8
#define PULSE_VIOLENCE    (  2 * PULSE_PER_SECOND )
#define PULSE_OBJFUN      (  4 * PULSE_PER_SECOND )
#define PULSE_TICK        ( 60 * PULSE_PER_SECOND )
#define PULSE_ROOMS       (  1 * PULSE_PER_SECOND )
#define PULSE_AREA        ( 80 * PULSE_PER_SECOND )
#define PULSE_BACKUP      ( 1800 * PULSE_PER_SECOND )
#define PULSE_TIME        ( 3600 * PULSE_PER_SECOND )
#define PULSE_REMAP       ( 7200 * PULSE_PER_SECOND )
#define PULSE_OBJECTS     ( PULSE_PER_SECOND * 5 )
#define PULSE_BOMB        ( PULSE_PER_SECOND )
#define PULSE_QUEST	  ( 15 * PULSE_PER_SECOND )
#define PULSE_SPEC        ( PULSE_PER_SECOND * 10 )
#define PULSE_REBOOT	  ( PULSE_PER_SECOND * 3600 * 6 )

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_TELEPORTER	32698
#define OBJ_VNUM_LOCATOR	32678
#define OBJ_VNUM_BLUEPRINTS	32693
#define OBJ_VNUM_ACID_TURRET_U	32694
#define OBJ_VNUM_FIRE_TURRET_U	32696
#define OBJ_VNUM_LASER_TOWER_U	32697
#define OBJ_VNUM_IDUP		32695
#define OBJ_VNUM_ACID_SPRAY	1010
#define OBJ_VNUM_CANNONBALL	32686
#define OBJ_VNUM_GRANADE	1012
#define OBJ_VNUM_SUIT_WARP	1013
#define OBJ_VNUM_SUIT_JUMP	1144
#define OBJ_VNUM_SCUD		32687
#define OBJ_VNUM_FLASH_GRENADE	1030
#define OBJ_VNUM_REFLECTOR	1031
#define OBJ_VNUM_COOKIE_LAUNCH	1032
#define OBJ_VNUM_COOKIE_AMMO	1033
#define OBJ_VNUM_MAIN_BOARD	1039
#define OBJ_VNUM_ATOM_BOMB	32692
#define OBJ_VNUM_CORPSE		32691
#define OBJ_VNUM_FLAG		32690
#define OBJ_VNUM_DART_BOARD	32689
#define OBJ_VNUM_QP_TOKEN	32688
#define OBJ_VNUM_ELEMENT	32682
#define OBJ_VNUM_CHINESE_TEA	32685
#define OBJ_VNUM_SMOKE_BOMB	32684
#define OBJ_VNUM_POISON_TEA	32683
#define OBJ_VNUM_CONTAINER	1077
#define OBJ_VNUM_LEAD_BOMB	999
#define OBJ_VNUM_BLACK_POWDER	998
#define OBJ_VNUM_BIO_GRENADE	997
#define OBJ_VNUM_BURN_GRENADE	996
#define OBJ_VNUM_SAFEHOUSE_INST	995
#define OBJ_VNUM_RESOURCE_PURE	994
#define OBJ_VNUM_DEPLEATED_URA	993
#define OBJ_VNUM_PULSE_NEUTRAL	992
#define OBJ_VNUM_COMPUTER	10
#define OBJ_VNUM_ALLI_BOARD	10000
#define OBJ_VNUM_PAINT_GUN	32679
#define OBJ_VNUM_STUN_GUN	989
#define OBJ_VNUM_PROCESSOR_UP	987
#define OBJ_VNUM_DISK_V		1138
#define OBJ_VNUM_DISK_C		1139
#define OBJ_VNUM_DISK_F		1140
#define OBJ_VNUM_DISK_S		1141
#define OBJ_VNUM_DISK_P		1145
#define OBJ_VNUM_SHOCK_BOMB	1150
#define OBJ_VNUM_MEDAL		32676
#define OBJ_VNUM_TOOLKIT	32669
#define OBJ_VNUM_DIRTY_BOMB	986
#define OBJ_VNUM_SCAFFOLD	32667
#define OBJ_VNUM_BROKEN_BONE	509
#define OBJ_VNUM_AIR2GROUNDBOMB	32686


#define MAX_PAWN		10
#define MAX_QUESTS		20
#define MAX_QP_OBJ		100
#define MIN_QUEST_OBJ		100
#define MAX_QUEST_OBJ		115

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_WMAP                3
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_JAIL		      1

/*
 * God Levels
 */
#define L_GOD           MAX_LEVEL
#define L_SUP           L_GOD - 1
#define L_DEI           L_SUP - 1
#define L_ANG           L_DEI - 1
#define L_HER           L_ANG - 1

/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3
#define SKY_MAX			    4

#define MOON_DOWN	0
#define MOON_RISE	1
#define MOON_LOW	2
#define MOON_PEAK	3
#define MOON_FALL	4
#define MOON_SET	5


#define MOON_NEW	0
#define MOON_WAX_CRE	1
#define MOON_WAX_HALF	2
#define MOON_WAX_GIB	3
#define MOON_FULL	4
#define MOON_WAN_GIB	5
#define MOON_WAN_HALF	6
#define MOON_WAN_CRE	7

/*
 * More Time and weather stuff. - Wyn
 */

/* Overall time */
#define HOURS_PER_DAY   24
#define DAYS_PER_WEEK    7
#define DAYS_PER_MONTH  30
#define MONTHS_PER_YEAR	10
#define DAYS_PER_YEAR   (DAYS_PER_MONTH * MONTHS_PER_YEAR)

/* PaB: Hours of the day */
/* Notes: Night is half of the day, so sunrise is 1/4 of the way
 * through the day, and sunset 3/4 of the day.  
 */
#define HOUR_DAY_BEGIN		(HOURS_PER_DAY / 4 - 1)
#define HOUR_SUNRISE		(HOUR_DAY_BEGIN + 1)
#define HOUR_NOON           (HOURS_PER_DAY / 2)
#define HOUR_SUNSET			((HOURS_PER_DAY / 4) * 3 + 1)
#define HOUR_NIGHT_BEGIN	(HOUR_SUNSET + 1)
#define HOUR_MIDNIGHT		HOURS_PER_DAY

/* PaB: Seasons */
/* Notes: Each season will be arbitrarily set at 1/4 of the year.
 */
#define SEASON_WINTER		0
#define SEASON_SPRING		1
#define SEASON_SUMMER		2
#define SEASON_FALL			3
#define SEASON_MAX         4

/*
 * Connected state for a channel.
 */

/* These values referenced by users command, BTW */

#define CON_PLAYING                      0
#define CON_GET_NAME                     -1
#define CON_GET_OLD_PASSWORD             -2
#define CON_CONFIRM_NEW_NAME             -3
#define CON_GET_NEW_PASSWORD             -4
#define CON_CONFIRM_NEW_PASSWORD         -5
#define CON_READ_MOTD                   -10
#define CON_FINISHED		 	    -12
#define CON_MENU		                -13
#define CON_COPYOVER_RECOVER		    -14
						/* For Hotreboot */
#define CON_QUITTING		          -15
#define CON_RECONNECTING                -16
#define CON_GET_NEW_CLASS		-17
#define CON_GET_ANSI			-18
#define CON_GET_RECREATION		-19
#define CON_GET_SEX			-20
#define CON_GET_BONUS			-21
#define CON_GET_NEW_PLANET		-22
#define CON_READ_RULES			-23
#define CON_GET_RESET			-24
#define CON_GET_NEW_MODE		-25
#define CON_SETTING_STATS		      1

/*
 * TO types for act.
 */
#define TO_ROOM             0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_CHAR             3


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_NO_MOB             BIT_3
#define ROOM_INDOORS            BIT_4


/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3


/*
 * Sector types.
 * Used in #ROOMS.
 */

#define SECT_NULL                     0
#define SECT_MAX                   18

#define SECT_ROCK			1
#define SECT_SAND			2
#define SECT_HILLS			3
#define SECT_MOUNTAIN			4
#define SECT_WATER			5
#define SECT_SNOW			6
#define SECT_FIELD			7
#define SECT_FOREST			8
#define SECT_LAVA			9
#define SECT_BURNED			10
#define SECT_SNOW_BLIZZARD		11
#define SECT_ASH			12
#define SECT_AIR			13
#define SECT_UNDERGROUND		14
#define SECT_ICE			15
#define SECT_OCEAN			16
#define SECT_SPACE			17

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE               -1
#define WEAR_HEAD               0
#define WEAR_EYES		1
#define WEAR_FACE               2
#define WEAR_SHOULDERS          3
#define WEAR_ARMS               4
#define WEAR_HOLD_HAND_L        5
#define WEAR_HOLD_HAND_R        6
#define WEAR_WAIST              7
#define WEAR_BODY               8
#define WEAR_LEGS               9
#define WEAR_FEET               10
#define MAX_WEAR                11

/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SNEAKING                  6
#define POS_STANDING                  7
#define POS_WRITING                   8	
#define POS_BUILDING                  9
#define POS_HACKING		     10
#define POS_NUKEM		     11
#define POS_CUSTOMIZE		     12
/*
 *  Configuration Bits for players
 */

#define CONFIG_SMALLMAP		BIT_1
#define CONFIG_EXITS		BIT_3
#define CONFIG_NOCOLORS		BIT_4
#define CONFIG_NOFOLLOW		BIT_5
#define CONFIG_COMBINE          BIT_6
#define CONFIG_PROMPT           BIT_7
#define CONFIG_TELNET_GA        BIT_8
#define CONFIG_COLOR            BIT_9
#define CONFIG_COMPRESS		BIT_10
#define CONFIG_FULL_ANSI        BIT_11
#define CONFIG_MXP		BIT_12
#define CONFIG_BLIND		BIT_13
#define CONFIG_PUBMAIL		BIT_14
#define CONFIG_LARGEMAP		BIT_15
#define CONFIG_MINCOLORS	BIT_16
#define CONFIG_SOUND		BIT_17
#define CONFIG_ECHAN		BIT_18
#define CONFIG_NOMAP		BIT_19
#define CONFIG_CLIENT		BIT_20
#define CONFIG_BRIEF		BIT_21
#define CONFIG_IMAGE		BIT_22
#define CONFIG_COMPRESS2	BIT_23
#define CONFIG_TINYMAP		BIT_24
#define CONFIG_NOBLACK		BIT_25
#define CONFIG_WHITEBG		BIT_26
#define CONFIG_INVERSE		BIT_27
#define CONFIG_NOLEGEND		BIT_28

/*
 * ACT bits for players.
 */
#define PFLAG_AFK			BIT_1
#define PFLAG_SNOOP			BIT_2
#define PFLAG_PRACTICE			BIT_3
#define PFLAG_ALIAS			BIT_4
#define PFLAG_RAD_SIL			BIT_5
#define PFLAG_HELPING			BIT_6
#define PLR_PDELETER			BIT_7
#define PLR_HOLYLIGHT                 	BIT_13
#define PLR_WIZINVIS                  	BIT_14
#define PLR_BUILDER                   	BIT_15        /* Is able to use the OLC */
#define PLR_SILENCE                   	BIT_16
#define PLR_NO_EMOTE                  	BIT_17
#define PLR_NO_TELL              	BIT_19
#define PLR_LOG                  	BIT_20
#define PLR_DENY                	BIT_21
#define PLR_FREEZE              	BIT_22
#define PLR_TAG				BIT_27 /* For Tag */
#define PLR_ASS				BIT_28
#define PLR_INCOG			BIT_31

/*
 * Obsolete bits.
 */
#if 0
#define PLR_AUCTION                   4 /* Obsolete     */
#define PLR_CHAT                    256 /* Obsolete     */
#define PLR_NO_SHOUT             131072 /* Obsolete     */
#endif


/*
 * Channel bits.
 */
#define CHANNEL_ALLIANCE        BIT_1
#define CHANNEL_GOSSIP          BIT_2
#define CHANNEL_MUSIC           BIT_3
#define CHANNEL_IMMTALK         BIT_4
#define CHANNEL_NEWBIE          BIT_5
#define CHANNEL_QUESTION        BIT_6
#define CHANNEL_SHOUT           BIT_7
#define CHANNEL_POLITICS        BIT_8
#define CHANNEL_FLAME           BIT_9
#define CHANNEL_ZZZ             BIT_10
#define CHANNEL_RACE            BIT_11
#define CHANNEL_CLAN            BIT_12
#define CHANNEL_NOTIFY          BIT_13
#define CHANNEL_INFO            BIT_14
#define CHANNEL_LOG		BIT_15
#define CHANNEL_CREATOR		BIT_16
#define CHANNEL_ALLALLI		BIT_17
#define CHANNEL_ALLRACE		BIT_18
#define CHANNEL_HERMIT		BIT_19		/* Turns off ALL channels */
#define CHANNEL_BEEP		BIT_20
#define CHANNEL_FAMILY		BIT_21
#define CHANNEL_DIPLOMAT	BIT_22
#define CHANNEL_CRUSADE		BIT_23
#define CHANNEL_REMORTTALK	BIT_24
#define CHANNEL_HOWL            BIT_25
#define CHANNEL_ADEPT	        BIT_26
#define CHANNEL_OOC             BIT_27
#define CHANNEL_QUEST           BIT_28
#define CHANNEL_CODE		BIT_29
#define CHANNEL_GAME		BIT_30

#define CHANNEL2_AFFIL		BIT_1
#define CHANNEL2_ALLAFFIL	BIT_2
#define CHANNEL2_PKOK		BIT_3
#define CHANNEL2_GUIDE		BIT_4
#define CHANNEL2_LANG		BIT_5

  /* NOTE 32 is the last allowable channel ZEN */

/* Monitor channels - for imms to select what mud-based info they receive */
#define MONITOR_CONNECT		BIT_1
#define MONITOR_AREA_UPDATE	BIT_2
#define MONITOR_AREA_BUGS	BIT_3
#define MONITOR_AREA_SAVING	BIT_4
#define MONITOR_GEN_IMM		BIT_5
#define MONITOR_GEN_MORT	BIT_6
#define MONITOR_COMBAT		BIT_7
#define MONITOR_BUILD		BIT_8
#define MONITOR_OBJ		BIT_9
#define MONITOR_ROOM		BIT_10
#define MONITOR_BAD		BIT_11
#define MONITOR_DEBUG   	BIT_12
#define MONITOR_SYSTEM  	BIT_13
#define MONITOR_LDEBUG		BIT_14
#define MONITOR_FAKE		BIT_15


/* build bits for OLC -S- */
#define ACT_BUILD_NOWT                0         /* not doing anything   */
#define ACT_BUILD_REDIT               1         /* editing rooms        */
#define ACT_BUILD_OEDIT               2         /* editing objects      */
#define ACT_BUILD_BEDIT               3         /* editing buildings    */
#define ACT_BUILD_MPEDIT	      4		/* editing mprogs	*/
#define ACT_BUILD_CEDIT               5         /* editing the clan table */
#define NO_USE			   -999		/* this table entry can	*/
						/* NOT be used, except  */
						/* by a Creator		*/

#define SEX_MALE                      1
#define SEX_FEMALE                    2


/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                    1
#define ITEM_AMMO                     2
#define ITEM_BOMB                     3
#define ITEM_BLUEPRINT	              4
#define ITEM_WEAPON                   5
#define ITEM_SUIT		      6
#define ITEM_MEDPACK		      7
#define ITEM_DRONE		      8
#define ITEM_ARMOR                    9
#define ITEM_TELEPORTER		     10
#define ITEM_INSTALLATION            11
#define ITEM_POTION		     12
#define ITEM_FLAG		     13
#define ITEM_DART_BOARD		     14
#define ITEM_CONTAINER		     16
#define ITEM_WEAPON_UP		     17
#define ITEM_PIECE		     18
#define ITEM_COMPUTER		     19
#define ITEM_LOCATOR		     20
#define ITEM_SKILL_UP		     21
#define ITEM_DISK		     23
#define ITEM_TRASH		     24
#define ITEM_ASTEROID		     25
#define ITEM_BACKUP_DISK	     26
#define ITEM_BOARD		     27
#define ITEM_VEHICLE_UP		     28
#define ITEM_TOOLKIT		     29
#define ITEM_SCAFFOLD		     30
#define ITEM_ORE		     31
#define ITEM_BIOTUNNEL		     32
#define ITEM_BATTERY		     33
#define ITEM_TOKEN		     35

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_NUCLEAR			1
#define ITEM_STICKY			2
#define ITEM_NOQP			4
#define ITEM_PAWN			8
#define ITEM_INVIS			32
#define ITEM_NODROP			128
#define ITEM_NOREMOVE			4096
#define ITEM_INVENTORY			8192
#define ITEM_NOSAVE			16384   /* For "quest" items :) */
#define ITEM_RARE			1048576
#define ITEM_NOLOOT			4194304
#define ITEM_UNIQUE			16777216

#define CLASS_ENGINEER		0
#define CLASS_DARKOP		1
#define CLASS_DRIVER		2
#define CLASS_SNIPER		3
#define CLASS_HACKER		4
#define CLASS_SPRINTER		5
#define CLASS_MEDIC		6
#define CLASS_SPY		7
#define CLASS_PSYCHIC		8
#define CLASS_GAMBLER		9
#define CLASS_UGLY		10
#define CLASS_WEAPONMASTER      11
#define CLASS_PHASER		12
/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_WEAR_NONE          BIT_0
#define ITEM_WEAR_HEAD          BIT_1
#define ITEM_WEAR_FACE          BIT_2
#define ITEM_WEAR_SHOULDERS     BIT_3
#define ITEM_WEAR_ARMS          BIT_4
#define ITEM_WEAR_HANDS         BIT_5
#define ITEM_WEAR_HOLD_HAND     BIT_6
#define ITEM_WEAR_WAIST         BIT_7
#define ITEM_WEAR_BODY          BIT_8
#define ITEM_WEAR_LEGS          BIT_9
#define ITEM_WEAR_FEET          BIT_10
#define ITEM_TAKE               BIT_24
#define ITEM_WEAR_EYES		BIT_12
/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_CLASS                   7
#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
#define APPLY_HEIGHT                 10
#define APPLY_WEIGHT                 11
#define APPLY_MANA                   12
#define APPLY_HIT                    13
#define APPLY_MOVE                   14
#define APPLY_GOLD                   15
#define APPLY_EXP                    16
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVING_PARA            20
#define APPLY_SAVING_ROD             21
#define APPLY_SAVING_PETRI           22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24



/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                1
#define CONT_PICKPROOF                2
#define CONT_CLOSED                   4
#define CONT_LOCKED                   8

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define LOG_DIR      	""	    	/* Log files                 */
#define PLAYER_DIR      ""              /* Player files                 */
#define SITE_DIR	""
#define NULL_FILE       "proto.are"     /* To reserve one stream        */
#endif

#if defined(MSDOS)
#define LOG_DIR      	""	    	/* Log files                 */
#define PLAYER_DIR      ""              /* Player files                 */
#define SITE_DIR	""
#define NULL_FILE       "nul"           /* To reserve one stream        */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"    /* Player files                 */
#define LOG_DIR      	"../log/"    	/* Log files                 */
#define SITE_DIR	"~/public_html/assault/who/" /* For online who list	*/
#define NULL_FILE       "/dev/null"     /* To reserve one stream        */
#endif

#if defined(linux)
#define PLAYER_DIR      "../player/"    /* Player files                 */
#define LOG_DIR      	"../log/"    	/* Log files                 */
#define SITE_DIR	"~/public_html/assault/who/"
#define NULL_FILE       "/dev/null"     /* To reserve one stream        */
#endif

#define AREA_LIST       "area.lst"      /* List of areas                */

#define MAIL_DIR        "/home4/demortes/mail/"
#define DATA_DIR        "../data/"
#define BUG_DIR         "../reports/"
#define LOG_DIR         "../log/"
#define BUG_FILE       BUG_DIR  "bugs.txt"      /* For 'bug' and bug( )         */
#define IDEA_FILE      BUG_DIR   "ideas.txt"     /* For 'idea'                   */
#define TYPO_FILE      BUG_DIR   "typos.txt"     /* For 'typo'                   */
#define HELP_FILE      BUG_DIR   "helps.txt"     /* For missing help files      */
#define SNOOP_FILE     LOG_DIR  "watch.txt"     /* For players who need to be watched   */
/* FIXME: boards.txt in here????? */
#define SHUTDOWN_FILE   BUG_DIR  "shutdown.txt"  /* For 'shutdown'               */
#define DISABLED_FILE	DATA_DIR "disabled.txt"  /* disabled commands - Wyn */
#define PLAYER_LIST_FILE DATA_DIR "playerlist.txt" /* Player list */

#define OBJECTS_FILE	DATA_DIR "objects.lst"
#define OBJECTS_FEST_FILE	DATA_DIR "objects.fst"
#define OBJECTS_BACKUP_FILE	DATA_DIR "objects.bak"
#define QUOTE_FILE	DATA_DIR "quotes.txt"
#define BANS_FILE	DATA_DIR "bans.lst"
#define BRANDS_FILE	DATA_DIR "brands.lst"
#define MAP_FILE	DATA_DIR "map.txt"
#define BUILDING_TABLE_FILE	DATA_DIR "building_table.txt"
#define BUILDING_FILE	DATA_DIR "buildings.txt"
#define BUILDING_FEST_FILE	DATA_DIR "buildings.fst"
#define BUILDING_BACKUP_FILE	DATA_DIR "buildings.bak"
#define VEHICLE_FILE	DATA_DIR  "vehicles.txt"
#define VEHICLE_FEST_FILE	DATA_DIR  "vehicles.fst"
#define VEHICLE_BACKUP_FILE	DATA_DIR  "vehicles.bak"
#define QUEST_FILE	DATA_DIR "quests.txt"
#define MAP_BACKUP_FILE	DATA_DIR  "map.bak"
#define SCORE_FILE	DATA_DIR "scores.txt"
#define RANK_FILE	DATA_DIR "ranks.txt"
#define SYSDAT_FILE	DATA_DIR "system.dat"
#define ALLIANCES_FILE	DATA_DIR "alliances.txt"
#define PALLIANCES_FILE DATA_DIR "palliances.txt"
#define PLANET_FILE	DATA_DIR "planets.txt"
#define MAX_PLAYERS_FILE DATA_DIR "players.txt"
#define MULTIPLAY_FILE	DATA_DIR "multiplay.txt"
#define SPACE_OBJ	DATA_DIR "space_obj.txt"


/* Other Stuff - Flar */
#define COPYOVER_FILE	"COPYOVER.TXT"	/* Temp data file used for copyover */
#define EXE_FILE		"../src/ack"	/* The one that runs the ACK! */

/* stuff for Quests */
#define QROOM_VNUM "299"
#define CLAN_MONEY 1039

// MySQL Settings
#define MYSQL_HOST "localhost"
#define MYSQL_USER "assaultmud"
#define MYSQL_DB   "assaultmud"
#define MYSQL_PWD  "18mother!"
