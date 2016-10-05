/**
 * Runs a player's game turn ...
 *
 * Can produce either a Hunter player or a Dracula player
 * depending on the setting of the I_AM_DRACULA #define
 *
 * This is a dummy version of the real player.c used when
 * you submit your AIs. It is provided so that you can
 * test whether your code is likely to compile ...
 *
 * Note that it used to drive both Hunter and Dracula AIs.
 * It first creates an appropriate view, and then invokes the
 * relevant decide-my-move function, which should use the 
 * registerBestPlay() function to send the move back.
 *
 * The real player.c applies a timeout, so that it will halt
 * decide-my-move after a fixed amount of time, if it doesn't
 * finish first. The last move registered will be the one
 * used by the game engine. This version of player.c won't stop
 * your decide-my-move function if it goes into an infinite
 * loop. Sort that out before you submit.
 *
 * Based on the program by David Collien, written in 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Game.h"
#include "Places.h"

#ifdef I_AM_DRACULA
#include "DracView.h"
#include "dracula.h"
#else
#include "HunterView.h"
#include "hunter.h"
#endif

// moves given by registerBestPlay are this long (including terminator)

#define MOVE_SIZE 3

static char *location(LocationID id); 

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

int main(int argc, char *argv[])
{
#ifdef I_AM_DRACULA
   DracView gameState;

   char *plays = "GGE.... SPA.... HMI.... MCF.... DSA.V.. GGE.... SGE.... HVE.... MCF.... DSOT... GST.... SST.... HMU.... MCF.... DBCT... GMI.... SST.... HZA.... MCF.... DGAT... GVE.... SST.... HMU.... MCF.... DKLT... GFL.... SLI.... HMU.... MCF.... DBDT... GVE.... SVI.... HZA.... MCF.... DSZT.V. GGO.... SVI.... HMU.... MCF.... DBET.M. GTS.... SVI.... HZA.... MCF.... DBCT.M. GGO.... SVI.... HMU.... MCF.... DSOT.M. GVE.... SVI.... HZA.... MCF.... DVRT.M. GGO.... SVI.... HMU.... MCF.... DCNT.M. GTS.... SVI.... HZA.... MCF.... DGAT.M. GGO.... SVI.... HMU.... MTO.... DCD.VM. GTS.... SVI.... HVI.... MSR.... DKLT.M. GTS.... SZA.... HVI.... MMA.... DBCT.M. GTS.... SZA.... HVE.... MLS.... DBET.M. GTS.... SZA.... HVI.... MSN.... DSOT.M. GTS.... SZA.... HZA.... MSR.... DVRT.M. GTS.... SZA.... HZA.... MMA.... DCNT.V. GTS.... SZA.... HZA.... MLS.... DGAT.M. GTS.... SZA.... HZA.... MSN.... DKLT.M. GTS.... SZA.... HZA.... MSR.... DCDT.M. GTS.... SZA.... HZA.... MMA.... DHIT.M. GTS.... SZA.... HZA.... MLS.... DD1T.M. GTS.... SZA.... HZA.... MSN.... DTP..M. GTS.... SZA.... HZA.... MSR.... DGA.VM. GTS.... SZA.... HZA.... MMA.... DKLT.M. GTS.... SZA.... HZA.... MLS....";

   
   PlayerMessage msgs[1850];

   int i = 0;
   for(i = 0; i < 1850; i++) strcpy(msgs[i], "");

   gameState = newDracView(plays,msgs);
   decideDraculaMove(gameState);

   printf("GameStat:\n");
   printf("Score: %d\n\n", giveMeTheScore(gameState)); 

   int player = 0;
   for(player = 0; player <= PLAYER_DRACULA; player++) {
      printf("Player %d:\n", player);
      printf("Life Points: %d\n", howHealthyIs(gameState, player));
      printf("Current Location: %s\n", location(whereIs(gameState, player)));
        

      LocationID trail[TRAIL_SIZE];
      giveMeTheTrail(gameState, player, trail);

      printf("Trail: ");
      int j = 0;
      for(j = 0; j < TRAIL_SIZE; j++) {
         printf("%s", location(trail[j]));

         if(j + 1 < TRAIL_SIZE) printf("--->");
      }

      printf("\n");
      
      LocationID moves[TRAIL_SIZE];
      giveMeTheMoves(gameState, player, moves);

      printf("Moves: ");
      for(j = 0; j < TRAIL_SIZE; j++) { 
         printf("%s", location(moves[j]));

         if(j + 1 < TRAIL_SIZE) printf("--->");
      }

      printf("\n\n");
      
   }
 

   disposeDracView(gameState);
#else
   HunterView gameState;
   char *plays = "GZA.... SED.... HBR....";
   PlayerMessage msgs[1850];
   int i = 0;
   for(i = 0; i < 1850; i++) strcpy(msgs[i], "");

   gameState = newHunterView(plays,msgs);

   int length = 0;
   LocationID *sPath = shortestPath(gameState, &length, PLAYER_VAN_HELSING, BERLIN, BELGRADE, 1, 1, 1);
   
   if(length > 0) {
       assert(sPath != NULL);

       printf("Shortes Path from Berlin to Belgrade:\n");
   }

   for(i = 0; i < length; i++) printf("%s\n", idToName(sPath[i]));
   

   decideHunterMove(gameState);
   disposeHunterView(gameState);
#endif 
   printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
   printf("\nDummy: %s\n", location(CASTLE_DRACULA));
   return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay (char *play, PlayerMessage message) {
   strncpy(latestPlay, play, MOVE_SIZE-1);
   latestPlay[MOVE_SIZE-1] = '\0';
     
   strncpy(latestMessage, message, MESSAGE_SIZE);
   latestMessage[MESSAGE_SIZE-1] = '\0';
}

static char *location(LocationID id) {
    char *loc = malloc(100 * sizeof(char));
    assert(loc != NULL);

    if(id >= MIN_MAP_LOCATION && id <= MAX_MAP_LOCATION) {
        strcpy(loc, idToName(id));
    } else if(id >= DOUBLE_BACK_1 && id <= DOUBLE_BACK_5) {
        switch(id) {
        case DOUBLE_BACK_1: strcpy(loc, "DOUBLE BACK 1"); break;
        case DOUBLE_BACK_2: strcpy(loc, "DOUBLE BACK 2"); break;
        case DOUBLE_BACK_3: strcpy(loc, "DOUBLE BACK 3"); break;
        case DOUBLE_BACK_4: strcpy(loc, "DOUBLE BACK 4"); break;
        case DOUBLE_BACK_5: strcpy(loc, "DOUBLE BACK 5"); break;
        }

    } else if(id == HIDE) {
       strcpy(loc, "Hide");
    } else if(id == CITY_UNKNOWN) {
       strcpy(loc, "Unknown City");
    } else if(id == SEA_UNKNOWN) {
       strcpy(loc, "Unknown Sea");
    } else if(id == UNKNOWN_LOCATION) {
       strcpy(loc, "Unknown location");
    }
 
    return loc;
}

