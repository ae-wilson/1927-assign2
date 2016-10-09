// dracula.c
// Implementation of your "Fury of Dracula" Dracula AI

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "Map.h"
#include "Game.h"
#include "Places.h"
#include "Queue.h"
#include "DracView.h"

#define TRUE 1
#define FALSE 0

#define NOT_IN_TRAIL -1
#define SAFE_DISTANCE 3

// ***  Private Functions   ***
static int isLegalMove(DracView gameState, LocationID move);
static int isFound(LocationID *array, LocationID location, int low, int high);
static int isAdjacent(DracView gameState, LocationID location);
static int isSafeCastle(DracView gameState);
static int isAtTheSameSea(DracView gameState);
static int hasDBInTrail(DracView gameState);
static int positionInTrail(DracView gameState, LocationID location);
static int numHuntersThere(DracView gameState, LocationID loc);

static LocationID firstMove(DracView gameState);
static LocationID BestMove(DracView gameState);
static LocationID randomMove(DracView gameState);
static LocationID goToLandOrSea(DracView gameState);
static LocationID goToSafeSea(DracView gameState);
static LocationID backToCastle(DracView gameState);
static LocationID awayFromHunters(DracView gameState);
static LocationID *connectedSeas(DracView gameState, int *numSeas);
static LocationID *connectedPorts(DracView gameState, int *numPorts);
static LocationID *safeConnectedLocations(DracView gameState, int *numLocations, int road, int sea);

static void sortLocIDArray(LocationID *array, int low, int high);
static void moveToAbbreviation(LocationID move, char *abbrev);


// Function to decide the move of Dracula
void decideDraculaMove(DracView gameState) {
    assert(gameState != NULL);

    // For indentation in the game log
    printf("\n");

    int i = 0;
    Round round = giveMeTheRound(gameState); 
    LocationID move = UNKNOWN_LOCATION;
   
    if(round > 0) {     
        // Determine the best move depending on the current state of the game
        move = BestMove(gameState);
    } else {
        // Move in first round (i.e: Round 0)
        move = firstMove(gameState);
    } 

    
    // Send the move to the game engine after converting the move
    // into string (with two characters)
    char abbrev[2];
    for(i = 0; i <= 2; i++) abbrev[i] = '\0';
    if(move == UNKNOWN_LOCATION) move = TELEPORT;
    moveToAbbreviation(move, abbrev);

    registerBestPlay(abbrev, "Dracula is coming");
}



// ***   Private Functions   ***
// Determine whether the given move is legal or not
static int isLegalMove(DracView gameState, LocationID move) {
    assert(gameState != NULL);

    // Unknown moves = illegal
    if(move < MIN_MAP_LOCATION || (move > MAX_MAP_LOCATION && move < HIDE) ||
       move >= TELEPORT) 
    {
        return FALSE;
    }
   
    // As Dracula can go anywhere except hospital in the first round of the game
    if(giveMeTheRound(gameState) == 0) {
        if(move != ST_JOSEPH_AND_ST_MARYS && (move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION)) {
            return TRUE;
        }
    }

 
    // Get Dracula's moves -- including special moves (DB?/HI/TP)
    LocationID dracMoves[TRAIL_SIZE];
    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) {
        dracMoves[i] = UNKNOWN_LOCATION;
    }

    giveMeTheMoves(gameState, PLAYER_DRACULA, dracMoves);
    for(i = 0; i < TRAIL_SIZE - 1; i++) {
        if(dracMoves[i] == TELEPORT) {
            dracMoves[i] = CASTLE_DRACULA;
        }
    }


    if(move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION) {
        // Dracula never have access to hosipital
        if(move == ST_JOSEPH_AND_ST_MARYS) {
            return FALSE;
        }  

        // Return FALSE if the move is already existed in Dracula's trail
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] == move) {
                return FALSE;
            }
        } 
        
        // Check whether the move is adjacent to where Dracula is currently
        if(isAdjacent(gameState, move) == FALSE) return FALSE;
 
    } else if(move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5) {
        Round round = giveMeTheRound(gameState);

        // Double back to an unknown location
        if(move - DOUBLE_BACK_1 >= round) return FALSE;
        if(hasDBInTrail(gameState) == TRUE) return FALSE;

        // Find out where Dracula doubled back to
        int pos = move - DOUBLE_BACK_1;
        if(dracMoves[pos] == HIDE) pos++;
        if(pos > 4) return FALSE;
        move = dracMoves[pos]; 
                
        if(move < MIN_MAP_LOCATION || move > MAX_MAP_LOCATION) return FALSE; 
        if(isAdjacent(gameState, move) == FALSE) return FALSE; 

    } else if(move == HIDE) {

        // Check if there is a HIDE move in Dracula's trail
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] == HIDE) {
                return FALSE;
            }
        }

        // Dracula cannot hide at sea
        LocationID curr = whereIs(gameState, PLAYER_DRACULA);
        if(idToType(curr) == SEA) return FALSE;
    }

    return TRUE;
}

