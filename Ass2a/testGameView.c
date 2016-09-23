// testGameView.c ... test the GameView ADT

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "GameView.h"

static void UnitTest1(void);
static void UnitTest2(void);

int main()
{
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

    printf("\n=====   More Tests   =====\n\n");
    UnitTest1();    
    UnitTest2();

    return 0;
}


static void UnitTest1(void) {
    printf("#####   Unit Test 1   #####\n");   
    printf("pastPlays string: GPA.... SVI.... HCD....\n"); 
    printf("PlayerMessages: Tom, John, Milly\n");

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
    printf("pastPlays string: GCD.... SSR.... HST.... MMN.... DSO.V.. GGA.... SAL.... HBU.... MED.... DVAT... GCN.... SBA.... HAM.... MNS.... DSJT... GBS.... SM    S.... HBU.... MAM.... DZAT... GIO.... SAL.... HCO.... MNS.... DC?T... GAT.... SMS.... HAM.... MAO.... DC?T... GAT.... SMS.... HAM.... MAO.... DC?T.V. GAT.... SMR.... H    CO.... MMS.... DC?T.M. GAT.... SMR.... HCO.... MMS.... DC?T.M. GAT.... SPA....\n"); 
    printf("PlayerMessages: Hello x 47\n\n");   
  
    PlayerMessage *messages1 = malloc(47 * sizeof(PlayerMessage));
    assert(messages1 != NULL);

    int i, j = 0;
    for(i = 0; i < 47; i++) strcpy(messages1[i], "Hello");

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
    assert(numLocations == 11);
    reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
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



    disposeGameView(gameView);
    printf("passed!\n");
}
