// dracula.c
// Implementation of your "Fury of Dracula" Dracula AI

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "Map.h"
#include "Game.h"
#include "Places.h"
#include "Queue.h"
#include "DracView.h"

#define TRUE 1
#define FALSE 0

// ***  Private Functions   ***
static int isLegalMove(DracView gameState, LocationID move);
static int isFound(LocationID *connLoc, LocationID location, int low, int high); 
static void sortLocIDArray(LocationID *connLoc, int low, int high);

static void idToAbbrev(LocationID move, char *abbrev);


void decideDraculaMove(DracView gameState)
{
	// TODO ...
	// Replace the line below by something better

    char *abbrev = malloc(4 * sizeof(char));
    assert(abbrev != NULL);
    idToAbbrev(2, abbrev);
    free(abbrev); //dummy

    printf("%d\n", isLegalMove(gameState, CASTLE_DRACULA));

    registerBestPlay("CD","Mwuhahahaha");
}



// ***   Private Functions   ***
static int isLegalMove(DracView gameState, LocationID move) {
    assert(gameState != NULL);
    assert((move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION) || (move >= HIDE && move <= TELEPORT));
    
    LocationID *dracTrail = malloc(TRAIL_SIZE * sizeof(LocationID));
    assert(dracTrail != NULL);
    LocationID *dracMoves = malloc(TRAIL_SIZE * sizeof(LocationID));
    assert(dracMoves != NULL);

    int i = 0;
    for(i = 0; i < TRAIL_SIZE; i++) {
        dracTrail[i] = UNKNOWN_LOCATION;
        dracMoves[i] = UNKNOWN_LOCATION;
    }

    giveMeTheTrail(gameState, PLAYER_DRACULA, dracTrail);
    giveMeTheMoves(gameState, PLAYER_DRACULA, dracMoves);

    int isLegal = TRUE;
    if(move >= MIN_MAP_LOCATION && move <= MAX_MAP_LOCATION) {
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracTrail[i] == move) {
                isLegal = FALSE;
                break;
            }
        } 
       
        if(isLegal != FALSE) { 
            int numLocations = 0;
            LocationID *connLoc = whereCanIgo(gameState, &numLocations, 1, 1);
            assert(connLoc != NULL);
            sortLocIDArray(connLoc, 0, numLocations);
            
            if(isFound(connLoc, move, 0, numLocations) != TRUE) isLegal = FALSE;

            free(connLoc);
        }
    } else if(move == HIDE) {
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] == HIDE) {
                isLegal = FALSE;
                break;
            }
        }
    } else if(move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5) {
        for(i = 0; i < TRAIL_SIZE - 1; i++) {
            if(dracMoves[i] >= DOUBLE_BACK_1 && dracMoves[i] <= DOUBLE_BACK_5) {
                isLegal = FALSE;
                break;
            }
        }
    } 

    free(dracTrail);
    free(dracMoves);

    return isLegal;
}


static void sortLocIDArray(LocationID *connLoc, int low, int high) {
    assert(connLoc != NULL);
    assert(low >= MIN_MAP_LOCATION);
    assert(high <= MAX_MAP_LOCATION);

    int i, j, indexOfMin = 0;

    for(i = low; i < high - 1; i++) {
        indexOfMin = i;

        for(j = i; j < high - 1; j++) {
            if(connLoc[indexOfMin] > connLoc[j]) indexOfMin = j;
        }

        if(indexOfMin != i) {
            LocationID temp = connLoc[i];
            connLoc[i] = connLoc[indexOfMin];
            connLoc[indexOfMin] = temp;
        }
    }

    // Check whether the given array is sorted
    for(i = low; i < high - 1; i++) assert((connLoc[i] < connLoc[i+1]) == TRUE);

}

// Binary search
static int isFound(LocationID *connLoc, LocationID location, int low, int high) {
    assert(connLoc != NULL);
    assert(low >= MIN_MAP_LOCATION);
    assert(high <= MAX_MAP_LOCATION);

    int isFound = FALSE;
    int midPoint = 0;

    while(low <= high) {
        midPoint = (low + high) / 2;

        if(connLoc[midPoint] == location) {
            isFound = TRUE;
            break;
        } else if(connLoc[midPoint] < location) {
            low = midPoint + 1;
        } else if(connLoc[midPoint] > location) {
            high = midPoint - 1;
        }
    }

    return isFound;
}

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
