# Train Loot game run by Processes communicating via Pipes
Automated Game using processes communicated with pipes in a Unix environment, where various strategies compete to loot a 'train'. Assignment for CSSE2310, 2017 at UQ.

Written from scratch in C.

## Usage
1. Compile with 'make' command in terminal
2. Run game with ./2310express [seed] [number of carriages] [./player1 ./player2 ...]
* Example: ./2310express 283 5 ./acrophobe ./bandit ./spoiler starts game with three players looting five carriages, of acrophobe, bandit, and spoiler strategies.

## How it works
The game is managed by the 'hub' which manages game rounds, game state. The hub keeps track of players and requests moves, as well as communicating game state with players.

The objective is for the players to clear out the train carriages of loot. The carriagesr have two levels. The game ends after 15 rounds and the one with most loot is the winner.

Players can loot, or shoot other players. Shooting causes a player to drop loot. Each player can take one action per round and logic found in each player type determines which action it takes.

The players communicate their moves to the hub when requested by the hub and follow various strategies.

* Acrophobes simply concentrate on looting and moving down/up the train to get more loot.

* Bandits try to loot, if there is no loot will either shoot the closest player or move to another level (1st or 2nd level of carriages). Bandits may also try to shoot from long distance.

* Spoilers concentrate on shooting, before they decide to loot.
