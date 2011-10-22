// Include for Space Module for Assault 4.0

#include "globals.h"

// Define the parameters of Space
#define MAX_SPACEX 500
#define MAX_SPACEY 500
#define MAX_SPACEZ 500

#define TRADE_VNUM	 	22

#define ROCKS			23
#define SILK			24
#define TITANIUM		25
#define FOOD			27

#define ASTEROID_VNUM		801

// Space.c Function Protocals
void move_space(CHAR_DATA *ch, int x, int y, int z);
void do_enterspace(CHAR_DATA *ch, char *argument);
void do_space_look(CHAR_DATA *ch);


typedef struct {
	int planet;
	char ocupants[MSL];   //Comma seperated list of occupants
	int trader;
} SPACE_DATA;

typedef struct {
	char name[20];   // Planet/Suns name
	int zval;        // Z_what to their orbit Z_SPACE_EARTH = Earth orbit
	int spacex;      // x coord in space (where they can enter the orbit)
	int spacey;      // y coord in space
	int spacez;      // z coord in space
	bool sun;        // Is it a sun?
	} PLANET_DATA;
	
typedef struct {
    int id;         // ID number of the blackhole.
    int destx;      // Destination Coord X
    int desty;      // Destination Coord y
    int destz;      // Destination Coord z
    int x;          // Coord X
    int y;          // Coord y
    int z;          // Coord z
    } BLACKHOLE_DATA;
    
typedef struct {
    int x;          // Coord x
    int y;          // Coord Y
    int z;          // Coord z
    int objvnum;    // Mined OBJ VNUM
    int quantity;   // How many in this astroid? (destroyed @ 0)
    } ASTROID_DATA;
    
typedef struct {
    int id;             // ID num of trading post
    int trading;        // Object traded (vnum)
    int sell;           // What does it sell at?
    int buy;          // Buys at this ratio
    int max;            // How much can it hold?
    int current;        // How much does it currently have?
    } TRADING_POST_DATA;

