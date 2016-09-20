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
     Map g;
     GameView gameView;
     PlayerMessage *ms;
};
     

// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[])
{
    assert(pastPlays != NULL);
    assert(messages != NULL);

    HunterView hunterView = malloc(sizeof(struct hunterView));
    assert(hunterView != NULL);
    hunterView->g = newMap();
    assert(hunterView->g != NULL); 
    hunterView->gameView = newGameView(pastPlays, messages);
    assert(hunterView->gameView != NULL); 
    
    int turn = getRound(hunterView->gameView) * NUM_PLAYERS + getCurrentPlayer(hunterView->gameView); 
    if(turn > 0) {
        hunterView->ms = malloc(turn * sizeof(PlayerMessage));
        assert(hunterView->ms != NULL);
        
        int i;
        for(i = 0; i < turn; i++) {
            assert(hunterView->ms[i] != NULL);
            memset(hunterView->ms[i], 0, MESSAGE_SIZE);
            strcpy(hunterView->ms[i], messages[i]);
        }
    }

    return hunterView;
}
     
     
// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted)
{
    assert(toBeDeleted != NULL);
    assert(toBeDeleted->g != NULL);
    assert(toBeDeleted->gameView != NULL);

    int turn = getRound(toBeDeleted->gameView) * NUM_PLAYERS + getCurrentPlayer(toBeDeleted->gameView);  
    if(turn > 0) {
        assert(toBeDeleted->ms != NULL);  
        free(toBeDeleted->ms); 
    }

    disposeMap(toBeDeleted->g);   
    disposeGameView(toBeDeleted->gameView); 
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView)
{
    return getRound(currentView->gameView);
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView)
{
    return getCurrentPlayer(currentView->gameView);
}

// Get the current score
int giveMeTheScore(HunterView currentView)
{
    return getScore(currentView->gameView);
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player)
{
    return getHealth(currentView->gameView, player);
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player)
{
    return getLocation(currentView->gameView, player);
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    getHistory(currentView->gameView, player, trail);   
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations,
                        int road, int rail, int sea)
{
    PlayerID player = getCurrentPlayer(currentView->gameView);
    LocationID from = getLocation(currentView->gameView, player);
    Round round = getRound(currentView->gameView);

    LocationID *moves = connectedLocations(currentView->gameView, numLocations, from, player, round, road, rail, sea);
    return moves;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    //LocationID from = getLocation(currentView->gameView. player);
    //Round round = getRound(currentView->gameView);
    
    return NULL;
}
