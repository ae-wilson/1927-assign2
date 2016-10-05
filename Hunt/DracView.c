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
    Map g;                        // The Map
    GameView gameView;            // The game view
    int turn;                     // Turn number
    int score;                    // Game score
    int *lastTurnHealth;          // Players' health in last turn
    int *health;                  // Players' health in this turn

    int *numTrap;                 // Number of traps [0..3] at specific location [0..70]
    int *numIV;                   // Number of immature vampire [0..1] at specific location [0..70]
    LocationID **trail_perPlayer; // stores trail for each player in 2D array 
    PlayerMessage *ms;            // Player Messages
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
    dracView->g = newMap();
    assert(dracView->g != NULL); 
    dracView->gameView = newGameView(pastPlays, messages);
    assert(dracView->gameView != NULL); 
    dracView->lastTurnHealth = malloc(NUM_PLAYERS * sizeof(int));
    assert(dracView->lastTurnHealth != NULL);
    dracView->health = malloc(NUM_PLAYERS * sizeof(int));
    assert(dracView->health != NULL);
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


    // Game data at the beginning of the game
    dracView->turn = 1;
    dracView->score = GAME_START_SCORE;    

    for(i = 0; i < PLAYER_DRACULA; i++) {
        dracView->lastTurnHealth[i] = GAME_START_HUNTER_LIFE_POINTS;
        dracView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    dracView->lastTurnHealth[i] = GAME_START_BLOOD_POINTS;
    dracView->health[i] = GAME_START_BLOOD_POINTS;    

    // Obtain useful data form pastPlays string
    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i += interval) {
        dracView->turn++;                             // increase the turn number
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
                if(dracView->health[player] == 0) {
                    dracView->lastTurnHealth[player] = 0;
                    dracView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                } else {
                    dracView->lastTurnHealth[player] = dracView->health[player];
                }

                if(pastPlays[j] == '.') break;  // No more encounters this turn           

                // Trigger the trap(s)
                LocationID currLoc = dracView->trail_perPlayer[player][0];
                assert(currLoc >= MIN_MAP_LOCATION && currLoc <= MAX_MAP_LOCATION);

                if(pastPlays[j] == 'T') {

                    // Disarm the trap and lose HP
                    assert(dracView->numTrap[currLoc] > 0);
                    assert(idToType(currLoc) != SEA);
                    dracView->health[player] -= LIFE_LOSS_TRAP_ENCOUNTER; 
                    dracView->numTrap[currLoc] -= 1;
                } else if(pastPlays[j] == 'V') {
 
                    // Discover an immature vampire and kill it
                    assert(idToType(currLoc) != SEA);
                    assert(dracView->numIV[currLoc] == 1);
                    dracView->numIV[currLoc] -= 1;
                } else if(pastPlays[j] == 'D') {

                    // Confront Dracula
                    assert(idToType(currLoc) != SEA);
                    dracView->health[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    dracView->health[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;   
                }

                // No more actions when HP gets to ZERO --> teleported to hospital
                if(dracView->health[player] <= 0) {
                    dracView->health[player] = 0;
                    dracView->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                    break;
                } 
            }
            
            if((dracView->trail_perPlayer[player][0] == dracView->trail_perPlayer[player][1]) && 
                dracView->health[player] > 0) {

                //take rest or research at the same location
                dracView->health[player] += LIFE_GAIN_REST;

                //make sure Hunter HP does not exceed the limit (HP: 9)
                if(dracView->health[player] > GAME_START_HUNTER_LIFE_POINTS) {
                    dracView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
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
                dracView->score -= SCORE_LOSS_VAMPIRE_MATURES;
            }
 
            //Update the health point at the end of turns
            dracView->lastTurnHealth[player] = dracView->health[player];
            if(currLoc == CASTLE_DRACULA) {

                //gain HP as Dracula is in his castle
                dracView->health[player] += LIFE_GAIN_CASTLE_DRACULA;
            } else if(idToType(currLoc) == SEA) {
                // lose 2 HP when Dracula is at the sea
                dracView->health[player] -= LIFE_LOSS_SEA;
                
            } 

            //score - 1 when Dracula finishes his turn
            dracView->score -= SCORE_LOSS_DRACULA_TURN;    
        }
    } 


    //The current Player got killed by Dracula in last turn, and this turn the hunter's health 
    //is restored to full health, and allowed to leave the hospital
    PlayerID currentPlayer = getCurrentPlayer(dracView->gameView);
    if(currentPlayer != PLAYER_DRACULA) {
        if(dracView->health[currentPlayer] == 0) {
            dracView->lastTurnHealth[currentPlayer] = 0;
            dracView->health[currentPlayer] = GAME_START_HUNTER_LIFE_POINTS;
        }
    }

    //Copy Player messages into the DracView ADT
    if(dracView->turn > 1) {
        dracView->ms = malloc((dracView->turn - 1) * sizeof(PlayerMessage));
        assert(dracView->ms != NULL);
        
        int k;
        for(k = 0; k < dracView->turn - 1; k++) {
            assert(dracView->ms[k] != NULL);
            memset(dracView->ms[k], 0, MESSAGE_SIZE);
            strcpy(dracView->ms[k], messages[k]);
        }
    }

    return dracView;
}
     
     
// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    validDracView(toBeDeleted);

    if(toBeDeleted->turn > 1) {
        assert(toBeDeleted->ms != NULL);
        free(toBeDeleted->ms);
    }

    int i;
    for(i = 0; i < NUM_PLAYERS; i++) {
        free(toBeDeleted->trail_perPlayer[i]);
    }

    disposeMap(toBeDeleted->g);   
    disposeGameView(toBeDeleted->gameView);
    free(toBeDeleted->trail_perPlayer); 
    free(toBeDeleted->lastTurnHealth);
    free(toBeDeleted->health);
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


