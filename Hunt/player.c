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

   char *plays = "GGE.... SPA.... HMI.... MCF.... DBD.V.. GZU.... SST.... HGE.... MTO.... DKLT... GMI.... SMI.... HZU.... MSR.... DCDT... GGE.... SZU.... HST.... MPA.... DGAT... GZU.... SST.... HZU.... MMA.... DCNT... GST.... SZU.... HFR.... MSR.... DVRT... GZU.... SFR.... HLI.... MTO.... DSOT.V. GFR.... SLI.... HNU.... MSR.... DVAT.M. GLI.... SNU.... HST.... MBO.... DSAT.M. GNU.... SST.... HMI.... MTO.... DHIT.M. GST.... SMI.... HZU.... MSR.... DD1T.M. GMI.... SZU.... HST.... MPA.... DIO..M. GZU.... SST.... HZU.... MMA.... DBS..M. GGE.... SBU.... HST.... MGR.... DCN.VM. GGO.... SAM.... HFR.... MMA.... DGAT.M. GMR.... SNS.... HNU.... MBO.... DCDT.M. GCF.... SAM.... HLI.... MCO.... DKLT.M. GMR.... SNS.... HZU.... MFR.... DBET... GZU.... SAM.... HGE.... MNU.... DSOT... GST.... SNS.... HMI.... MLI.... DSJT.V. GFR.... SAM.... HGO.... MZU.... DZAT.M. GNU.... SNS.... HMR.... MGE.... DVIT.M. GLI.... SAM.... HCF.... MMI.... DBDT.M. GZU.... SNS.... HMR.... MGO.... DSZT.M. GGE.... SAM.... HZU.... MMR.... DKLT.M. GMI.... SNS.... HST.... MCF.... DCDT.M. GGO.... SAM.... HFR.... MMR.... DGA.VM. GMR.... SNS.... HNU.... MZU.... DBCT.M. GCF.... SAM.... HLI.... MST.... DCNT.M. GMR.... SNS.... HZU.... MFR.... DVRT.M. GCF.... SEC.... HMU.... MLI.... DSOT.M. GGE.... SNS.... HZA.... MCO.... DSAT.M. GST.... SEC.... HMU.... MBR.... DVAT.V. GCO.... SNS.... HMI.... MLI.... DSJT.M. GPA.... SEC.... HVE.... MHA.... DZAT.M. GBO.... SNS.... HFL.... MNS.... DVIT.M. GSR.... SEC.... HVE.... MEC.... DD2T.M. GBA.... SNS.... HFL.... MNS.... DMUT.M. GBO.... SEC.... HRO.... MEC.... DVET.M. GSR.... SNS.... HFL.... MNS.... DAS..M. GBA.... SEC.... HVET... MEC.... DIO..M. GMS.... SNS.... HFL.... MNS.... DTS..M. GAO.... SEC.... HRO.... MEC.... DGOT.M. GCA.... SNS.... HFL.... MNS.... DMRT.M. GGR.... SEC.... HVE.... MEC.... DTOT... GCA.... SNS.... HFL.... MNS.... DSRT... GGR.... SEC.... HRO.... MHA.... DBOT... GAL.... SPL.... HFL.... MPR.... DNAT... GMA.... SLO.... HGE.... MHA.... DLET.M. GLS.... SMN.... HMRT... MCO.... DPAT... GMA.... SLV.... HCF.... MHA.... DSTT.M. GCA.... SIR.... HNAT... MPR.... DMUT.M. GGR.... SDU.... HPAT... MHA.... DMI.VM. GAL.... SIR.... HPA.... MCO.... DZUT... GBA.... SDU.... HLET... MHA.... DGET... GMA.... SIR.... HLE.... MPR.... DCFT... GLS.... SDU.... HMR.... MHA.... DNAT.M. GCA.... SIR.... HSR.... MCO.... DLET.M. GGR.... SDU.... HMA.... MHA.... DBUT.V. GAL.... SIR.... HLS.... MPR.... DSTT.M. GMA.... SLV.... HCA.... MNU.... DPAT.M. GGR.... SSW.... HLS.... MSTT... DCFT.M. GMA.... SMN.... HMA.... MFR.... DTOT.M. GLS.... SLO.... HGR.... MZU.... DSRT.M. GMA.... SPL.... HMA.... MGE.... DD2T.M. GGR.... SEC.... HLS.... MZU.... DBA.V.. GMA.... SPL.... HMA.... MST.... DMS..M. GLS.... SEC.... HGR.... MLI.... DTS..M. GMA.... SPL.... HMA.... MFR.... DROT.M. GGR.... SEC.... HLS.... MNU.... DNPT.M. GMA.... SPL.... HMA.... MST.... DBIT.M. GLS.... SEC.... HGR.... MLI.... DHIT.V. GMA.... SPL.... HMA.... MFR.... DD1T... GAL.... SEC.... HCA.... MNU.... DAS.... GSN.... SLE.... HLS.... MLI.... DIO..M. GMA.... SNA.... HMA.... MPR.... DBS..M. GAL.... SBO.... HSN.... MHA.... DCNT.M. GGR.... STO.... HMA.... MLI.... DGAT.M. GAL.... SBO.... HAL.... MCO.... DCD.VM. GSN.... SCF.... HGR.... MPA.... DKLT... GMA.... SBO.... HAL.... MBU.... DBCT... GAL.... STO.... HSN.... MCO.... DBET... GSN.... SBO.... HMA.... MAM.... DSOT.M. GMA.... SCF.... HAL.... MBU.... DSJT.M. GSN.... SPA.... HMS.... MAM.... DVAT.V. GBB.... SBO.... HMR.... MCO.... DSAT.M. GAO.... SNA.... HMI.... MLI.... DHIT.M. GIR.... SBB.... HMU.... MMU.... DD1T.M. GAO.... SAO.... HVE.... MBR.... DSOT.M. GIR.... SIR.... HAS.... MLI.... DBET.M. GAO.... SAO.... HIO.... MBR.... DBCT.M. GIR.... SIR.... HTS.... MPR.... DGA.VM. GAO.... SAO.... HNP.... MLI.... DCDT.M. GIR.... SIR.... HBI.... MBR.... DKLT.M. GAO.... SAO.... HNP.... MLI.... DBDT.M. GIR.... SGW.... HTS.... MHA.... DZAT.M. GSW.... SDU.... HRO.... MLI.... DSZT.M. GLO.... SIR.... HGO.... MFR.... DBET.V. GSW.... SSW.... HMI.... MST.... DSOT.M. GLV.... SLO.... HGO.... MNU.... DVRT.M. GSW.... SSW.... HMR.... MHA.... DCNT.M. GLO.... SMN.... HGO.... MNS.... DGAT.M. GSW.... SSW.... HMI.... MED.... DCDT.M. GLV.... SLO.... HGO.... MNS.... DKLT.M. GSW.... SSW.... HMR.... MED.... DBD.VM. GLO.... SMN.... HGO.... MNS.... DSZT.M. GMN.... SLV.... HMI.... MED.... DBET.M. GLV.... SIR.... HZU.... MLO.... DBCT.M. GIR.... SDU.... HFR.... MMN.... DCNT.M. GDU.... SIR.... HPR.... MED.... DVRT.M. GIR.... SDU.... HBR.... MNS.... DSOT.V. GDU.... SIR.... HHA.... MED.... DSJT.M. GIR.... SDU.... HLI.... MMN.... DBET.M. GDU.... SIR.... HST.... MED.... DKLT.M. GIR.... SDU.... HCO.... MNS.... DCDT.M. GDU.... SIR.... HHA.... MEC.... DGAT.M. GIR.... SSW.... HNS.... MNS.... DBCT.M. GSW.... SMN.... HEC.... MEC.... DSO.VM. GMN.... SSW.... HNS.... MNS.... DVAT.M. GSW.... SIR.... HEC.... MEC.... DATT.M. GIR.... SSW.... HNS.... MNS.... DHIT.M. GSW.... SMN.... HEC.... MEC.... DD1T.M. GMN.... SSW.... HNS.... MNS.... DIO..M. GSW.... SIR.... HEC.... MEC.... DTS..V. GIR.... SSW.... HNS.... MNS.... DNPT.M. GSW.... SMN.... HEC.... MAO.... DBIT.M. GIR.... SED.... HLE.... MIR.... DROT.M. GAO.... SMN.... HBU.... MAO.... DFLT.M. GIR.... SED.... HLE.... MIR.... DGOT... GAO.... SLV.... HMR.... MAO.... DVET... GIR.... SIR.... HCF.... MIR.... DMI.VM. GAO.... SAO.... HBO.... MAO.... DMUT.M. GIR.... SIR.... HBU.... MIR.... DNUT.M. GAO.... SAO.... HMR.... MAO.... DLIT.M. GIR.... SIR.... HCF.... MIR.... DCOT.M. GDU.... SDU.... HGE.... MDU.... DFRT.M. GIR.... SIR.... HFL.... MIR.... DHIT.V. GDU.... SDU.... HGE.... MDU.... DD1T.M. GIR.... SIR.... HST.... MIR.... DNUT.M. GDU.... SDU.... HCOT... MDU.... DPRT.M. GIR.... SIR.... HBU.... MIR.... DVIT... GDU.... SDU.... HFRTT.. MDU.... DMUT... GIR.... SIR.... HSJ.... MIR.... DVET... GDU.... SDU.... HVA.... MAO.... DFL.VM. GGW.... SGW.... HSJ.... MBB.... DROT.M. GAO.... SAO.... HVA.... MAO.... DBIT.M. GBB.... SBB.... HSJ.... MBB.... DNPT.M. GAO.... SAO.... HVA.... MAO.... DHIT.M. GBB.... SBB.... HSJ.... MBB.... DD1T.M. GAO.... SAO.... HVA.... MAO.... DTS..V. GBB.... SBB.... HSJ.... MBB.... DMS..M. GAO.... SAO.... HVA.... MNA.... DCGT.M. GMS.... SMS.... HSO.... MPA.... DTPT.M. GBA.... SBA.... HVA.... MGE.... DKLT.M. GMA.... SLS.... HSO.... MMI.... DSZT.M. GBO.... SMA.... HSA.... MGO.... DZAT... GTO.... SGR.... HVR.... MMR.... DSJ.V.. GSR.... SMA.... HCN.... MZU.... DSOT.M. GSN.... SBO.... HGA.... MST.... DVAT.M. GLS.... SSR.... HKLT... MGE.... DATT... GSN.... SMA.... HCD.... MCF.... DHIT.M. GSR.... SCA.... HKL.... MNA.... DD1T.M. GLS.... SMA.... HCD.... MLE.... DIO..V. GBO.... SLS.... HKL.... MMR.... DTS..M. GSR.... SSN.... HCD.... MTO.... DNPT.M. GMA.... SMA.... HKL.... MMR.... DBIT.M. GBA.... SSR.... HSZ.... MGE....";


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
    }
 
    return loc;
}