// Function to decide where Dracula goes firstly
static LocationID firstMove(DracView gameState) {
    assert(gameState != NULL);
    
    int i, hunter = 0;
    int occupied[NUM_MAP_LOCATIONS];
    LocationID firstMove = UNKNOWN_LOCATION;
  
    // Initialse + don't want to start the game at sea (because -2 HP at the end of turn) 
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        if(idToType((LocationID) i) != SEA) {
            occupied[i] = 0;
        } else {
            occupied[i] = 1;
        }
    }
    occupied[ST_JOSEPH_AND_ST_MARYS] = 1;  // Never start at the hospital


    // Find out where can hunters go next turn and
    // start at a city where hunters cannot reach next turn
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        int numLocations = 0;
        LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter, 1, 1, 1);
 
        if(numLocations > 0) assert(connLoc != NULL);

        for(i = 0; i < numLocations; i++) {
            LocationID v = connLoc[i];
            occupied[v] = 1;
        }

        if(numLocations > 0) free(connLoc);
    }

    // Count how many 'unoccupied' cities that Dracula can go
    int count = 0;
    LocationID unoccupied[NUM_MAP_LOCATIONS];
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) unoccupied[i] = -1;
   
    int loc = 0;
    for(loc = 0; loc < NUM_MAP_LOCATIONS; loc++) {
        if(!occupied[loc] && idToType(loc) != SEA) unoccupied[count++] = loc;
    } 
    assert(count > 0);

    srand(time(NULL));
    int index = rand() % count;
    firstMove = unoccupied[index];

    return firstMove;
} 

// Determine the best move according to the current game state
static LocationID BestMove(DracView gameState) {
    assert(gameState != NULL);

    LocationID move = UNKNOWN_LOCATION;   
 
    if(idToType(whereIs(gameState, PLAYER_DRACULA)) == SEA) { 
        // when Dracula is at sea
        move = goToLandOrSea(gameState);
    } else {
        if(isSafeCastle(gameState)) {
            // Go back to his castle and gain 10HP 
            // if the castle is so called 'safe'
            move = backToCastle(gameState);
        } else {
            // Escape 
            move = awayFromHunters(gameState);
        }
    }   

    if(move == UNKNOWN_LOCATION) move = randomMove(gameState);
    
    return move;
}

// Function to make a random and legal(*priority = 1) move
static LocationID randomMove(DracView gameState) {
    assert(gameState != NULL);

    // Find out where Dracula can go
    int numLocations = 0;
    LocationID *connLoc = whereCanIgo(gameState, &numLocations, 1, 1);
    assert(numLocations >= 1);
    assert(connLoc != NULL);

    LocationID move = UNKNOWN_LOCATION;

    // Use the isLegalMove Function to find out all the possible LEGAL moves
    // Note: not include DB/HI
    int i = 0;
    int numLM = 0;
    LocationID *legalMoves = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));
    assert(legalMoves != NULL);

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) legalMoves[i] = UNKNOWN_LOCATION;

    for(i = 0; i < numLocations; i++) {
        if(isLegalMove(gameState, connLoc[i]) == TRUE) {
            legalMoves[numLM++] = connLoc[i];
        }    
    }

    // For the game log
    printf("\nRandom Move:\n");
    printf("numLM = %d\n", numLM);


    if(numLM > 0) {
        srand(time(NULL));
        int index = rand() % numLM;
        move = legalMoves[index];

        if(numHuntersThere(gameState, move) > 0) {
            if(numLM > 1) {
                // If more than one legal moves and find out the locations 
                // where there is no hunters 
                for(i = 0; i < numLM; i++) {
                    if(numHuntersThere(gameState, legalMoves[i]) == 0) {
                        move = legalMoves[i];
                        break;
                    } 
                }

            } else {
                // TRY to escape from the hunters by taking HIDE or DOUBLE BACK
                // if there is only one legal move and that location is occupied by 
                if(isLegalMove(gameState, HIDE) == TRUE && 
                   numHuntersThere(gameState, whereIs(gameState, PLAYER_DRACULA)) == 0)
                {
                    move = HIDE;
                } else if(hasDBInTrail(gameState) == FALSE) {
                    LocationID trail[TRAIL_SIZE];
                    for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
                    giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

                    for(i = 0; i < TRAIL_SIZE - 1; i++) {
                        LocationID double_back = DOUBLE_BACK_1 + i;
                        
                        if(isLegalMove(gameState, double_back) && trail[i] != UNKNOWN_LOCATION) {
                            if(numHuntersThere(gameState, trail[i]) == 0) {
                                move = double_back;
                                break;
                            }
                        }
                    }
                }
  
            }    
        }    

    } else {

        // If DB/HI are possible actions for Dracula only
        if(isLegalMove(gameState, HIDE) == TRUE) {
            move = HIDE;
        } else if(hasDBInTrail(gameState) == FALSE) {
            move = DOUBLE_BACK_1;

            LocationID trail[TRAIL_SIZE];
            for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
            giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

            for(i = 0; i < TRAIL_SIZE - 1; i++) {
                LocationID double_back = DOUBLE_BACK_1 + i;
                        
                if(isLegalMove(gameState, double_back) && trail[i] != UNKNOWN_LOCATION) {
                    if(numHuntersThere(gameState, trail[i]) == 0) {
                        move = double_back;
                        break;
                    }
                }
            } 
  
        } else {
            // There is no legal moves and Dracula will be 
            // teleported to his castle
            move = TELEPORT;
        }
    }
   
    free(connLoc);
    free(legalMoves);
 
    return move;
}

