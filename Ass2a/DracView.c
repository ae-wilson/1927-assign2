// Assignment 2a --- The View --- DracView
// Name: Amelia Wilson, John Cawood, Thomas Castagnone, Weng Chon Fan
// Tutorial Group: T09A 
// Tutor: Addo Wondo
// DracView.c ... DracView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "DracView.h"
#include "Queue.h"

#define TRUE 1
#define FALSE 0
  

typedef struct vNode *VList;
struct vNode {
   LocationID  v;    // ALICANTE, etc
   TransportID type; // ROAD, RAIL, BOAT
   VList       next; // link to next node
};

struct MapRep {
   int   nV;         // #vertices
   int   nE;         // #edges
   VList connections[NUM_MAP_LOCATIONS]; // array of lists
};
   
struct dracView {
    GameView gameView;            // The game view
    
    int *numIV;                   // Number of immature vampire [0..1] at specific location [0..70]
    int *numTrap;                 // Number of traps [0..3] at specific location [0..70]
    LocationID **trail_perPlayer; // stores trail for each player in 2D array 
};
    
//Private Functions
static PlayerID whichPlayer(char c);
static void validDracView(DracView dracView);
static void frontInsert(LocationID **trail_perPlayer, PlayerID player, char *location);
static void removeLocation(int *numLocations, LocationID *connLoc, LocationID v, int pos);

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    assert(pastPlays != NULL);
    assert(messages != NULL);

    //Initialise the DracView ADT
    DracView dracView = malloc(sizeof(struct dracView));
    assert(dracView != NULL);
    dracView->gameView = newGameView(pastPlays, messages);
    assert(dracView->gameView != NULL); 
    dracView->numTrap = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(dracView->numTrap != NULL);
    dracView->numIV = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(dracView->numIV != NULL);
    dracView->trail_perPlayer = malloc(NUM_PLAYERS * sizeof(LocationID *));
    assert(dracView->trail_perPlayer != NULL); 

    int i, j = 0; 
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dracView->numTrap[i] = 0;
        dracView->numIV[i] = 0;
    }

    for(i = 0; i < NUM_PLAYERS; i++) {
        dracView->trail_perPlayer[i] = malloc(GAME_START_SCORE * sizeof(LocationID));
        assert(dracView->trail_perPlayer[i] != NULL);
 
        for(j = 0; j < GAME_START_SCORE; j++){
            dracView->trail_perPlayer[i][j] = UNKNOWN_LOCATION;
        }
    }

    // Obtain useful data form pastPlays string
    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i += interval) {
        PlayerID player = whichPlayer(pastPlays[i]);  // find out which player

        char *location = malloc(4 * sizeof(char));    // get the abbrev of locations
        assert(location != NULL);
        for(j = 0; j < 4; j++) location[j] = 0;
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];

        frontInsert(dracView->trail_perPlayer, player, location); // update the trail_perPlayer
        free(location);

        //player = one of the hunters
        if(player != PLAYER_DRACULA) {
            for(j = i + 3; j < i + interval - 1; j++) {

                if(pastPlays[j] == '.') break;  // No more encounters this turn           

                // Trigger the trap(s)
                LocationID currLoc = dracView->trail_perPlayer[player][0];
                assert(currLoc >= MIN_MAP_LOCATION && currLoc <= MAX_MAP_LOCATION);

                if(pastPlays[j] == 'T') {

                    // Disarm the trap and lose HP
                    assert(dracView->numTrap[currLoc] > 0);
                    assert(idToType(currLoc) != SEA);
                    dracView->numTrap[currLoc] -= 1;
                } else if(pastPlays[j] == 'V') {
 
                    // Discover an immature vampire and kill it
                    assert(idToType(currLoc) != SEA);
                    assert(dracView->numIV[currLoc] == 1);
                    dracView->numIV[currLoc] -= 1;
                } else if(pastPlays[j] == 'D') {

                    // Confront Dracula
                    assert(idToType(currLoc) != SEA);
                }
            }

        } else {

            // player = Dracula     
            // Make sure all the locations in Dracula trail are precise location (i.e: 0..70)   
            int pos = 0;
            if(dracView->trail_perPlayer[player][0] == HIDE) {
                pos = 1;
                if(dracView->trail_perPlayer[player][1] >= DOUBLE_BACK_1 && dracView->trail_perPlayer[player][1] <= DOUBLE_BACK_5) {
                    pos += dracView->trail_perPlayer[player][1] - DOUBLE_BACK_1 + 1;
                }

                if(dracView->trail_perPlayer[player][pos] == TELEPORT) {
                    dracView->trail_perPlayer[player][0] = CASTLE_DRACULA;
                } else {
                    assert(dracView->trail_perPlayer[player][pos] >= MIN_MAP_LOCATION && dracView->trail_perPlayer[player][pos] <= MAX_MAP_LOCATION);
                    assert(idToType(dracView->trail_perPlayer[player][pos]) != SEA);
                    dracView->trail_perPlayer[player][0] = dracView->trail_perPlayer[player][pos];
                }
            } else if(dracView->trail_perPlayer[player][0] >= DOUBLE_BACK_1 && dracView->trail_perPlayer[player][0] <= DOUBLE_BACK_5) {
                pos = dracView->trail_perPlayer[player][0] - DOUBLE_BACK_1 + 1;
                if(dracView->trail_perPlayer[player][pos] == HIDE) pos++;
                
                if(dracView->trail_perPlayer[player][pos] == TELEPORT) {
                    dracView->trail_perPlayer[player][0] = CASTLE_DRACULA;
                } else {
                    assert(dracView->trail_perPlayer[player][pos] >= MIN_MAP_LOCATION && dracView->trail_perPlayer[player][pos] <= MAX_MAP_LOCATION);
                    dracView->trail_perPlayer[player][0] = dracView->trail_perPlayer[player][pos];
                }
            } else if(dracView->trail_perPlayer[player][0] == TELEPORT) {
                dracView->trail_perPlayer[player][0] = CASTLE_DRACULA;
            }
               
            LocationID currLoc = dracView->trail_perPlayer[player][0];
            assert(currLoc >= MIN_MAP_LOCATION && currLoc <= MAX_MAP_LOCATION);

            if(pastPlays[i+5] == 'M') {
                // The Trap malfunctions
                LocationID leaveTrail = dracView->trail_perPlayer[player][6];
                assert(leaveTrail >= MIN_MAP_LOCATION && leaveTrail <= MAX_MAP_LOCATION);
                assert(idToType(leaveTrail) != SEA);
                assert(dracView->numTrap[leaveTrail] > 0);
                dracView->numTrap[leaveTrail] -= 1;
            }

            if(pastPlays[i+3] == 'T') {
                // Place a trap
                assert(idToType(currLoc) != SEA);       //it is impossible for Dracula to place encounter(s) when he is at the sea
                assert(dracView->numTrap[currLoc] < 3);
                dracView->numTrap[currLoc] += 1;
            } 

            if(pastPlays[i+4] == 'V') {
                //places an immature vampire
                assert(idToType(currLoc) != SEA);
                assert(dracView->numIV[currLoc] == 0);
                dracView->numIV[currLoc] = 1;
            }

            if(pastPlays[i+5] == 'V') {
                //An immature vampire ---> wakes up and become mature vampire 
                LocationID whereIV = dracView->trail_perPlayer[player][6];
                assert(whereIV >= MIN_MAP_LOCATION && whereIV <= MAX_MAP_LOCATION);
                assert(idToType(whereIV) != SEA);
                assert(dracView->numIV[whereIV] == 1);
                dracView->numIV[whereIV] = 0;
            }
        } 

    }

    return dracView;
}
     
     
// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    validDracView(toBeDeleted);

    int i;
    for(i = 0; i < NUM_PLAYERS; i++) {
        free(toBeDeleted->trail_perPlayer[i]);
    }

    disposeGameView(toBeDeleted->gameView);
    free(toBeDeleted->trail_perPlayer); 
    free(toBeDeleted->numTrap);
    free(toBeDeleted->numIV);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView)
{
    validDracView(currentView);
    return getRound(currentView->gameView);
}

