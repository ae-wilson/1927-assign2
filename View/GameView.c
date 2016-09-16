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

//Private functions
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
     
    //Update the state of the game
    gameView->turn = 1;
    gameView->score = GAME_START_SCORE;    

    for(i = 0; i < PLAYER_DRACULA; i++) {
        gameView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    gameView->health[i] = GAME_START_BLOOD_POINTS;    

    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i += interval) {
        gameView->turn++;  //increase the turn number
        PlayerID player = whichPlayer(pastPlays[i]);  //find out which player

        char *location = malloc(2 * sizeof(char));    //get the abbrev of locations
        assert(location != NULL);
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];
 
        frontInsert(gameView->trail[player], location); //update the trail
        free(location);

        //player = one of the hunters
        if(player < PLAYER_DRACULA) {
            //if HP = 0 in last turn, HP = 9 in this turn 
            if(gameView->health[player] == 0) {
                gameView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
            }

            for(j = i + 3; j < i + interval - 1; j++) {
                if(pastPlays[j] == '.') break;  //no encounters            

                //trigger the trap(s)
                if(pastPlays[j] == 'T') {
                    gameView->health[player] -= LIFE_LOSS_TRAP_ENCOUNTER;
                } else if(pastPlays[j] == 'D') {
                    //confront Dracula
                    gameView->health[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    gameView->health[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;   
                }

                //no more actions when HP gets to ZERO
                if(gameView->health[player] <= 0) {
                    gameView->health[player] = 0;
                    gameView->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                    break;
                } 
            }
            
            if((gameView->trail[player][0] == gameView->trail[player][1]) && 
                gameView->health[player] > 0) {
                //take rest at the same location
                gameView->health[player] += LIFE_GAIN_REST;

                //make sure Hunter HP does not exceed the limit (HP: 9)
                if(gameView->health[player] > GAME_START_HUNTER_LIFE_POINTS) {
                    gameView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                }
            } 
            
        } else {
            //player = Dracula
            if(pastPlays[i+5] == 'V') gameView->score -= SCORE_LOSS_VAMPIRE_MATURES;
           
            if(gameView->trail[player][0] == CASTLE_DRACULA) {
                //gain HP as Dracula is in his castle
                gameView->health[player] += LIFE_GAIN_CASTLE_DRACULA;
            } else if(gameView->trail[player][0] >= ADRIATIC_SEA && gameView->trail[player][0] <= ZURICH) {
                //lose 2 HP when Dracula is at the sea
                if(idToType(gameView->trail[player][0]) == SEA) gameView->health[player] -= LIFE_LOSS_SEA;
            }

            //score -= 1 when Dracula ends his turn
            gameView->score -= SCORE_LOSS_DRACULA_TURN;    
        }
    }  
     
    
    /*printf("Messages: \n");
    int y = 0;
    for(y = 0; y < gameView->turn - 1; y++) printf("%s\n", messages[y]); */ //ignore this in this C file

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
    Round round = (Round) ((currentView->turn - 1) / 5);
    return round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    PlayerID player = (PlayerID) ((currentView->turn - 1) % NUM_PLAYERS);
    return player;
}

// Get the current score
int getScore(GameView currentView)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return currentView->health[player];
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    //REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return currentView->trail[player][0];
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


//Private functions
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

static void frontInsert(LocationID *trail, char *location) {
    assert(trail != NULL);
    assert(location != NULL);    

    char *unknown_city = "C?";
    char *unknown_sea = "S?";
    char *hide = "HI";
    char *teleport = "TP";
    char *db[5] = {"D1", "D2", "D3", "D4", "D5"};
   
    LocationID id = UNKNOWN_LOCATION;
   
    if(strcmp(location, unknown_city) == 0) {
        id = CITY_UNKNOWN;
    } else if(strcmp(location, unknown_sea) == 0) {
        id = SEA_UNKNOWN;
    } else if(strcmp(location, hide) == 0) {
        id = HIDE;
    } else if(strcmp(location, teleport) == 0) {
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

    assert((id >= UNKNOWN_LOCATION && id <= ZURICH) || (id >= CITY_UNKNOWN && id <= TELEPORT));

    int j;
    for(j = TRAIL_SIZE - 1; j > 0; j--) trail[j] = trail[j-1];

    trail[j] = id;
}


