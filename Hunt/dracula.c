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

// ***  Private Functions   ***
static int isLegalMove(DracView gameState, LocationID move);
static int isFound(LocationID *array, LocationID location, int low, int high);
static int isAdjacent(DracView gameState, LocationID location);
static int numHuntersThere(DracView gameState, LocationID loc);

static LocationID firstMove(DracView gameState);
static LocationID randomMove(DracView gameState);
static LocationID lowHPMove(DracView gameState);
static LocationID highHPMove(DracView gameState);
 
static void sortLocIDArray(LocationID *array, int low, int high);
static void idToAbbrev(LocationID move, char *abbrev);


// Function to decide the move of Dracula
void decideDraculaMove(DracView gameState) {
    assert(gameState != NULL);
    srand(time(NULL));

    int i = 0;
    Round round = giveMeTheRound(gameState); 
    LocationID move = UNKNOWN_LOCATION;
   
    if(round > 0) {     
        int health = howHealthyIs(gameState, PLAYER_DRACULA);
        
        if(health > 26) {
            move = highHPMove(gameState);
        } else {
            move = lowHPMove(gameState);
        } 

    } else {
        // Move in first round (Round 0)
        move = firstMove(gameState);
    } 


    // Send the move to the game engine
    char abbrev[2];
    for(i = 0; i <= 2; i++) abbrev[i] = '\0';
    if(move == UNKNOWN_LOCATION) move = TELEPORT;
    idToAbbrev(move, abbrev);

    registerBestPlay(abbrev, "Dracula is coming");
}



// ***   Private Functions   ***
// Determine whether the given move is legal or not
static int isLegalMove(DracView gameState, LocationID move) {
    assert(gameState != NULL);

    if(move < MIN_MAP_LOCATION || (move > MAX_MAP_LOCATION && move < HIDE) ||
       move >= TELEPORT) 
    {
        return FALSE;
    }
    
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
        if(move == ST_JOSEPH_AND_ST_MARYS) {
            return FALSE;
        }  

        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] == move) {
                return FALSE;
            }
        } 
        
        if(isAdjacent(gameState, move) == FALSE) return FALSE;
 
    } else if(move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5) {
        Round round = giveMeTheRound(gameState);
        if(move - DOUBLE_BACK_1 >= round) return FALSE;

        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] >= DOUBLE_BACK_1 && dracMoves[i] <= DOUBLE_BACK_5) {
                return FALSE;
            }
        }

        int pos = move - DOUBLE_BACK_1;
        if(dracMoves[pos] == HIDE) pos++;
        if(pos > 4) return FALSE;
        move = dracMoves[pos]; 
                
        if(move < MIN_MAP_LOCATION || move > MAX_MAP_LOCATION) return FALSE; 
        if(isAdjacent(gameState, move) == FALSE) return FALSE; 

    } else if(move == HIDE) {
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
    
    int i, j = 0;
    LocationID *startLoc = malloc(4 * sizeof(LocationID));
    assert(startLoc != NULL);
    startLoc[0] = SALONICA;
    startLoc[1] = CADIZ;
    startLoc[2] = PLYMOUTH;
    startLoc[3] = VIENNA;

    LocationID firstMove = UNKNOWN_LOCATION;
    int noHunterThere = TRUE;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < PLAYER_DRACULA; j++) {
            if(whereIs(gameState, j) == startLoc[i]) {
                noHunterThere = FALSE;
                break;
            }
        }
            
        if(noHunterThere == TRUE) {
            firstMove = startLoc[i];
            break;
        }
    }    

    if(firstMove == UNKNOWN_LOCATION) firstMove = CASTLE_DRACULA;
    free(startLoc);

    return firstMove;
} 

