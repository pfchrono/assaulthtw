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
#include <stdio.h>
#include <time.h>
#include "ack.h"

struct quest_type quest_table[MAX_QUESTS];
struct map_type map_table;
struct alliance_type alliance_table [MAX_ALLIANCE];
struct palliance_type palliance_table [MAX_ALLIANCE];
struct score_type	score_table[100];
struct rank_type	rank_table[30];
struct multiplay_type	multiplay_table[30];
struct build_type	build_table[MAX_POSSIBLE_BUILDING];
struct build_help_type	build_help_table[MAX_POSSIBLE_BUILDING];
OBJ_DATA *pawn_obj[MAX_PAWN];

const int building_limits[MAX_HQS_ALLOWED+1] =
{
	0,200,240,280,320,360
};
const char * dam_type[]=
{
"Unknown",
"Bullets",
"Blast",
"Acid",
"Flame",
"Laser",
"Paint",
"Sound",
"Psychic",
"EMP",
"Water"
};

const struct clip_type	clip_table[MAX_AMMO]	 =
{
//	Name			        Dam	BuildDam		Hit	   Explode	  Speed  	Type   
    { "9mm Ammo", 		    30, 	2, 			53, 	FALSE,   	4,	  DAMAGE_BULLETS }, //Pistol
    { "17mm-Remington Ammo",40, 	2, 			65, 	FALSE,  	6,    DAMAGE_BULLETS }, //Magnum
    { "45mm-Colt Ammo", 	40, 	2, 			60, 	FALSE, 	    4,	  DAMAGE_BULLETS }, //Colt 
    { "Rockets", 		    230, 	90, 		35, 	TRUE, 	    40,	  DAMAGE_BLAST }, //Bazookas
    { "Acid", 			    90, 	30, 		45, 	FALSE,  	2,	  DAMAGE_ACID }, //Acid Sprayer
    { "Rockets", 		    100, 	30, 		75, 	TRUE,   	30,	  DAMAGE_BLAST }, //Hmmm... Tanks maybe?
    { "Railgun", 		    20, 	7,		75, 	FALSE, 	    2,	  DAMAGE_BULLETS }, //Vehicles
    { "Flame", 			    40, 	15, 		35, 	FALSE,  	2,	  DAMAGE_FLAME }, //Flamethrower
    { "Explosive Cookie", 	    40, 	50, 		65, 	TRUE, 	    20,	  DAMAGE_GENERAL }, //Cookie launcher *sigh*
    { "308 Winchester Ammo",	    300, 	1, 			85, 	FALSE,   	100,  DAMAGE_BULLETS }, //Sniper Rifle
    { "Bomb", 			    200,	600,		90,	    TRUE,   	0,	  DAMAGE_BLAST }, //Stealth Floaters
    { "BBQ Fire", 		    40, 	20, 		55, 	FALSE,  	0,	  DAMAGE_FLAME }, //BBQer
    { "Laser", 			    30, 	20, 		90, 	FALSE,  	5,	  DAMAGE_LASER }, //Laser Guns
    { "Laser", 			    30, 	20, 		70, 	FALSE,  	3,	  DAMAGE_LASER }, //Laser Machine-Guns
    { "20x100 Hispano", 	400, 	1, 			99, 	FALSE,  	70,	  DAMAGE_BULLETS }, //Sniper Rifle (LS)
    { "7.62mm Ammo", 		30, 	20, 		50, 	FALSE,  	2,	  DAMAGE_BULLETS }, //AK47
    { "Paint", 			    1, 	    0, 			50, 	FALSE,  	6,	  DAMAGE_PAINT }, //Paintball
    { "Laser Beam", 		70, 	15, 		50, 	FALSE,  	4,	  DAMAGE_LASER }, //Laser Tank
    { "Fire Rocket", 		150, 	75, 			20, 	TRUE,   	100,  DAMAGE_FLAME }, //Fire Bazooka
    { "Shotgun Shell", 		70, 	7, 			20, 	FALSE,  	30,	  DAMAGE_BULLETS }, //Shotgun
    { "Sound Blast",        50,     0,         100,     FALSE,      10,   DAMAGE_SOUND }, 
    { "EMP Charge", 		0, 	    0, 			 0, 	FALSE,  	10,	  DAMAGE_EMP }, //EMP guns


    { "lvl 1 Laser Cannon", 50, 	0, 			50, 	FALSE, 	    3,	  DAMAGE_LASER }, //Space
    { "lvl 2 Laser Cannon", 75, 	0, 			60, 	FALSE, 	    4,	  DAMAGE_LASER }, //Space
    { "lvl 3 Laser Cannon", 100, 	0, 			70, 	FALSE, 	    5,	  DAMAGE_LASER }, //Space
    { "lvl 1 Plasma Cannon",75, 	0, 			60, 	FALSE, 	    3,	  DAMAGE_LASER }, //Space
    { "lvl 2 Plasma Cannon",90, 	0, 			65, 	FALSE, 	    3,	  DAMAGE_LASER }, //Space
    { "lvl 3 Plasma Cannon",110, 	0, 			75, 	FALSE, 	    4,	  DAMAGE_LASER }, //Space
    { "lvl 1 Torpedo", 		40, 	0, 			70, 	TRUE, 	    6,	  DAMAGE_BLAST }, //Space
    { "lvl 2 Torpedo", 		45, 	0, 			75, 	TRUE, 	    6,	  DAMAGE_BLAST }, //Space
    { "lvl 3 Torpedo", 		60, 	0, 			80, 	TRUE, 	    7,	  DAMAGE_BLAST }, //Space
    { "lvl 1 Ion Cannon", 	90, 	0, 			75, 	FALSE, 	    4,	  DAMAGE_LASER }, //Space
    { "lvl 2 Ion Cannon", 	110, 	0, 			85, 	FALSE, 	    4,	  DAMAGE_LASER }, //Space
    { "lvl 3 Ion Cannon", 	200, 	0, 			95, 	FALSE, 	    5,	  DAMAGE_LASER }, //Space

    { "Psychic Blast", 		100, 	0, 			100, 	FALSE, 	    16,	  DAMAGE_PSYCHIC }, //Psychic Blaster
    { "Ion Blaster", 		8000, 	0, 			100, 	FALSE, 	    5,	  DAMAGE_LASER }, //Space
    { "5.56mm Ammo", 		40, 	10, 		70, 	FALSE, 	    3,	  DAMAGE_BULLETS }, //M16
    { "Beanbag",		    0,	    0,			100,	FALSE,      8,	  DAMAGE_BULLETS }, //Beanbag Rifle
    { ".50 Action Express Ammo",50,	4,		    30,	    FALSE,	    6,	  DAMAGE_BULLETS }, //Desert Eagle
    { "4.7mm Caseless Ammo",35,	    5,		    85,	    FALSE,	    4,	  DAMAGE_BULLETS }, //G11
    { "Bio Spray",		    0,	    0,		    0,	    FALSE,	    0,	  DAMAGE_ACID }, //Bio floater
    { "Devastation Ray",    50,    500,         90,     FALSE,      80,   DAMAGE_BLAST }, //Devastation Platform
    { "Seeker Bullets",     200,     10,        100,    TRUE,       1,    DAMAGE_BULLETS }, // Hunter Mechs
    { "Musket Ball",        150,     45,        45,     TRUE,       1,    DAMAGE_BLAST }, // Hunter Mechs

    { "Mining Laser",             0,    0,                       0,     FALSE,      1,    DAMAGE_MINING }, //Mining laser for space
};

