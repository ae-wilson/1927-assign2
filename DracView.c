// THIS FILE ONLY CONTAINS THE *WHERECANIGO AND *WHERECANTHEYGO FILES FOR DRACVIEW.C


LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea)
{
        assert(currentView != NULL);    // check that there is currently a player
        assert(currentView->g != NULL); // check that there is a map in play
        assert(numLocations != NULL);   // check that there are locations to visit

        // need to find out the current location of Dracula
        LocationID here = whereIs(currentView, PLAYER_DRACULA);
//      assert(here != NULL);

        // need to find out the current round
        Round round = giveMeTheRound(currentView);
//      assert(turn != NULL);


        // we will use the connectedLocations funciton in GameView.c to find
        // all the possible locations which Dracula can visit
        LocationID *connLoc = connectedLocations(currentView->gameView, numLocations, here,$

        LocationID *dracTrail = malloc(TRAIL_SIZE * sizeof(LocationID));
        assert(dracTrail != NULL);
    giveMeTheTrail(currentView, PLAYER_DRACULA, dracTrail);
        LocationID *trail = currentView->trail_perPlayer[PLAYER_DRACULA];

    int counterA = 0;
    int counterB = 0;
    while(counterA < *numLocations){
        counterB = 0;
        while(counterB < TRAIL_SIZE){
            if(connLoc[counterA] == trail[counterB] && isHideInTrail(trail) == TRUE){
                removeLocation(numLocations, trail, connLoc[counterA], counterA);
            }
            counterB++;
        }
        counterA++;
    }

        return connLoc;
}


static void removeLocation(int *numLocations, LocationID *trail, LocationID v, int pos){
    assert(trail[pos] == v);

    int i;
    for(i = pos; i < *numLocations - 1; i++) {
        trail[i] = trail[i+1];
    }

    *numLocations = *numLocations - 1;
}

static int isHideInTrail(LocationID *trail){
    int counter = 0;
    while(counter < TRAIL_SIZE){
        if(trail[counter] == HIDE){
            return TRUE;
        }
        counter++;
    }
    return FALSE;
}

LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea)
{
        assert(currentView != NULL);    // check that there is currently a player
        assert(currentView->g != NULL); // check that there is a map in play
        assert(numLocations != NULL);   // check that there are locations to visit
        assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER);

        // need to find out the current location of the player
        LocationID there = whereIs (currentView, player);
//      assert(there != NULL);

        // need to find out the current round
        Round turn = giveMeTheRound(currentView);
//      assert(turn != NULL);

        // we will use the connectedLocations funciton in GameView.c to find
        // all the possible locations which Dracula can visit
        return connectedLocations(currentView->gameView, numLocations, there, player, turn,$
}
