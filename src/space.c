// Space module for Assault 4.0

#include "space.h"

PLANET_DATA planet_space[] =
    { //      Name                     zval               spacex  spacey  spacey,     sun (T/F)
    { "Earth",                       Z_SPACE_EARTH,        15,      7,       10,        FALSE  },
    // Add new planets here.
    { "INVALID",                        -1,                 0,      0,        0,        TRUE }
    };


void do_enterspace(CHAR_DATA *ch, char *argument)
{
  bool found = FALSE;
  int i =0;
  
  send_to_char("This feature not yet fully implemented\r\n", ch);
  for(i=0;!strcmp(planet_space[i].name, "INVALID");i++)
  {
	if(ch->z == planet_space[i].zval) {found = TRUE;}
  }
  
  if(!found) 
  {
    send_to_char("You are not in an orbit.\r\n", ch);
  } else {
    ch->z = Z_SUPER_SPACE;
    move_space(ch, planet_space[i].spacex, planet_space[i].spacey, planet_space[i].spacez);
  }

  return;
}

void move_space(CHAR_DATA *ch, int x, int y, int z)
{
// Set the coords on the player.
  if(ch == NULL)
    return;
  if(ch->z != Z_SUPER_SPACE)
  {
	send_to_char("You don't seem to be in \"Super Space\".", ch);
	return;
  }  
  ch->spacex = x;
  ch->spacey = y;
  ch->spacez = z;
  do_space_look(ch);
 
  return;
}

void do_space_look(CHAR_DATA *ch)
{
  int i = 0, j = 0, x, y, z;
  x = ch->spacex;
  y = ch->spacey;
  z = ch->spacez;
  
  send_to_char("*", ch);                     //Space header
  for(i=0;i < ch->pcdata->term_columns;i++)
    send_to_char("-", ch);
  send_to_char("*\r\n", ch);
  
  for(i=0;i < ch->pcdata->term_rows;i++)       // Actual map of space around them
  {
    send_to_char("|", ch);
    for(j=0;j < ch->pcdata->term_columns;j++)
    {
      send_to_char("@@d**@@n ", ch);//Display the space around them.
    }
    send_to_char("|\r\n", ch);
  }
  
  send_to_char("*", ch);                     //Space footer
  for(i=0;i < ch->pcdata->term_columns;i++)
    send_to_char("-", ch);
  send_to_char("*\r\n", ch);
  
}

