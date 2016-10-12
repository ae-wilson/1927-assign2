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
#define SAFE_DISTANCE 5

// ***  Private Functions   ***
static int isLegalMove(DracView gameState, LocationID move);
static int isFound(LocationID *array, LocationID location, int low, int high);
static int isAdjacent(DracView gameState, LocationID location);
static int isSafeCastle(DracView gameState);
static int hasDBInTrail(DracView gameState);
static int numHuntersThere(DracView gameState, LocationID loc);
static int *occupiedPlaces(DracView gameState);

static LocationID firstMove(DracView gameState);
static LocationID BestMove(DracView gameState);
static LocationID legalMove(DracView gameState);
static LocationID positionInTrail(DracView gameState, LocationID location);
static LocationID goToLandOrSea(DracView gameState);
static LocationID backToCastle(DracView gameState);
static LocationID awayFromHunters(DracView gameState);
static LocationID doubleBackToSafeLoc(DracView gameState);

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

    registerBestPlay(abbrev, "Hunters, Listen and Obey!");
}



// ***   Private Functions   ***
// Determine whether the given move is legal or not
static int isLegalMove(DracView gameState, LocationID move) {
    assert(gameState != NULL);

    // Unknown moves = illegal
    if(move < MIN_MAP_LOCATION || (move > MAX_MAP_LOCATION && move < HIDE) ||
       move > TELEPORT) 
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
        if(hasDBInTrail(gameState)) return FALSE;

        // Find out where Dracula doubled back to
        int pos = move - DOUBLE_BACK_1;
        if(dracMoves[pos] == HIDE) {
            pos++;
            
            // Because that move is no longer in the trail (i.e 6th move)
            if(pos > 4) return FALSE;
        } 

        if(dracMoves[pos] == TELEPORT) {
            move = CASTLE_DRACULA;
        } else {
            move = dracMoves[pos];
        }

                
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
        LocationID currLoc = whereIs(gameState, PLAYER_DRACULA);
        if(idToType(currLoc) == SEA) return FALSE;
    }

    return TRUE;
}

// Function to decide where Dracula goes firstly
static LocationID firstMove(DracView gameState) {
    assert(gameState != NULL);
    
    int i = 0;
    int *occupied = occupiedPlaces(gameState);
    assert(occupied != NULL); 
  

    // Count the number of safe starting point 
    int count = 0;
    LocationID safeFirstMove[NUM_MAP_LOCATIONS];
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) safeFirstMove[i] = 0; 

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
        LocationID location = (LocationID) i;
        if(idToType(location) == SEA) continue;
        if(location == ST_JOSEPH_AND_ST_MARYS) continue;

        if(!occupied[location]) safeFirstMove[count++] = location;
    }

    free(occupied);
     

    // Randomly choose safe starting point
    srand(time(NULL));
    int index = rand() % count;
    LocationID firstMove = safeFirstMove[index];

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

    if(move == UNKNOWN_LOCATION) move = legalMove(gameState);
    
    return move;
}

