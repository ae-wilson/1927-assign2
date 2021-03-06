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
static void validHunterView(HunterView hunterView);
static void frontInsert(LocationID **trail_perPlayer, PlayerID player, char *location);     


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
    hunterView->lastTurnHealth = malloc(NUM_PLAYERS * sizeof(int));
    assert(hunterView->lastTurnHealth != NULL);
    hunterView->health = malloc(NUM_PLAYERS * sizeof(int));
    assert(hunterView->health != NULL);
    hunterView->trail_perPlayer = malloc(NUM_PLAYERS * sizeof(LocationID *));
    assert(hunterView->trail_perPlayer != NULL);

    int i, j = 0;
    for(i = 0; i < NUM_PLAYERS; i++) {
        hunterView->trail_perPlayer[i] = malloc(GAME_START_SCORE * sizeof(LocationID));
        assert(hunterView->trail_perPlayer[i] != NULL);
 
        for(j = 0; j < GAME_START_SCORE; j++){
            hunterView->trail_perPlayer[i][j] = UNKNOWN_LOCATION;
        }
    }


    //Update the state of the game
    hunterView->turn = 1;
    hunterView->score = GAME_START_SCORE;    

    for(i = 0; i < PLAYER_DRACULA; i++) {
        hunterView->lastTurnHealth[i] = GAME_START_HUNTER_LIFE_POINTS;
        hunterView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    hunterView->lastTurnHealth[i] = GAME_START_BLOOD_POINTS;
    hunterView->health[i] = GAME_START_BLOOD_POINTS;    


    int interval = 8;
    for(i = 0; i < strlen(pastPlays); i += interval) {
        hunterView->turn++;  //increase the turn number
        PlayerID player = whichPlayer(pastPlays[i]);  //find out which player

        char location[2];    //get the abbrev of locations
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];
        location[2] = '\0';

        frontInsert(hunterView->trail_perPlayer, player, location); //update the trail_perPlayer

        //player = one of the hunters
        if(player != PLAYER_DRACULA) {
            for(j = i + 3; j < i + interval - 1; j++) {
                if(hunterView->health[player] == 0) {
                    hunterView->lastTurnHealth[player] = 0;
                    hunterView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                } else {
                    hunterView->lastTurnHealth[player] = hunterView->health[player];
                }

                if(pastPlays[j] == '.') break;  //no encounters            

                //trigger the trap(s)
                if(pastPlays[j] == 'T') {
                    hunterView->health[player] -= LIFE_LOSS_TRAP_ENCOUNTER;
                } else if(pastPlays[j] == 'D') {
                    //confront Dracula
                    hunterView->health[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    hunterView->health[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;   
                }

                //no more actions when HP gets to ZERO
                if(hunterView->health[player] <= 0) {
                    hunterView->health[player] = 0;
                    hunterView->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                    break;
                } 
            }
            
            if((hunterView->trail_perPlayer[player][0] == hunterView->trail_perPlayer[player][1]) && 
                hunterView->health[player] > 0) {
                //take rest or research at the same location
                hunterView->health[player] += LIFE_GAIN_REST;

                //make sure Hunter HP does not exceed the limit (HP: 9)
                if(hunterView->health[player] > GAME_START_HUNTER_LIFE_POINTS) {
                    hunterView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                }
            }        
        } else {
            //player = Dracula
            if(pastPlays[i+5] == 'V') hunterView->score -= SCORE_LOSS_VAMPIRE_MATURES;
           
            hunterView->lastTurnHealth[player] = hunterView->health[player];

            if(hunterView->trail_perPlayer[player][0] == CASTLE_DRACULA || hunterView->trail_perPlayer[player][0] == TELEPORT) {
                //gain HP as Dracula is in his castle

                hunterView->health[player] += LIFE_GAIN_CASTLE_DRACULA;
            } else if(hunterView->trail_perPlayer[player][0] >= MIN_MAP_LOCATION && hunterView->trail_perPlayer[player][0] <= MAX_MAP_LOCATION) {
                //lose 2 HP when Dracula is at the sea

                if(idToType(hunterView->trail_perPlayer[player][0]) == SEA) {
                    hunterView->health[player] -= LIFE_LOSS_SEA;
                } 
            } else if(hunterView->trail_perPlayer[player][0] == SEA_UNKNOWN) {
                hunterView->health[player] -= LIFE_LOSS_SEA;
            } else if(hunterView->trail_perPlayer[player][0] >= DOUBLE_BACK_1 && hunterView->trail_perPlayer[player][0] <= DOUBLE_BACK_5) {
                int steps = hunterView->trail_perPlayer[player][0] - DOUBLE_BACK_1 + 1;               

                if(hunterView->trail_perPlayer[player][steps] == HIDE) steps += 1;
                assert(steps < GAME_START_SCORE);

                if(hunterView->trail_perPlayer[player][steps] >= MIN_MAP_LOCATION && 
                   hunterView->trail_perPlayer[player][steps] <= MAX_MAP_LOCATION) 
                {
                    if(idToType(hunterView->trail_perPlayer[player][steps]) == SEA) {
                        hunterView->health[player] -= LIFE_LOSS_SEA; 
                    }
                } else if(hunterView->trail_perPlayer[player][steps] == SEA_UNKNOWN) {
                    hunterView->health[player] -= LIFE_LOSS_SEA;
                } 
            }

            //score -= 1 when Dracula ends his turn
            hunterView->score -= SCORE_LOSS_DRACULA_TURN;    
        }
    } 

    if(hunterView->turn > 1) {
        hunterView->ms = malloc((hunterView->turn - 1) * sizeof(PlayerMessage));
        assert(hunterView->ms != NULL);
        
        int k;
        for(k = 0; k < hunterView->turn - 1; k++) {
            assert(hunterView->ms[k] != NULL);
            memset(hunterView->ms[k], 0, MESSAGE_SIZE);
            strcpy(hunterView->ms[k], messages[k]);
        }
    }


    return hunterView;
}
     
     
// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted)
{
    validHunterView(toBeDeleted);
     
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
    LocationID from = getLocation(currentView->gameView, player);
    Round round = getRound(currentView->gameView);

    if(from != UNKNOWN_LOCATION) {
        LocationID *moves = connectedLocations(currentView->gameView, numLocations, from, player, round, road, rail, sea);
        return moves;
    } else {
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

    // need to find out the current round
    Round turn = giveMeTheRound(currentView);

    // we will use the connectedLocations funciton in GameView.c to find
    // all the possible locations which Dracula can visit
    return connectedLocations(currentView->gameView, numLocations, there, player, turn, road, rail, sea);
}



// *** Private Functions ***

//check whether the given hunterView is valid
static void validHunterView(HunterView hunterView) {
    assert(hunterView != NULL);
    assert(hunterView->g != NULL);
    assert(hunterView->gameView != NULL);
    assert(hunterView->lastTurnHealth != NULL);
    assert(hunterView->health != NULL);
    assert(hunterView->trail_perPlayer != NULL);
    if(hunterView->turn != 1) assert(hunterView->ms != NULL);

    int i;
    for(i = 0; i < NUM_PLAYERS; i++) assert(hunterView->trail_perPlayer[i] != NULL);

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