// Determine whether Dracula should trvael to another sea
// Or go to a 'safe' port city
static LocationID goToLandOrSea(DracView gameState) {
    assert(gameState != NULL);
    assert(idToType(whereIs(gameState, PLAYER_DRACULA)) == SEA);

    LocationID move = UNKNOWN_LOCATION;
    
    if(!isAtTheSameSea(gameState)) {
        // Landing Or go to a safe port city if no hunters occupy the sea 
        // where Dracula is

        // Find out all the connected port cities
        int numPorts = 0;
        LocationID *ports = connectedPorts(gameState, &numPorts);
        assert(ports != NULL);

        // Find out all the cities are occupied by hunters
        int occupied[NUM_MAP_LOCATIONS];
        int i = 0;
        for(i = 0; i < NUM_MAP_LOCATIONS; i++) occupied[i] = 0;

        int hunter = 0;
        for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
            int numLocations = 0;
            LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter,
                                                 1, 1, 1);
            assert(connLoc != NULL);

            for(i = 0; i < numLocations; i++) {
                LocationID v = connLoc[i];
                occupied[v] = 1;
            }
            free(connLoc);
        }       


        // Find out the 'safe' / unoccupied port cities
        int numSP = 0;
        LocationID safePorts[NUM_MAP_LOCATIONS];

        for(i = 0; i < numPorts; i++) {
            LocationID p = ports[i];

            // Ports that connect to only one city
            if(p == ATHENS) continue;
            if(p == PLYMOUTH) continue;
            

            if(!occupied[p]) safePorts[numSP++] = p;
        }
        free(ports);       
 
        if(numSP > 0) {
            srand(time(NULL));    
            int index = rand() % numSP;

            printf("\nLanding ......\n");

            move = safePorts[index];
            assert(isLegalMove(gameState, move)); 
        } else {
            LocationID fSea = goToSafeSea(gameState);
        
            if(fSea != UNKNOWN_LOCATION) {
                printf("\nUnable to land :( ......\n");
                printf("Sea Travel again :( ......\n");

                if(positionInTrail(gameState, fSea) == NOT_IN_TRAIL) {
                    move = fSea; 
                } else if(!hasDBInTrail(gameState)) {
                    int pos = positionInTrail(gameState, fSea);
                    move = DOUBLE_BACK_1 + pos;

                    assert(isLegalMove(gameState, move));
                }
            }
        }
 
    } else {
        LocationID fSea = goToSafeSea(gameState);
        
        if(fSea != UNKNOWN_LOCATION) {
            printf("\nSea Travel again :( ......\n");

            if(positionInTrail(gameState, fSea) == NOT_IN_TRAIL) {
                move = fSea; 
            } else if(!hasDBInTrail(gameState)) {
                int pos = positionInTrail(gameState, fSea);
                move = DOUBLE_BACK_1 + pos;

                assert(isLegalMove(gameState, move));
            }
        }

    }               
 
    if(move == UNKNOWN_LOCATION) move = awayFromHunters(gameState);

    return move;
}