// Function to Legal (priority) move
static LocationID legalMove(DracView gameState) {
    assert(gameState != NULL);

    // Find out where Dracula can go
    int numLocations = 0;
    LocationID *connLoc = whereCanIgo(gameState, &numLocations, 1, 1);
    assert(numLocations >= 1);
    assert(connLoc != NULL);


    // Use the isLegalMove Function to find out all the possible LEGAL moves
    // Note: not include DOUBLE BACK / HIDE
    int i = 0;
    int numLM = 0;
    LocationID legalMoves[NUM_MAP_LOCATIONS];
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) legalMoves[i] = UNKNOWN_LOCATION;
    for(i = 0; i < numLocations; i++) {
        LocationID v = connLoc[i];

        if(isLegalMove(gameState, v)) {
            legalMoves[numLM++] = v;
        }    
    }
    free(connLoc);


    // A slightly different version of occupiedPlaces
    int hunter = 0;   
    int occupied[NUM_MAP_LOCATIONS];
    for(i = 0; i < NUM_MAP_LOCATIONS; i++) occupied[i] = 0;
    occupied[ST_JOSEPH_AND_ST_MARYS] = 999;
  
    // Find where hunters can go in next round
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter, 1, 1, 1);
        assert(connLoc != NULL);

        for(i = 0; i < numLocations; i++) {
            LocationID v = connLoc[i];
            
            if(idToType(v) == SEA) continue;

            occupied[v] = occupied[v] + 1;
        }

        free(connLoc);
    }


    // For the game log
    printf("Making a Legal Move ......\n");
    printf("Number Of Legal Moves = %d\n", numLM);
    for(i = 0; i < numLM; i++) {
        LocationID v = legalMoves[i];
        printf("%s\n", idToName(v));
    }
    printf("\n");


    
    LocationID move = UNKNOWN_LOCATION;

    if(numLM > 0) {
        int risk = 0;  // Number of hunters who may go to there;

        // For Game Log
        printf("Try to escape from hunters' tracking ......\n\n");

        for(risk = 0; risk < NUM_PLAYERS; risk++) {
            for(i = 0; i < numLM; i++) {
                LocationID v = legalMoves[i];
 
                if(occupied[v] == risk) {

                    move = v;
                    break;
                }
            }
        }


        if(!hasDBInTrail(gameState)) {
            printf("Try to use Double Back to escape from tracking\n\n");

            LocationID trail[TRAIL_SIZE];
            for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
            giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

            for(i = 0; i < TRAIL_SIZE - 1; i++) {
                LocationID v = trail[i];
                LocationID double_back = DOUBLE_BACK_1 + i;
                        
                if(isLegalMove(gameState, double_back) && v != UNKNOWN_LOCATION) {
                    if(occupied[v] < risk) {
                        move = double_back;
                    }
                }
            }
             
            // Try to use HIDE instead of DOUBLE_BACK_1
            if(move == DOUBLE_BACK_1) {
                if(isLegalMove(gameState, HIDE)) move = HIDE;
            } 
        } else {

            // Try to stay at the same location and use traps 
            // to kill hunters before the encounter
            LocationID currLoc = whereIs(gameState, PLAYER_DRACULA);
            if(occupied[currLoc] < risk) {
                if(isLegalMove(gameState, currLoc)) {
                    printf("Place more millions / Stay at the same sea ......\n\n");
                    move = currLoc;
                } else if(isLegalMove(gameState, HIDE)) {
                    printf("Place more millions / Stay at the same sea ......\n\n");
                    move = HIDE;
                } else if(isLegalMove(gameState, DOUBLE_BACK_1)) {
                    printf("Place more millions / Stay at the same sea ......\n\n");               
                    move = DOUBLE_BACK_1;
                }  
            }
        }


        // Make sure there is at least a legal move
        if(move == UNKNOWN_LOCATION) {
            printf("Bug! :(\n\n");
            srand(time(NULL));
            move = legalMoves[rand() % numLM];
        }
    } else {

        // If DB/HI are possible actions for Dracula only 
        if(!hasDBInTrail(gameState)) {
            LocationID trail[TRAIL_SIZE];
            for(i = 0; i < TRAIL_SIZE; i++) trail[i] = UNKNOWN_LOCATION;
            giveMeTheTrail(gameState, PLAYER_DRACULA, trail);

            int risk = 0;
            for(risk = 0; risk < NUM_PLAYERS; risk++) {
                for(i = 0; i < TRAIL_SIZE - 1; i++) {
                    LocationID v = trail[i];
                    LocationID double_back = DOUBLE_BACK_1 + i;
                        
                    if(isLegalMove(gameState, double_back) && v != UNKNOWN_LOCATION) {
                        if(occupied[v] == risk) move = double_back;
                    }
                }
            } 
          
            // Try to use HIDE instead of DOUBLE_BACK_1
            if(move == DOUBLE_BACK_1) {
                if(isLegalMove(gameState, HIDE)) move = HIDE;
            } 

            // Make sure there is at least a legal Double back
            if(move == UNKNOWN_LOCATION) move = DOUBLE_BACK_1;
        } else if(isLegalMove(gameState, HIDE)) {
            move = HIDE;
        } else {
            // There is no legal moves and Dracula will be 
            // teleported to his castle
            move = TELEPORT;
        }
    }
   
 
    return move;
}

