// GameView.c ... GameView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"

struct gameView {
   Map g;
   int turn;
   int score;
   int *health;
   LocationID **trail;     
   PlayerMessage *ms; 
}; 

static PlayerID whichPlayer(char c);
static void frontInsert(LocationID *trail, char *location);

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //Initialising the GameView ADT
    GameView gameView = malloc(sizeof(struct gameView));
    assert(gameView != NULL);
    gameView->g = newMap();
    assert(gameView->g != NULL);
    gameView->health = malloc(NUM_PLAYERS * sizeof(int));
    assert(gameView->health != NULL);
    gameView->trail = malloc(NUM_PLAYERS * sizeof(int *));
    assert(gameView->trail != NULL);
 
    int i, j = 0;
    for(i = 0; i < NUM_PLAYERS; i++) {
       gameView->trail[i] = malloc(TRAIL_SIZE * sizeof(int));
       assert(gameView->trail[i] != NULL);

       for(j = 0; j < TRAIL_SIZE; j++) {
          gameView->trail[i][j] = UNKNOWN_LOCATION;
       }
    }
     

    //Adding the state of the game into the ADT
    gameView->turn = 1;
    gameView->score = GAME_START_SCORE;    

    for(i = 0; i < PLAYER_DRACULA; i++) {
        gameView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    gameView->health[i] = GAME_START_BLOOD_POINTS;    

    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i = i + interval) {
        gameView->turn++;
        PlayerID player = whichPlayer(pastPlays[i]);
        assert(player != -1); //unknown player

        char location[2];
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];
        
        frontInsert(gameView->trail[player], abbrevToID(location));
    }  


    gameView->ms = &messages[0];
    assert(gameView->ms != NULL);     
       
    return gameView;
}
     
     
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    assert(toBeDeleted != NULL);
    assert(toBeDeleted->g != NULL);
    assert(toBeDeleted->health != NULL);

    disposeMap(toBeDeleted->g);
    free(toBeDeleted->health);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the current score
int getScore(GameView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations

LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return NULL;
}



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

    return id;

}

static void frontInsert(LocationID *trail, char *location) {
    assert(trail != NULL);
    
    int i;
    for(i = TRAIL_SIZE - 1; i > 0; i--) trail[i] = trail[i-1];

    trail[i] = id;
}


