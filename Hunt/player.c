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

   char *plays = "GGE.... SPA.... HMI.... MCF.... DKL.V.. GST.... SBO.... HNP.... MPA.... DCDT... GBU.... SNA.... HRO.... MCF.... DHIT... GAM.... SBB.... HNP.... MPA.... DGAT... GCO.... SNA.... HRO.... MGE.... DCNT... GLI.... SBB.... HGE.... MCF.... DBCT... GFR.... SNA.... HCF.... MPA.... DKLT.V. GST.... SBB.... HPA.... MBO.... DCDT.M. GFR.... SNA.... HBO.... MNA.... DHIT.M. GNU.... SBB.... HNA.... MBB.... DGAT.M. GPR.... SNA.... HBB.... MNA.... DCNT.M. GBR.... SBB.... HAO.... MBB.... DBCT.M. GHA.... SAO.... HBB.... MAO.... DBET.M. GBR.... SBB.... HAO.... MBB.... DKL.VM. GFR.... SAO.... HBB.... MAO.... DCDT.M. GBR.... SBB.... HAO.... MBB.... DHIT.M. GHA.... SAO.... HBB.... MAO.... DGAT.M. GBR.... SBB.... HAO.... MBB.... DBCT.M. GFR.... SAO.... HBB.... MAO.... DBET.M. GBR.... SBB.... HAO.... MBB.... DKLT.V. GHA.... SAO.... HBB.... MAO.... DCDT.M. GBR.... SBB.... HAO.... MBB.... DHIT.M. GFR.... SAO.... HBB.... MAO.... DGAT.M. GBR.... SBB.... HAO.... MBB.... DBCT.M. GHA.... SAO.... HBB.... MAO.... DBET.M. GBR.... SBB.... HAO.... MBB.... DKLT.M. GFR.... SAO.... HBB.... MAO.... DCD.VM. GBR.... SBB.... HAO.... MBB.... DHIT.M. GHA.... SAO.... HBB.... MAO.... DGAT.M. GBR.... SBB.... HAO.... MBB.... DBCT.M. GFR.... SAO.... HBB.... MAO.... DBET.M. GBR.... SBB.... HAO.... MBB.... DKLT.M. GLI.... SSN.... HNS.... MSN.... DCDT.V. GCO.... SBB.... HAO.... MSR.... DHIT.M. GPA.... SSN.... HNS.... MBA.... DGAT.M. GBO.... SSR.... HAO.... MBO.... DCNT.M. GCF.... SBA.... HNS.... MBA.... DVRT.M. GGE.... SBO.... HAO.... MSR.... DSOT.M. GZU.... SBA.... HNS.... MBA.... DBCT.M. GMI.... SSR.... HAO.... MBO.... DKL.VM. GMR.... SBA.... HNS.... MBA.... DCDT.M. GZU.... SBO.... HAO.... MSR.... DGAT.M. GST.... SBA.... HNS.... MBA.... DHIT.M. GBU.... SSR.... HAO.... MBO.... DCNT.M. GCO.... SBA.... HNS.... MBA.... DVRT.M. GBU.... SBO.... HAO.... MSR.... DSOT.V. GST.... SBA.... HNS.... MBA.... DBCT.M. GBU.... SSR.... HAO.... MBO.... DGAT.M. GCO.... SBA.... HCA.... MAL.... DCDT.M. GHA.... SMS.... HGR.... MMA.... DKLT.M. GBR.... SCG.... HAL.... MLS.... DBET.M. GCO.... STS.... HBA.... MMA.... DSJT.M. GHA.... SMS.... HMS.... MAL.... DSO.VM. GCO.... SCG.... HCG.... MMA.... DBCT.M. GLI.... STS.... HTS.... MLS.... DGAT.M. GBU.... SMS.... HMS.... MMA.... DCDT.M. GAM.... SCG.... HCG.... MAL.... DKLT.M. GBU.... STS.... HTS.... MMA.... DBDT.M. GBO.... SMS.... HMS.... MLS.... DSZT.V. GAL.... SCG.... HCG.... MMA.... DBET.M. GMA.... STS.... HTS.... MAL.... DBCT.M. GLS.... SMS.... HMS.... MMA.... DGAT.M. GMA.... SCG.... HCG.... MLS.... DCDT.M. GAL.... STS.... HTS.... MSR.... DKLT.M. GSR.... SRO.... HRO.... MTO.... DSZT.M. GTO.... SBI.... HGO.... MBA.... DZA.VM. GBA.... SRO.... HFL.... MTO.... DBDT.M. GLS.... STS.... HVE.... MBA.... DD4T.M. GSN.... SRO.... HGO.... MLS.... DCDT.M. GSR.... SBI.... HMR.... MSN.... DKLT.M. GLS.... SRO.... HTO.... MSR.... DBET.M. GBO.... STS.... HBA.... MLS.... DHIT.V. GBB.... SRO.... HTO.... MBO.... DSZT.M. GSN.... SBI.... HBA.... MBB.... DZAT.M. GAL.... SRO.... HTO.... MSN.... DBDT.M. GLS.... STS.... HBA.... MAL.... DKLT.M. GCA.... SMS.... HMS.... MMS.... DCDT.M. GGR.... SCG.... HCG.... MCG.... DGAT.M. GAL.... STS.... HTS.... MTS.... DBC.VM. GMS.... SMS.... HMS.... MMS.... DHIT.M. GCG.... SCG.... HCG.... MCG.... DBET.M. GTS.... STS.... HTS.... MTS.... DSOT.M. GMS.... SMS.... HMS.... MMS.... DVAT.M. GCG.... SCG.... HCG.... MCG.... DSAT.M. GTS.... STS.... HTS.... MTS.... DD3T.V. GMS.... SMS.... HMS.... MMS.... DSJT.M. GCG.... SCG.... HCG.... MCG.... DBET.M. GTS.... STS.... HTS.... MTS.... DKLT.M. GMS.... SMS.... HIO.... MIO.... DCDT.M. GBA.... SBA.... HBS.... MBS.... DHIT.M. GMA.... STO.... HVR.... MVR.... DGAT.M. GSR.... SBO.... HSO.... MBS.... DD4.VM. GBO.... SNA.... HSJ.... MVR.... DSZT.M. GNA.... SBO.... HSO.... MSO.... DZAT.M. GBO.... SPA.... HSA.... MSA.... DMUT.M. GPA.... SGE.... HVA.... MIO.... DVET.M. GGE.... SMI.... HIO.... MBS.... DHIT.M. GMI.... SZU.... HBS.... MVR.... DAS..V. GZU.... SGO.... HVR.... MSO....";



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
 
