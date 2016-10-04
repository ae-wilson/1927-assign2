// GameView.c ... GameView ADT implementation
// COMP1927 16s2 ... ... basic GameView (supplied code)
// Code by TheGroup from COMP1927 14s2

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Places.h"
#include "Map.h"

#define TURN_LENGTH 8

// Encounterable items in cities
#define MAX_TRAPS 3
#define NO_ITEM -1

static int matches (char *pastPlays, char *input, int index);
static int numTurns (char *pastPlays);
static void testMatches ();
static void addLocation (GameView view, PlayerID player, LocationID newLoc);
static int seaLocation (LocationID loc);


struct city {
    // Values are either NO_ITEM = -1 for not existant, or the turn number the item was placed
    int trap[MAX_TRAPS];
    int vamp;
};

struct gameView {
    int score;
    int lifePts[4];
    int bloodPts;

    //these are all typedefed as ints
    LocationID locHistory[NUM_PLAYERS][TRAIL_SIZE];
    Round round;
    PlayerID currPlayer;

    // Trap and immature vamp tracking
    struct city cities[NUM_MAP_LOCATIONS];
};


// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[])
{
    testMatches();

    GameView view = malloc(sizeof(struct gameView));

    int i, j;
    view->score = GAME_START_SCORE;
    for (i = 0; i < NUM_PLAYERS - 1; i++)
        view->lifePts[i] = GAME_START_HUNTER_LIFE_POINTS;

    view->bloodPts = GAME_START_BLOOD_POINTS;

    for (i = 0; i < NUM_PLAYERS; i++)
        for (j = 0; j < TRAIL_SIZE; j++)
            view->locHistory[i][j] = NOWHERE;

    for (i=0; i < NUM_MAP_LOCATIONS; i++ )
    {
        for (j=0; j< MAX_TRAPS; j++ )
            view->cities[i].trap[j] = NO_ITEM;
        view->cities[i].vamp = NO_ITEM;
    }



    int currPlayer = PLAYER_LORD_GODALMING;
    int index;

    int totalTurns = numTurns(pastPlays);
    int turn = 0;
    int dracOnWater = FALSE;
    while (turn < totalTurns) {
        index = turn * TURN_LENGTH; //index in pastPlays string

        if (matches(pastPlays, "D------", index)) { //drac's turn

            currPlayer = PLAYER_DRACULA;

            //score decreases when it's drac's turn
            view->score -= SCORE_LOSS_DRACULA_TURN;

            //settin drac's location for all the different location codes he could have
            LocationID newLoc;
            if (matches(pastPlays, "DC?", index)) {
                newLoc = CITY_UNKNOWN;
                dracOnWater = FALSE;
            } else if (matches(pastPlays, "DS?----", index)) {
                newLoc = SEA_UNKNOWN;
                dracOnWater = TRUE;
            } else if (matches(pastPlays, "DHI----", index)) {
                newLoc = HIDE;
                dracOnWater = FALSE;
            } else if (matches(pastPlays, "DD1----", index)) {
                newLoc = DOUBLE_BACK_1;
                dracOnWater = seaLocation(view->locHistory[PLAYER_DRACULA][0]);
            } else if (matches(pastPlays, "DD2----", index)) {
                newLoc = DOUBLE_BACK_2;
                dracOnWater = seaLocation(view->locHistory[PLAYER_DRACULA][1]);
            } else if (matches(pastPlays, "DD3----", index)) {
                newLoc = DOUBLE_BACK_3;
                dracOnWater = seaLocation(view->locHistory[PLAYER_DRACULA][2]);
            } else if (matches(pastPlays, "DD4----", index)) {
                newLoc = DOUBLE_BACK_4;
                dracOnWater = seaLocation(view->locHistory[PLAYER_DRACULA][3]);
            } else if (matches(pastPlays, "DD5----", index)) {
                newLoc = DOUBLE_BACK_5;
                dracOnWater = seaLocation(view->locHistory[PLAYER_DRACULA][4]);
            } else if (matches(pastPlays, "DTP----", index)) {
                newLoc = TELEPORT;
                dracOnWater = FALSE;
            } else if (matches(pastPlays, "DNO----", index)) {
                newLoc = NOWHERE;
                dracOnWater = FALSE;
            } else {
                //using the abbrevToID function if it's a normal location code
                char abbrev[3] = { pastPlays[index + 1], pastPlays[index + 2], 0 };
                newLoc = abbrevToID(abbrev);
                if ( newLoc == NOWHERE )
                {
                    printf("Error: irregular location code %s\n", abbrev);
                    abort();
                }
            }
            addLocation(view, PLAYER_DRACULA, newLoc);

            // Immature vampired placed
            if (matches(pastPlays, "D--V---|D---V--", index)) {
                view->cities[newLoc].vamp = turn;
            }
            // Trap placed
            if (matches(pastPlays, "D--T---|D---T--", index)) {
                int i = 0;
                int trap_placed = 0;
                while (trap_placed != 1)
                {
                    assert( i < MAX_TRAPS );
                    if ( view->cities[newLoc].trap[i] == NO_ITEM )
                    {
                        view->cities[newLoc].trap[i] = turn;
                        trap_placed = 1;
                    }
                    i++;
                }
            }

            //vamp matures
            if (matches(pastPlays, "D----V-", index)) {
               view->score -= SCORE_LOSS_VAMPIRE_MATURES;
            }
            //drac is at sea
            if (matches(pastPlays, "DS?|DAS|DAO|DBB|DBS|DEC|DIO|DIR|DMS|DNS|DTS", index) || dracOnWater) {
                view->bloodPts -= LIFE_LOSS_SEA;
                dracOnWater = TRUE;
            }
            //drac at castle
            if (matches(pastPlays, "DCD|DTP", index)) {
                view->bloodPts += LIFE_GAIN_CASTLE_DRACULA;
            }

        } else { //hunters turn

            //determine which hunter's turn it is
            if (matches(pastPlays, "G------", index)) currPlayer = PLAYER_LORD_GODALMING;
            else if (matches(pastPlays, "S------", index)) currPlayer = PLAYER_DR_SEWARD;
            else if (matches(pastPlays, "H------", index)) currPlayer = PLAYER_VAN_HELSING;
            else if (matches(pastPlays, "M------", index)) currPlayer = PLAYER_MINA_HARKER;
            else assert(0 && "doesn't match anyone's turn");

            //setting new location of hunter
            char abbrev[3] = { pastPlays[index + 1], pastPlays[index + 2], 0 };
            LocationID newLoc = abbrevToID(abbrev);

            //hunter rested
            if (newLoc == getLocation(view, currPlayer)) {
                view->lifePts[currPlayer] += LIFE_GAIN_REST;
            }

            addLocation(view, currPlayer, newLoc);

            //hunter encounters up to 3 traps
            int trapsEncountered = 0;
            if (matches(pastPlays, "---T---", index)) {
                view->lifePts[currPlayer] -= LIFE_LOSS_TRAP_ENCOUNTER;
                trapsEncountered++;
            }
            if (matches(pastPlays, "----T--", index)) {
                view->lifePts[currPlayer] -= LIFE_LOSS_TRAP_ENCOUNTER;
                trapsEncountered++;
            }
            if (matches(pastPlays, "-----T-", index)) {
                view->lifePts[currPlayer] -= LIFE_LOSS_TRAP_ENCOUNTER;
                trapsEncountered++;
            }
            if (matches(pastPlays, "------T", index)) {
                view->lifePts[currPlayer] -= LIFE_LOSS_TRAP_ENCOUNTER;
                trapsEncountered++;
            }
            assert( trapsEncountered <= MAX_TRAPS );
            while ( trapsEncountered > 0 )
            {
                trapsEncountered--;
                view->cities[newLoc].trap[trapsEncountered] = NO_ITEM;
            }

            // hunter vanquishes immature vampire
            if (matches(pastPlays, "---V---|----V--|-----V-|------V", index))
            {
                view->cities[newLoc].vamp = NO_ITEM;
            }
            //hunter encounters drac
            if (matches(pastPlays, "---D---|----D--|-----D-|------D", index)) {
                view->lifePts[currPlayer] -= LIFE_LOSS_DRACULA_ENCOUNTER;
                view->bloodPts -= LIFE_LOSS_HUNTER_ENCOUNTER;
            }

            //hunter gets teleported to the hospital
            if (view->lifePts[currPlayer] <= 0) {
                view->score -= SCORE_LOSS_HUNTER_HOSPITAL;
                view->lifePts[currPlayer] = 9;
                view->locHistory[currPlayer][0] = ST_JOSEPH_AND_ST_MARYS;
            }
        }

        // Cull old items/encounters in cities
        {
            int city;
            for (city=0; city < NUM_MAP_LOCATIONS; city++)
            {
                int i;
                for (i=0; i< MAX_TRAPS; i++)
                {
                    if ( view->cities[city].trap[i] < turn - (TRAIL_SIZE*NUM_PLAYERS) )
                        view->cities[city].trap[i] = NO_ITEM;
                }
                if ( view->cities[city].vamp < turn - (TRAIL_SIZE*NUM_PLAYERS) )
                    view->cities[city].vamp = NO_ITEM;
            }
        }

        //next player's turn
        currPlayer = (currPlayer + 1) % NUM_PLAYERS;
        turn++;
    }

    //assuming round is 0 based
    view->round = turn / NUM_PLAYERS;
    view->currPlayer = currPlayer;

    return view;
}

