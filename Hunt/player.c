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

   char *plays = "GKL.... SBR.... HMR.... MMS.... DLS.V.. GSZ.... SLI.... HBO.... MTS.... DMAT... GBD.... SHA.... HBB.... MRO.... DALT... GSZ.... SNS.... HSN.... MNP.... DGRT... GZA.... SHA.... HMAT... MMI.... DD3T... GMU.... SCO.... HSR.... MMR.... DCAT... GZU.... SBO.... HSN.... MMS.... DHIT.V. GMR.... STO.... HSR.... MALT... DAO.... GTO.... SBO.... HBO.... MSN.... DGWT... GSR.... SMAT... HLS.... MLS.... DDUT.M. GLS.... SLS.... HAO.... MAO.... DIR.... GAO.... SAO.... HIR.... MIR....";
 

   PlayerMessage msgs[1850];

   int i = 0;
   for(i = 0; i < 1850; i++) strcpy(msgs[i], "");

   gameState = newDracView(plays,msgs);
   decideDraculaMove(gameState);



   printf("GameStat:\n");
   printf("Score: %d\n", giveMeTheScore(gameState)); 
   printf("Round: %d\n\n", giveMeTheRound(gameState));

   int player = 0;
   for(player = 0; player <= PLAYER_DRACULA; player++) {
      printf("Player %d:\n", player);
      printf("Life Points: %d\n", howHealthyIs(gameState, player));
        
      char *currLoc = location(whereIs(gameState, player));
      assert(currLoc != NULL);

      printf("Current Location: %s\n", currLoc);
      free(currLoc);

      LocationID trail[TRAIL_SIZE];
      giveMeTheTrail(gameState, player, trail);

      printf("Trail: ");
      int j = 0;
      for(j = 0; j < TRAIL_SIZE; j++) {
         char *place = location(trail[j]);
         assert(place != NULL);

         printf("%s", place);
         free(place);

         if(j + 1 < TRAIL_SIZE) printf("--->");
      }
      

      printf("\n");
      
      LocationID moves[TRAIL_SIZE];
      giveMeTheMoves(gameState, player, moves);

      printf("Moves: ");
      for(j = 0; j < TRAIL_SIZE; j++) {
         char *place = location(moves[j]);
         assert(place != NULL);
 
         printf("%s", place);
         free(place);

         if(j + 1 < TRAIL_SIZE) printf("--->");
      }

      printf("\n\n");
      
   }
 

   disposeDracView(gameState);
#else
   HunterView gameState;
   char *plays = "GGE.... SKL...."; 
   PlayerMessage msgs[1850];
   int i = 0;
   for(i = 0; i < 1850; i++) strcpy(msgs[i], "");

   gameState = newHunterView(plays,msgs);

   int length = 0;
   LocationID *sPath = shortestPath(gameState, &length, PLAYER_DR_SEWARD, KLAUSENBURG, SALONICA, 1, 1, 1);
   assert(sPath != NULL);
 
   printf("\nShortest Path:\n");
   for(i = 0; i < length; i++) printf("%s\n", idToName(sPath[i]));

   decideHunterMove(gameState);
   disposeHunterView(gameState);
#endif 
   printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
  
   char *dummy = location(CASTLE_DRACULA);
   assert(dummy != NULL);

   printf("\nDummy: %s\n", dummy);
   free(dummy);

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
    } else if(id == TELEPORT) {
       strcpy(loc, "Teleport");
    }

    return loc;
}
 
