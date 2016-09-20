// DracView.c ... DracView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
// #include "Map.h" ... if you decide to use the Map ADT
     
struct dracView {
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    int hello;
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
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
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
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return 0;
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
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    // NOT FINISHED YET!!!!
    // STILL WORKING ON IT!!!!
    /*assert(currentView != NULL);
    assert(numLocations != NULL);
    
    LocationID *whereCanIgo = malloc(sizeof(LocationID) * NUM_MAP_LOCATIONS);
    int numOfLoc = 0;
    whereCanIgo[numOfLoc] = 0;
    VList curr;
    while(curr != NULL){
         if(road == TRUE && curr->type == ROAD){
              assert(curr != ST_JOSEPH_AND_ST_MARYS);
              whereCanIgo[numOfLoc++] = curr->v;
         } else if(sea == TRUE && curr->type == SEA){
              // lose 2 blood points
              whereCanIgo[numOfLoc++] = curr->v;
         }
         curr = curr->next;
    }
    *numLocations = numOfLoc;
    return whereCanIgo;*/
    return NULL;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    // NOT FINISHED!!
    // STILL WORKING ON IT!!!
  /*  assert(currentView != NULL);
    assert(numLocations != NULL);
    LocationID *whereCanTheyGo = malloc(sizeof(LocationID)*NUM_MAP_LOCATIONS);
    int numOfLoc = 0;
    whereCanTheyGo[numOfLoc] = 0;
    int railMoves = (round + player) % 4;
    VList curr;
    while(curr != NULL){
         if(road == TRUE && curr->type == ROAD){
               whereCanTheyGo[numOfLoc++] = curr->v;
          } else if(rail == TRUE && curr->type == RAIL){
               // check for railmoves
               whereCanTheyGo[numOfLoc++] = curr->v;
          } else if(sea == TRUE && curr->type == SEA){
               whereCanTheyGo[numOfLoc++] = curr->v;
          }
          curr = curr->next;
    }
    *numLocations = numOfLoc;
    //return NULL;
    return whereCanTheyGo;*/
    return NULL;
}