static int seaLocation (LocationID loc)
{
    if (loc == ADRIATIC_SEA
            || loc == ATLANTIC_OCEAN
            || loc == BAY_OF_BISCAY
            || loc == BLACK_SEA
            || loc == ENGLISH_CHANNEL
            || loc == IONIAN_SEA
            || loc == IRISH_SEA
            || loc == MEDITERRANEAN_SEA
            || loc == NORTH_SEA
            || loc == TYRRHENIAN_SEA
            || loc == SEA_UNKNOWN) {

        return TRUE;
    }

    return FALSE;
}

static void addLocation (GameView view, PlayerID player, LocationID newLoc)
{
    //each player has an array of LocationIDs with length TRAIL_SIZE
    //representing the last 6 places they've been
    //add location pushes all the values down and adds a new one on the top
    int i;
    for (i = TRAIL_SIZE - 1; i > 0; i--) {
        view->locHistory[player][i] = view->locHistory[player][i-1];
    }
    view->locHistory[player][0] = newLoc;
}


static int numTurns (char *pastPlays)
{
//counts the spaces
    int count = 1;
    int i = 0;
    while (pastPlays[i] != 0) {
        if (pastPlays[i] == ' ') {
            count++;
        }
        i++;
    }

    if (i == 0) {
        return 0;
    } else {
        return count;
    }
}