LocationID *adjacentLocations(DracView currentView, int *numLocations) {
    validDracView(currentView);
    assert(numLocations != NULL);

    Round round = giveMeTheRound(currentView);
    LocationID loc = whereIs(currentView, PLAYER_DRACULA);
    return connectedLocations(currentView->gameView, numLocations, loc, PLAYER_DRACULA, round, 1, 0, 1);
    
}

LocationID *shortestPath(DracView currentView, int *length, LocationID start, LocationID end, 
                         int road, int sea)
{
    validDracView(currentView);
    assert(length != NULL);

    if(start < MIN_MAP_LOCATION || start > MAX_MAP_LOCATION) {
        *length = 0;
        return NULL;
    }

    if(end < MIN_MAP_LOCATION || end > MAX_MAP_LOCATION) {
        *length = 0;
        return NULL;
    }

    if(start == end) {
        *length = 1;
        LocationID *sPath = malloc(*length * sizeof(LocationID));
        assert(sPath != NULL);
        sPath[0] = start;
        return sPath;
    }

    LocationID dracMoves[TRAIL_SIZE];

    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) dracMoves[i] = UNKNOWN_LOCATION; 
    giveMeTheMoves(currentView, PLAYER_DRACULA, dracMoves);

    int hasDB = FALSE;
    for(i = 0; i < TRAIL_SIZE - 1; i++) {
        if(dracMoves[i] >= DOUBLE_BACK_1 && dracMoves[i] <= DOUBLE_BACK_5) {
            hasDB = TRUE;
            break;
        }
    } 


    int maxCost = 99999999;
    int dist[NUM_MAP_LOCATIONS];
    int pred[NUM_MAP_LOCATIONS];
    int visited[NUM_MAP_LOCATIONS];

    int forbidden[NUM_MAP_LOCATIONS];

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dist[i] = maxCost;
        pred[i] = UNKNOWN_LOCATION;
        visited[i] = 0;
     
        forbidden[i] = 0;
    }
    dist[start] = 0;
    pred[start] = start;

    if(hasDB == TRUE) {
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] >= MIN_MAP_LOCATION && dracMoves[i] <= MAX_MAP_LOCATION 
               && dracMoves[i] != start) 
            {
                LocationID s = dracMoves[i];
                forbidden[s] = 1;
            } else if(dracMoves[i] == TELEPORT) {
                forbidden[CASTLE_DRACULA] = 1;
            }
        }
    }


    Queue q = newQueue();
    assert(q != NULL);
    enterQueue(q, start);

    while(!emptyQueue(q)) {
        LocationID loc = leaveQueue(q);

        if(visited[loc]) continue;

        visited[loc] = 1;

        int cost = 1;
        int numLocations = 0;
        LocationID *connLoc = connectedLocations(currentView->gameView, &numLocations, loc,
                                                 PLAYER_DRACULA, giveMeTheRound(currentView),
                                                 road, 0, sea);
        assert(connLoc != NULL);

        for(i = 0; i < numLocations; i++) {
            if(forbidden[connLoc[i]]) continue;
            if(!visited[connLoc[i]]) enterQueue(q, connLoc[i]);
       
            if(dist[loc] + cost < dist[connLoc[i]]) { 
                dist[connLoc[i]] = dist[loc] + cost;         
                pred[connLoc[i]] = loc;
            }
        }
        
        free(connLoc);
    }

    disposeQueue(q);

    
    if(pred[end] == UNKNOWN_LOCATION) {
        *length = 0;
        return NULL;
    } 


    int count = 0;
    for(i = end; i != start; i = pred[i]) {
        count++;
    }
    count++;

    *length = count;
    LocationID *sPath = malloc(*length * sizeof(LocationID));
    assert(sPath != NULL);

    count--;
    for(i = end; i != start && count > 0; i = pred[i]) {
        sPath[count--] = i;
    }
    sPath[0] = start;

    return sPath;
}