const struct bonus_type bonus_table[] =
{
    { "Laptop", "Comes with a fully charged battery. Make some extra cash with it, or use it to destroy!", 998 },
    { "Dr. Norton", "A 1-time complete virus-clear installation for the entire base.", 1173 },
    { "Sniper Rifle", "You wanna defend yourself? They say attacking is the best defense.", 1122 },
    { "Teleporter", "Takes you back home in a 50-room range.", 32594 },
    { "Security Kit", "Basic internal defense for a building of your choice.", 1181 },
    { "Nuke"     , "A bomb that leaves radiation.", 1028 },
    { "", "", -1 },
};

const   struct color_type      color_table    [MAX_color]            =
{
   { "say",              0 },
   { "tell",             1 },
   { "gossip",           2 },
   { "music",            4 },
   { "flame",            5 },
   { "info",             6 },
   { "stats",            7 },
   { "objects",          8 },
   { "ooc",              9 },
};

const   struct  ansi_type       ansi_table      [MAX_ANSI]              =
{
   { "gray",            "\033[0;37m",    0,     'g',    7 },
   { "red",             "\033[0;31m",    1,     'R',    7 },
   { "green",           "\033[0;32m",    2,     'G',    7 },
   { "brown",           "\033[0;33m",    3,     'b',    7 },
   { "blue",            "\033[0;34m",    4,     'B',    7 },
   { "magenta",         "\033[0;35m",    5,     'm',    7 },
   { "cyan",            "\033[0;36m",    6,     'c',    7 },
   { "black",           "\033[0;30m",    7,     'k',    7 }, /* was 0;33 */ 
   { "yellow",          "\033[1;33m",    8,     'y',    7 },
   { "white",           "\033[1;37m",    9,     'W',    7 },
//   { "normal",          "\033[0;0m\033[0;40m",      10,	'N',	13 },
   { "color reset",           "\033[0;0m\033[0;49m",      10,	'N',	13 },
   { "purple",          "\033[1;35m",   11,     'p',    7 },
   { "dark_grey",       "\033[1;30m",   12,     'd',    7 },
   { "light_blue",      "\033[1;34m",   13,     'l',    7 },
   { "light_green",     "\033[1;32m",   14,     'r',    7 },
   { "light_cyan",      "\033[1;36m",   15,     'a',    7 },
   { "light_red",       "\033[1;31m",   16,     'e',    7 },
   { "bold",            "\033[1m",    17,       'x',    4 },
   { "flashing", 	"\033[7m",    18, 	'f', 	4 },
   { "inverse",         "\033[7m",    19,       'i',    4 },
   { "back_red",             "\033[0;41m",	 20,	'2',	7 },
   { "back_green",           "\033[0;42m",	 21,	'3',	7 },
   { "back_yellow",           "\033[0;43m",	 22,	'4',	7 },
   { "back_blue",            "\033[0;44m",	 23,	'1',	7 },
   { "back_magenta",         "\033[0;45m",	 24,	'5',	7 },
   { "back_cyan",            "\033[0;46m",	 25,	'6',	7 },
   { "back_black",           "\033[0;40m",	 26,	'0',	7 }, 
   { "back_white",           "\033[1;47m",    27,	'7',	7 },
   { "@@CR@@Ca@@Cn@@Cd@@Co@@Cm@@N",		"",			28,	'C',	7 },
   { "blue",            "\033[0;34m",    29,     'J',    7 },
   { "red",             "\033[0;31m",    30,     'Q',    7 },
   { "gray",            "\033[0;37m",    31,     'o',    7 },
};