// Function which returns the furthermost connected sea 
// Note: will not consider the sea where Dracula is currently at
static LocationID goToSafeSea(DracView gameState) {
    assert(gameState != NULL);

    // Find out all the connected seas (not including the sea where Dracula is currently at)
    int numSeas = 0;
    LocationID *seas = connectedSeas(gameState, &numSeas);
    assert(seas != NULL);

    // Try to find out the furthermost sea from hunters 
    LocationID fSea = UNKNOWN_LOCATION;
    int i = 0;
    int length = 0;
    int hunter = 0;
    int distFromH = 999;

    for(i = 0; i < numSeas; i++) {
        for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
            if(whereIs(gameState, hunter) == whereIs(gameState, PLAYER_DRACULA)) continue;
            
            int turnsReach = 0;
            LocationID *sPath = sPathForHunters(gameState, &turnsReach, hunter, whereIs(gameState, hunter),
                                                    seas[i], 1, 1, 1);

            // There must be a shortest path for hunters
            assert(sPath != NULL);            

            // Keep track of where the furthermost sea is
            if(turnsReach < distFromH) {
                distFromH = turnsReach;
            }
                
            free(sPath);
        }

        if(distFromH > length) {
            length = distFromH;
            fSea = seas[i];
        }
    } 
 
    free(seas);

    return fSea;
}


// Find the connected seas which are connected to Dracula's current location (Loc = a sea)
// Note: if there is no Double Back in the trail, then all the adjacent seas in the trail will
//       be considered
//       
//       The sea where dracula is at wont be included
static LocationID *connectedSeas(DracView gameState, int *numSeas) {
    assert(gameState != NULL);
    assert(numSeas != NULL);
    
    int numLocations = 0;
    LocationID *connLoc = whereCanIgo(gameState, &numLocations, 0, 1);
    assert(connLoc != NULL);
    

    int number = 0;
    LocationID *seas = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));
    assert(seas != NULL);

    int i = 0;
    for(i = 0; i < numLocations; i++) {
        LocationID v = connLoc[i];

        if(idToType(v) == SEA && v != whereIs(gameState, PLAYER_DRACULA)) {
            seas[number++] = v;
        }
    } 
    free(connLoc);


    if(!hasDBInTrail(gameState)) {
        LocationID dracTrail[TRAIL_SIZE];
        for(i = 0; i < TRAIL_SIZE; i++) {
            dracTrail[i] = UNKNOWN_LOCATION;
        }
        giveMeTheTrail(gameState, PLAYER_DRACULA, dracTrail);

        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            LocationID v = dracTrail[i];

            if(v != UNKNOWN_LOCATION && v != whereIs(gameState, PLAYER_DRACULA)) {
                if(isAdjacent(gameState, v) && idToType(v) == SEA) {
                    seas[number++] = v;
                } 
            }
        }
    }


    *numSeas = number;
    return seas;
}

// Find out all the connected port cities (when Dracula's curr Loc = Sea)
static LocationID *connectedPorts(DracView gameState, int *numPorts) {
    assert(gameState != NULL);
    assert(numPorts != NULL);

    int numLocations = 0;
    LocationID *connLoc = whereCanIgo(gameState, &numLocations, 0, 1);
    assert(connLoc != NULL);

 
    int number = 0;
    LocationID *ports = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));
    assert(ports != NULL);

    int i = 0;
    for(i = 0; i < numLocations; i++) {
        LocationID v = connLoc[i];

        if(idToType(v) == LAND) {
            ports[number++] = v;
        }
    } 
    free(connLoc);

    
    *numPorts = number;
    return ports;
}

// Check if there are hunter(s) at the same sea as Dracula
static int isAtTheSameSea(DracView gameState) {
    assert(gameState != NULL);
    assert(idToType(whereIs(gameState, PLAYER_DRACULA)) == SEA);

    int isSameSea = FALSE;
        
    int hunter = 0;
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        if(whereIs(gameState, PLAYER_DRACULA) == whereIs(gameState, hunter)) {
            isSameSea = TRUE;
            break;
        }
    }

    return isSameSea;
}