// Determine whether Dracula should go to a 'safe' port city
// or escape from hunters
static LocationID goToLandOrSea(DracView gameState) {
    assert(gameState != NULL);
    assert(idToType(whereIs(gameState, PLAYER_DRACULA)) == SEA);
   
        
    // Find out all the connected port cities     
    int numPorts = 0;
    LocationID *ports = connectedPorts(gameState, &numPorts);
    assert(ports != NULL);

    // Find out all the cities are occupied by hunters (expect hunters are at the sea)
    int *occupied = occupiedPlaces(gameState);
    assert(occupied != NULL);       


    // Find out the 'safe' / unoccupied port cities
    int i, numSP = 0;
    LocationID safePorts[NUM_MAP_LOCATIONS];

    for(i = 0; i < numPorts; i++) {
        LocationID p = ports[i];

        // Ports which are not 'safe' / or need to be checked
        if(p == ATHENS)    continue;
        if(p == PLYMOUTH)  continue;
        if(p == BARI)      continue;

        if(p == SALONICA || p == VALONA) {
            if(occupied[SOFIA])      continue;
            if(occupied[BUCHAREST])  continue;
        }

        if(p == CAGLIARI) {
            if(numHuntersThere(gameState, MEDITERRANEAN_SEA) > 0) continue; 
            if(numHuntersThere(gameState, TYRRHENIAN_SEA) > 0)    continue;
        }   

        if(p == GALWAY || p == DUBLIN) { 
            if(numHuntersThere(gameState, IRISH_SEA) > 0)       continue;
            if(numHuntersThere(gameState, ATLANTIC_OCEAN) > 0)  continue;
        }

        if(p == EDINBURGH || p == LIVERPOOL || p == LONDON ||  
           p == SWANSEA) 
        {
            if(occupied[EDINBURGH])  continue;
            if(occupied[LIVERPOOL])  continue;
            if(occupied[LONDON])     continue;
            if(occupied[MANCHESTER]) continue;
            if(occupied[PLYMOUTH])   continue;
            if(occupied[SWANSEA])    continue;
            if(numHuntersThere(gameState, ENGLISH_CHANNEL) > 0) continue;
        }

        if(!occupied[p]) safePorts[numSP++] = p;
    }

    free(ports);       
    free(occupied); 


    LocationID move = UNKNOWN_LOCATION;

    if(numSP > 0 && numHuntersThere(gameState, whereIs(gameState, PLAYER_DRACULA)) == 0) {
        srand(time(NULL));    
        int index = rand() % numSP;

        printf("Landing ......\n\n");
        move = safePorts[index];
        assert(isLegalMove(gameState, move)); 
    } else {
        printf("Unable to Land :( ......\n\n");
        
        int numSeas = 0;
        LocationID *connSeas = whereCanIgo(gameState, &numSeas, 0, 1);
        assert(connSeas != NULL);

        int hunter = 0;   
        int occupiedSeas[NUM_MAP_LOCATIONS];
        for(i = 0; i < NUM_MAP_LOCATIONS; i++) occupiedSeas[i] = 0;
 
 
        for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
            if(idToType(whereIs(gameState, hunter)) == SEA) continue;

            int numLocations = 0;
            LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter, 0, 0, 1);
            assert(connLoc != NULL);

            for(i = 0; i < numLocations; i++) {
                LocationID sea = connLoc[i];
                occupiedSeas[sea] = 1;
            }

            free(connLoc);
        }


        int nSafeSeas;
        LocationID safeSeas[NUM_MAP_LOCATIONS];
        for(i = 0; i < NUM_MAP_LOCATIONS; i++) {
            safeSeas[i] = UNKNOWN_LOCATION;
        }

        for(i = 0; i < numSeas; i++) {
            LocationID sea = connSeas[i];

            if(numHuntersThere(gameState, sea) == 0 && !occupiedSeas[sea]) {
                safeSeas[nSafeSeas++] = sea;
            }
        }
     
        free(connSeas); 
        

        if(nSafeSeas > 0) {
            srand(time(NULL));
            int index = rand() % nSafeSeas;
            move = safeSeas[index];
        } else if(!hasDBInTrail(gameState)) {
            move = doubleBackToSafeLoc(gameState);
        }

    }
 
 
    if(move == UNKNOWN_LOCATION) move = legalMove(gameState);

    return move;
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


