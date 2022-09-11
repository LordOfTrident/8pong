#include "game.h"

void game_init(game_t *p_game) {
	memset(p_game, 0, sizeof(game_t));

	/* init SDL2 */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Initialized video");

	p_game->window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                                  SCREEN_W * 14, SCREEN_H * 14, SDL_WINDOW_RESIZABLE);
	if (p_game->window == NULL) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Created the window");

	p_game->renderer = SDL_CreateRenderer(p_game->window, -1, SDL_RENDERER_ACCELERATED);
	if (p_game->renderer == NULL) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Created the renderer");

	/* we need a separate texture for the screen in order to achieve a true pixelation effect
	   (drawing lines is a bit broken) */
	p_game->screen = SDL_CreateTexture(p_game->renderer, SDL_PIXELFORMAT_RGBA8888,
	                                   SDL_TEXTUREACCESS_TARGET, SCREEN_W, SCREEN_H);
	if (p_game->screen == NULL) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Created the screen texture");

	p_game->screen_rect.w = SCREEN_W;
	p_game->screen_rect.h = SCREEN_H;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (SDL_RenderSetLogicalSize(p_game->renderer, SCREEN_W, SCREEN_H) != 0) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Render logical size was set");

	SDL_SetRenderDrawBlendMode(p_game->renderer, SDL_BLENDMODE_BLEND);

	p_game->key_states = SDL_GetKeyboardState(NULL);

	/* load assets */
	p_game->right_texture = game_load_texture(p_game, "./res/right.bmp");
	p_game->left_texture  = game_load_texture(p_game, "./res/left.bmp");
	p_game->pause_texture = game_load_texture(p_game, "./res/pause.bmp");

	/* other init */
	time_t random = rand();
	srand(time(&random));

	game_reset_paddles(p_game);
	game_reset_proj(p_game);

	SDL_Log("Finished initialization");
}

void game_run(game_t *p_game) {
	p_game->running = true;

	size_t fps_timer = 0;

	do {
		/* calculate fps */
		size_t now   = SDL_GetTicks();
		size_t delta = now - fps_timer;

		p_game->fps = 1000 / delta;
		fps_timer   = now;

		char title[32] = {0};
		snprintf(title, sizeof(title), GAME_TITLE" FPS: %zu", p_game->fps);

		SDL_SetWindowTitle(p_game->window, title);

		game_update(p_game);
		game_render(p_game);

		++ p_game->tick;

		SDL_Delay(1000 / FPS_CAP);
	} while (p_game->running);
}

void game_finish(game_t *p_game) {
	SDL_DestroyTexture(p_game->right_texture);
	SDL_DestroyTexture(p_game->left_texture);
	SDL_DestroyTexture(p_game->pause_texture);

	SDL_DestroyTexture(p_game->screen);
	SDL_Log("Destroyed the screen");

	SDL_Log("Destroyed the textures");

	SDL_DestroyRenderer(p_game->renderer);
	SDL_Log("Destroyed the renderer");

	SDL_DestroyWindow(p_game->window);
	SDL_Log("Destroyed the window");

	SDL_Quit();
}