// Function to make a random (and legal) move
static LocationID randomMove(DracView gameState) {
    assert(gameState != NULL);

    int numLocations = 0;
    LocationID *connLoc = whereCanIgo(gameState, &numLocations, 1, 1);
    assert(numLocations >= 1);
    assert(connLoc != NULL);

    LocationID move = UNKNOWN_LOCATION;

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

    if(numLM > 0) {
        int index = rand() % numLM;
        move = legalMoves[index];

        if(numHuntersThere(gameState, move) > 0) {
            if(numLM > 1) {
                for(i = 0; i < numLM; i++) {
                    if(numHuntersThere(gameState, legalMoves[i]) == 0) {
                        move = legalMoves[i];
                        break;
                    } 
                }

            } else {
                if(isLegalMove(gameState, HIDE) == TRUE) {
                    move = HIDE;
                } else if(isLegalMove(gameState, DOUBLE_BACK_1) == TRUE) {
                    move = DOUBLE_BACK_1;
                }  
            }
        }    

    } else {
        if(isLegalMove(gameState, HIDE) == TRUE) {
            move = HIDE;
        } else if(isLegalMove(gameState, DOUBLE_BACK_1) == TRUE) {
            move = DOUBLE_BACK_1;
        } else {
            move = TELEPORT;
        }
    }
   
    free(connLoc);
    free(legalMoves);
 
    return move;
}

static LocationID lowHPMove(DracView gameState) {
    assert(gameState != NULL);

    LocationID curr = whereIs(gameState, PLAYER_DRACULA);
    if(curr == CASTLE_DRACULA && isLegalMove(gameState, HIDE) == TRUE) {
        return HIDE;
    } else if(curr == CASTLE_DRACULA && isLegalMove(gameState, DOUBLE_BACK_1) == TRUE) {
        return DOUBLE_BACK_1;
    }

    int length = 0;
    LocationID *sPath = shortestPath(gameState, &length, curr, CASTLE_DRACULA, 1, 1);

    if(length > 1) {
        assert(sPath != NULL);
        LocationID next = sPath[1];
        
        if(isLegalMove(gameState, next) == TRUE) {
            return next;
        } else {
            LocationID dracMoves[TRAIL_SIZE];
            giveMeTheMoves(gameState, PLAYER_DRACULA, dracMoves);
            
            int i = 0;
            for(i = 0; i < TRAIL_SIZE - 1; i++) {
                if(dracMoves[i] == TELEPORT) dracMoves[i] = CASTLE_DRACULA;
            }           

            int DBackPos = -1;
            for(i = 0; i < TRAIL_SIZE - 1; i++) {
                if(dracMoves[i] == next) {
                    DBackPos = i;
                    break;
                }
            }

            if(DBackPos != -1) {
                next = DOUBLE_BACK_1 + DBackPos;
                assert(next >= DOUBLE_BACK_1 && next <= DOUBLE_BACK_5);
              
                return next;
            } else {
                return randomMove(gameState);
            }

        } 
    } else {
        return randomMove(gameState);
    }
}


static LocationID highHPMove(DracView gameState) {
    assert(gameState != NULL);
    return randomMove(gameState);
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
        
    sortLocIDArray(adLoc, 0, numLocations);
    if(isFound(adLoc, location, 0, numLocations) == FALSE) {
        free(adLoc);
        return FALSE;
    }
        
    free(adLoc);
    return TRUE;
}

static int numHuntersThere(DracView gameState, LocationID loc) {
    assert(gameState != NULL);
    assert(loc >= MIN_MAP_LOCATION && loc <= MAX_MAP_LOCATION);

    int i, num = 0;
    for(i = 0; i < PLAYER_DRACULA; i++) {
        if(whereIs(gameState, i) == loc) num++;
    }

    return num;
}


// Selection Sort
static void sortLocIDArray(LocationID *array, int low, int high) {
    assert(array != NULL);
    assert(low >= MIN_MAP_LOCATION);
    assert(high <= MAX_MAP_LOCATION);

    int i, j, indexOfMin = 0;

    for(i = low; i < high - 1; i++) {
        indexOfMin = i;

        for(j = i; j < high - 1; j++) {
            if(array[indexOfMin] > array[j]) indexOfMin = j;
        }

        if(indexOfMin != i) {
            LocationID temp = array[i];
            array[i] = array[indexOfMin];
            array[indexOfMin] = temp;
        }
    }

    // Check whether the array is sorted
    for(i = low; i < high - 1; i++) {
        assert(array[i] < array[i+1]);
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
static void idToAbbrev(LocationID move, char *abbrev) {
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
