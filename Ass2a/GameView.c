// GameView.c ... GameView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Queue.h"

#define TRUE 1
#define FALSE 0

//ADT(s)
struct gameView {
   Map g;                        //The Map
   int turn;                     //Turn number
   int score;                    //The score of the game
   int *lastTurnHealth;          //players' health in last turn
   int *health;                  //players' health in this turn
   LocationID **trail_perPlayer; // stores trail for each player in 2D array
}; 


typedef struct vNode *VList;
struct vNode {
    LocationID v;   
    TransportID type; 
    VList next; 
};

struct MapRep {
    int nV;
    int nE;
    VList connections[NUM_MAP_LOCATIONS]; 
};


// *** Private functions ***
static PlayerID whichPlayer(char c);
static void validGameView(GameView gameView);
static void frontInsert(LocationID **trail_perPlayer, PlayerID player, char *location);
static void railConnection(GameView gameView, int *reachable, LocationID from, int railMoves);


// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    //Initialising the GameView ADT
    GameView gameView = malloc(sizeof(struct gameView));
    assert(gameView != NULL);
    gameView->g = newMap();
    assert(gameView->g != NULL);
    gameView->lastTurnHealth = malloc(NUM_PLAYERS * sizeof(int));
    assert(gameView->lastTurnHealth != NULL);
    gameView->health = malloc(NUM_PLAYERS * sizeof(int));
    assert(gameView->health != NULL);
    gameView->trail_perPlayer = malloc(NUM_PLAYERS * sizeof(LocationID *));
    assert(gameView->trail_perPlayer != NULL);
 
    int i, j = 0;
    for(i = 0; i < NUM_PLAYERS; i++) {
        gameView->trail_perPlayer[i] = malloc(GAME_START_SCORE * sizeof(LocationID));
        assert(gameView->trail_perPlayer[i] != NULL);

        for(j = 0; j < GAME_START_SCORE; j++){
            gameView->trail_perPlayer[i][j] = UNKNOWN_LOCATION;           
        }
    }


    //Initial turn number and game score
    gameView->turn = 1;
    gameView->score = GAME_START_SCORE;    

    //The health of players at the beginning of the game
    for(i = 0; i < PLAYER_DRACULA; i++) {
        gameView->lastTurnHealth[i] = GAME_START_HUNTER_LIFE_POINTS;
        gameView->health[i] = GAME_START_HUNTER_LIFE_POINTS;
    }
    gameView->lastTurnHealth[i] = GAME_START_BLOOD_POINTS;
    gameView->health[i] = GAME_START_BLOOD_POINTS;    

    //Scanning through the pastPlays string to obtain necessary information
    int interval = 8; 
    for(i = 0; i < strlen(pastPlays); i += interval) {
        gameView->turn++;                             //increase the turn number by 1
        PlayerID player = whichPlayer(pastPlays[i]);  //find out which player 

        char *location = malloc(4 * sizeof(char));    //get the abbreviation of locations
        assert(location != NULL);
        for(j = 0; j < 4; j++) location[j] = 0;
        location[0] = pastPlays[i+1];
        location[1] = pastPlays[i+2];

        //Update the trail of the current player(Hunter)
        frontInsert(gameView->trail_perPlayer, player, location); 
        free(location);

        //player = one of the Hunters
        if(player != PLAYER_DRACULA) {
            for(j = i + 3; j < i + interval - 1; j++) {

                //restore to 9 HP if the hunter was killed by Dracula in last turn
                if(gameView->health[player] == 0) {
                    gameView->lastTurnHealth[player] = 0;
                    gameView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                } else {
                    gameView->lastTurnHealth[player] = gameView->health[player];
                }

                if(pastPlays[j] == '.') break;  //No more encounters --> exit the loop by break       
                
                //Trigger the trap(s)
                if(pastPlays[j] == 'T') {
                    gameView->health[player] -= LIFE_LOSS_TRAP_ENCOUNTER;
                } else if(pastPlays[j] == 'D') {         
                    //Confront Dracula 
                    gameView->health[player] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                    gameView->health[PLAYER_DRACULA] -= LIFE_LOSS_HUNTER_ENCOUNTER;   
                }

                //No more actions when HP becomes ZERO --> HP = 0 and reduce the game score
                if(gameView->health[player] <= 0) {
                    gameView->health[player] = 0;
                    gameView->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                    break;
                } 
            }
            
            //When hunter stays at the same location as the previous turn (when HP is not zero)
            if((gameView->trail_perPlayer[player][0] == gameView->trail_perPlayer[player][1]) && 
                gameView->health[player] > 0) {
                //take rest or research at the same location
                gameView->health[player] += LIFE_GAIN_REST;

                //make sure Hunter HP does not exceed the limit (i.e HP <= 9)
                if(gameView->health[player] > GAME_START_HUNTER_LIFE_POINTS) {
                    gameView->health[player] = GAME_START_HUNTER_LIFE_POINTS;
                }
            }        
        } else {
            //player = Dracula
             
            //Immuture vampires become mature and game score will be reduced
            if(pastPlays[i+5] == 'V') gameView->score -= SCORE_LOSS_VAMPIRE_MATURES;
           
            gameView->lastTurnHealth[player] = gameView->health[player];

            if(gameView->trail_perPlayer[player][0] == CASTLE_DRACULA || gameView->trail_perPlayer[player][0] == TELEPORT) {
                gameView->health[player] += LIFE_GAIN_CASTLE_DRACULA;    //gain HP as Dracula is in his castle
            } else if(gameView->trail_perPlayer[player][0] >= MIN_MAP_LOCATION && gameView->trail_perPlayer[player][0] <= MAX_MAP_LOCATION) {
                //lose 2 HP when Dracula is at the sea
                if(idToType(gameView->trail_perPlayer[player][0]) == SEA) {
                    gameView->health[player] -= LIFE_LOSS_SEA;
                } 
            } else if(gameView->trail_perPlayer[player][0] == SEA_UNKNOWN) {
                gameView->health[player] -= LIFE_LOSS_SEA;
            } else if(gameView->trail_perPlayer[player][0] >= DOUBLE_BACK_1 && gameView->trail_perPlayer[player][0] <= DOUBLE_BACK_5) {
                int steps = gameView->trail_perPlayer[player][0] - DOUBLE_BACK_1 + 1;               

                if(gameView->trail_perPlayer[player][steps] == HIDE) steps += 1;
                assert(steps < GAME_START_SCORE);

                if(gameView->trail_perPlayer[player][steps] >= MIN_MAP_LOCATION && gameView->trail_perPlayer[player][steps] <= MAX_MAP_LOCATION) {
                    if(idToType(gameView->trail_perPlayer[player][steps]) == SEA) {
                        gameView->health[player] -= LIFE_LOSS_SEA; 
                    }
                } else if(gameView->trail_perPlayer[player][steps] == SEA_UNKNOWN) {
                    gameView->health[player] -= LIFE_LOSS_SEA;
                } 
            }

            //score - 1 when Dracula finishes his turn
            gameView->score -= SCORE_LOSS_DRACULA_TURN;    
        }
    }  

    //if current player is hunter and got killed by Dracula in last round
    //his health will be restored and can leave the hospital
    PlayerID currentPlayer = getCurrentPlayer(gameView);
    if(currentPlayer != PLAYER_DRACULA) {
        if(gameView->health[currentPlayer] == 0) {
            gameView->lastTurnHealth[currentPlayer] = 0;
            gameView->health[currentPlayer] = GAME_START_HUNTER_LIFE_POINTS;
        }
    }
     
    return gameView;
}
     
// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    validGameView(toBeDeleted);    

    int i = 0;
    for(i = 0; i < NUM_PLAYERS; i++) {
        free(toBeDeleted->trail_perPlayer[i]); 
    }

    disposeMap(toBeDeleted->g);
    free(toBeDeleted->trail_perPlayer); 
    free(toBeDeleted->lastTurnHealth);
    free(toBeDeleted->health);
    free(toBeDeleted);
}

//// Functions that return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView)
{ 
    validGameView(currentView); 
    Round round = (Round) ((currentView->turn - 1) / 5);
    return round;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{ 
    validGameView(currentView); 
    PlayerID player = (PlayerID) ((currentView->turn - 1) % NUM_PLAYERS);
    return player;
}

// Get the current score
int getScore(GameView currentView)
{
    validGameView(currentView); 
    return currentView->score;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    validGameView(currentView); 
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 

    if(player != PLAYER_DRACULA) {
        assert(currentView->health[player] >= 0 && currentView->health[player] <= GAME_START_HUNTER_LIFE_POINTS);
        return currentView->health[player];
    } 
        
    assert(currentView->health[PLAYER_DRACULA] > 0);
    return currentView->health[PLAYER_DRACULA];
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    validGameView(currentView); 
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);

    if(player == PLAYER_DRACULA) {
        //Dracula is teleported to his castle
        if(currentView->trail_perPlayer[player][0] == TELEPORT) return CASTLE_DRACULA;
    } else {
        //if the current health or the health in last turn is ZERO, then the hunter must be in the hospital
        if(currentView->health[player] == 0) return ST_JOSEPH_AND_ST_MARYS;
        if(currentView->lastTurnHealth[player] == 0) return ST_JOSEPH_AND_ST_MARYS;   
    }

    return currentView->trail_perPlayer[player][0];
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    validGameView(currentView); 
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA); 
    
    int i;
    for(i = 0; i < TRAIL_SIZE; i++) trail[i] = currentView->trail_perPlayer[player][i];
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations
LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    validGameView(currentView); 
    assert(numLocations != NULL);   // check that there is a location to visit
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    
    //If the location of the player is unknown (for example, HIDE, CITY_UNKNOWN, ......)
    if(from < MIN_MAP_LOCATION || from > MAX_MAP_LOCATION) {
        *numLocations = 0;
        return NULL;
    } 
    

    // An array which tells which location can be reached (depends on the 3 parameters - road, rail, sea)
    int *reachable = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(reachable != NULL);

    //Initialise the array
    int i;
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) reachable[i] = 0;
    reachable[from] = 1;
    
    VList curr = currentView->g->connections[from]; 

    //Start looking for connected cities and seas 
    // **  this function won't take Dracula's trail into account (as required) **
    if(player == PLAYER_DRACULA) {
        assert(from != ST_JOSEPH_AND_ST_MARYS); // Darcula is impossibly in the hospital

        while(curr != NULL){
            if(road == TRUE) {
                if(curr->type == ROAD && curr->v != ST_JOSEPH_AND_ST_MARYS) {
                    reachable[curr->v] = 1;
					 }
            } else if(sea == TRUE) {
				    if(curr->type == BOAT) reachable[curr->v] = 1;            
            }
                
            curr = curr->next;
        }
    } else {    

        // player is a hunter
        while(curr != NULL) {
            if(road == TRUE) {
                if(curr->type == ROAD) reachable[curr->v] = 1;                
            } else if(sea == TRUE) {
                if(curr->type == BOAT) reachable[curr->v] = 1;
            }

            curr = curr->next;
        }   

        // Rail moves: The maximum distance that can be moved via rail
        int railMoves = (round + player) % 4;
        if(rail == TRUE && railMoves == 1) {
            curr = currentView->g->connections[from];
            while(curr != NULL) {
                if(curr->type == RAIL) reachable[curr->v] = 1;
                curr = curr->next;
            }
        } else if(rail == TRUE && railMoves > 1) {
            //looking for more rail connected cities according to how many railMoves can be made in this round
            railConnection(currentView, reachable, from, railMoves); 
        }

    }

    // The size of the array is stored in the variable pointed to by numLocations
    LocationID *connLocations = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));
    assert(connLocations != NULL);

    int length = 0;
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if(reachable[i] == 1) connLocations[length++] = i;
    }

    free(reachable); 
    assert(length >= 1);     //as it is possible to stay at the same place
    *numLocations = length;  //length of the connLocations array
   
    return connLocations;
}

