#include "title.h"

extern SDL_Surface * cursor;
extern SDL_Surface * mainmenu;
extern SDL_Surface * title;

extern int gamestate;
extern int vol_music;
extern int mx, my;
extern unsigned char dx_mode;

extern char cd_mode;
extern SDL_CD * cdrom;

extern Mix_Music * tunes[];

int title_main()
{
	SDL_Surface * screen = SDL_GetVideoSurface();
	SDL_Rect center_screen, cursor_rect, menu_rect;
	center_screen.w = center_screen.h = 0;
	center_screen.x = 0;
	center_screen.y = 120;
	menu_rect.w = menu_rect.h = 0;
	menu_rect.x = 0;
	menu_rect.y = 360;
	cursor_rect.w = cursor_rect.h = 0;

	/* This is the music to play. */
	if (vol_music)
		playtune(SNG_TITLE, 0);

//	int done = 0, ret=1;
	while (gamestate == GAMESTATE_TITLE) {
		cursor_rect.x = mx;
		cursor_rect.y = my;
		// draw title and cursor to screen
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface(title, NULL, screen, &center_screen);
		SDL_BlitSurface(mainmenu, NULL, screen, &menu_rect);
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
					gamestate = GAMESTATE_DONE;

				break;

			case SDL_MOUSEBUTTONUP:
				mx = event.button.x;
				my = event.button.y;

				if (event.button.y > menu_rect.y + 15 && event.button.y < menu_rect.y + 45) {
					if (event.button.x > 15 && event.button.x < 130) {
						dx_mode = 0;
						gamestate = GAMESTATE_LOAD_CHARS;
					} else if (event.button.x > 170 && event.button.x < 365) {
						dx_mode = 1;
						gamestate = GAMESTATE_LOAD_CHARS;
					} else if (event.button.x > 405 && event.button.x < 525) gamestate = GAMESTATE_OPTIONS;
					else if (event.button.x > 570 && event.button.x < 630) gamestate = GAMESTATE_DONE;
				}

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

//	Mix_FreeMusic(music);
//.	music = NULL;
	return gamestate;
}

