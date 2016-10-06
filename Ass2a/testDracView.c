// testDracView.c ... test the DracView ADT

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "DracView.h"

static void UnitTest1(void);
static void UnitTest2(void);

int main(int argc, char *argv[]) {
    int i;
    DracView dv;

    printf("Test for basic functions, just before Dracula's first move\n");
    PlayerMessage messages1[] = {"Hello","Rubbish","Stuff",""};
    dv = newDracView("GST.... SAO.... HZU.... MBB....", messages1);
    assert(giveMeTheRound(dv) == 0);
    assert(whereIs(dv,PLAYER_LORD_GODALMING) == STRASBOURG);
    assert(whereIs(dv,PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
    assert(whereIs(dv,PLAYER_VAN_HELSING) == ZURICH);
    assert(whereIs(dv,PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
    assert(whereIs(dv,PLAYER_DRACULA) == UNKNOWN_LOCATION);
    assert(howHealthyIs(dv,PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
    printf("passed\n");
    disposeDracView(dv);

    printf("Test for encountering Dracula and hunter history\n");
    PlayerMessage messages2[] = {"Hello","Rubbish","Stuff","","Mwahahah","Aha!","","",""};
    dv = newDracView("GST.... SAO.... HCD.... MAO.... DGE.... "
                     "GGED... SAO.... HCD.... MAO....", messages2);
    assert(giveMeTheRound(dv) == 1);
    assert(whereIs(dv,PLAYER_DRACULA) == GENEVA);
    assert(howHealthyIs(dv,PLAYER_LORD_GODALMING) == 5);
    assert(howHealthyIs(dv,PLAYER_DRACULA) == 30);
    assert(whereIs(dv,PLAYER_LORD_GODALMING) == GENEVA);
    LocationID history[TRAIL_SIZE];
    giveMeTheTrail(dv,PLAYER_DRACULA,history);
    assert(history[0] == GENEVA);
    assert(history[2] == UNKNOWN_LOCATION);
    giveMeTheTrail(dv,PLAYER_LORD_GODALMING,history);
    assert(history[0] == GENEVA);
    assert(history[1] == STRASBOURG);
    assert(history[2] == UNKNOWN_LOCATION);
    giveMeTheTrail(dv,PLAYER_DR_SEWARD,history);
    assert(history[0] == ATLANTIC_OCEAN);
    assert(history[1] == ATLANTIC_OCEAN);
    assert(history[2] == UNKNOWN_LOCATION);
    printf("passed\n");        
    disposeDracView(dv);

    printf("Test for Dracula leaving minions\n");
    PlayerMessage messages3[] = {"Hello","Rubbish","Stuff","","Mwahahah","Aha!","","","","Drop a V","Party in Strasbourg","Party","Party","Party"};
    dv = newDracView("GGE.... SGE.... HGE.... MGE.... DED.V.. "
                     "GST.... SST.... HST.... MST.... DMNT... "
                     "GST.... SST.... HST.... MST....", messages3);
    int nT, nV;
    whatsThere(dv,EDINBURGH,&nT,&nV);
    assert(nT == 0 && nV == 1);
    whatsThere(dv,MANCHESTER,&nT,&nV);
    assert(nT == 1 && nV == 0);
    assert(whereIs(dv,PLAYER_DRACULA) == MANCHESTER);
    giveMeTheTrail(dv,PLAYER_DRACULA,history);
    assert(history[0] == MANCHESTER);
    assert(history[1] == EDINBURGH);
    assert(history[2] == UNKNOWN_LOCATION);
    giveMeTheTrail(dv,PLAYER_MINA_HARKER,history);
    assert(history[0] == STRASBOURG);
    assert(history[1] == STRASBOURG);
    assert(history[2] == GENEVA);
    assert(history[3] == UNKNOWN_LOCATION);
    printf("passed\n");
    disposeDracView(dv);

    printf("Test for connections\n");
    int size, seen[NUM_MAP_LOCATIONS], *edges;

    printf("Checking Galatz road connections\n");
    PlayerMessage messages5[] = {"Gone to Galatz"};
    dv = newDracView("GGA....", messages5);
    edges = whereCanTheyGo(dv,&size,PLAYER_LORD_GODALMING,1,0,0);
    memset(seen, 0, NUM_MAP_LOCATIONS*sizeof(int));
    for (i = 0; i< size ; i++) seen[edges[i]] = 1;
    assert(size == 5); assert(seen[GALATZ]); assert(seen[CONSTANTA]);
    assert(seen[BUCHAREST]); assert(seen[KLAUSENBURG]); assert(seen[CASTLE_DRACULA]);
    free(edges);
    disposeDracView(dv);

    printf("Checking Ionian Sea sea connections\n");
    PlayerMessage messages6[] = {"Sailing the Ionian"};
    dv = newDracView("GIO....", messages6);
    edges = whereCanTheyGo(dv,&size,PLAYER_LORD_GODALMING,0,0,1);
    memset(seen, 0, NUM_MAP_LOCATIONS*sizeof(int));
    for (i = 0; i < size; i++) seen[edges[i]] = 1;
    assert(size == 7); assert(seen[IONIAN_SEA]); assert(seen[BLACK_SEA]);
    assert(seen[ADRIATIC_SEA]); assert(seen[TYRRHENIAN_SEA]);
    assert(seen[ATHENS]); assert(seen[VALONA]); assert(seen[SALONICA]);
    free(edges);
    disposeDracView(dv);

    printf("Checking Athens rail connections (none)\n");
    PlayerMessage messages7[] = {"Leaving Athens by train"};
    dv = newDracView("GAT....", messages7);
    edges = whereCanTheyGo(dv,&size,PLAYER_LORD_GODALMING,0,1,0);
    assert(size == 1);
    assert(edges[0] == ATHENS);
    free(edges);
    disposeDracView(dv);

    printf("passed\n");

    printf("\n=====   More Tests on DracView ADT   =====\n\n");
    UnitTest1();
    UnitTest2();    

    return 0;
}

static void UnitTest1(void) {
    printf("*****   Unit Test 1  *****\n");
    printf("Number of traps and immature vampires\n");    

    PlayerMessage messages1[] = {"Hello"};
    DracView dracView = newDracView("GMN.... SCD.... HMA.... MST.... DLV.V.. GLO.... SKL.... HSR.... MZU.... DSWT... GSWTD.. SBD.... HBO.... MMR....", messages1);
    assert(dracView != NULL);
    assert(giveMeTheRound(dracView) == 2);
    assert(howHealthyIs(dracView, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_TRAP_ENCOUNTER - LIFE_LOSS_DRACULA_ENCOUNTER);
    int numTraps, numVamps = 0;
    whatsThere(dracView, LIVERPOOL, &numTraps, &numVamps);
    assert(numTraps == 0);
    assert(numVamps == 1);

    whatsThere(dracView, SWANSEA, &numTraps, &numVamps);
    assert(numTraps == 0);
    assert(numVamps == 0);
    disposeDracView(dracView);

    dracView = newDracView("GMN.... SCD.... HMA.... MST.... DLV.V.. GLO.... SKL.... HSR.... MZU.... DSWT... GSWTD.. SBD.... HBO.... MMR.... DLOT... GSW.... SVI.... HBB.... MMS.... DPLT... GIR.... SPR.... HAO.... MCG.... DEC.... GLVV... SBR.... HIR.... MTS.... DNS.... GLV.... SBR.... HIR.... MTS.... DAO.... GIR.... SPR.... HAO.... MIO.... DBB.... GAO.... SVI.... HMS.... MAS.... DNAT.M. GMS.... SVE.... HTS.... MAS....", messages1);
    assert(dracView != NULL);
    whatsThere(dracView, LONDON, &numTraps, &numVamps);
    assert(numTraps == 0);
    assert(numVamps == 0);
    whatsThere(dracView, NANTES, &numTraps, &numVamps);
    assert(numTraps == 1);
    assert(numVamps == 0);  
    disposeDracView(dracView);

    dracView = newDracView("GMN.... SCD.... HMA.... MST.... DLV.V.. GLO.... SKL.... HSR.... MZU.... DSWT... GSWTD.. SBD.... HBO.... MMR.... DLOT... GSW.... SVI.... HBB.... MMS.... DPLT... GIR.... SPR.... HAO.... MCG.... DEC.... GLVV... SBR.... HIR.... MTS.... DNS.... GLV.... SBR.... HIR.... MTS.... DAO.... GIR.... SPR.... HAO.... MIO.... DBB.... GAO.... SVI.... HMS.... MAS.... DNAT.M. GMS.... SVE.... HTS.... MAS.... DBOT.M. GTS.... SAS.... HIO.... MAS.... DSRT... GIO.... SAS.... HAS.... MAS.... DBAT... GIO.... SAS.... HAS.... MAS.... DTOT... GAS.... SAS.... HAS.... MAS.... DMR.V.. GAS.... SAS.... HAS.... MAS.... DZUT.M. GVE.... SVE.... HVE.... MVE.... DSTT.M. GGO.... SGO.... HGO.... MGO.... DCOT.M. GTS.... STS.... HTS.... MTS.... DBUT.M. GTS.... STS.... HTS.... MTS.... DPAT.M. GIO.... SIO.... HIO.... MIO.... DCFT.V. GIO.... SIO.... HIO.... MIO....", messages1);
    assert(dracView != NULL);
    whatsThere(dracView, MARSEILLES, &numTraps, &numVamps);
    assert(numVamps == 0);
    disposeDracView(dracView);

    printf("passed!\n\n");
}

static void UnitTest2(void) {
    printf("#####   Unit Test 2   #####\n");

    DracView dracView;
    PlayerMessage messages1[] = {"Hello"} ;
    LocationID *possibleMoves;
    int numLocations = 0;

    printf("Test on whereCanIgo when Draculas's castle is in his trail\n");
    dracView = newDracView("GPA.... SBC.... HMU.... MSR.... DCD.V.. GPA.... SBC.... HMU.... MSR.... DHIT... GPA.... SBC.... HMU.... MSR.... DD1T... GPA.... SBC.... HMU.... MSR.... DGAT... GPA.... SBC.... HMU.... MSR.... DKLT... GPA.... SBC.... HMU.... MSR....", messages1);
    assert(dracView != NULL);
    possibleMoves = whereCanIgo(dracView, &numLocations, 1, 1);
    assert(possibleMoves != NULL);
    assert(numLocations == 5);
    int *reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);

    int i, j = 0;
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    }

    assert(reachable[KLAUSENBURG] == 1);
    assert(reachable[BUDAPEST] == 1);
    assert(reachable[SZEGED] == 1);
    assert(reachable[BELGRADE] == 1);
    assert(reachable[BUCHAREST] == 1);
    assert(reachable[GALATZ] == 0);          // Test for no connections !  
    assert(reachable[CASTLE_DRACULA] == 0);  // Test for no connections !
    free(possibleMoves);
    free(reachable);
    disposeDracView(dracView);
    printf("passed!\n");

    printf("Test on whereCanIgo when Dracula's castle leaves his trail   (by Hard coding)\n");
    dracView = newDracView("GPA.... SBC.... HMU.... MSR.... DCD.V.. GPA.... SBC.... HMU.... MSR.... DHIT... GPA.... SBC.... HMU.... MSR.... DD1T... GPA.... SBC.... HMU.... MSR.... DGAT... GPA.... SBC.... HMU.... MSR.... DKLT... GPA.... SBC.... HMU.... MSR.... DGAT... GPA.... SBC.... HMU.... MSR....", messages1);
    assert(dracView != NULL);
    possibleMoves= whereCanIgo(dracView, &numLocations, 1, 1);
    assert(possibleMoves != NULL);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    }
 
    assert(reachable[CASTLE_DRACULA] == 1);
    free(possibleMoves);
    free(reachable); 
    disposeDracView(dracView);
    printf("passed!\n");

    printf("Test on whereIs and giveMeTheTrail\n");
    dracView = newDracView("GPA.... SBC.... HMU.... MSR.... DCD.V.. GPA.... SBC.... HMU.... MSR.... DHIT...", messages1);
    assert(dracView != NULL);
    assert(whereIs(dracView, PLAYER_DRACULA) == CASTLE_DRACULA);
   
    LocationID *trail = malloc(TRAIL_SIZE * sizeof(LocationID));
    assert(trail != NULL);
    giveMeTheTrail(dracView, PLAYER_DRACULA, trail);
    assert(trail != NULL);
    assert(trail[0] == CASTLE_DRACULA);
    assert(trail[1] == CASTLE_DRACULA);
    
    for(i = 2; i < TRAIL_SIZE - 1; i++) assert(trail[i] == UNKNOWN_LOCATION); 

    free(trail);
    disposeDracView(dracView);
    printf("passed!\n\n");
}