// Determine what to do next in order to go back to Castle Dracula
// Note: shortest Path
//       If there is DB in the trail, a different shortest path may be obtained
static LocationID backToCastle(DracView gameState) {
    assert(gameState != NULL);

    // If Dracula is already in his castle
    LocationID curr = whereIs(gameState, PLAYER_DRACULA);
    if(curr == CASTLE_DRACULA && isLegalMove(gameState, HIDE) == TRUE) {
        return HIDE;
    } else if(curr == CASTLE_DRACULA && isLegalMove(gameState, DOUBLE_BACK_1) == TRUE) {
        return DOUBLE_BACK_1;
    }

    // Find the shortest path from Dracula's current location to his castle
    // *** will take Dracula's trail into account when there's a Double back
    //     move in his trail;
    int length = 0;
    LocationID *sPath = shortestPath(gameState, &length, curr, CASTLE_DRACULA, 1, 1);


    LocationID move = UNKNOWN_LOCATION;

    if(length > 1) {
        assert(sPath != NULL);
        LocationID next = sPath[1];
      
        // If the next move in shortest Path is currently occupied by hunters
        if(numHuntersThere(gameState, next) > 0) return awayFromHunters(gameState);
 

        if(isLegalMove(gameState, next) == TRUE) {
            printf("\n(`vv`) --> Castle Dracula\n");

            move = next;
        } else {
           
            // Take Double Back and go to Castle Dracula along the shortest path calculated
            LocationID trail[TRAIL_SIZE];
            giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

            int DBackPos = positionInTrail(gameState, next);

            if(DBackPos != NOT_IN_TRAIL) {
                next = DOUBLE_BACK_1 + DBackPos;
                assert(next >= DOUBLE_BACK_1 && next <= DOUBLE_BACK_5);
                assert(isLegalMove(gameState, next) == TRUE);                   
          
                printf("\n(`vv`) --> Castle Dracula\n");

                move = next;
            } else {
                // if Dracula cannot go back to his castle along teh shortest path
                // Just make sure Dracula will make another (**legal) move
                // i.e: when this happens, possibly there is a bug  
                printf("\nBug Bug Bug !!!!!!\n");

                move = awayFromHunters(gameState);
            }
              
            free(sPath);
        } 
    } else {
        
        // Escape when there is no shortest path
        move = awayFromHunters(gameState);
    }


    // Make sure that Dracula takes a random and of course a legal move
    if(move == UNKNOWN_LOCATION) {
        move = randomMove(gameState);
    }

    return move;
}

// Determine move which will make Dracula moves away from hunters
static LocationID awayFromHunters(DracView gameState) {
    assert(gameState != NULL);
    LocationID move = UNKNOWN_LOCATION;         

    // Discover where hunters can't reach while Dracula can reach
    int numSL = 0;
    LocationID *safeLoc = safeConnectedLocations(gameState, &numSL, 1, 1);
    assert(safeLoc != NULL);  
  

    if(numSL > 1) {
        srand(time(NULL));
        int loc = rand() % numSL;
 
        // Try not stay at the same place
        if(safeLoc[loc] == whereIs(gameState, PLAYER_DRACULA)) { 
            int i = 0;
            int count = 0;
            int random[NUM_MAP_LOCATIONS];

            for(i = 0; i < NUM_MAP_LOCATIONS; i++) random[i] = -1;

            for(i = 0; i < numSL; i++) {
                if(safeLoc[i] != whereIs(gameState, PLAYER_DRACULA)) random[count++] = safeLoc[i];
            }

            srand(time(NULL));
            loc = rand() % count;
            move = random[loc];
        } else {
            move = safeLoc[loc];
        }
         
        assert(isLegalMove(gameState, move) == TRUE);
    } else if(numSL == 1) {
        
        // If the safe spot is at Dracula's current location, take Hide or Double back move
        if(safeLoc[0] == whereIs(gameState, PLAYER_DRACULA)) {
            if(isLegalMove(gameState, HIDE) == TRUE) {
                move = HIDE;
            } else if(isLegalMove(gameState, DOUBLE_BACK_1)) {
                move = DOUBLE_BACK_1;
            }
        } else {
            move = safeLoc[0];
            assert(isLegalMove(gameState, move) == TRUE);
        }
    } else {

        // Consider Double Back in order to go to a safe spot
        int hasDB = hasDBInTrail(gameState);
        
        if(hasDB != TRUE) {
            LocationID trail[TRAIL_SIZE];
            giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

            int hunter = 0;
            int occupied[NUM_MAP_LOCATIONS];
            
            int i = 0;
            for(i = 0; i < NUM_MAP_LOCATIONS; i++) occupied[i] = 0;

            for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
                int numLocations = 0;
                LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter, 1, 1, 1);
                assert(connLoc != NULL);

                for(i = 0; i < numLocations; i++) {
                    LocationID v = connLoc[i];
                    if(idToType(v) == SEA) continue;

                    occupied[v] = 1;
                }
                free(connLoc);
            }

            for(i = 0; i < TRAIL_SIZE - 1; i++) {
                if(trail[i] != UNKNOWN_LOCATION) {
                    if(!occupied[trail[i]] && isLegalMove(gameState, DOUBLE_BACK_1 + i)) { 
                        move = DOUBLE_BACK_1 + i;
                        break;
                    }
                }
            }             
        }

    }


    // For the game Log
    printf("\nS Moves:\n");
    
    int i = 0;
    for(i = 0; i < numSL; i++) {
        printf("%s\n", idToName(safeLoc[i]));
    }
    printf("\n");

    free(safeLoc);

    // Make sure Dracula will make a random and legal move
    if(move == UNKNOWN_LOCATION) move = randomMove(gameState);    

    return move;
}


