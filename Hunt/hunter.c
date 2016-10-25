// hunter.c
// Implementation of your "Fury of Dracula" hunter AI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "Map.h"
#include "Game.h"
#include "Places.h"
#include "Queue.h"
#include "HunterView.h"
#include "GameView.h"

#define TRUE 1
#define FALSE 0

#define MIN_HEALTH 3

// ***  Private Functions   ***
static int isLegalMove(HunterView gameState, LocationID move);
static int isAdjacent(HunterView gameState, LocationID location);
static int isFound(LocationID *array, LocationID location, int low, int high);

static LocationID firstMove(HunterView gameState);
static LocationID randomMove(HunterView gameState);
static LocationID Rest(HunterView gameState);

static void sortLocIDArray(LocationID *array, int low, int high);
static void moveToAbbreviation(LocationID move, char *abbrev);

static LocationID BestMove(HunterView gameState);

void decideHunterMove(HunterView gameState)
{
    assert(gameState != NULL);

    LocationID move = UNKNOWN_LOCATION;
    Round round = giveMeTheRound(gameState);

    if(round > 0) {
        int health = howHealthyIs(gameState, whoAmI(gameState));

        if(health >= MIN_HEALTH) {
            //move = randomMove(gameState);
	    move = BestMove(gameState);
        } else {
            move = Rest(gameState);
        }
    } else {
        move = firstMove(gameState);
    }


    if(move == UNKNOWN_LOCATION) move = randomMove(gameState);
    assert(move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION);

    char abbrev[2];
    int i = 0;
    for(i = 0; i < 2; i++) abbrev[i] = '\0';
    moveToAbbreviation(move, abbrev);         

    registerBestPlay(abbrev, "Hi Dracula, your death descends HAHA");
}


// ***   Private Functions   ***
static int isLegalMove(HunterView gameState, LocationID move) {
    assert(gameState != NULL);

    return isAdjacent(gameState, move);
}

static LocationID firstMove(HunterView gameState) {
    assert(gameState != NULL);

    PlayerID player = whoAmI(gameState);
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER);

    LocationID move = GENEVA;    

    switch(player) {
    case PLAYER_LORD_GODALMING:  move = SZEGED;             break;
    case PLAYER_DR_SEWARD:       move = FRANKFURT;          break;
    case PLAYER_VAN_HELSING:     move = VENICE;             break;
    case PLAYER_MINA_HARKER:     move = MADRID;             break; 
    }

    return move;
}

static LocationID randomMove(HunterView gameState) {
    assert(gameState != NULL);

    int numLocations = 0;
    LocationID *adLoc = whereCanIgo(gameState, &numLocations, 1, 1, 0);
    assert(adLoc != NULL);   
    assert(numLocations > 0);

    srand(time(NULL));
    LocationID move = adLoc[0];

    if(numLocations > 1) {
        LocationID diff[NUM_MAP_LOCATIONS];

        int i = 0;
        int count = 0;

        for(i = 0; i < NUM_MAP_LOCATIONS; i++) diff[i] = UNKNOWN_LOCATION;
        
        for(i = 0; i < numLocations; i++) {
            if(adLoc[i] != whereIs(gameState, whoAmI(gameState))) {
                diff[count++] = adLoc[i];
            }
        }

        srand(time(NULL));
        int index = rand() % count;
        move = diff[index];
        assert(move != whereIs(gameState, whoAmI(gameState)));     
    }

    free(adLoc); 

    assert(isLegalMove(gameState, move) == TRUE); 
    return move;
}

static LocationID Rest(HunterView gameState) {
    assert(gameState != NULL);
 
    return whereIs(gameState, whoAmI(gameState));
}

static int isAdjacent(HunterView gameState, LocationID location) {
    assert(gameState != NULL);

    if(location < MIN_MAP_LOCATION || location > MAX_MAP_LOCATION) {
        return FALSE;
    }

    int numLocations = 0;
    LocationID *adLoc = whereCanIgo(gameState, &numLocations, 1, 1, 1);
    assert(adLoc != NULL);
    
    sortLocIDArray(adLoc, 0, numLocations - 1);
    if(isFound(adLoc, location, 0, numLocations - 1) == FALSE) {
        free(adLoc);
        return FALSE;
    }

    free(adLoc);

    return TRUE;
}

// Selection Sort
static void sortLocIDArray(LocationID *array, int low, int high) {
    assert(array != NULL);
    assert(low >= MIN_MAP_LOCATION);
    assert(high <= MAX_MAP_LOCATION);

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


// Convert the given move into a two-character string
static void moveToAbbreviation(LocationID move, char *abbrev) {
    assert(move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION);
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

    default: 
        fprintf(stdout, "Location of hunter is unknown !!!\n");
        exit(EXIT_FAILURE);
    }

}

static LocationID BestMove(HunterView gameState){
// essentially this tactic aims to check if any of the possible locations which the hunter can enter in is in draculas trail. 
//If the hunter has the option to move to a locaiton which is in draculas trail then they do as they are then closer to dracula. 

	assert(gameState != NULL);		// assert game is valid
	PlayerID player = whoAmI(gameState);		// find out who the player is
	assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER);
	
	LocationID bestMove = UNKNOWN_LOCATION;	// move will be the best possible move
	int numLocations = 0;
	LocationID *adLoc = whereCanIgo(gameState, &numLocations,1,1,0);		//find all possible locations
	assert(adLoc != NULL);
	assert(numLocations > 0);

	LocationID *trail = giveMeTheTrail(gameState,PLAYER_DRACULA,trail);		// return an array of draculas trail

	int i,j;
	// check if any of the possible moves are in draculas trail
	// if the location is in dracs trail then move to this location as hunter is then closer to dracula
	for(i = 0; i < numLocations; i++){
		for(j = 0; j < TRAIL_SIZE; j++){
			if(adLoc[i] == trail[j]) {
				bestMove = adLoc[i];
				assert(isLegalMove(gameState, bestMove) == TRUE); 
				return bestMove;
			}
		}
	}
	// if none of the possible moves are in dracs trail then just make a random move
	return bestMove = randomMove(gameState);	
}
