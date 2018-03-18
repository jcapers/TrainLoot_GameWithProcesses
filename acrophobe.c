#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "player.h"
#include "comms.h"

/*
 * ===========================================================================
 * 2310 Assignment 3
 * ACROPHOBE Player
 * ===========================================================================
 */

 /*
  * ===========================================================================
  * Player Game Functions
  * ===========================================================================
  */
/*
 * Player chooses a direction or target according to hub request.
 *
 * @param *game     the player's view of the game state.
 * @param id        this player's id
 * @param message   the request from the hub
 */
void describe_action(Game *game, int id, char message[]) {
    // String containing reply from player
    char action[2] = {'\0'};

    // Last direction of player
    char lastDirection = game->players[id]->orders[1];
    int currentHPos = game->players[id]->pos.x;
    if (strcmp(message, GET_DIR) == 0) {
        // Find a valid horizontal move
        if (lastDirection == DIR_LEFT && currentHPos > 0) {
            action[0] = DIR_LEFT;
        } else if (lastDirection == DIR_RIGHT &&
                currentHPos < game->numCarriages - 1) {
            action[0] = DIR_RIGHT;
        } else if (currentHPos == 0) {
            action[0] = DIR_RIGHT;
        } else if (currentHPos == game->numCarriages - 1) {
            action[0] = DIR_LEFT;
        } else {
            action[0] = DIR_LEFT;
        }
        send_message(stdout, GO_DIR, action);
    } else if (strcmp(message, GET_S_TARGET) == 0) {
        action[0] = NO_TARGET;
        send_message(stdout, AIM_SHORT, action);
    } else if (strcmp(message, GET_L_TARGET) == 0) {
        action[0] = NO_TARGET;
        send_message(stdout, AIM_LONG, action);
    }
}

/*
 * Player chooses a move based on its strategy.
 *
 * @param *game     player's view of the game state.
 * @param id        this player's id
 */
void choose_move(Game *game, int id) {
    // String containing reply from player
    char move[2] = {'\0'};

    // Current position of this player.
    Position currentPos = game->players[id]->pos;
    int index = currentPos.y * game->numCarriages + currentPos.x;

    // Loot if loot is available
    if (game->train[index] > 0) {
        move[0] = LOOT;
    } else {
        // Otherwise, move horizontally.
        move[0] = MOVE_H;
    }

    send_message(stdout, PLAY, move);
}

int main(int argc, char **argv) {
    player_main(argc, argv);

    return EXIT_SUCCESS;
}
