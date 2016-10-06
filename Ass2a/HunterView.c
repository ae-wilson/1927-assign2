// Assignment 2a --- The View --- GameView
// Name: Amelia Wilson, John Cawood, Thomas Castagnone, Weng Chon Fan
// Tutorial Group: T09A 
// Tutor: Addo Wondo
// HunterView.c ... HunterView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h" 
     
struct hunterView {
    GameView gameView;            // the gameView
};

//Private Functions
static void validHunterView(HunterView hunterView);


// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[])
{
    assert(pastPlays != NULL);
    assert(messages != NULL);

    // Initialise the HunterView ADT
    HunterView hunterView = malloc(sizeof(struct hunterView));
    assert(hunterView != NULL);
    hunterView->gameView = newGameView(pastPlays, messages);
    assert(hunterView->gameView != NULL); 

    return hunterView;
}
     
     
// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted)
{
    validHunterView(toBeDeleted);
     
    disposeGameView(toBeDeleted->gameView);
    free(toBeDeleted); 
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView)
{
    validHunterView(currentView);
    return getRound(currentView->gameView);
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView)
{
    validHunterView(currentView);
    return getCurrentPlayer(currentView->gameView);
}

// Get the current score
int giveMeTheScore(HunterView currentView)
{
    validHunterView(currentView);
    return getScore(currentView->gameView);
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player)
{
    validHunterView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 
    return getHealth(currentView->gameView, player);
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player)
{
    validHunterView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 
    return getLocation(currentView->gameView, player);
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    validHunterView(currentView);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 
    getHistory(currentView->gameView, player, trail);   
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations,
                        int road, int rail, int sea)
{
    validHunterView(currentView);
    PlayerID player = getCurrentPlayer(currentView->gameView);
    assert(player != PLAYER_DRACULA);
    LocationID from = getLocation(currentView->gameView, player);
    Round round = getRound(currentView->gameView);

    if(from != UNKNOWN_LOCATION) {
        LocationID *moves = connectedLocations(currentView->gameView, numLocations, from, player, round, road, rail, sea);
        return moves;
    } else {
        // No moves made by current Player (at the beginning of the game)
        *numLocations = 0;
        return NULL;
    }
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    validHunterView(currentView);  
    assert(numLocations != NULL);   // check that there are locations to visit
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER);

    // need to find out the current location of the player
    LocationID there = whereIs(currentView, player);

    // if the player is Dracula and he is teleported to his castle
    if(player == PLAYER_DRACULA) {
        if(there == TELEPORT) there = CASTLE_DRACULA;
    }
    
    if(there < MIN_MAP_LOCATION || there > MAX_MAP_LOCATION) {
        // Dracula location is unknown (i.e: not a precise location 0...70)
        *numLocations = 0;
        return NULL;
    }
    

    // need to find out the current round
    Round round = giveMeTheRound(currentView);

    // we will use the connectedLocations funciton in GameView.c to find
    // all the possible locations which Dracula can visit
    return connectedLocations(currentView->gameView, numLocations, there, player, round, road, rail, sea);
}


// *** Private Functions ***

//check whether the given hunterView is valid
static void validHunterView(HunterView hunterView) {
    assert(hunterView != NULL);
    assert(hunterView->gameView != NULL);
}

