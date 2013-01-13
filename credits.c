#include "credits.h"

extern SDL_Surface *cursor;
extern SDL_Surface *credits;

extern unsigned char dx_mode,secret_chars;

extern int gamestate;
extern int vol_music;
extern int mx, my;

extern char cd_mode;
extern SDL_CD *cdrom;

extern Mix_Music *tunes[];

int credits_main()
{
    SDL_Surface *screen = SDL_GetVideoSurface();

    SDL_Rect center_screen, cursor_rect;
    center_screen.x=center_screen.w=center_screen.h=0;
    center_screen.y=120;
    cursor_rect.w=cursor_rect.h=0;

	/* This is the music to play. */
	if (vol_music) {
        if (dx_mode) {
            playtune(SNG_DXCREDITS,0);
        } else {
            playtune(SNG_CREDITS,0);
        }
	}

	while ( gamestate == GAMESTATE_CREDITS )
	{
	    cursor_rect.x=mx;
	    cursor_rect.y=my;
	    // draw title and cursor to screen
        SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
        SDL_BlitSurface(credits,NULL,screen,&center_screen);
        SDL_BlitSurface(cursor,NULL,screen,&cursor_rect);
        SDL_Flip(screen);

		/* Don't run too fast */
		SDL_Delay (1);

		SDL_Event event;
		/* Check for events */
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_ESCAPE)
						gamestate = GAMESTATE_TITLE;
					break;
				case SDL_MOUSEBUTTONUP:
					mx=event.button.x;
					my=event.button.y;
					if (event.button.x > 240 && event.button.x < 300 &&
                        event.button.y > 300 && event.button.y < 315) secret_chars = 1; else secret_chars = 0;
					gamestate = GAMESTATE_SELECT;
					break;
				case SDL_MOUSEMOTION:
					mx=event.motion.x;
					my=event.motion.y;
					break;
				case SDL_QUIT:
					gamestate = GAMESTATE_DONE;
					break;
				default:
					break;
			}
		}
	}

    if (vol_music)
    {
        stoptune
    }

	return gamestate;
}