static void testMatches () {
    assert (matches ("GST.... SAO.... HCD.... MAO.... DGE.... GGED...", "G------", 0) == 1);
    assert (matches ("GST.... SAO.... HCD.... MAO.... DGE.... GGED...", "G-------", 0) == 0);
    assert (matches ("GST.... SAO.... HCD.... MAO.... DGE.... GGED...", "G--D---", 40) == 1);
    assert (matches ("GST.... SAO.... HCD.... MAO.... DGE.... GGED...", "S|G|H|M", 0) == 1);
    assert (matches ("cat and other animals", "dog|cat", 0) == 1);
}


//returns true if the sub string matches string at the given index
// '-' is treated as any character except ' '
// '|' is treated as the OR character
// for example matches(string, "cat|dog", i) returns true if either
//the word cat OR dog is found at index i in string
static int matches (char *string, char *sub, int index)
{
    int i = 0; //position in sub string
    int isMatching = 1;
    while (string[index + i] != 0 && sub[i] != 0) {
        if (sub[i] == '|') {
            if (isMatching) {
                return 1;
            } else {
                return matches(string, &sub[i+1], index);
            }
        } else {
            if (sub[i] != '-' || string[index + i] == ' ') {
                if (sub[i] != string[index + i]) {
                    isMatching = 0;
                }
            }
            i++;
        }
    }

    return (isMatching);
}


// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted)
{
    free( toBeDeleted );
}


//// Functions to return simple information about the current state of the game
// Get the current round
Round getRound(GameView currentView)
{
    return currentView->round;
}


// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView)
{
    return currentView->currPlayer;
}


// Get the current score
int getScore(GameView currentView)
{
    return currentView->score;
}


// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player)
{
    //assert valid playerID

    int health;
    if (player == PLAYER_DRACULA) {
        health = currentView->bloodPts;
    } else {
        health = currentView->lifePts[player];
    }

    return health;
}


// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player)
{
    //assert valid playerID
    return currentView->locHistory[player][0];
}


//// Functions that return information about the history of the game
// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE])
{
    //goes through the locHistory[] for that player and copies it to trail[]
    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = currentView->locHistory[player][i];
    }

}


//// Functions that query the map to find information about connectivity
// Returns an array of LocationIDs for all directly connected locations
LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player, Round round,
                               int road, int rail, int sea)
{
    Map europe = newMap();

    int drac = (player == PLAYER_DRACULA);

    int railLength = (player + round) % 4;
    if (!rail) railLength = 0;
    if (drac) railLength = 0;

    return reachableLocations(europe, numLocations, from, drac, railLength, road, sea);
}


// Find out what minions are placed at the specified location
void getMinions(GameView game, LocationID where, int *numTraps, int *numVamps)
{
    int traps = 0;
    int i;

    for (i=0; i<MAX_TRAPS; i++)
    {
        if ( game->cities[where].trap[i] != NO_ITEM )
            traps++;
    }

    int vamps;
    if ( game->cities[where].vamp != NO_ITEM )
        vamps = 1;
    else
        vamps = 0;

    *numTraps = traps;
    *numVamps = vamps;
}
