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

   char *plays = "GGE.... SPA.... HMI.... MCF.... DBA.V.. GZU.... SBU.... HST.... MTO....";

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
   char *plays = "GGE.... SPA.... HMI.... MCF.... DC?.V.. GMI.... SBO.... HMU.... MMR.... DHIT... GZU.... SCO.... HMI.... MMI.... DD1T... GLI.... SFR.... HFL.... MZU.... DS?.... GBR.... SCO.... HGE.... MLI.... DC?T... GPR.... SAM.... HZU.... MBR.... DC?T... GBR.... SNS.... HGE.... MPR.... DC?T.V. GMU.... SAM.... HMI.... MBR.... DC?T.M. GMI.... SNS.... HZU.... MMU.... DC?T.M. GFL.... SAM.... HLI.... MMI.... DHIT... GGE.... SNS.... HBR.... MFL.... DD1T.M. GZU.... SAM.... HPR.... MGE.... DC?T.M. GGE.... SNS.... HBR.... MZU.... DC?T.M. GMI.... SAM.... HMU.... MGE.... DC?.VM. GZU.... SNS.... HMI.... MMI.... DC?T.M. GLI.... SAM.... HFL.... MZU.... DHIT.M. GBR.... SNS.... HGE.... MLI.... DC?T.M. GPR.... SHA.... HGO.... MNU.... DD1T.M. GVE.... SCO.... HVE.... MST.... DSRT.M. GVI.... SST.... HVI.... MZU.... DC?T.V. GMU.... SCO.... HVE.... MGO.... DCFT.M. GZA.... SPA.... HVI.... MVE.... DTOT.M. GVI.... SCFT... HMU.... MVI.... DBAT.M. GHA.... STOT... HZA.... MVE.... DS?..M. GCO.... SSRT... HVI.... MVI.... DD1.... GST.... SBAT... HHA.... MMU.... DS?..M. GZU.... SBA.... HCO.... MZA.... DC?.V.. GGO.... STO.... HST.... MVI.... DC?T... GVE.... SSR.... HZU.... MHA.... DHIT... GVI.... SBA.... HGO.... MCO.... DC?T... GVE.... SLS.... HVE.... MST.... DD1T... GVI.... SCA.... HVI.... MZU.... DS?.... GMU.... SGR.... HVE.... MGO.... DS?..V. GZA.... SMA.... HVI.... MVE.... DC?T.M. GSJ.... SBO.... HPR.... MGO.... DC?T.M. GZA.... STO.... HVI.... MVE.... DHIT.M. GSJ.... SSR.... HVE.... MGO.... DD1T.M. GZA.... SSN.... HGO.... MVE.... DTPT... GSJ.... SMA.... HVE.... MGO.... DC?T... GZA.... SGR.... HGO.... MVE.... DC?.VM. GSJ.... SMA.... HVE.... MGO.... DC?T.M. GZA.... SSN.... HGO.... MVE.... DC?T.M. GSJ.... SMA.... HVE.... MGO.... DHIT.M. GZA.... SGR.... HGO.... MVE.... DD1T.M. GSJ.... SMA.... HVE.... MGO.... DC?T.M. GZA.... SSN.... HGO.... MVE.... DC?T.V. GSJ.... SMA.... HVE.... MGO.... DC?T.M. GZA.... SGR.... HGO.... MVE.... DC?T.M. GSJ.... SMA.... HVE.... MGO.... DHIT.M. GZA.... SSN.... HGO.... MVE.... DC?T.M. GSJ.... SMA.... HVE.... MGO.... DD1T.M. GZA.... SGR.... HGO.... MVE.... DC?T.M. GSZ.... SCA.... HGE.... MBD.... DC?.VM. GJM.... SLS.... HFL.... MVI.... DSOT.M. GSZ.... SBA.... HRO.... MPR.... DC?T.M. GSOT... SSR.... HNP.... MBD.... DVAT.M. GVATD.. SSN.... HRO.... MVI.... DS?..M. GVA.... SLS.... HGE.... MPR.... DD1..M. GSA.... SBA.... HPA.... MNU.... DS?..V. GVA.... SSR.... HCF.... MPR.... DCNT... GSA.... SSN.... HPA.... MBD.... DC?T.M. GSO.... SLS.... HFR.... MVI.... DHIT... GBC.... SBA.... HNU.... MPR.... DSOT... GCNT... SSR.... HMU.... MBD.... DD1T... GCN.... SSN.... HNU.... MVI.... DSJT... GGA.... SLS.... HFR.... MPR.... DVA.V.. GCN.... SBA.... HNU.... MNU.... DC?T.M. GBS.... SSR.... HMU.... MLI.... DHIT.M. GVR.... SBO.... HVE.... MMU.... DS?..M. GSOT... SMA.... HSZ.... MZA.... DD1.... GVAV... SSR.... HJM.... MSJT... DS?.... GSJ.... SBO.... HBE.... MVA.... DC?T... GVA.... SNA.... HVR.... MSJ.... DC?T.M. GSJ.... SLE.... HCN.... MVA.... DHIT.M. GVA.... SMR.... HBS.... MSJ.... DC?T... GSJ.... STO.... HVR.... MVA.... DC?T... GVA.... SMR.... HSA.... MSJ.... DC?T... GSJ.... SLE.... HSO.... MVA.... DC?T.M. GVA.... SMR.... HSA.... MSJ.... DC?.VM. GSJ.... SGE.... HSO.... MVA.... DHIT.M. GVA.... SST.... HBC.... MSJ.... DD1T.M. GSJ.... SPA.... HCN.... MVA.... DC?T.M. GVA.... SBU.... HGA.... MSJ.... DC?T.M. GSJ.... SPA.... HCN.... MVA.... DC?T.M. GVA.... SCF.... HSZ.... MSJ.... DC?T.V. GSJ.... SPA.... HSO.... MVA.... DHIT.M. GVA.... SBU.... HVA.... MSJ.... DC?T.M. GSJ.... SPA.... HSJ.... MVA.... DD1T.M. GVA.... SCF.... HVA.... MSJ.... DC?T.M. GSJ.... SPA.... HAT.... MBE.... DC?T.M. GBE.... SCF.... HVA.... MSZ.... DC?T.M. GSA.... STO.... HAT.... MKL.... DC?.VM. GVA.... SBA.... HVA.... MSZ.... DC?T.M. GAT.... STO.... HAT.... MBE.... DC?T.M. GVA.... SBA.... HVA.... MSZ.... DC?T.M. GAT.... STO.... HAT.... MKL.... DC?T.M. GVA.... SBA.... HVA.... MSZ.... DCOT.M. GAT.... STO.... HAT.... MBE.... DC?T.V. GVA.... SBA.... HVA.... MSZ.... DC?T.M. GAT.... STO.... HAT.... MKL.... DLIT.M. GIO.... SCF.... HIO.... MBD.... DC?T.M. GBS.... SMR.... HBS.... MKL.... DBRT.M. GVR.... SCOT... HVR.... MBD.... DPRT... GCN.... SCO.... HSO.... MSZ.... DHIT.M. GBS.... SFR.... HSJ.... MVR.... DD1.VM. GVR.... SLIT... HJM.... MSO.... DC?T... GCN.... SBRT... HBE.... MSA.... DZAT.M. GBD.... SBR.... HSJ.... MSO.... DC?T... GKL.... SPRTT.. HJM.... MSJ.... DC?T... GBD.... SBE.... HBE.... MJM.... DC?T... GZAT...";

   PlayerMessage msgs[1850];
   int i = 0;
   for(i = 0; i < 1850; i++) strcpy(msgs[i], "");

   gameState = newHunterView(plays,msgs);

   int length = 0;
   LocationID *connLoc = whereCanTheyGo(gameState, &length, whoAmI(gameState), 1, 1, 1);   
   assert(connLoc != NULL);

   printf("Round number: %d\n", giveMeTheRound(gameState));
   printf("Current Location: %s\n\n", idToName(whereIs(gameState, whoAmI(gameState))));

   for(i = 0; i < length; i++) printf("%s\n", idToName(connLoc[i]));
   

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