/*
 * Class table.
 */
const   struct  class_type      class_table     [MAX_CLASS]     =
{
    { "Eng",  "Engineer", "Buildings can start at L2.", TRUE,0 },
    { "Dop",  "Dark-Op", "22%% Invisibility on map, can evade guard turrets.", FALSE, 0 },
    { "Drv",  "Driver", "Faster driving in vehicles, vehicles regenerate.", FALSE, 2 },
    { "Snp",  "Sniper", "+1 Range to all weapons.", FALSE, 5 },
    { "Hak",  "Hacker", "Protect yourself against hackers and viruses easily, and bypass firewalls to upload them yourself.", FALSE, 0 },
    { "Spr",  "Sprinter", "Run quickly in fight mode.", FALSE, 10 },
    { "Med",  "Medic", "Increased form of healing using the Heal command with no need for medpacks.", FALSE, 50 },
    { "Spy",  "Spy", "Receive additional building stats when using the INFO command.", FALSE, 10 },
    { "Psy",  "Psychic", "Take less damage from psychic attacks.", FALSE, 5 },
    { "Gmb",  "Gambler", "Win or lose money every minute.", FALSE, 5 },
    { "Ugl",  "Ugly", "People enter combat lag just by seeing you!", FALSE, 10 },
    { "Wem",  "Weaponmaster", "Increased damage with guns!", FALSE, 20 }
};

