#include "select.h"

extern SDL_Surface * cursor;
extern SDL_Surface * usual_suspects;
extern SDL_Surface * csel;

extern SDL_Surface * sprite[NUM_CHARS][NUM_SPRITES];

const int cs_x[] = {3, 74, 145, 216, 287, 358, 429, 500, 571, 287};
const int cs_y[] = {308, 234, 162, 90, 18, 90, 162, 234, 308, 272};

const int cs_sdx_x[] = {11, 214, 563, 360};
const int cs_sdx_y[] = {18, 272, 18, 272};

const int cs_snodx_x[] = {11, 287, 563};
const int cs_snodx_y[] = {18, 272, 18};

extern unsigned char secret_chars;
extern unsigned char dx_mode;

extern unsigned char p_choice[];

extern char cd_mode;
extern SDL_CD * cdrom;

extern int gamestate;
extern int vol_music;
extern int mx, my;

extern Mix_Music * tunes[];

int select_main()
{
	int i, player = 0;
	SDL_Surface * screen = SDL_GetVideoSurface();
	SDL_Rect center_screen, cursor_rect, char_rect, button_src_rect;
	center_screen.x = center_screen.w = center_screen.h = center_screen.y = 0;
	cursor_rect.w = cursor_rect.h = 0;
	char_rect.h = char_rect.w = 0;

	/* This is the music to play. */
	if (vol_music) {
		if (dx_mode) {
			if (secret_chars)
				playtune(SNG_S_DXSELECT, -1);

			else
				playtune(SNG_DXSELECT, -1);
		} else {
			if (secret_chars)
				playtune(SNG_S_SELECT, -1);

			else
				playtune(SNG_SELECT, -1);
		}
	}

	while (gamestate == GAMESTATE_SELECT) {
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
		SDL_BlitSurface(usual_suspects, NULL, screen, &center_screen);
		// draw "exit" button
		button_src_rect.x = 324;
		button_src_rect.y = 41;
		button_src_rect.w = 96;
		button_src_rect.h = 32;
		char_rect.x = 272;
		char_rect.y = 444;
		SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
		// draw "player 1/2" sign
		button_src_rect.x = 421;
		char_rect.y = 188;
		char_rect.x = 270;
		SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
		char_rect.y = 229;
		char_rect.x = 308;
		button_src_rect.w = 16;
		button_src_rect.x = 535 + (16 * player);
		SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
		button_src_rect.h = 41;
		button_src_rect.w = 65;
		button_src_rect.y = 0;

		for (i = 0; i < (dx_mode ? 10 : 9); i++) {
			button_src_rect.x = 65 * i;
			char_rect.x = cs_x[i];
			char_rect.y = cs_y[i];
			SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
			//char_rect.x=cs_x[i]+1;
			char_rect.y = cs_y[i] + 41;
			SDL_BlitSurface(sprite[i][0], NULL, screen, &char_rect);
		}

		if (secret_chars) {
			button_src_rect.y = 41;
			button_src_rect.w = 81;

			if (dx_mode) {
				for (i = 0; i < 4; i++) {
					button_src_rect.x = 81 * i;
					char_rect.x = cs_sdx_x[i] - 8;
					char_rect.y = cs_sdx_y[i] + 128;
					SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
					//char_rect.x=cs_x[i]+1;
					char_rect.x = cs_sdx_x[i];
					char_rect.y = cs_sdx_y[i];
					SDL_BlitSurface(sprite[i + 10][0], NULL, screen, &char_rect);
				}
			} else {
				for (i = 0; i < 3; i++) {
					button_src_rect.x = 81 * i;
					char_rect.x = cs_snodx_x[i] - 8;
					char_rect.y = cs_snodx_y[i] + 128;
					SDL_BlitSurface(csel, &button_src_rect, screen, &char_rect);
					//char_rect.x=cs_x[i]+1;
					char_rect.x = cs_snodx_x[i];
					char_rect.y = cs_snodx_y[i];
					SDL_BlitSurface(sprite[i + 10][0], NULL, screen, &char_rect);
				}
			}
		}

		char_rect.x = cs_x[i];
		char_rect.y = cs_y[i];
		cursor_rect.x = mx;
		cursor_rect.y = my;
		SDL_BlitSurface(cursor, NULL, screen, &cursor_rect);
		SDL_Flip(screen);
		/* Don't run too fast */
		SDL_Delay(1);
		SDL_Event event;

		/* Check for events */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					gamestate = GAMESTATE_TITLE;

				break;

			case SDL_MOUSEBUTTONUP:
				mx = event.button.x;
				my = event.button.y;

				for (i = 0; i < (dx_mode ? 10 : 9); i++) {
					if (mx >= cs_x[i] && mx < cs_x[i] + 65 &&
						my >= cs_y[i] && my < cs_y[i] + 169) {
						p_choice[player] = i;
						player ++;

						if (player >= 2) gamestate = GAMESTATE_GAME;

						break;
					}
				}

				if (secret_chars) {
					if (dx_mode) {
						for (i = 0; i < 4; i++) {
							if (mx >= cs_sdx_x[i] - 8 && mx < cs_sdx_x[i] + 81 &&
								my >= cs_sdx_y[i] + 128 && my < cs_sdx_y[i] + 169) {
								p_choice[player] = i + 10;
								player ++;

								if (player >= 2) gamestate = GAMESTATE_GAME;

								break;
							} else if (mx >= cs_sdx_x[i] && mx < cs_sdx_x[i] + 65 &&
								my >= cs_sdx_y[i] && my < cs_sdx_y[i] + 128) {
								p_choice[player] = i + 10;
								player ++;

								if (player >= 2) gamestate = GAMESTATE_GAME;

								break;
							}
						}
					} else {
						for (i = 0; i < 3; i++) {
							if (mx >= cs_snodx_x[i] - 8 && mx < cs_snodx_x[i] + 81 &&
								my >= cs_snodx_y[i] + 128 && my < cs_snodx_y[i] + 169) {
								p_choice[player] = i + 10;
								player ++;

								if (player >= 2) gamestate = GAMESTATE_GAME;

								break;
							} else if (mx >= cs_snodx_x[i] && mx < cs_snodx_x[i] + 65 &&
								my >= cs_snodx_y[i] && my < cs_snodx_y[i] + 128) {
								p_choice[player] = i + 10;
								player ++;

								if (player >= 2) gamestate = GAMESTATE_GAME;

								break;
							}
						}
					}
				}

				if (mx >= 272 && mx < 368 &&
					my >= 444 && my <= 476)
					gamestate = GAMESTATE_TITLE;

				break;

			case SDL_MOUSEMOTION:
				mx = event.motion.x;
				my = event.motion.y;
				break;

			case SDL_QUIT:
				gamestate = GAMESTATE_DONE;
				break;

			default:
				break;
			}
		}
	}

	if (vol_music) {
		stoptune
	}

	return gamestate;
}