// *** Private Functions ***

//check whether the given gameView is valid
static void validGameView(GameView gameView) {
    assert(gameView != NULL);
    assert(gameView->g != NULL);
    assert(gameView->lastTurnHealth != NULL);
    assert(gameView->health != NULL);
    assert(gameView->trail_perPlayer != NULL);

    int i;
    for(i = 0; i < NUM_PLAYERS; i++) assert(gameView->trail_perPlayer[i] != NULL);

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


// Inserts current location to trail accordingly 
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

// Finds the rail connected cities according to the railMoves
static void railConnection(GameView gameView, int *reachable, LocationID from, int railMoves) {
    validGameView(gameView);
    assert(reachable != NULL);
    assert(from >= MIN_MAP_LOCATION && from <= MAX_MAP_LOCATION);
    assert(railMoves > 1);

    // how many rounds it needs to be checked
    int roundCheck = railMoves - 1;
    Queue *qList = malloc(roundCheck * sizeof(Queue *)); //An array of Queue ADT
    assert(qList != NULL);

    int i;
    for(i = 0; i < roundCheck; i++) {
        qList[i] = newQueue();
        assert(qList[i] != NULL);
    }
   
    //Finds all the adjacent cities connected by rails in the current city
    VList curr = gameView->g->connections[from];
    while(curr != NULL) {
        if(curr->type == RAIL) {
            reachable[curr->v] = 1;    
            enterQueue(qList[0], curr->v); 
        }

        curr = curr->next;
    }    

    //tells which locations have been visited
    int *visited = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(visited != NULL);
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) visited[i] = 0;
    visited[from] = 1; 

   
    //BFS x 1 or BFS x 2 (depending on the number of railMoves can be made)
    for(i = 0; i < roundCheck; i++) {
        while(!emptyQueue(qList[i])) {
            LocationID s = leaveQueue(qList[i]);
            assert(from >= MIN_MAP_LOCATION && from <= MAX_MAP_LOCATION);
 
            if(visited[s] == 1) continue;
            visited[s] = 1;

            curr = gameView->g->connections[s];
            while(curr != NULL) {
                if(curr->type == RAIL) {
                    reachable[curr->v] = 1;

                    if(!visited[curr->v] && i+1 < roundCheck) enterQueue(qList[i+1], curr->v);
                }
            
                curr = curr->next;
            }
        }
    }

    for(i = 0; i < roundCheck; i++) {
        assert(qList[i] != NULL);
        disposeQueue(qList[i]);
    }

    free(qList);
    free(visited);
}


