#ifndef GAME_H__HEADER_GUARD__
#define GAME_H__HEADER_GUARD__

#include <stdlib.h>  /* size_t, exit, EXIT_FAILURE, rand, srand */
#include <stdbool.h> /* bool, true, false */
#include <time.h>    /* time */
#include <string.h>  /* memset */
#include <math.h>    /* cos, sin */
#include <stdio.h>   /* snprintf */
#include <SDL2/SDL.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define DEG_TO_RAD(p_deg) (float)((p_deg) * (M_PI / 180))

#define GAME_TITLE "8pong"

#define SCREEN_W 35
#define SCREEN_H 28

#define FPS_CAP 60
#define DELTA_TIME_SEC (1.0 / FPS_CAP)

#define PADDLE_LENGTH 6
#define PADDLE_SPEED  (23 * DELTA_TIME_SEC)

#define PROJ_START_SPEED 22
#define PROJ_SPEEDUP     2

#define ROUND_DELAY 60

typedef enum {
	SIDE_RIGHT = 0,
	SIDE_LEFT
} side_t;

typedef struct {
	SDL_Window   *window;
	SDL_Renderer *renderer;
	SDL_Texture  *screen;
	SDL_Rect      screen_rect;

	SDL_Texture *right_texture, *left_texture, *pause_texture;

	SDL_Event    event;
	const Uint8 *key_states;

	bool   running;
	size_t tick, fps;

	bool paused;

	size_t lost_timer;
	side_t side;

	float paddle1_y, paddle2_y;
	float proj_x, proj_y, proj_angle_x, proj_angle_y;
	float proj_speed;
} game_t;

void game_init(game_t *p_game);
void game_run(game_t *p_game);
void game_finish(game_t *p_game);

SDL_Texture *game_load_texture(game_t *p_game, const char *p_path);

void game_get_proj_rect(game_t *p_game, SDL_Rect *p_rect);
void game_get_paddle1_rect(game_t *p_game, SDL_Rect *p_rect);
void game_get_paddle2_rect(game_t *p_game, SDL_Rect *p_rect);

void game_reset_proj(game_t *p_game);
void game_reset_paddles(game_t *p_game);

void game_update(game_t *p_game);
void game_handle_events(game_t *p_game);
void game_render(game_t *p_game);

#endif
