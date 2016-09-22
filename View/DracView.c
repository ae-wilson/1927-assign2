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


     
struct dracView {
    Map g;
    GameView gameView;
    int turn;
    int score;
    int player;
    int *lastTurnHealth;
    int *health;
    LocationID **trail_perPlayer; // stores trail for each player in 2D array 
    PlayerMessage *ms;
};
    
//Private Functions
static PlayerID whichPlayer(char c);
static void validDracView(DracView dracView);
static void frontInsert(LocationID **trail_perPlayer, PlayerID player, char *location);

// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[])
{
    assert(pastPlays != NULL);
    assert(messages != NULL);

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
    dracView->trail_perPlayer = malloc(NUM_PLAYERS * sizeof(LocationID *));
    assert(dracView->trail_perPlayer != NULL);

    int i, j = 0;
    for(i = 0; i < NUM_PLAYERS; i++) {
        dracView->trail_perPlayer[i] = malloc(GAME_START_SCORE * sizeof(LocationID));
        assert(dracView->trail_perPlayer[i] != NULL);
 
        for(j = 0; j < GAME_START_SCORE; j++){
            dracView->trail_perPlayer[i][j] = UNKNOWN_LOCATION;
        }
    }


    //Update the state of the game
    dracView->turn = 1;
    dracView->score = GAME_START_SCORE;    

    for(i = 0; i < PLAYER_DRACULA; i++) {
        dracView->lastTurnHealth[i] = GAME_START_HUNTER_LIFE_POINTS;
        dracView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    dracView->lastTurnHealth[i] = GAME_START_BLOOD_POINTS;
    dracView->health[i] = GAME_START_BLOOD_POINTS;    


    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i += interval) {
        dracView->turn++;  //increase the turn number
        PlayerID player = whichPlayer(pastPlays[i]);  //find out which player

        char location[2];    //get the abbrev of locations
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];
        location[2] = '\0';

        frontInsert(dracView->trail_perPlayer, player, location); //update the trail_perPlayer

        //player = one of the hunters
        if(player != PLAYER_DRACULA) {
            for(j = i + 3; j < i + interval - 1; j++) {
                if(dracView->health[player] == 0) {
                    dracView->lastTurnHealth[player] = 0;
                    dracView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                } else {
                    dracView->lastTurnHealth[player] = dracView->health[player];
                }

                if(pastPlays[j] == '.') break;  //no encounters            

                //trigger the trap(s)
                if(pastPlays[j] == 'T') {
                    dracView->health[player] -= LIFE_LOSS_TRAP_ENCOUNTER;
                } else if(pastPlays[j] == 'D') {
                    //confront Dracula
                    dracView->health[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    dracView->health[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;   
                }

                //no more actions when HP gets to ZERO
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
            //player = Dracula
            if(pastPlays[i+5] == 'V') dracView->score -= SCORE_LOSS_VAMPIRE_MATURES;
           
            dracView->lastTurnHealth[player] = dracView->health[player];

            if(dracView->trail_perPlayer[player][0] == CASTLE_DRACULA || dracView->trail_perPlayer[player][0] == TELEPORT) {
                //gain HP as Dracula is in his castle

                dracView->health[player] += LIFE_GAIN_CASTLE_DRACULA;
            } else if(dracView->trail_perPlayer[player][0] >= MIN_MAP_LOCATION && dracView->trail_perPlayer[player][0] <= MAX_MAP_LOCATION) {
                //lose 2 HP when Dracula is at the sea

                if(idToType(dracView->trail_perPlayer[player][0]) == SEA) {
                    dracView->health[player] -= LIFE_LOSS_SEA;
                } 
            } else if(dracView->trail_perPlayer[player][0] == SEA_UNKNOWN) {
                dracView->health[player] -= LIFE_LOSS_SEA;
            } else if(dracView->trail_perPlayer[player][0] >= DOUBLE_BACK_1 && dracView->trail_perPlayer[player][0] <= DOUBLE_BACK_5) {
                int steps = dracView->trail_perPlayer[player][0] - DOUBLE_BACK_1 + 1;               

                if(dracView->trail_perPlayer[player][steps] == HIDE) steps += 1;
                assert(steps < GAME_START_SCORE);

                if(dracView->trail_perPlayer[player][steps] >= MIN_MAP_LOCATION && 
                   dracView->trail_perPlayer[player][steps] <= MAX_MAP_LOCATION) 
                {
                    if(idToType(dracView->trail_perPlayer[player][steps]) == SEA) {
                        dracView->health[player] -= LIFE_LOSS_SEA; 
                    }
                } else if(dracView->trail_perPlayer[player][steps] == SEA_UNKNOWN) {
                    dracView->health[player] -= LIFE_LOSS_SEA;
                } 
            }

            //score -= 1 when Dracula ends his turn
            dracView->score -= SCORE_LOSS_DRACULA_TURN;    
        }
    } 

    if(dracView->turn > 1) {
        dracView->ms = malloc((dracView->turn - 1) * sizeof(PlayerMessage));
        assert(dracView->ms != NULL);
        
        int k;
        for(k = 0; i < dracView->turn - 1; k++) {
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
    return currentView->trail_perPlayer[player][0];
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end)
{
    validDracView(currentView);
    return;
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps)
{
    validDracView(currentView);
    return;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    validDracView(currentView);
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
	validDracView(currentView);
	assert(numLocations != NULL);	// check that there are locations to visit

	// need to find out the current location of Dracula
	LocationID here = whereIs(currentView, PLAYER_DRACULA);

	// need to find out the current round
	Round turn = giveMeTheRound(currentView);

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

	// need to find out the current round
	Round turn = giveMeTheRound(currentView);

	// we will use the connectedLocations funciton in GameView.c to find
	// all the possible locations which Dracula can visit
	return connectedLocations(currentView->gameView, numLocations, there, player, turn, road, rail, sea);
}



// *** Private Functions ***

//check whether the given dracView is valid
static void validDracView(DracView dracView) {
    assert(dracView != NULL);
    assert(dracView->g != NULL);
    assert(dracView->gameView != NULL);
    assert(dracView->lastTurnHealth != NULL);
    assert(dracView->health != NULL);
    assert(dracView->trail_perPlayer != NULL);
    if(dracView->turn != 1) assert(dracView->ms != NULL);

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