LocationID *sPathForHunters(DracView currentView, int *length, PlayerID player, LocationID start, LocationID end,
                         int road, int rail, int sea) 
{
    validDracView(currentView);
    assert(length != NULL);

    if(start < MIN_MAP_LOCATION || start > MAX_MAP_LOCATION) {
        *length = 0;
        return NULL;
    }

    if(end < MIN_MAP_LOCATION || end > MAX_MAP_LOCATION) {
        *length = 0;
        return NULL;
    }

    if(start == end) {
        *length = 1;
        LocationID *sPath = malloc(*length * sizeof(LocationID));
        assert(sPath != NULL);
        sPath[0] = start;
        return sPath;
    }
   
    int i = 0;
    int cost = 1;
    int maxCost = 99999999;
    int dist[NUM_MAP_LOCATIONS], pred[NUM_MAP_LOCATIONS];
    int visited[NUM_MAP_LOCATIONS];

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        dist[i] = maxCost;
        pred[i] = UNKNOWN_LOCATION;
        visited[i] = 0;
    }
    dist[start] = 0;
    pred[start] = start;


    Queue *qList = malloc(NUM_MAP_LOCATIONS * sizeof(Queue));
    assert(qList != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        qList[i] = newQueue();
        assert(qList[i] != NULL);
    }
    enterQueue(qList[0], start);


    Round round = giveMeTheRound(currentView);

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        while(!emptyQueue(qList[i])) {
            LocationID s = leaveQueue(qList[i]);

            if(visited[s]) continue;
            visited[s] = 1;

            int numLocations = 0;
            LocationID *connLoc = connectedLocations(currentView->gameView, &numLocations,
                                                     s, player, round + i, road, rail, sea);
            assert(connLoc != NULL);
             
            int j = 0;
            for(j = 0; j < numLocations; j++) {
                if(i + 1 < NUM_MAP_LOCATIONS) enterQueue(qList[i+1], connLoc[j]);

                if(dist[s] + cost < dist[connLoc[j]]) {
                    dist[connLoc[j]] = dist[s] + cost;
                    pred[connLoc[j]] = s;
                }
            }

            free(connLoc);
        }
    }    

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        assert(qList[i] != NULL);
        disposeQueue(qList[i]);
    }
    free(qList);

    if(pred[end] == UNKNOWN_LOCATION) {
        *length = 0;
        return NULL;
    }



    int count = 0;
    for(i = end; i != start; i = pred[i]) {
        count++;
    }
    count++;
    *length = count;

    LocationID *sPath = malloc(*length * sizeof(LocationID));
    assert(sPath != NULL);

    count--;
    for(i = end; i != start && count > 0; i = pred[i]) {
        sPath[count--] = i;
    } 
    sPath[0] = start;

    return sPath;
}


// *** Private Functions ***

//check whether the given dracView is valid
static void validDracView(DracView dracView) {
    assert(dracView != NULL);
    assert(dracView->g != NULL);
    assert(dracView->gameView != NULL);
    assert(dracView->lastTurnHealth != NULL);
    assert(dracView->health != NULL);
    assert(dracView->numTrap != NULL);
    assert(dracView->numIV != NULL);
    assert(dracView->trail_perPlayer != NULL);
    if(dracView->turn > 1) assert(dracView->ms != NULL);

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


