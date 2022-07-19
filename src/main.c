#include "game.h"

int main(void) {
	game_t game;

	game_init(&game);
	game_run(&game);
	game_finish(&game);

	return 0;
}
