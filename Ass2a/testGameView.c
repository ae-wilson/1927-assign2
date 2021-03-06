// testGameView.c ... test the GameView ADT

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "GameView.h"

static void UnitTest1(void);
static void UnitTest2(void);
static void UnitTest3(void);

int main(int argc, char *argv[]) {
    int i;
    GameView gv;
    
    printf("Test basic empty initialisation\n");
    PlayerMessage messages1[] = {};
    gv = newGameView("", messages1);
    assert(getCurrentPlayer(gv) == PLAYER_LORD_GODALMING);
    assert(getRound(gv) == 0);
    assert(getHealth(gv,PLAYER_DR_SEWARD) == GAME_START_HUNTER_LIFE_POINTS);
    assert(getHealth(gv,PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
    assert(getScore(gv) == GAME_START_SCORE);
    assert(getLocation(gv,PLAYER_LORD_GODALMING) == UNKNOWN_LOCATION);
    printf("passed\n");
    disposeGameView(gv);

    printf("Test for Dracula trail and basic functions\n");
    PlayerMessage messages2[] = {"Hello","Rubbish","Stuff","","Mwahahah"};
    gv = newGameView("GST.... SAO.... HZU.... MBB.... DC?....", messages2);
    assert(getCurrentPlayer(gv) == PLAYER_LORD_GODALMING);
    assert(getRound(gv) == 1);
    assert(getLocation(gv,PLAYER_LORD_GODALMING) == STRASBOURG);
    assert(getLocation(gv,PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
    assert(getLocation(gv,PLAYER_VAN_HELSING) == ZURICH);
    assert(getLocation(gv,PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
    assert(getLocation(gv,PLAYER_DRACULA) == CITY_UNKNOWN);
    assert(getHealth(gv,PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
    printf("passed\n");
    disposeGameView(gv);

    printf("Test for encountering Dracula and hunter history\n");
    PlayerMessage messages3[] = {"Hello","Rubbish","Stuff","","Mwahahah","Aha!"};
    gv = newGameView("GST.... SAO.... HCD.... MAO.... DGE.... GGED...", messages3);
    assert(getLocation(gv,PLAYER_DRACULA) == GENEVA);
    assert(getHealth(gv,PLAYER_LORD_GODALMING) == 5);
    assert(getHealth(gv,PLAYER_DRACULA) == 30);
    assert(getLocation(gv,PLAYER_LORD_GODALMING) == GENEVA);
    LocationID history[TRAIL_SIZE];
    getHistory(gv,PLAYER_DRACULA,history);
    assert(history[0] == GENEVA);
    assert(history[1] == UNKNOWN_LOCATION);
    getHistory(gv,PLAYER_LORD_GODALMING,history);
    assert(history[0] == GENEVA);
    assert(history[1] == STRASBOURG);
    assert(history[2] == UNKNOWN_LOCATION);
    getHistory(gv,PLAYER_DR_SEWARD,history);
    assert(history[0] == ATLANTIC_OCEAN);
    assert(history[1] == UNKNOWN_LOCATION);
    printf("passed\n");        
    disposeGameView(gv);

    printf("Test for Dracula doubling back at sea, and losing blood points (Hunter View)\n");
    PlayerMessage messages4[] = {"Hello","Rubbish","Stuff","","Mwahahah","Aha!","","","","Back I go"};
    gv = newGameView("GGE.... SGE.... HGE.... MGE.... DS?.... "
                     "GST.... SST.... HST.... MST.... DD1....", messages4);
    assert(getLocation(gv,PLAYER_DRACULA) == DOUBLE_BACK_1);
    getHistory(gv,PLAYER_DRACULA,history);
    assert(history[0] == DOUBLE_BACK_1);
    assert(history[1] == SEA_UNKNOWN);
    printf("\nHealth of Dracula = %d\n", getHealth(gv, PLAYER_DRACULA));

    assert(getHealth(gv,PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - 4);
    assert(getCurrentPlayer(gv) == 0);
    printf("passed\n");
    disposeGameView(gv);

    printf("Test for Dracula doubling back at sea, and losing blood points (Drac View)\n");
    PlayerMessage messages5[] = {"Hello","Rubbish","Stuff","","Mwahahah","Aha!","","","","Back I go"};
    gv = newGameView("GGE.... SGE.... HGE.... MGE.... DEC.... "
                     "GST.... SST.... HST.... MST.... DD1....", messages5);
    assert(getLocation(gv,PLAYER_DRACULA) == DOUBLE_BACK_1);
    getHistory(gv,PLAYER_DRACULA,history);
    assert(history[0] == DOUBLE_BACK_1);
    assert(history[1] == ENGLISH_CHANNEL);
    assert(getHealth(gv,PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - 4);
    assert(getCurrentPlayer(gv) == 0);
    printf("passed\n");
    disposeGameView(gv);

    printf("Test for connections\n");
    int size, seen[NUM_MAP_LOCATIONS], *edges;
    gv = newGameView("", messages1);  
    printf("Checking Galatz road connections\n");
    edges = connectedLocations(gv,&size,GALATZ,PLAYER_LORD_GODALMING,0,1,0,0);
    memset(seen, 0, NUM_MAP_LOCATIONS*sizeof(int));
    for (i = 0; i< size ; i++) seen[edges[i]] = 1;
    assert(size == 5); assert(seen[GALATZ]); assert(seen[CONSTANTA]);
    assert(seen[BUCHAREST]); assert(seen[KLAUSENBURG]); assert(seen[CASTLE_DRACULA]);
    free(edges);
    printf("Checking Ionian Sea sea connections\n");
    edges = connectedLocations(gv,&size,IONIAN_SEA,PLAYER_LORD_GODALMING,0,0,0,1);
    memset(seen, 0, NUM_MAP_LOCATIONS*sizeof(int));
    for (i = 0; i < size; i++) seen[edges[i]] = 1;
    assert(size == 7); assert(seen[IONIAN_SEA]); assert(seen[BLACK_SEA]);
    assert(seen[ADRIATIC_SEA]); assert(seen[TYRRHENIAN_SEA]);
    assert(seen[ATHENS]); assert(seen[VALONA]); assert(seen[SALONICA]);
    free(edges);
    printf("Checking Athens rail connections (none)\n");
    edges = connectedLocations(gv,&size,ATHENS,PLAYER_LORD_GODALMING,0,0,1,0);
    assert(size == 1);
    assert(edges[0] == ATHENS);
    free(edges);
    printf("passed\n");
    disposeGameView(gv);

    printf("\n=====   More Tests   =====\n");
    UnitTest1();    
    UnitTest2();
    UnitTest3();

    return 0;
}


static void UnitTest1(void) {
    printf("#####   Unit Test 1   #####\n");   

    PlayerMessage messages1[] = {"Tom", "John", "Milly"};
    GameView gameView = newGameView("GPA.... SVI.... HCD....", messages1);
    assert(gameView != NULL);
    assert(getCurrentPlayer(gameView) == PLAYER_MINA_HARKER);
    assert(getScore(gameView) == GAME_START_SCORE);
    assert(getLocation(gameView, PLAYER_LORD_GODALMING) == PARIS);
    assert(getLocation(gameView, PLAYER_DR_SEWARD) == VIENNA);
    assert(getLocation(gameView, PLAYER_VAN_HELSING) == CASTLE_DRACULA);
    assert(getLocation(gameView, PLAYER_MINA_HARKER) == UNKNOWN_LOCATION);
    assert(getLocation(gameView, PLAYER_DRACULA) == UNKNOWN_LOCATION);

    printf("Testing the function connectedLocations ......\n");
    int numLocations;
    LocationID *possibleMoves = connectedLocations(gameView, &numLocations, VIENNA, PLAYER_DR_SEWARD, 0, 1, 1, 1);
    assert(numLocations == 6);
    int *reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    int i, j = 0;
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[VIENNA] == 1);
    assert(reachable[PRAGUE] == 1);
    assert(reachable[MUNICH] == 1);
    assert(reachable[BUDAPEST] == 1);
    assert(reachable[ZAGREB] == 1);
    assert(reachable[VENICE] == 1);   
    free(reachable);
    free(possibleMoves);

    possibleMoves = connectedLocations(gameView, &numLocations, CASTLE_DRACULA, PLAYER_VAN_HELSING, 0, 1, 1, 0);
    assert(numLocations == 3);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[CASTLE_DRACULA] == 1);
    assert(reachable[GALATZ] == 1);
    assert(reachable[KLAUSENBURG] == 1);
    free(reachable);
    free(possibleMoves);

    possibleMoves = connectedLocations(gameView, &numLocations, CASTLE_DRACULA, PLAYER_VAN_HELSING, 0, 0, 1, 1);
    assert(numLocations == 1);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[CASTLE_DRACULA] == 1);
    free(reachable);
    free(possibleMoves);

    possibleMoves = connectedLocations(gameView, &numLocations, PARIS, PLAYER_LORD_GODALMING, 0, 1, 1, 1);
    assert(numLocations == 7);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[PARIS] == 1);
    assert(reachable[NANTES] == 1);
    assert(reachable[STRASBOURG] == 1);
    assert(reachable[GENEVA] == 1);
    assert(reachable[CLERMONT_FERRAND] == 1);
    assert(reachable[BRUSSELS] == 1);
    assert(reachable[LE_HAVRE] == 1);
    free(reachable);
    free(possibleMoves);

    disposeGameView(gameView);
    printf("passed!\n\n");
}


static void UnitTest2(void) {
    printf("*****   Unit Test 2   *****\n");
  
    PlayerMessage messages1[] = {"", ""};

    int i, j = 0;

    GameView gameView = newGameView("GCD.... SSR.... HST.... MMN.... DSO.V.. GGA.... SAL.... HBU.... MED.... DVAT... GCN.... SBA.... HAM.... MNS.... DSJT... GBS.... SMS.... HBU.... MAM.... DZAT... GIO.... SAL.... HCO.... MNS.... DC?T... GAT.... SMS.... HAM.... MAO.... DC?T... GAT.... SMS.... HAM.... MAO.... DC?T.V. GAT.... SMR.... HCO.... MMS.... DC?T.M. GAT.... SMR.... HCO.... MMS.... DC?T.M. GAT.... SPA....", messages1);
    assert(gameView != NULL);

    printf("More Tests for functions getLocation, getScore, etc ......\n");
    assert(getRound(gameView) == 9);
    assert(getCurrentPlayer(gameView) == PLAYER_VAN_HELSING);
    assert(getScore(gameView) == GAME_START_SCORE - getRound(gameView) - SCORE_LOSS_VAMPIRE_MATURES);
    for(i = 0; i < PLAYER_DRACULA; i++) assert(getHealth(gameView, i) == GAME_START_HUNTER_LIFE_POINTS);
    assert(getHealth(gameView, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
    assert(getLocation(gameView, PLAYER_DRACULA) == CITY_UNKNOWN);
    printf("passed!\n");  

    printf("More Tests for getHistory ......\n");
    LocationID *trail = malloc(TRAIL_SIZE * sizeof(LocationID));
    assert(trail != NULL);
    getHistory(gameView, PLAYER_DR_SEWARD, trail);   
    assert(trail[0] == PARIS);
    assert(trail[1] == MARSEILLES);
    assert(trail[2] == MARSEILLES);
    assert(trail[3] == MEDITERRANEAN_SEA);
    assert(trail[4] == MEDITERRANEAN_SEA);
    assert(trail[5] == ALICANTE);

    getHistory(gameView, PLAYER_LORD_GODALMING, trail);
    assert(trail[0] == ATHENS);
    assert(trail[1] == ATHENS);
    assert(trail[2] == ATHENS);
    assert(trail[3] == ATHENS);
    assert(trail[4] == ATHENS);
    assert(trail[5] == IONIAN_SEA);
    free(trail); 
    printf("passed!\n");

    printf("More Tests for connectedLocations --- road moves + 2 x rail moves\n");
    int numLocations;
    LocationID *possibleMoves = connectedLocations(gameView, &numLocations, PARIS, PLAYER_DR_SEWARD, 9, 1, 1, 0);
    assert(numLocations == 11);
    int *reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[PARIS] == 1);
    assert(reachable[BRUSSELS] == 1);
    assert(reachable[LE_HAVRE] == 1);
    assert(reachable[NANTES] == 1);
    assert(reachable[BORDEAUX] == 1);
    assert(reachable[CLERMONT_FERRAND] == 1);
    assert(reachable[MARSEILLES] == 1);
    assert(reachable[GENEVA] == 1);
    assert(reachable[STRASBOURG] == 1);
    assert(reachable[COLOGNE] == 1);
    assert(reachable[SARAGOSSA] == 1);
    assert(reachable[LISBON] == 0);     // test for no connection !
    free(reachable);
    free(possibleMoves);

    printf("More Tests for connectedLocations --- road moves + 3 x rail moves + sea moves\n");
    possibleMoves = connectedLocations(gameView, &numLocations, GENOA, PLAYER_MINA_HARKER, 0, 1, 1, 1);
    assert(numLocations == 10);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;    
    for(i = 0; i < numLocations; i++) {
        j = possibleMoves[i];
        reachable[j] = 1;
    } 
    assert(reachable[GENOA] == 1);
    assert(reachable[MARSEILLES] == 1);
    assert(reachable[FLORENCE] == 1);
    assert(reachable[VENICE] == 1);
    assert(reachable[TYRRHENIAN_SEA] == 1);
    assert(reachable[MILAN] == 1);
    assert(reachable[GENEVA] == 1);
    assert(reachable[ROME] == 1);
    assert(reachable[STRASBOURG] == 1);
    assert(reachable[ZURICH] == 1);
    assert(reachable[SARAGOSSA] == 0);  // test for no connections !
    assert(reachable[LISBON] == 0);     // test for no connections !
    free(reachable);
    free(possibleMoves);

    disposeGameView(gameView);
    printf("passed!\n\n");
}

static void UnitTest3(void) {
    printf("#####   Unit Test 3   #####\n");
    printf("-----   Tests on death of hunter   -----\n");

    PlayerMessage messages1[] = {"Hi", "My name is ?????"};
    GameView gameView = newGameView("GMN.... SCD.... HMA.... MST.... DLV.V.. GLO.... SKL.... HSR.... MZU.... DSWT... GSWTD.. SBD.... HBO.... MMR.... DC?T... GSW.... SVI.... HBB.... MMS.... DC?T... GIR.... SPR.... HAO.... MCG.... DS?.... GLVV... SBR.... HIR.... MTS.... DS?.... GLV.... SBR.... HIR.... MTS.... DAO.... GIR.... SPR.... HAO.... MIO.... DS?.... GAO.... SVI.... HMS.... MAS.... DNAT.M. GMS.... SVE.... HTS.... MAS.... DC?T.M. GTS.... SAS.... HIO.... MAS.... DC?T... GIO.... SAS.... HAS.... MAS.... DC?T... GIO.... SAS.... HAS.... MAS.... DTOT... GAS.... SAS.... HAS.... MAS.... DC?.V.. GAS.... SAS.... HAS.... MAS.... DZUT.M. GVE.... SVE.... HVE.... MVE.... DC?T.M. GGO.... SGO.... HGO.... MGO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?T.M. GTS.... STS.... HTS.... MTS.... DPAT.M. GIO.... SIO.... HIO.... MIO.... DC?T.V. GIO.... SIO.... HIO.... MIO.... DC?T.M. GAS.... SAS.... HAS.... MAS.... DC?T.M. GIO.... SIO.... HIO.... MIO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?T.M. GTS.... STS.... HTS.... MTS.... DTOT.M. GIO.... SIO.... HIO.... MIO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?.VM. GMS.... SMS.... HMS.... MMS.... DC?T.M. GAO.... SAO.... HAO.... MAO.... DC?T.M. GMS.... SMS.... HMS.... MMS.... DC?T.M. GMS.... SMS.... HMS.... MMS.... DPAT.M. GTS.... STS.... HTS.... MTS.... DCFT.M. GGO.... SGO.... HGO.... MGO.... DNAT.V. GMR.... SMR.... HMR.... MMR.... DBOT.M. GCFT... SPAT... HCF.... MGE.... DSRT.M. GTO.... SNAT... HPA.... MPA.... DBAT.M. GTO.... SNA.... HPA.... MPA.... DTOT... GSRT... SLE.... HBOT... MLE.... DMRT... GSN.... SBU.... HSR.... MBU.... DZUT... GBB.... SST.... HBAT... MCO.... DST.V.. GNA.... SNU.... HMS.... MHA.... DC?T... GCF.... SPR.... HMRT... MLI.... DC?T... GCF.... SPR.... HMR.... MLI.... DPAT.M. GGE.... SVI.... HMI.... MFR.... DCFT... GZUT... SMU.... HVE.... MSTV... DNAT... GST.... SST.... HMU.... MST.... DBOT... GGE.... SGE.... HMI.... MGE.... DSRT.M. GCFT... SPAT... HGO.... MPA.... DC?T.M. GCF.... SPA.... HGO.... MPA.... DTOT... GBOT... SMR.... HMR.... MNAT... DMRT... GSRT... SZU.... HTOT... MCF.... DZUT... GSR.... SMU.... HCF.... MMRT... DSTT... GTO.... SZA.... HPA.... MMI.... DC?.V.. GMR.... SVI.... HNA.... MMU.... DC?T.M. GMR.... SVI.... HNA.... MMU.... DPAT... GGE.... SPR.... HLE.... MVI.... DCFT... GGE.... SVI.... HPAT... MVE.... DC?T.M. GSTT... SBD.... HGE.... MGO.... DC?T... GZU.... SSZ.... HCFT... MMR.... DC?T.V. GMU.... SZA.... HMR.... MMI.... DC?T.M. GMU.... SZA.... HMR.... MMI.... DTOT... GVI.... SBD.... HMI.... MMU.... DMRT... GVE.... SVI.... HMI.... MMI.... DZUT.M. GGO.... SMU.... HMRT... MGE.... DSTT.M. GTS.... SZUT... HTOT... MPA.... DCOT.M. GRO.... SSTT... HCF.... MLE.... DBU.VM. GRO.... SST.... HCF.... MLE.... DPAT... GFL.... SPATD.. HNA.... MBUV... DC?T... GGO.... SPA.... HLE.... MCOT... DC?T... GVE.... SNATD..", messages1);
   
    assert(gameView != NULL);
    assert(getRound(gameView) == 69);
    assert(getScore(gameView) == GAME_START_SCORE - 69 - 3 * SCORE_LOSS_VAMPIRE_MATURES - SCORE_LOSS_HUNTER_HOSPITAL);
    assert(getCurrentPlayer(gameView) == PLAYER_VAN_HELSING);
    assert(getHealth(gameView, PLAYER_DR_SEWARD) == 0);
    assert(getLocation(gameView, PLAYER_DR_SEWARD) == ST_JOSEPH_AND_ST_MARYS);
    disposeGameView(gameView);
    printf("passed!\n");

    printf("$$$$$   Tests on the revival of hunter (i.e: restore to full health in hospital after death)   $$$$$\n");
    gameView = newGameView("GMN.... SCD.... HMA.... MST.... DLV.V.. GLO.... SKL.... HSR.... MZU.... DSWT... GSWTD.. SBD.... HBO.... MMR.... DC?T... GSW.... SVI.... HBB.... MMS.... DC?T... GIR.... SPR.... HAO.... MCG.... DS?.... GLVV... SBR.... HIR.... MTS.... DS?.... GLV.... SBR.... HIR.... MTS.... DAO.... GIR.... SPR.... HAO.... MIO.... DS?.... GAO.... SVI.... HMS.... MAS.... DNAT.M. GMS.... SVE.... HTS.... MAS.... DC?T.M. GTS.... SAS.... HIO.... MAS.... DC?T... GIO.... SAS.... HAS.... MAS.... DC?T... GIO.... SAS.... HAS.... MAS.... DTOT... GAS.... SAS.... HAS.... MAS.... DC?.V.. GAS.... SAS.... HAS.... MAS.... DZUT.M. GVE.... SVE.... HVE.... MVE.... DC?T.M. GGO.... SGO.... HGO.... MGO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?T.M. GTS.... STS.... HTS.... MTS.... DPAT.M. GIO.... SIO.... HIO.... MIO.... DC?T.V. GIO.... SIO.... HIO.... MIO.... DC?T.M. GAS.... SAS.... HAS.... MAS.... DC?T.M. GIO.... SIO.... HIO.... MIO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?T.M. GTS.... STS.... HTS.... MTS.... DTOT.M. GIO.... SIO.... HIO.... MIO.... DC?T.M. GTS.... STS.... HTS.... MTS.... DC?.VM. GMS.... SMS.... HMS.... MMS.... DC?T.M. GAO.... SAO.... HAO.... MAO.... DC?T.M. GMS.... SMS.... HMS.... MMS.... DC?T.M. GMS.... SMS.... HMS.... MMS.... DPAT.M. GTS.... STS.... HTS.... MTS.... DCFT.M. GGO.... SGO.... HGO.... MGO.... DNAT.V. GMR.... SMR.... HMR.... MMR.... DBOT.M. GCFT... SPAT... HCF.... MGE.... DSRT.M. GTO.... SNAT... HPA.... MPA.... DBAT.M. GTO.... SNA.... HPA.... MPA.... DTOT... GSRT... SLE.... HBOT... MLE.... DMRT... GSN.... SBU.... HSR.... MBU.... DZUT... GBB.... SST.... HBAT... MCO.... DST.V.. GNA.... SNU.... HMS.... MHA.... DC?T... GCF.... SPR.... HMRT... MLI.... DC?T... GCF.... SPR.... HMR.... MLI.... DPAT.M. GGE.... SVI.... HMI.... MFR.... DCFT... GZUT... SMU.... HVE.... MSTV... DNAT... GST.... SST.... HMU.... MST.... DBOT... GGE.... SGE.... HMI.... MGE.... DSRT.M. GCFT... SPAT... HGO.... MPA.... DC?T.M. GCF.... SPA.... HGO.... MPA.... DTOT... GBOT... SMR.... HMR.... MNAT... DMRT... GSRT... SZU.... HTOT... MCF.... DZUT... GSR.... SMU.... HCF.... MMRT... DSTT... GTO.... SZA.... HPA.... MMI.... DC?.V.. GMR.... SVI.... HNA.... MMU.... DC?T.M. GMR.... SVI.... HNA.... MMU.... DPAT... GGE.... SPR.... HLE.... MVI.... DCFT... GGE.... SVI.... HPAT... MVE.... DC?T.M. GSTT... SBD.... HGE.... MGO.... DC?T... GZU.... SSZ.... HCFT... MMR.... DC?T.V. GMU.... SZA.... HMR.... MMI.... DC?T.M. GMU.... SZA.... HMR.... MMI.... DTOT... GVI.... SBD.... HMI.... MMU.... DMRT... GVE.... SVI.... HMI.... MMI.... DZUT.M. GGO.... SMU.... HMRT... MGE.... DSTT.M. GTS.... SZUT... HTOT... MPA.... DCOT.M. GRO.... SSTT... HCF.... MLE.... DBU.VM. GRO.... SST.... HCF.... MLE.... DPAT... GFL.... SPATD.. HNA.... MBUV... DC?T... GGO.... SPA.... HLE.... MCOT... DNAT... GVE.... SNATD.. HBU.... MHA.... DC?T... GMU....", messages1); 
    
    assert(gameView != NULL);
    assert(getHealth(gameView, PLAYER_DR_SEWARD) == GAME_START_HUNTER_LIFE_POINTS);
    assert(getLocation(gameView, PLAYER_DR_SEWARD) == ST_JOSEPH_AND_ST_MARYS);
    
    int numLocations;
    LocationID *possibleMoves = connectedLocations(gameView, &numLocations, ST_JOSEPH_AND_ST_MARYS, PLAYER_DR_SEWARD, 70, 0, 1, 1);
    assert(possibleMoves != NULL);
    assert(numLocations == 1);
    assert(possibleMoves[0] == ST_JOSEPH_AND_ST_MARYS);
    free(possibleMoves);

    possibleMoves = connectedLocations(gameView, &numLocations, ST_JOSEPH_AND_ST_MARYS, PLAYER_DR_SEWARD, 70, 1, 1, 1);
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
    
    assert(reachable[ST_JOSEPH_AND_ST_MARYS] == 1);
    assert(reachable[ZAGREB] == 1);
    assert(reachable[SARAJEVO] == 1);
    assert(reachable[SZEGED] == 1);
    assert(reachable[BELGRADE] == 1);
    assert(reachable[CASTLE_DRACULA] == 0); // Test for no connections !
    assert(reachable[BERLIN] == 0);         // Test for no connections !

    free(reachable);
    free(possibleMoves);
    disposeGameView(gameView);
    printf("passed!\n\n");
}