SDL_Texture *game_load_texture(game_t *p_game, const char *p_path) {
	SDL_Surface *surface = SDL_LoadBMP(p_path);
	if (surface == NULL) {
		SDL_Log("%s", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load asset", SDL_GetError(), 0);

		exit(EXIT_FAILURE);
	} else
		SDL_Log("Loaded asset '%s'", p_path);

	Uint32 color_key = SDL_MapRGB(surface->format, 123, 0, 123);
	SDL_SetColorKey(surface, true, color_key);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(p_game->renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
}

void game_get_proj_rect(game_t *p_game, SDL_Rect *p_rect) {
	p_rect->x = p_game->proj_x;
	p_rect->y = p_game->proj_y;
	p_rect->w = 1;
	p_rect->h = 1;
}

void game_get_paddle1_rect(game_t *p_game, SDL_Rect *p_rect) {
	p_rect->x = 1;
	p_rect->y = p_game->paddle1_y;
	p_rect->w = 1;
	p_rect->h = PADDLE_LENGTH;
}

void game_get_paddle2_rect(game_t *p_game, SDL_Rect *p_rect) {
	p_rect->x = SCREEN_W - 2;
	p_rect->y = p_game->paddle2_y;
	p_rect->w = 1;
	p_rect->h = PADDLE_LENGTH;
}

void game_reset_proj(game_t *p_game) {
	p_game->proj_x = SCREEN_W / 2;
	p_game->proj_y = SCREEN_H / 2;

	p_game->proj_speed = PROJ_START_SPEED;

	int angles[4] = {
		20, -20,
		160, 200
	};

	int angle = angles[rand() % (sizeof(angles) / sizeof(int))];

	p_game->proj_angle_x = cos(DEG_TO_RAD(angle));
	p_game->proj_angle_y = sin(DEG_TO_RAD(angle));
}

void game_reset_paddles(game_t *p_game) {
	p_game->paddle1_y = SCREEN_H / 2 - PADDLE_LENGTH / 2;
	p_game->paddle2_y = p_game->paddle1_y;
}

void game_update(game_t *p_game) {
	game_handle_events(p_game);

	if (p_game->paused)
		return;

	if (p_game->lost_timer > 0) {
		-- p_game->lost_timer;

		if (p_game->lost_timer == 0) {
			game_reset_proj(p_game);
			game_reset_paddles(p_game);
		}

		return;
	}

	float prev_x = p_game->proj_x, prev_y = p_game->proj_y;
	p_game->proj_x += p_game->proj_angle_x * p_game->proj_speed * DELTA_TIME_SEC;

	SDL_Rect proj_rect, paddle1_rect, paddle2_rect;
	game_get_proj_rect(p_game, &proj_rect);
	game_get_paddle1_rect(p_game, &paddle1_rect);
	game_get_paddle2_rect(p_game, &paddle2_rect);

	if (p_game->proj_x > SCREEN_W || p_game->proj_x <= 0) {
		p_game->lost_timer = ROUND_DELAY;
		p_game->side = p_game->proj_x < 0? SIDE_LEFT : SIDE_RIGHT;

		p_game->proj_x = prev_x;

		return;
	} else if (SDL_HasIntersection(&proj_rect, &paddle1_rect)) {
		p_game->proj_x        = prev_x;
		p_game->proj_angle_x *= -1;

		p_game->proj_speed += PROJ_SPEEDUP;
	} else if (SDL_HasIntersection(&proj_rect, &paddle2_rect)) {
		p_game->proj_x        = prev_x;
		p_game->proj_angle_x *= -1;

		p_game->proj_speed += PROJ_SPEEDUP;
	}

	p_game->proj_y += p_game->proj_angle_y * p_game->proj_speed * DELTA_TIME_SEC;
	game_get_proj_rect(p_game, &proj_rect);

	if (p_game->proj_y > SCREEN_H || p_game->proj_y <= 0) {
		p_game->proj_y        = prev_y;
		p_game->proj_angle_y *= -1;
	} else if (SDL_HasIntersection(&proj_rect, &paddle1_rect)) {
		p_game->proj_y        = prev_y;
		p_game->proj_angle_y *= -1;
	} else if (SDL_HasIntersection(&proj_rect, &paddle2_rect)) {
		p_game->proj_y        = prev_y;
		p_game->proj_angle_y *= -1;
	}
}

void game_handle_events(game_t *p_game) {
	while (SDL_PollEvent(&p_game->event)) {
		switch (p_game->event.type) {
		case SDL_QUIT: p_game->running = false; break;
		case SDL_KEYDOWN:
			switch (p_game->event.key.keysym.sym) {
			case SDLK_SPACE: p_game->paused = !p_game->paused;
			}

			break;

		default: break;
		}
	}

	if (p_game->key_states[SDL_SCANCODE_Q])
		p_game->running = false;

	if (p_game->lost_timer > 0)
		return;

	SDL_Rect proj_rect, paddle_rect;
	game_get_proj_rect(p_game, &proj_rect);

	if (p_game->paused)
		return;

	float prev_y = p_game->paddle1_y;
	if (p_game->key_states[SDL_SCANCODE_W]) {
		p_game->paddle1_y -= PADDLE_SPEED;
		game_get_paddle1_rect(p_game, &paddle_rect);

		if (p_game->paddle1_y < 0)
			p_game->paddle1_y = 0;
		else if (SDL_HasIntersection(&proj_rect, &paddle_rect))
			p_game->paddle1_y = prev_y;
	} else if (p_game->key_states[SDL_SCANCODE_S]) {
		float prev_y = p_game->paddle1_y;
		p_game->paddle1_y += PADDLE_SPEED;
		game_get_paddle1_rect(p_game, &paddle_rect);

		if (p_game->paddle1_y + PADDLE_LENGTH >= SCREEN_H)
			p_game->paddle1_y = SCREEN_H - PADDLE_LENGTH;
		else if (SDL_HasIntersection(&proj_rect, &paddle_rect))
			p_game->paddle1_y = prev_y;
	}

	prev_y = p_game->paddle2_y;
	if (p_game->key_states[SDL_SCANCODE_UP]) {
		p_game->paddle2_y -= PADDLE_SPEED;
		game_get_paddle2_rect(p_game, &paddle_rect);

		if (p_game->paddle2_y < 0)
			p_game->paddle2_y = 0;
		else if (SDL_HasIntersection(&proj_rect, &paddle_rect))
			p_game->paddle2_y = prev_y;
	} else if (p_game->key_states[SDL_SCANCODE_DOWN]) {
		p_game->paddle2_y += PADDLE_SPEED;
		game_get_paddle2_rect(p_game, &paddle_rect);

		if (p_game->paddle2_y + PADDLE_LENGTH >= SCREEN_H)
			p_game->paddle2_y = SCREEN_H - PADDLE_LENGTH;
		else if (SDL_HasIntersection(&proj_rect, &paddle_rect))
			p_game->paddle2_y = prev_y;
	}
}

void game_render(game_t *p_game) {
	SDL_SetRenderDrawColor(p_game->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(p_game->renderer);

	SDL_SetRenderTarget(p_game->renderer, p_game->screen);
	SDL_RenderClear(p_game->renderer);

	SDL_SetRenderDrawColor(p_game->renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < SCREEN_W; ++ i) {
		if (i % 2 != 0)
			continue;

		SDL_Rect rect = {
			.x = SCREEN_W / 2, .y = i,
			.w = 1, .h = 1
		};

		SDL_RenderFillRect(p_game->renderer, &rect);
	}

	SDL_SetRenderDrawColor(p_game->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);

	SDL_Rect proj_rect, paddle1_rect, paddle2_rect;
	game_get_proj_rect(p_game, &proj_rect);
	game_get_paddle1_rect(p_game, &paddle1_rect);
	game_get_paddle2_rect(p_game, &paddle2_rect);

	SDL_RenderFillRect(p_game->renderer, &proj_rect);

	SDL_SetRenderDrawColor(p_game->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	SDL_RenderFillRect(p_game->renderer, &paddle1_rect);
	SDL_RenderFillRect(p_game->renderer, &paddle2_rect);

	if (p_game->lost_timer > 0) {
		SDL_Rect win_rect = {
			.x = SCREEN_W / 2 - 25 / 2, .y = SCREEN_H / 2 - 25 / 2,
			.w = 25, .h = 5
		};

		if (p_game->side == SIDE_RIGHT) /* if right side lost, left side won */
			SDL_RenderCopy(p_game->renderer, p_game->left_texture, NULL, &win_rect);
		else
			SDL_RenderCopy(p_game->renderer, p_game->right_texture, NULL, &win_rect);
	}

	if (p_game->paused) {
		SDL_SetRenderDrawColor(p_game->renderer, 255, 255, 0, 50);

		SDL_Rect shade_rect = {
			.x = 0, .y = 0,
			.w = SCREEN_W, .h = SCREEN_H
		};

		SDL_RenderFillRect(p_game->renderer, &shade_rect);

		SDL_SetRenderDrawColor(p_game->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

		SDL_Rect pause_rect = {
			.x = SCREEN_W / 2 - 3, .y = SCREEN_H / 2 - 4,
			.w = 7, .h = 8
		};

		SDL_RenderCopy(p_game->renderer, p_game->pause_texture, NULL, &pause_rect);
	}

	SDL_SetRenderTarget(p_game->renderer, NULL);
	SDL_RenderCopy(p_game->renderer, p_game->screen, NULL, &p_game->screen_rect);
	SDL_RenderPresent(p_game->renderer);
}
