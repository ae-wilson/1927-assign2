// DracView.c ... DracView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "DraView.h"
// #include "Map.h" ... if you decide to use the Map ADT
     
struct dracView {
   int hello;
   Map g;
   int turn;
   int score;
   int *health;
   LocationID **trail_perPlayer; // stores trail for each player in 2D array
   GameView gameView;
};
     

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    DracView dracView = malloc(sizeof(struct dracView));
    dracView->hello = 42;
    return dracView;
}
     
     
// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted)
{
    //COMPLETE THIS IMPLEMENTATION
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView)
{
    Round round = (Round) ((currentView->turn - 1) / 5);
    return round;
}

// Get the current score
int giveMeTheScore(DracView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player)
{
    return currentView->trail_perPlayer[player][0];
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return;
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
	assert(currentView != NULL);	// check that there is currently a player
	assert(currentView->g != NULL);	// check that there is a map in play
	assert(numLocations != NULL);	// check that there are locations to visit

	// need to find out the current location of Dracula
	LocationID here = whereIs(currentView, PLAYER_DRACULA);
	assert(here != NULL);

	// need to find out the current round
	Round turn = giveMeTheRound(currentView);
	assert(turn != NULL);

	// dracula cannot move by rail hence
	int rail = 0;

	// we will use the connectedLocations funciton in GameView.c to find
	// all the possible locations which Dracula can visit
	return connectedLocations(currentView->gameView, numLocations, here, PLAYER_DRACULA, turn, road, rail, sea);
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
	assert(currentView != NULL);	// check that there is currently a player
	assert(currentView->g != NULL);	// check that there is a map in play
	assert(numLocations != NULL);	// check that there are locations to visit
	assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER); 

	// need to find out the current location of the player
	LocationID there = whereIs (currentView, player);
	assert(there != NULL);

	// need to find out the current round
	Round turn = giveMeTheRound(currentView);
	assert(turn != NULL);

	// we will use the connectedLocations funciton in GameView.c to find
	// all the possible locations which Dracula can visit
	return connectedLocations(currentView->gameView, numLocations, there, player, turn, road, rail, sea);
}
