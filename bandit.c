#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "player.h"
#include "comms.h"

/*
 * ===========================================================================
 * 2310 Assignment 3
 * BANDIT Player
 * ===========================================================================
 */
/* Function prototypes unique for player */
int most_loot_on_level(Game *game);
char side_with_most_loot(Game *game, int id);

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
    int idLeft = id, idRight = id, closestLeft, closestRight;
    Position checkPos, playerPos;
    playerPos = game->players[id]->pos;
    for (int i = 0; i < game->numPlayers; i++) {
        checkPos = game->players[i]->pos;
        if (i != id && checkPos.y == playerPos.y && checkPos.x < playerPos.x
                && playerPos.y == 1) {
            // Check uppper long, left
            if (idLeft == id || (i < idLeft && checkPos.x >= closestLeft)) {
                idLeft = i;
                closestLeft = checkPos.x;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x > playerPos.x && playerPos.y == 1) {
            // Check upper long, right
            if (idRight == id || (i < idRight
                    && checkPos.x <= closestRight)) {
                idRight = i;
                closestRight = checkPos.x;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x == playerPos.x - 1 && checkPos.x >= 0
                && playerPos.y == 0) {
            // Check lower long, left
            if (idLeft == id || i < idLeft) {
                idLeft = i;
            }
        } else if (i != id && checkPos.y == playerPos.y
                && checkPos.x == playerPos.x + 1
                && checkPos.x < game->numCarriages && playerPos.y == 0) {
            // Check lower long, right
            if (idRight == id || i < idRight) {
                idRight = i;
            }
        }
    }
    if (idLeft < idRight) {
        return idLeft;
    } else {
        return idRight;
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
        // Select target if target available.
        if (!player_here(game, id)) {
            action[0] = NO_TARGET;
        } else {
            int target = select_short(game, id);
            action[0] = 'A' + target;
        }
        send_message(stdout, AIM_SHORT, action);
    } else if (strcmp(message, GET_DIR) == 0) {
        // Decide where to move
        char direction = side_with_most_loot(game, id);
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
    } else if (strcmp(message, GET_L_TARGET) == 0) {
        // Select long target if target available.
        if (!has_long_target(game, id)) {
            action[0] = NO_TARGET;
        } else {
            int target = select_long(game, id);
            action[0] = 'A' + target;
        }
        send_message(stdout, AIM_LONG, action);
    }
}

/*
 * Checks which level of train has most loot.
 *
 * @param *game     current game state.
 * @return  1 if upper level has more loot, 0 if lower, -1 if even.
 */
int most_loot_on_level(Game *game) {
    int lower = 0, upper = 0;

    // Gather intel on loot situation
    for (int i = 0; i < game->numCarriages; i++) {
        lower += game->train[i];
        upper += game->train[game->numCarriages + i];
    }

    if (lower > upper) {
        return 0;
    } else if (upper > lower) {
        return 1;
    } else {
        // loot is even
        return -1;
    }
}

/*
 * Checks which side of train has more loot from bandit's position.
 *
 * @param *game     current game state.
 * @param id        the bandit's id.
 * @return '-' or '+' depending on which direction has more, else '?'
 */
char side_with_most_loot(Game *game, int id) {
    int left = 0, right = 0;
    Position currentPos = game->players[id]->pos;

    // Left side
    for (int i = 0; i < currentPos.x; i++) {
        left += game->train[i] + game->train[game->numCarriages + i];
    }

    // Right side
    for (int i = currentPos.x + 1; i < game->numCarriages; i++) {
        right += game->train[i] + game->train[game->numCarriages + i];
    }

    if (left > right) {
        return DIR_LEFT;
    } else if (right > left) {
        return DIR_RIGHT;
    } else {
        return '?';
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
    // Last order
    char order = game->players[id]->orders[0];
    // Loot distribution
    int levelLoot, sidewaysLoot;

    // (1) Loot if loot is available
    if (game->train[index] > 0) {
        move[0] = LOOT;
    } else if (player_here(game, id) && order != SHOOT_S &&
            order != SHOOT_L) {
        // (2) Short as player is here, and no shot taken previously
        move[0] = SHOOT_S;
    } else if ((levelLoot = most_loot_on_level(game)) != -1 &&
            levelLoot != currentPos.y) {
        // (3) Vertical if other level has more loot
        move[0] = MOVE_V;
    } else if ((sidewaysLoot = side_with_most_loot(game, id)) != '?') {
        // (4) Horizontal movement if either direction has more loot
        move[0] = MOVE_H;
    } else if (has_long_target(game, id)) {
        // (5) Try a long target
        move[0] = SHOOT_L;
    } else {
        // (6) Else, move vertically
        move[0] = MOVE_V;
    }

    send_message(stdout, PLAY, move);
}

int main(int argc, char **argv) {
    player_main(argc, argv);

    return EXIT_SUCCESS;
}