// Find out places where are not connected to hunters' locations
static LocationID *safeConnectedLocations(DracView gameState, int *numLocations, int road, int sea) {
    assert(gameState != NULL);

    // Initialise
    int loc = 0;
    int hunter = 0;
    int reachable[NUM_MAP_LOCATIONS];

    int length = 0;
    LocationID *connLoc = whereCanIgo(gameState, &length, road, sea);
    assert(connLoc != NULL);

    LocationID *safePlaces = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));   
    assert(safePlaces != NULL);

    for(loc = 0; loc < NUM_MAP_LOCATIONS; loc++) {
        reachable[loc] = 0;
        safePlaces[loc] = 0;
    }


    // Update the array reachable if that specific location is connected
    // to Dracula's current Location
    for(loc = 0; loc < length; loc++) {
        LocationID v = connLoc[loc];
        assert(v >= MIN_MAP_LOCATION && v <= MAX_MAP_LOCATION);

        reachable[v] = 1;
    }
    reachable[ST_JOSEPH_AND_ST_MARYS] = 0; // Never have access to the hospital
    free(connLoc);

    // Find out where hunters can reach and try to figure out
    // safe spots
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        int number = 0;
        LocationID *link = whereHuntersCanGoNext(gameState, &number, hunter, 1, 1, 1);
        assert(link != NULL);
        
        for(loc = 0; loc < number; loc++) {
            LocationID v = link[loc];
            if(idToType(v) == SEA) continue;

            assert(v >= MIN_MAP_LOCATION && v <= MAX_MAP_LOCATION);

            reachable[v] = 0;
        }
        free(link);
    }
    

    // Count the length and fill in the safePlace array with reachable locations
    int count = 0;
    for(loc = 0; loc < NUM_MAP_LOCATIONS; loc++) {
        if(reachable[loc] == 1) safePlaces[count++] = loc;
    }

    *numLocations = count;
    return safePlaces;
} 


// Check if the given location is adjacent to Dracula's current location
static int isAdjacent(DracView gameState, LocationID location) {
    assert(gameState != NULL);

    if(location < MIN_MAP_LOCATION || location > MAX_MAP_LOCATION) {
        return FALSE;
    }

    int numLocations = 0;
    LocationID *adLoc = adjacentLocations(gameState, &numLocations);
    assert(adLoc != NULL);
        
    sortLocIDArray(adLoc, 0, numLocations - 1);
    if(isFound(adLoc, location, 0, numLocations - 1) == FALSE) {
        free(adLoc);
        return FALSE;
    }
        
    free(adLoc);
    return TRUE;
}

// Check whether there is double back in the trail
static int hasDBInTrail(DracView gameState) {
    assert(gameState != NULL);

    LocationID dracMoves[TRAIL_SIZE];
    
    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) dracMoves[i] = UNKNOWN_LOCATION;
    giveMeTheMoves(gameState, PLAYER_DRACULA, dracMoves);

    for(i = 0; i < TRAIL_SIZE - 1; i++) {
        if(dracMoves[i] >= DOUBLE_BACK_1 && dracMoves[i] <= DOUBLE_BACK_5) {
            return TRUE;
        }
    }

    return FALSE;
}


// Check how many hunters at the given location
static int numHuntersThere(DracView gameState, LocationID loc) {
    assert(gameState != NULL);
    assert(loc >= MIN_MAP_LOCATION && loc <= MAX_MAP_LOCATION);

    int i, num = 0;
    for(i = 0; i < PLAYER_DRACULA; i++) {
        if(whereIs(gameState, i) == loc) num++;
    }

    return num;
}