// Determine what to do next in order to go back to Castle Dracula
// Note: shortest Path
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
    int length = 0;
    LocationID *sPath = shortestPath(gameState, &length, curr, CASTLE_DRACULA, 1, 1);


    LocationID move = UNKNOWN_LOCATION;

    if(length > 1) {
        assert(sPath != NULL);
        LocationID next = sPath[1];
      
        // If the next move in shortest Path is currently occupied by hunters
        if(numHuntersThere(gameState, next) > 0) return awayFromHunters(gameState);
 

        if(isLegalMove(gameState, next) == TRUE) {
            printf("(`vv`) --> Castle Dracula\n\n");

            move = next;
        } else {
            move = awayFromHunters(gameState);
        } 
             
        free(sPath);
         
    } else {
        
        // Escape when there is no shortest path
        move = awayFromHunters(gameState);
    }


    // Make sure that Dracula takes a random and of course a legal move
    if(move == UNKNOWN_LOCATION) {
        move = legalMove(gameState);
    }

    return move;
}

// Determine move which will make Dracula moves away from hunters
static LocationID awayFromHunters(DracView gameState) {
    assert(gameState != NULL);
    LocationID move = UNKNOWN_LOCATION;         

    // Discover where hunters can't reach while Dracula can reach
    int numSL = 0;
    LocationID *safeLoc = safeConnectedLocations(gameState, &numSL, 1, 0);
    assert(safeLoc != NULL);  
 
     
    if(numSL <= 1) {
        free(safeLoc);

        safeLoc = safeConnectedLocations(gameState, &numSL, 1, 1);
        assert(safeLoc != NULL);  
    } 

    // For the game Log
    printf("Safe Locations:\n");
    printf("Number of Safe Locations = %d\n\n", numSL);
    
    int n = 0;
    for(n = 0; n < numSL; n++) {
        printf("%s\n", idToName(safeLoc[n]));
    }
    printf("\n");

 

    if(numSL > 1) {
        srand(time(NULL));
        int loc = rand() % numSL;
 
        // Try not stay at the same place
        if(safeLoc[loc] == whereIs(gameState, PLAYER_DRACULA)) {
            if(!isLegalMove(gameState, whereIs(gameState, PLAYER_DRACULA)) && 
               isLegalMove(gameState, HIDE)) 
            {
                move = HIDE;
            } else { 
                int i = 0;
                int count = 0;
                int random[NUM_MAP_LOCATIONS];

                for(i = 0; i < NUM_MAP_LOCATIONS; i++) random[i] = -1;

                for(i = 0; i < numSL; i++) {
                    if(safeLoc[i] != whereIs(gameState, PLAYER_DRACULA)) random[count++] = safeLoc[i];
                }

                srand(time(NULL));
                loc = rand() % count;

                printf("\nGo to random safe spot ......\n");

                move = random[loc];
            } 
        } else {
            move = safeLoc[loc];
        }
         
        assert(isLegalMove(gameState, move) == TRUE);
    } else if(numSL == 1) {
        
        // If the safe spot is at Dracula's current location, take Hide or Double back move
        LocationID v = safeLoc[0];

        if(v == whereIs(gameState, PLAYER_DRACULA)) {
            if(isLegalMove(gameState, HIDE) == TRUE) {
                move = HIDE;
            } else if(!hasDBInTrail(gameState)) {
                move = doubleBackToSafeLoc(gameState); 
            }
        } else {
            move = v;
            assert(isLegalMove(gameState, move) == TRUE);
        }
    } else {

        // Consider Double Back in order to go to a safe spot
        if(!hasDBInTrail(gameState)) {
            move = doubleBackToSafeLoc(gameState);             
        }
    }


    free(safeLoc);

    // Make sure Dracula will make a random and legal move
    if(move == UNKNOWN_LOCATION) move = legalMove(gameState);    

    return move;
}

