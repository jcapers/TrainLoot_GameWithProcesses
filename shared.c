#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "shared.h"
#include "comms.h"


/*
 * ===========================================================================
 * CSSE2310 Assignment 3
 * Shared functions between Hub and Player
 * ===========================================================================
 */

/*
 * ===========================================================================
 * Utility functions
 * ===========================================================================
 */
/*
 * Checks if a string is a number.
 *
 * @param *arg  the string we are validating.
 * @returns true if string is a number,false otherwise.
 */
bool arg_is_number(char *arg) {
    int length = (int) strlen(arg);
    for (int i = 0; i < length; i++) {
        if (!isdigit(arg[i])) {
            return false;
        }
    }
    return true;
}

/*
 * Get number of digits of a number.
 *
 * @param num   a positive integer of any size
 * @return the number of digits in num.
 */
int num_digits(int num) {
    if (num < 10) {
        return 1;
    }
    // Take log of the number and return result + 1 for number of digits
    int result = (int) log10(num);
    return result + 1;
}

/*
 * ===========================================================================
 * Shared game setup functions.
 * ===========================================================================
 */

/*
 * Creates the game.
 *
 * @param numPlayers    number of players for this game.
 * @param numCarriages  max number of carriages for this game.
 * @param seed          game seed for setup.
 * @returns pointer to game, setup with players.
 */
Game *make_game(int numPlayers, int numCarriages, unsigned int seed) {
    Game *game = (Game *) malloc(sizeof(Game));
    game->players = (Player **) malloc(sizeof(Player *) * numPlayers);
    for (int i = 0; i < numPlayers; i++) {
        game->players[i] = (Player *) malloc(sizeof(Player) * numPlayers);
    }

    // Setup parameters
    game->numPlayers = numPlayers;
    game->numCarriages = numCarriages;
    game->seed = seed;
    game->execute = false;
    game->round = 1;

    // Setup Train, 2D array of carriages.
    game->train = (int *) malloc(sizeof(int) * game->numCarriages * 2);

    // Allocate loot
    int totalLoot = ((game->seed % 4) + 1) * game->numCarriages;
    int lootX = 0, lootY = 0;

    for (int i = 0; i < totalLoot; i++) {
        if (i == 0) {
            lootX = (int) ceil((float) numCarriages / 2);
            lootY = 0;
            game->train[lootY * numCarriages + lootX]++;
        } else {
            lootX = (lootX + (game->seed % 101)) % numCarriages;
            lootY = (lootY + (game->seed % 2)) % 2;
            game->train[lootY * numCarriages + lootX]++;
        }
    }

    // Initialise players
    for (int i = 0; i < game->numPlayers; i++) {
        game->players[i] = make_player(game, i);
    }

    return game;
}

/*
 * Sets up player information struct for storing in game struct.
 *
 * @param *game     game struct holding parameters of game.
 * @param thisID    the id of this player.
 * @return pointer to a player.
 */
Player *make_player(Game *game, int thisID) {
    Player *player = (Player *) malloc(sizeof(Player));

    // Player ID
    player->id = thisID;
    player->symbol = 'A' + thisID;

    // Player start position
    player->pos.x = player->id % game->numCarriages;
    player->pos.y = 0;

    // Player starting status
    player->hits = 0;
    player->loot = 0;

    return player;
}
