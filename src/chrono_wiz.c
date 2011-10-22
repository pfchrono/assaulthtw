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

