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

   char *plays = "GSR.... SST.... HLO.... MSZ.... DSJ.V.. GBO.... SLI.... HLV.... MKL.... DSOT... GLE.... SBU.... HSW.... MBD.... DHIT... GBU.... SLE.... HLO.... MSZ.... DVAT... GCO.... SPA.... HLV.... MZA.... DATT... GFR.... SSR.... HED.... MSJV... DIO.... GZU.... SMR.... HMN.... MSOTT.. DBS.... GGO.... SGE.... HLO.... MBC.... DVRT... GMR.... SCF.... HLV.... MGA.... DHIT... GMI.... SMR.... HED.... MKL.... DCNT.M. GMI.... SMR.... HED.... MKL.... DGAT.M. GFL.... SGE.... HMN.... MBD.... DCDT... GVE.... SMI.... HED.... MKL.... DD1T... GVI.... SVE.... HNS.... MCDTTD. DKL.VM. GBR.... SAS.... HAO.... MZA.... DBET.M. GFR.... SIO.... HLS.... MSJ.... DSZT.M. GST.... SBS.... HSR.... MSO.... DBDT.M. GNU.... SVR.... HMR.... MSA.... DHIT... GLI.... SBET... HGE.... MVA.... DZAT... GBU.... SSJ.... HCF.... MAT.... DVIT.V. GBU.... SSJ.... HCF.... MAT.... DMUT... GLE.... SSO.... HMR.... MVA.... DNUT.M. GMR.... SVR.... HZU.... MSA.... DLIT.M. GMI.... SCN.... HMI.... MVR.... DHAT.M. GMUT... SBC.... HMU.... MBE.... DHIT.M. GZU.... SSO.... HBR.... MSO.... DCOT.M. GST.... SVR.... HPR.... MSJ.... DAM.V.. GNUT... SCN.... HVI.... MBE.... DBUT... GMU.... SBC.... HZA.... MSZ.... DPAT.M. GZU.... SSO.... HSJ.... MKL.... DLET.M. GZU.... SSO.... HSJ.... MKL.... DNAT.M. GGO.... SSJ.... HSO.... MCD.... DBOT.M. GMR.... SBE.... HVR.... MGA.... DSRT.V. GZU.... SSO.... HSA.... MBC.... DSNT.M. GST.... SVR.... HVA.... MCN.... DLST.M. GNU.... SCN.... HAT.... MSZ.... DCAT.M. GMU.... SBC.... HIO.... MKL.... DHIT.M. GZU.... SSO.... HSA.... MCD.... DMAT.M. GST.... SVR.... HVA.... MGA.... DGRT.M. GNU.... SCN.... HAT.... MBC.... DAL.VM. GNU.... SCN.... HAT.... MBC.... DSRT.M. GMU.... SBC.... HVA.... MGA.... DBOT.M. GZU.... SSO.... HSA.... MCD.... DCFT.M. GGO.... SSJ.... HSO.... MKL.... DPAT.M. GMR.... SBE.... HVR.... MBD.... DLET.M. GZU.... SSO.... HSA.... MSZ.... DBUT.V. GST.... SVR.... HVA.... MBE.... DAMT.M. GGE.... SCN.... HIO.... MSO.... DCOT.M. GCFT... SBC.... HAT.... MBD.... DLIT... GBO.... SSO.... HVA.... MSZ.... DHIT.M. GBO.... SSO.... HVA.... MSZ.... DFRT.M. GPA.... SBC.... HIO.... MKL.... DNUT.M. GCF.... SBE.... HAT.... MBC.... DST.VM. GGE.... SSO.... HVA.... MBE.... DMUT.M. GFL.... SBD.... HIO.... MKL.... DD3T.M. GGO.... SSZ.... HAT.... MBC.... DPRT.M. GTS.... SBE.... HVA.... MVI.... DBRT.M. GCG.... SSO.... HIO.... MMUT... DLIT.M. GMS.... SBD.... HAT.... MZU.... DHAT.V. GAO.... SSZ.... HVA.... MFL.... DHIT... GAO.... SSZ.... HVA.... MFL.... DCOT.M. GBB.... SKL.... HIO.... MVE.... DFRT.M. GBO.... SBC.... HAT.... MVI.... DSTT.M. GPA.... SBE.... HVA.... MZA.... DZUT.M. GCF.... SKL.... HIO.... MMU.... DMRT.M. GGE.... SBC.... HAT.... MZUT... DTO.VM. GMRT... SCN.... HVA.... MSTT... DBAT.M. GBU.... SGA.... HSO.... MLI.... DSRT.M. GST.... SCD.... HSJ.... MZU.... DMAT... GMU.... SKL.... HZA.... MST.... DCAT... GMU.... SKL.... HZA.... MST.... DGRT... GBR.... SGA.... HSZ.... MLI.... DHIT.V. GHA.... SCN.... HSO.... MZU.... DALT.M. GCO.... SVR.... HSA.... MST.... DSRT.M. GFR.... SSA.... HIO.... MMU.... DBOT.M. GMU.... SIO.... HTS.... MVE.... DTOT.M. GVI.... STS.... HNP.... MAS.... DCFT.M. GZA.... SNP.... HRO.... MBI.... DMRT.M. GSZ.... SRO.... HFL.... MRO.... DHI.VM. GVE.... SFL.... HGO.... MNP.... DMS..M. GVE.... SFL.... HGO.... MNP.... DBAT.M. GMI.... SGO.... HGE.... MTS.... DSRT.M. GGO.... SZU.... HZU.... MIO.... DBOT.M. GZU.... SST.... HST.... MBS.... DCFT.M. GST.... SCO.... HGE.... MVR.... DTOT.V. GCO.... SBU.... HCFT... MSO.... DD5T... GBU.... SAM.... HMR.... MSA.... DHIT.M. GAM.... SCO.... HPA.... MVA.... DMS..M. GCO.... SFR.... HCF.... MSO.... DBAT.M. GFR.... SZU.... HMR.... MSA.... DSRT... GFR.... SZU.... HMR.... MSA.... DALT.M. GBR.... SST.... HPA.... MVA.... DGR.VM. GPR.... SCO.... HCF.... MSO.... DHIT.M. GVI.... SBU.... HMR.... MSA.... DMAT... GVE.... SAM.... HGE.... MVA.... DLST.M. GPR.... SCO.... HCF.... MSO.... DSNT.M. GVI.... SFR.... HMR.... MSZ.... DBB..M. GZA.... SNU.... HPA.... MKL.... DBOT.V. GBD.... SPR.... HBU.... MBE.... DNAT.M. GVE.... SBR.... HLE.... MSO.... DCFT.M. GVE.... SBR.... HLE.... MSO.... DTOT.M. GAS.... SFR.... HNAT... MBE.... DSRT.M. GIO.... SZU.... HCFT... MBC.... DMAT... GVA.... SMU.... HBOT... MKL.... DGRT... GSJ.... SZA.... HBO.... MBE.... DCA.V.. GSO.... SBD.... HBA.... MBC.... DHIT... GVA.... SVE.... HSRT... MSZ.... DLST.M. GSJ.... SMI.... HSR.... MKL.... DAO.... GSO.... SMU.... HSN.... MBE.... DGWT.M. GBE.... SZA.... HMA.... MBC.... DDUT.M. GBE.... SZA.... HMA.... MBC.... DIR..V. GSO.... SVI.... HSR.... MBE.... DSWT.M. GVR.... SMU.... HBO.... MSO.... DLVT.M. GCN.... SVE.... HMA.... MVR.... DMNT... GBC.... SGO.... HSR.... MCN.... DEDT.M. GGA.... SMR.... HTO.... MBC.... DHIT.M. GCN.... SMI.... HCF.... MGA.... DNS.... GBC.... SGE.... HPA.... MCN.... DHA.VM. GBE.... SGO.... HST.... MBC.... DBRT.M. GSO.... SMR.... HNU.... MBE.... DD2T.M. GSO.... SMR.... HNU.... MBE.... DLIT.M. GVR.... SBU.... HMU.... MSZ.... DCOT.M. GSA.... SLE.... HST.... MZA.... DAMT... GVA.... SNA.... HNU.... MVI.... DBUT.V. GSO.... SPA.... HBRT... MPR.... DLET... GBE.... SST.... HPR.... MNU.... DNAT.M. GSA.... SBR.... HVI.... MST.... DCFT.M. GIO.... SPR.... HZA.... MZU.... DHIT.M. GAS.... SVI.... HBD.... MMU.... DTOT.M. GVE.... SVE.... HBC.... MST.... DBOT.M. GVE.... SVE.... HBC.... MST.... DSR.VM. GMU.... SMU.... HSZ.... MZU.... DBAT.M. GST.... SST.... HZA.... MMU.... DMS..M. GGE.... SZU.... HBD.... MST.... DCGT.M. GMR.... SMU.... HKL.... MGE.... DHIT.M. GMS.... SST.... HBE.... MCF.... DTS..M. GTS.... SGE.... HBC.... MTO.... DIO..V. GRO.... SCF.... HSZ.... MMR.... DVAT.M. GFL.... STO.... HKL.... MGE.... DSAT... GMI.... SMR.... HGA.... MCF.... DSOT.M. GMI.... SMR.... HGA.... MCF.... DVRT.M. GGE.... STO.... HBD.... MTO.... DCNT... GCF.... SCF.... HZA.... MMR.... DGAT... GTO.... SPA.... HVI.... MMI.... DCD.VM. GMR.... SST.... HBD.... MMU.... DHIT.M. GMI.... SMU.... HKL.... MZA.... DD1T.M. GVE.... SZA.... HCDTTVD MSZ.... DKLT.M. GGO.... SBD.... HCD.... MKLTD.. DBET.M. GMI.... SVI.... HCD.... MKL.... DSJT.M. GVE.... SPR.... HKL.... MBC.... DVAT... GVE.... SPR.... HKL.... MBC.... DATT... GMI.... SNU.... HBC.... MKL.... DHIT... GZU.... SST.... HGA.... MBET... DIO.... GST.... SPA.... HCN.... MBC.... DSAT... GPA.... SMA.... HVR.... MSZ.... DD5T.M. GMA.... SCA.... HBS.... MKL.... DSJT.M. GCA.... SLS.... HIO.... MBC.... DZA.VM. GLS.... SAL.... HATT... MSO.... DHIT... GSN.... SGR.... HVAT... MVR.... DVIT... GBO.... SMA.... HSAT... MSA.... DBDT... GBO.... SMA.... HSA.... MSA.... DKLT... GTO.... SLS.... HVA.... MIO.... DCDT.M. GMR.... SAO.... HSO.... MSA.... DGAT.V. GZU.... SIR.... HVR.... MVR.... DHIT.M. GMI.... SSW.... HSA.... MSO.... DCNT.M. GMU.... SLO.... HVA.... MVA.... DBCT.M. GVI.... SPL.... HAT.... MSA.... DKLT.M. GBD.... SEC.... HIO.... MVR.... DCDT.M. GKLT... SNS.... HSA.... MCNT... DGAT.M. GSZ.... SHA.... HVA.... MGATTD. DD2.V.. GKL.... SVI.... HSO.... MSZ.... DHIT... GCDTTVD SBD.... HBCT... MKL.... DTPT... GSZ.... SKL.... HKL.... MCDTD.. DKLT... GKLTD.. SKLD... HKLD... MKLD...";

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
 
