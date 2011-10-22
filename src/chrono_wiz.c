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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
/* For forks etc. */
#include <unistd.h>
#include <fcntl.h>

#include "ack.h"
#include "tables.h"

RELEVEL_DATA * HasRelevel( char * strName )
{
   RELEVEL_DATA * pRelevel = NULL;

   /* Scan through the linked list */
   for( pRelevel = rlvldata.pRelevelList; pRelevel != NULL; pRelevel = pRelevel->pNext ) {
      if( str_cmp( pRelevel->strName, strName ) ) continue;

      /* Return the data */
      return pRelevel;
   }

   /* We didn't find what we was looking for, so return NULL (nothing) */
   return NULL;
}

void do_saverelevel( void )
{
   RELEVEL_DATA * pRelevel = NULL;
   FILE * fp = NULL;

   /* Open the file for writing */
   if( ( fp = fopen( RELEVEL_FILE, "w" ) ) == NULL ) {
      //logf( "do_saverelevel: Failed to open file for writing!" );
      return;
   }
   else {
      /* Write the linked list to the file */
      for( pRelevel = rlvldata.pRelevelList; pRelevel != NULL; pRelevel = pRelevel->pNext ) {
         fprintf( fp, "%s", "#RELEVEL\n" );
         fprintf( fp, "Name         %s~\n", pRelevel->strName  );
         fprintf( fp, "Level        %d\n",  pRelevel->iLevel   );
         fprintf( fp, "%s", "End\n\n" );
      }
      fprintf( fp, "%s", "#END\n" );

      /* Close the file */
      fclose( fp );
   }
   return;
}