// Get the current score
int giveMeTheScore(DracView currentView)
{
    validDracView(currentView);
    return getScore(currentView->gameView);
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player)
{
    validDracView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 
    return getHealth(currentView->gameView, player);
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player)
{
    validDracView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
  
    // Make sure the returned location is a precise location
    if(player != PLAYER_DRACULA) return getLocation(currentView->gameView, player);

    return currentView->trail_perPlayer[player][0];
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end)
{
    validDracView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    assert(start != NULL && end != NULL);
    *start = currentView->trail_perPlayer[player][1];
    *end = currentView->trail_perPlayer[player][0]; 
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps)
{
    validDracView(currentView);
    assert(where >= MIN_MAP_LOCATION && where <= MAX_MAP_LOCATION);
    *numTraps = currentView->numTrap[where];
    *numVamps = currentView->numIV[where];
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    validDracView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    assert(trail != NULL);

    int i;
    for(i = TRAIL_SIZE - 1; i >= 0; i--) {
        LocationID loc = currentView->trail_perPlayer[player][i];
        assert((loc >= MIN_MAP_LOCATION && loc <= MAX_MAP_LOCATION) || loc == UNKNOWN_LOCATION);
        trail[i] = currentView->trail_perPlayer[player][i];
    }
}


// Fill the trail array with location IDs and also special moves
// if the player is Dracula (i.e: D1, HI, TP, ......)
// ** Similar to the function giveMeTheTrail, but this function won't 
//    return precise locations
void giveMeTheMoves(DracView currentView, PlayerID player, LocationID trail[TRAIL_SIZE]) {
    assert(currentView != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    assert(trail != NULL);

    getHistory(currentView->gameView, player, trail);
}


//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
    validDracView(currentView);
    assert(numLocations != NULL);	// check that there are locations to visit

    LocationID here = whereIs(currentView, PLAYER_DRACULA);

    // need to find out the current round
    Round round = giveMeTheRound(currentView);

    // we will use the connectedLocations funciton in GameView.c to find
    // all the possible locations which Dracula can visit
    LocationID *connLoc = connectedLocations(currentView->gameView, numLocations, here, 
                          PLAYER_DRACULA, round, road, 0, sea);

    // get Dracula's trail
    LocationID *dracTrail = malloc(TRAIL_SIZE * sizeof(LocationID));
    assert(dracTrail != NULL);
    getHistory(currentView->gameView, PLAYER_DRACULA, dracTrail);

    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) {
        if(dracTrail[i] == TELEPORT) dracTrail[i] = CASTLE_DRACULA;
    }

    int counterA = 0;
    int counterB = 0;
    while(counterA < *numLocations) {
        counterB = 0;
        //Trail size - 1 as the last place in Dracula's trail is removed first
        while(counterB < TRAIL_SIZE - 1 && counterA < *numLocations) {   
            if(connLoc[counterA] == dracTrail[counterB] && connLoc[counterA] != here) {
                // if the location is in Dracula's trail and not where he currently is
                removeLocation(numLocations, connLoc, connLoc[counterA], counterA);
                counterB = 0;   //after the removal, compare the location again with the trail
            } else {
                counterB++;
            }
        }

        counterA++;
    }

    assert(*numLocations >= 1);
    free(dracTrail);
    return connLoc;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    validDracView(currentView);
    assert(numLocations != NULL);	// check that there are locations to visit
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 

    // Use whereCanIgo if the player is Dracula
    if(player == PLAYER_DRACULA) return whereCanIgo(currentView, numLocations, road, sea);

    // need to find out the current location of the player (hunter)
    LocationID there = whereIs(currentView, player);
    assert(there >= MIN_MAP_LOCATION && there <= MAX_MAP_LOCATION);

    // need to find out the current round
    Round round = giveMeTheRound(currentView);

    // we will use the connectedLocations funciton in GameView.c to find
    // all the possible locations which Dracula can visit
    return connectedLocations(currentView->gameView, numLocations, there, player, round, road, rail, sea);
}


