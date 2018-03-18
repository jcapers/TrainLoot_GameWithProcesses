#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "player.h"
#include "comms.h"

/*
 * ===========================================================================
 * 2310 Assignment 3
 * SPOILER Player
 * ===========================================================================
 */
/* Function prototypes unique for player */
bool player_above_below(Game *game, int id);
char most_players(Game *game, int id);

 /*
  * ===========================================================================
  * Player Game Functions
  * ===========================================================================
  */

/*
 * Finds a short target.
 *
 * @param *game     current game state.
 * @param id        id of player taking shot.
 * @return player id, in integer form.
 */
int select_short(Game *game, int id) {
    int highestID = -1;
    Position checkPos, playerPos;
    playerPos = game->players[id]->pos;

    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        // Find highest ID target for short
        if (checkPos.y == playerPos.y && checkPos.x == playerPos.x
                && i > highestID && i != id) {
            highestID = i;
        }
    }

    return highestID;
}

/*
 * Finds a long target.
 *
 * @param *game     current game state.
 * @param id        id of player takingg shot.
 * @return player id, in integer form.
 */
int select_long(Game *game, int id) {
    int idLeft = -1, idRight = -1, closestLeft, closestRight;
    Position checkPos, playerPos;
    playerPos = game->players[id]->pos;
    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        if (i != id && checkPos.y == playerPos.y && checkPos.x < playerPos.x
                && playerPos.y == 1) {
            // Check uppper long, left
            if (idLeft == -1 || (i > idLeft && checkPos.x >= closestLeft)) {
                idLeft = i;
                closestLeft = checkPos.x;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x > playerPos.x && playerPos.y == 1) {
            // Check upper long, right
            if (idRight == -1 || (i > idRight
                    && checkPos.x <= closestRight)) {
                idRight = i;
                closestRight = checkPos.x;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x == playerPos.x - 1 && checkPos.x >= 0
                && playerPos.y == 0) {
            // Check lower long, left
            if (idLeft == -1 || i > idLeft) {
                idLeft = i;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x == playerPos.x + 1
                && checkPos.x < game->numCarriages && playerPos.y == 0) {
            // Check lower long, right
            if (idRight == -1 || i > idRight) {
                idRight = i;
            }
        }
    }
    if (idLeft > idRight) {
        return idLeft;
    } else {
        return idRight;
    }
}

/*
 * Checks which side of spoiler has most players.
 *
 * @param *game     the current game state.
 * @param id        spoiler id
 * @return char representing direction to move towards.
 */
char most_players(Game *game, int id) {
    int left = 0, right = 0;
    Position spoiler = game->players[id]->pos;
    Position check;

    // Count players on either side (both levels)
    for (int i = 0; i < game->numPlayers; i++) {
        check = game->players[i]->pos;
        if (check.x < spoiler.x) {
            left++;
        } else if (check.x > spoiler.x) {
            right++;
        }
    }

    // Give direction, if same then neither direction valid.
    if (left > right) {
        return DIR_LEFT;
    } else if (right > left) {
        return DIR_RIGHT;
    } else {
        return '?';
    }
}

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
    // Current bandit data
    Position currentPos = game->players[id]->pos;

    // Check and reply
    if (strcmp(message, GET_S_TARGET) == 0) {
        // Select short target
        if (!player_here(game, id)) {
            action[0] = NO_TARGET;
        } else {
            int target = select_short(game, id);
            action[0] = 'A' + target;
        }
        send_message(stdout, AIM_SHORT, action);
    } else if (strcmp(message, GET_L_TARGET) == 0) {
        // Select long target
        if (!has_long_target(game, id)) {
            action[0] = NO_TARGET;
        } else {
            int target = select_long(game, id);
            action[0] = 'A' + target;
        }
        send_message(stdout, AIM_LONG, action);
    } else if (strcmp(message, GET_DIR) == 0) {
        // Select movement based on player locations
        char direction = most_players(game, id);
        if (direction == DIR_LEFT) {
            action[0] = DIR_LEFT;
        } else if (direction == DIR_RIGHT) {
            action[0] = DIR_RIGHT;
        } else if (currentPos.x == 0) {
            action[0] = DIR_RIGHT;
        } else {
            action[0] = DIR_LEFT;
        }
        send_message(stdout, GO_DIR, action);
    }
}

/*
 * Checks if there is a player above the spoiler.
 *
 * @param *game     current game state.
 * @param id        id of spoiler.
 * @return true if there is a player above, false otherwise.
 */
bool player_above_below(Game *game, int id) {
    Position currentPos = game->players[id]->pos;
    Position checkPos;

    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        if (i != id && currentPos.y != checkPos.y &&
                currentPos.x == checkPos.x) {
            return true;
        }
    }

    // No player above/below
    return false;
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
    // Last order
    char order = game->players[id]->orders[0];

    // (1) Short or long if not used last turn
    if (order != SHOOT_S && order != SHOOT_L && player_here(game, id)) {
        move[0] = SHOOT_S;
    } else if (order != SHOOT_S && order != SHOOT_L &&
            has_long_target(game, id)) {
        move[0] = SHOOT_L;
    } else if (player_above_below(game, id)) {
        // (2) Move vertically if players on different level
        move[0] = MOVE_V;
    } else if (game->train[index] > 0) {
        // (3) Loot if loot exists
        move[0] = LOOT;
    } else {
        // (4) Otherwise move horizontally
        move[0] = MOVE_H;
    }

    send_message(stdout, PLAY, move);
}

int main(int argc, char **argv) {
    player_main(argc, argv);

    return EXIT_SUCCESS;
}