// Use Double back to go to safe Location
static LocationID doubleBackToSafeLoc(DracView gameState) {
    assert(gameState != NULL);
    assert(!hasDBInTrail(gameState));    

    LocationID move = UNKNOWN_LOCATION;

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

            // Seas are safe though -2 HP when Dracula is at sea
            if(idToType(v) == SEA) continue;

            occupied[v] = 1;
        }
        free(connLoc);
    }

    for(i = TRAIL_SIZE - 2; i >= 0; i--) {
        LocationID loc = trail[i];

        if(loc != UNKNOWN_LOCATION) {
            if(!occupied[loc] && isLegalMove(gameState, DOUBLE_BACK_1 + i)) {
                printf("\nDouble Back to safe spot ......\n");

                move = DOUBLE_BACK_1 + i;
                break;
            }
        }
    }


    return move;
}

// Find out places where are not connected to hunters' locations
static LocationID *safeConnectedLocations(DracView gameState, int *numLocations, int road, int sea) {
    assert(gameState != NULL);

    // Initialise
    int loc = 0;
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
    int *occupied = occupiedPlaces(gameState);
    assert(occupied != NULL);
    
    
    // Count the length and fill in the safePlace array with reachable locations
    int count = 0;
    for(loc = 0; loc < NUM_MAP_LOCATIONS; loc++) {
        if(reachable[loc]) {
            if(!occupied[loc]) {
                safePlaces[count++] = loc;
            }
        }
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

static int *occupiedPlaces(DracView gameState) {
    assert(gameState != NULL);

    int i = 0;
    int hunter = 0;   
    int *occupied = malloc(NUM_MAP_LOCATIONS * sizeof(int));
    assert(occupied != NULL);

    for(i = 0; i < NUM_MAP_LOCATIONS; i++) occupied[i] = 0;
    occupied[ST_JOSEPH_AND_ST_MARYS] = 1;
  
 
    // Find where hunters can go in next round
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        int numLocations = 0;
        LocationID *connLoc = whereHuntersCanGoNext(gameState, &numLocations, hunter, 1, 1, 1);
        assert(connLoc != NULL);

        for(i = 0; i < numLocations; i++) {
            LocationID v = connLoc[i];
            occupied[v] = 1;
        }

        free(connLoc);
    }

    return occupied;
}


// Check whether the Castle Dracula is 'safe'
static int isSafeCastle(DracView gameState) {
    assert(gameState != NULL);

    if(positionInTrail(gameState, CASTLE_DRACULA) != NOT_IN_TRAIL) {
        return FALSE;
    }

    int hunter = 0;
    int greaterThanSD = 0;
    int lessThanSD = 0;
 
    for(hunter = 0; hunter < PLAYER_DRACULA; hunter++) {
        int length = 0;
        LocationID *sPath = sPathForHunters(gameState, &length, hunter, whereIs(gameState, hunter),
                                            CASTLE_DRACULA, 1, 1, 1); 
            
        assert(sPath != NULL);

        if(length < SAFE_DISTANCE) {
            lessThanSD++;
        } else {
            greaterThanSD++;
        }

        free(sPath);
    }

    int distFromD = 0;
    LocationID *sPath = shortestPath(gameState, &distFromD, whereIs(gameState, PLAYER_DRACULA),
                                     CASTLE_DRACULA, 1, 1);
   
    int isSafe = FALSE;
    if(distFromD == 0) return FALSE;

    if(distFromD < SAFE_DISTANCE - 1 && lessThanSD <= 1) {
        isSafe = TRUE;
    } 
    free(sPath);

    return isSafe;
}

static LocationID positionInTrail(DracView gameState, LocationID location) {
    assert(gameState != NULL);

    LocationID dracMoves[TRAIL_SIZE];
    giveMeTheMoves(gameState, PLAYER_DRACULA, dracMoves);

    int i = 0;
    for(i = 0; i < TRAIL_SIZE - 1; i++) {
        LocationID v = dracMoves[i];

        if(v != UNKNOWN_LOCATION) {
            if(v == location) return i;
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