const struct wildmap_type wildmap_table[SECT_MAX] =
{
//  Color  Symbol   ID    Name   		Heat	Speed	Fuel

  { "@@k", "++", "00", "Wall", 			0, 	    0, 	    0 	},
  { "@@d", "_-", "01", "Rock", 			0, 	    -4, 	1 	},
  { "@@y", "._", "02", "Sand", 			6, 	    4, 	    3 	},
  { "@@b", "--", "03", "Hills", 		-2, 	2, 	    3 	},
  { "@@b", "~\\","04", "Mountain", 		-3, 	4, 	    4 	},
  { "@@l", "~~", "05", "Water", 		-4, 	1, 	    1 	},
  { "@@W", "~~", "06", "Snow", 			-6, 	3, 	    2	},
  { "@@r", "..", "07", "Field", 		1, 	    -4, 	1	},
  { "@@G", "^^", "08", "Forest", 		1, 	    2, 	    1	},
  { "@@e", "~o", "09", "Lava", 			20, 	2, 	    6 	},
  { "@@d", "^v", "10", "Burned Ground", 0, 	    0, 	    1 	},
  { "@@W", "~~", "11", "Snow - Blizzard",-20, 	6, 	    4	},
  { "@@d", "~~", "12", "Ash", 			5, 	    0, 	    2	},
  { "@@a", "--", "13", "Air", 			0, 	    1, 	    1	},
  { "@@b", "..", "14", "Underground",	0, 	    0, 	    3	},
  { "@@a", "~~", "15", "Ice", 			-10, 	10, 	0	},
  { "@@J", "oo", "16", "Ocean", 		0, 	    4, 	    1 	},
  { "@@d", "oo", "17", "Space",         0,      10,     3   },
};

const struct planet_type planet_table[]  =
{//	     Name     System  Gravity            Z Value               Terrain      		Description
	{ "Earth",	    1,	    1,	        Z_GROUND,	    TERRAIN_BALANCED,	"Basic playing grid." },
	{ "Air",	    0,	    1,	        Z_AIR,		    TERRAIN_NONE,          "Air around Earth" },
	{ "Game Arena",	    0,	    3,	        Z_PAINTBALL,	    TERRAIN_PAINTBALL,	    "Paintball Arena" },
	{ "Underground",    0,      1,          Z_UNDER,	    TERRAIN_UG,		  "Earth Underground" },
        { "Earth Orbit",    0,      0,          Z_SPACE_EARTH,      TERRAIN_NONE,       "Orbit around Earth." },
	{ "Super Space",    0,      0,          Z_SUPER_SPACE,      TERRAIN_NONE, "Ships are to be put here." },
	{ NULL,		    0,	    0,	        0,		        0,			        "" },	
};

char * const vehicle_name [MAX_VEHICLE] =
{
 "Cheetah Mech Vehicle",
 "Tank Vehicle",
 "Devastation Platform Vehicle",
 "Laser Floater Vehicle",
 "Truck Vehicle",
 "Chinook Chopper Helicopter Vehicle",
 "Bomber Aircraft Vehicle",
 "Missile Ship",
 "Laser Tank Vehicle",
 "Recon Plane Vehicle",
 "Aircraft Carrier Ship",
 "Bio Floater Vehicle",
 "Creeper Vehicle",
 "Boat",
 "Gunship",
 "Hunter Mech Vehicle",
 "Power-suit Vehicle",
 "Dragon Tyrant Vehicle",
 "A basic spaceship",
};

char * const vehicle_desc [MAX_VEHICLE] =
{
 "A Cheetah Mech",
 "A Tank",
 "A Devastation Platform",
 "A Laser-Floater",
 "A worn-out Truck",
 "A Chinook",
 "A Bomber Aircraft",
 "A Missile Ship",
 "A Laser Tank",
 "A Recon Plane",
 "An Aircraft Carrier",
 "A Bio-Floater",
 "A Creeper",
 "An old Boat",
 "A small Gunship",
 "A Hunter Mech",
 "A Dragon Tyrant",
 "A Basic Spaceship",
};

char * const poison_name[] =
{
 "Nothing",
 "Anthrax",
 "Tranquilizer",
 "Lye",
};