// *** Private Functions ***

//check whether the given dracView is valid
static void validDracView(DracView dracView) {
    assert(dracView != NULL);
    assert(dracView->gameView != NULL);
    assert(dracView->numTrap != NULL);
    assert(dracView->numIV != NULL);
    assert(dracView->trail_perPlayer != NULL);

    int i;
    for(i = 0; i < NUM_PLAYERS; i++) assert(dracView->trail_perPlayer[i] != NULL);
}

// Returns Id of current player
static PlayerID whichPlayer(char c) {
    PlayerID id;   

    switch(c) {
    case 'G': id = PLAYER_LORD_GODALMING; break;
    case 'S': id = PLAYER_DR_SEWARD; break;
    case 'H': id = PLAYER_VAN_HELSING; break;
    case 'M': id = PLAYER_MINA_HARKER; break;
    case 'D': id = PLAYER_DRACULA; break;
    default: id = -1; break;
    }

    assert(id >= PLAYER_LORD_GODALMING && id <= PLAYER_DRACULA);
    return id;
}

// Inserts previous location to trail accordingly 
static void frontInsert(LocationID **trail_perPlayer, PlayerID player, char *location) {

    assert(trail_perPlayer != NULL);
    assert(trail_perPlayer[player] != NULL);
    assert(location != NULL);    

    char *db[5] = {"D1", "D2", "D3", "D4", "D5"};
   
    LocationID id = UNKNOWN_LOCATION;
   
    if(strcmp(location, "C?") == 0) {
        id = CITY_UNKNOWN;
    } else if(strcmp(location, "S?") == 0) {
        id = SEA_UNKNOWN;
    } else if(strcmp(location, "HI") == 0) {
        id = HIDE;
    } else if(strcmp(location, "TP") == 0) {
        id = TELEPORT;
    } else {
       int i;
       for(i = 1; i <= 5; i++) {
           if(strcmp(location, db[i-1]) == 0) break;
       }

       if(i == 1) {
           id = DOUBLE_BACK_1;
       } else if(i == 2) {
           id = DOUBLE_BACK_2;
       } else if(i == 3) {
           id = DOUBLE_BACK_3;
       } else if(i == 4) {
           id = DOUBLE_BACK_4;
       } else if(i == 5) {
           id = DOUBLE_BACK_5;
       } else {
           id = abbrevToID(location);
       }
    }

    assert((id >= UNKNOWN_LOCATION && id <= MAX_MAP_LOCATION) || (id >= CITY_UNKNOWN && id <= TELEPORT));

    int j;
    for(j = GAME_START_SCORE - 1; j > 0; j--) {
        trail_perPlayer[player][j] = trail_perPlayer[player][j-1];
    }

    trail_perPlayer[player][j] = id;
}

// Remove the location from the connLoc array
static void removeLocation(int *numLocations, LocationID *connLoc, LocationID v, int pos){
    assert(numLocations != NULL);
    assert(connLoc != NULL);
    assert(pos >= 0 && pos < *numLocations);

    int i;
    for(i = pos; i < *numLocations - 1; i++) {
        connLoc[i] = connLoc[i+1];
    }

    *numLocations = *numLocations - 1;
}