// Check whether the Castle Dracula is 'safe'
static int isSafeCastle(DracView gameState) {
    assert(gameState != NULL);

    LocationID trail[TRAIL_SIZE];
    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
    giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

    for(i = 0; i < TRAIL_SIZE; i++) {
        if(trail[i] ==  CASTLE_DRACULA) return FALSE;
    }


    int hunter = 0; 
    int distFromH = NUM_MAP_LOCATIONS + 1;
 
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        int length = 0;
        LocationID *sPath = sPathForHunters(gameState, &length, hunter, whereIs(gameState, hunter),
                                            CASTLE_DRACULA, 1, 1, 1); 
            
        assert(sPath != NULL);

        if(length < distFromH) distFromH = length;
        free(sPath);
    }

    int distFromD = 0;
    LocationID *sPath = shortestPath(gameState, &distFromD, whereIs(gameState, PLAYER_DRACULA),
                                     CASTLE_DRACULA, 1, 1);
   
    int isSafe = FALSE;
    if(distFromD == 0) return FALSE;

    if(distFromH - distFromD >= SAFE_DISTANCE) {
        isSafe = TRUE;
    } 
    free(sPath);

    return isSafe;
}

// Find out if the given location exists in the trail
// Note: wont take the last move in the trail as it falls off
//       from the trail at the beginning of current round
static int positionInTrail(DracView gameState, LocationID location) {
    assert(gameState != NULL);
    
    LocationID trail[TRAIL_SIZE];
    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
    giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

    for(i = 0; i < TRAIL_SIZE - 1; i++) {
        if(trail[i] != UNKNOWN_LOCATION && trail[i] == location) {
            return i;
        }
    }


    return NOT_IN_TRAIL;
}


// Selection Sort
static void sortLocIDArray(LocationID *array, int low, int high) {
    assert(array != NULL);

    int i, j, indexOfMin = 0;

    for(i = low; i < high; i++) {
        indexOfMin = i;

        for(j = i + 1; j <= high; j++) {
            if(array[indexOfMin] > array[j]) indexOfMin = j;
        }

        if(indexOfMin != i) {
            LocationID temp = array[i];
            array[i] = array[indexOfMin];
            array[indexOfMin] = temp;
        }
    }

    // Check whether the array is sorted
    for(i = low; i < high; i++) {
        assert(array[i] <= array[i+1]);
    }
}

// Binary search --> to find whether the given location is in the given array or not
static int isFound(LocationID *array, LocationID location, int low, int high) {
    assert(array != NULL);
    assert(low >= MIN_MAP_LOCATION);
    assert(high <= MAX_MAP_LOCATION);

    int isFound = FALSE;
    int midPoint = 0;

    while(low <= high) {
        midPoint = (low + high) / 2;

        if(array[midPoint] == location) {
            isFound = TRUE;
            break;
        } else if(array[midPoint] < location) {
            low = midPoint + 1;
        } else if(array[midPoint] > location) {
            high = midPoint - 1;
        }
    }

    return isFound;
}