const struct shop_type shop_table[] =
{
  // Name 		            Type		        	Building	 	Level	Cost	HP	   Fuel	    Ammo	Amtype	Range	Flags	        Desc
  { "Cheetah Mech", 	    VEHICLE_JEEP,	BUILDING_GARAGE, 	1,      1000,	500,   2000,	0,	    0,	    0,	    VEHICLE_FLOATS,	"Better than nothing." },
  { "Medium Tank", 	    VEHICLE_TANK,	BUILDING_GARAGE, 	1,	    2000,	1500,	500,	20,	    5,	    2,	    0,	            "Can deal some building damage." },
  { "Mark II Tank", 	    VEHICLE_TANK,	BUILDING_GARAGE, 	2,	    3000,	3000,  1000,	20,	    5,	    4,	    0,	            "Better armor and range than the medium tank." },
  { "Polarity Tank", 	    VEHICLE_TANK,	BUILDING_GARAGE, 	2,	    5000,	5000,  1000,	5,	    5,	    2,	    VEHICLE_FLOATS,	"Hovers over Water, low ammo/range." },
  { "Mark III Tank", 	    VEHICLE_TANK,	BUILDING_GARAGE, 	3,	    4000,	4500,  1500,	20,	    5,	    4,	    0,	            "Superior armor." },
  { "Hunter Mech",          VEHICLE_TANK,       BUILDING_GARAGE,    5,      10000,	3000,  1500,    150, 	42, 	5, 	    0,              "A Mech designed to hunt players/vehicles." },
  { "Devastation Platform", VEHICLE_TANK,       BUILDING_GARAGE,    10,     20000,  1500,  1000,    20, 	41, 	9,      VEHICLE_FLOATS, "Floats to destroy enemies. Low armor/Lowish Range. EXPENSIVE." },
  { "Chinook", 		    VEHICLE_CHINOOK,	BUILDING_AIRFIELD, 	1,	    1000,	100,   1500,	0,	    -1,	    0,	    0,	            "Can carry vehicles through air." },
  { "Bomber", 		    VEHICLE_BOMBER,	BUILDING_AIRFIELD, 	1,	    5000,	350,   2500,	2,	    10,	    3,	    0,	            "Good vs. Buildings" },
  { "Armored Bomber", 	    VEHICLE_BOMBER,	BUILDING_AIRFIELD, 	2,	    5000,	1050,  2500,	2,	    10,	    3,	    0,	            "Has increased armor." },
  { "Heavy Bomber", 	    VEHICLE_BOMBER,	BUILDING_AIRFIELD, 	2,	    5000,	350,   2500,	5,	    10,	    3,	    0,	            "Carries more bombs" },
  { "Recon Plane", 	    VEHICLE_RECON,	BUILDING_AIRFIELD,	2,	    500,	1000,  1000,	0,	    -1,	    0,	    0,	            "90% Evasion. Type LOOK GROUND to see the map." },
  { "A Dragon Tyrant",    VEHICLE_DRAGON_TYRANT,BUILDING_AIRFIELD,  10,     50000,	4000,  3000,    5,      10,	    3, 	    0,              "A beast" },
  { "Old Boat",		    VEHICLE_BOAT,	BUILDING_SHIPYARD,	1,	    400,	50,     500,	50,	    6,	    3,	    VEHICLE_SWIM,	"It will get you across the ocean, no more." },
  { "Gunship",		    VEHICLE_GUNSHIP,	BUILDING_SHIPYARD,	1,	    3000,	500,	800,	100,	6,	    4,	    VEHICLE_SWIM,	"Can do some moderate damage." },
  { "Aircraft Carrier",     VEHICLE_CARRIER,	BUILDING_SHIPYARD,	1,	    9000,	2000, 10000,	300,    6,	    3,	    VEHICLE_SWIM,	"You can land an aircraft in it." },
  { "Armored Gunship",	    VEHICLE_GUNSHIP,	BUILDING_SHIPYARD,	2,	    4000,	1250,	800,	100,	6,	    2,	    VEHICLE_SWIM,	"Has much more health than the normal Gunship." },
  { "Missile Ship",	    VEHICLE_MISSILE_SHIP,BUILDING_SHIPYARD,	3,	    8000,	700,	800,	30,	    5,	    8,	    VEHICLE_SWIM,	"Can fire missiles up to 8 rooms away!" },
  { "Battle Cruiser",       VEHICLE_GUNSHIP,    BUILDING_SHIPYARD,  10,     45000,  4000,  5000,    200,    42,     8,      VEHICLE_SWIM,   "Massive ship that can take out almost anything at a range." },
  { "A Basic Spaceship",    VEHICLE_SPACESHIP,  BUILDING_SPACEYARD, 1,      1000,	1000,  1000,    0,      -1,   	0,      0,              "Basic Spaceship" },
  { "A Fighter Spaceship",  VEHICLE_SPACESHIP,  BUILDING_SPACEYARD, 2,      2000,   750,   1000,    300,    22,     5,      0,              "Your basic space fighter." },
  { "THE Spaceship",        VEHICLE_SPACESHIP,  BUILDING_SPACEYARD, 10,     50000,  2000, 50000,    300,    30,     9,      0,              "A spacepilots dream ship..." },
  { NULL,		            -1,			        -1,			        0,	    0,	    0,	      0,    0,	    0,	    0,	    0,	            NULL }
};