void do_readrelevel( FILE * fp, RELEVEL_DATA * pRelevel )
{
   const char * word = NULL;
   bool fMatch;

   for( ; ; ) {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch( UPPER( word[0] ) ) {
         case 'E':
            if( !str_cmp( word, "End" ) ) {
               return;
            }
            break;
         case 'L':
            KEY( "Level",     pRelevel->iLevel,       fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name",      pRelevel->strName,      fread_string( fp ) );
            break;
      }
      if( !fMatch ) {
         //logf( "do_readrelevel: no match: %s", word );
      }
   }
}

void do_loadrelevel( void )
{
   RELEVEL_DATA * pRelevel = NULL;
   FILE * fp = NULL;

   /* Open the file for reading */
   if( ( fp = fopen( RELEVEL_FILE, "r" ) ) == NULL ) {
      return;
   }

   /* Enter a loop to read all its contents */
   for( ; ; ) {
      char * strWord = NULL;

      strWord = fread_word( fp );
      if( !str_cmp( strWord, "#RELEVEL" ) ) {

         /* Allocate memory, I use calloc as it clears the memory allocated */
         if( ( pRelevel = (RELEVEL_DATA *)calloc( 1, sizeof( RELEVEL_DATA ) ) ) == NULL ) {
            //logf( "do_loadrelevel: unable to allocate memory, aborting program!" );
            abort();
         }

         /* Read in the data */
         do_readrelevel( fp, pRelevel );

         /* Add the relevel data into the list */
         pRelevel->pNext = rlvldata.pRelevelList;
         rlvldata.pRelevelList = pRelevel;
         continue;
      }
      else if( !str_cmp( strWord, "#END" ) ) {
         break;
      }
      else {
         //logf( "do_loadrelevel: bad section: '%s'.", strWord );
         continue;
      }
   }
   fclose( fp );
   return;
}

void do_setrelevel( CHAR_DATA * dch, char * arg )
{
   char strBuffer[MAX_STRING_LENGTH] = { '\0' };
   char strArg1[MAX_INPUT_LENGTH] = { '\0' };
   char strArg2[MAX_INPUT_LENGTH] = { '\0' };
   char strArg3[MAX_INPUT_LENGTH] = { '\0' };
   RELEVEL_DATA * pRelevel = NULL;
   CHAR_DATA * xch = NULL;

   /* Store the Arguments */
   arg = one_argument( arg, strArg1 );
   arg = one_argument( arg, strArg2 );
   arg = one_argument( arg, strArg3 );

   if( strArg1[0] == '\0' || strArg2[0] == '\0' ) {
      send_to_char( "Syntax: SetRelevel <Grant/Revoke> <Character> (Level)\n\n\r", dch );
      send_to_char( "Level is only required when granting access!\n\r", dch );
      return;
   }

   if( !str_cmp( strArg1, "grant" ) || !str_cmp( strArg1, "give" ) ) {
      /* Make sure they have the level parameter */
      if( strArg3[0] == '\0' ) {
         sprintf( strBuffer, "You must supply a level! (1-%d)\n\r", MAX_LEVEL );
         send_to_char( strBuffer, dch );
         return;
      }
      /* Level Check */
      if( atoi( strArg3 ) < 1 || atoi( strArg3 ) > MAX_LEVEL ) {
         sprintf( strBuffer, "The level must be between 1 and %d!\n\r", MAX_LEVEL );
         send_to_char( strBuffer, dch );
         return;
      }

      /* Find the Character to grant relevel access to */
      if( ( xch = get_char_world( dch, strArg2 ) ) == NULL ) {
         send_to_char( "They are not online.\n\r", dch );
         return;
      }

      /* Don't allow NPC's */
      if( IS_NPC( xch ) ) {
         send_to_char( "You can not add NPCs to relevel.\n\r", dch );
         return;
      }

      /* Does xch already have access to relevel? */
      if( ( pRelevel = HasRelevel( xch->name ) ) != NULL ) {
         send_to_char( "They already have access to relevel!\n\r", dch );
         return;
      }

      /* Allocate memory */
      if( ( pRelevel = (RELEVEL_DATA *)calloc( 1, sizeof( RELEVEL_DATA ) ) ) == NULL ) {
         send_to_char( "Unable to allocate memory, aborting!\n\r", dch );
         //logf( "do_loadrelevel: unable to allocate memory!" );
         return;
      }

      /* Add the relevel data into the list */
      pRelevel->pNext = rlvldata.pRelevelList;
      rlvldata.pRelevelList = pRelevel;

      /* Set the data */
      pRelevel->strName = strdup( xch->name );
      pRelevel->iLevel = atoi( strArg3 );

      /* Inform the other character that they now have access to relevel */
      send_to_char( "You now have access to relevel.\n\r", xch );

      /* Inform the admin that it was succesful */
      sprintf( strBuffer, "%s was successfully added to relevel.\n\r", xch->name );
      send_to_char( strBuffer, dch );

      /* Save the modified relevel list */
      do_saverelevel();
   }
   else if( !str_cmp( strArg1, "revoke" ) || !str_cmp( strArg1, "take" ) ) {
      if( ( pRelevel = HasRelevel( strArg2 ) ) == NULL ) {
         send_to_char( "They do not have access to relevel!\n\r", dch );
         return;
      }

      /* Remove the data from the linked list */
      if( pRelevel == rlvldata.pRelevelList ) {
         rlvldata.pRelevelList = pRelevel->pNext;
      }
      else {
         RELEVEL_DATA * pPrev = NULL;

         for( pPrev = rlvldata.pRelevelList; pPrev != NULL; pPrev = pPrev->pNext ) {
            if( pPrev->pNext == pRelevel ) {
               pPrev->pNext = pRelevel->pNext;
               break;
            }
         }
         if( pPrev == NULL ) {
            //logf( "do_setrelevel: data not found." );
            return;
         }
      }
      /* Clear the pointer */
      pRelevel->pNext = NULL;

      /* Inform the admin of the Success */
      sprintf( strBuffer, "You have revoked relevel access from %s!\n\r", pRelevel->strName );
      send_to_char( strBuffer, dch );

      /* Free the used memory */
      free( pRelevel->strName );
      pRelevel->iLevel = 0;

      /* Save the modified relevel list */
      do_saverelevel();
   }
   else {
      do_setrelevel( dch, "" );
   }
   return;
}

void do_relevel( CHAR_DATA * dch, char * arg )
{
   RELEVEL_DATA * pRelevel = NULL;

   /* Make sure they have relevel access */
   if( ( pRelevel = HasRelevel( dch->name ) ) == NULL ) {
      send_to_char( "Access Denied!\n\r", dch );
      return;
   }

   /* Send them a Message and set their level */
   send_to_char( "Relevel Complete!\n\r", dch );
   dch->level = pRelevel->iLevel;
   dch->trust = pRelevel->iLevel;
   return;
}


void do_delevel( CHAR_DATA * dch, char * arg )
{
   /* Send them a Message and set their level */
   send_to_char( "Delevel Complete!\n\r", dch );
   dch->level = 1;
   dch->trust = 0;
   return;
}