// Function to turn the ID of given move to a two-character string
static void moveToAbbreviation(LocationID move, char *abbrev) {
    assert((move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION) || (move >= HIDE && move <= TELEPORT));
    assert(abbrev != NULL);

    switch(move) {
    case ADRIATIC_SEA:             strcpy(abbrev, "AS");  break;
    case ALICANTE:                 strcpy(abbrev, "AL");  break;
    case AMSTERDAM:                strcpy(abbrev, "AM");  break;
    case ATHENS:                   strcpy(abbrev, "AT");  break; 
    case ATLANTIC_OCEAN:           strcpy(abbrev, "AO");  break;

    case BARCELONA:                strcpy(abbrev, "BA");  break;
    case BARI:                     strcpy(abbrev, "BI");  break;
    case BAY_OF_BISCAY:            strcpy(abbrev, "BB");  break;
    case BELGRADE:                 strcpy(abbrev, "BE");  break;
    case BERLIN:                   strcpy(abbrev, "BR");  break;
    case BLACK_SEA:                strcpy(abbrev, "BS");  break;
    case BORDEAUX:                 strcpy(abbrev, "BO");  break;
    case BRUSSELS:                 strcpy(abbrev, "BU");  break;
    case BUCHAREST:                strcpy(abbrev, "BC");  break;
    case BUDAPEST:                 strcpy(abbrev, "BD");  break;

    case CADIZ:                    strcpy(abbrev, "CA");  break;
    case CAGLIARI:                 strcpy(abbrev, "CG");  break;
    case CASTLE_DRACULA:           strcpy(abbrev, "CD");  break;
    case CLERMONT_FERRAND:         strcpy(abbrev, "CF");  break;
    case COLOGNE:                  strcpy(abbrev, "CO");  break;
    case CONSTANTA:                strcpy(abbrev, "CN");  break;

    case DUBLIN:                   strcpy(abbrev, "DU");  break;
    
    case EDINBURGH:                strcpy(abbrev, "ED");  break;
    case ENGLISH_CHANNEL:          strcpy(abbrev, "EC");  break;

    case FLORENCE:                 strcpy(abbrev, "FL");  break;
    case FRANKFURT:                strcpy(abbrev, "FR");  break;

    case GALATZ:                   strcpy(abbrev, "GA");  break;
    case GALWAY:                   strcpy(abbrev, "GW");  break;
    case GENEVA:                   strcpy(abbrev, "GE");  break;
    case GENOA:                    strcpy(abbrev, "GO");  break;
    case GRANADA:                  strcpy(abbrev, "GR");  break;

    case HAMBURG:                  strcpy(abbrev, "HA");  break;
   
    case IONIAN_SEA:               strcpy(abbrev, "IO");  break;
    case IRISH_SEA:                strcpy(abbrev, "IR");  break;
    
    case KLAUSENBURG:              strcpy(abbrev, "KL");  break;
   
    case LEIPZIG:                  strcpy(abbrev, "LI");  break;
    case LE_HAVRE:                 strcpy(abbrev, "LE");  break;
    case LISBON:                   strcpy(abbrev, "LS");  break;
    case LIVERPOOL:                strcpy(abbrev, "LV");  break;
    case LONDON:                   strcpy(abbrev, "LO");  break;

    case MADRID:                   strcpy(abbrev, "MA");  break;
    case MANCHESTER:               strcpy(abbrev, "MN");  break;
    case MARSEILLES:               strcpy(abbrev, "MR");  break;
    case MEDITERRANEAN_SEA:        strcpy(abbrev, "MS");  break;
    case MILAN:                    strcpy(abbrev, "MI");  break;
    case MUNICH:                   strcpy(abbrev, "MU");  break;

    case NANTES:                   strcpy(abbrev, "NA");  break;
    case NAPLES:                   strcpy(abbrev, "NP");  break;
    case NORTH_SEA:                strcpy(abbrev, "NS");  break;
    case NUREMBURG:                strcpy(abbrev, "NU");  break;

    case PARIS:                    strcpy(abbrev, "PA");  break;
    case PLYMOUTH:                 strcpy(abbrev, "PL");  break;
    case PRAGUE:                   strcpy(abbrev, "PR");  break;

    case ROME:                     strcpy(abbrev, "RO");  break;

    case SALONICA:                 strcpy(abbrev, "SA");  break;
    case SANTANDER:                strcpy(abbrev, "SN");  break;
    case SARAGOSSA:                strcpy(abbrev, "SR");  break;
    case SARAJEVO:                 strcpy(abbrev, "SJ");  break;
    case SOFIA:                    strcpy(abbrev, "SO");  break;
    case ST_JOSEPH_AND_ST_MARYS:   strcpy(abbrev, "JM");  break;
    case STRASBOURG:               strcpy(abbrev, "ST");  break;
    case SWANSEA:                  strcpy(abbrev, "SW");  break;
    case SZEGED:                   strcpy(abbrev, "SZ");  break;

    case TOULOUSE:                 strcpy(abbrev, "TO");  break;
    case TYRRHENIAN_SEA:           strcpy(abbrev, "TS");  break;

    case VALONA:                   strcpy(abbrev, "VA");  break;
    case VARNA:                    strcpy(abbrev, "VR");  break;
    case VENICE:                   strcpy(abbrev, "VE");  break;
    case VIENNA:                   strcpy(abbrev, "VI");  break;

    case ZAGREB:                   strcpy(abbrev, "ZA");  break;
    case ZURICH:                   strcpy(abbrev, "ZU");  break;

    //Special Moves
    case HIDE:                     strcpy(abbrev, "HI");  break;
    case DOUBLE_BACK_1:            strcpy(abbrev, "D1");  break;
    case DOUBLE_BACK_2:            strcpy(abbrev, "D2");  break;
    case DOUBLE_BACK_3:            strcpy(abbrev, "D3");  break;
    case DOUBLE_BACK_4:            strcpy(abbrev, "D4");  break;
    case DOUBLE_BACK_5:            strcpy(abbrev, "D5");  break;
    case TELEPORT:                 strcpy(abbrev, "TP");  break;

    default: 
        fprintf(stdout, "Location of hunter is unknown !!!\n");
        exit(EXIT_FAILURE);
    }
}
