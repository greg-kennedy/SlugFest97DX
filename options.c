#include "options.h"

extern SDL_Surface *cursor;
extern SDL_Surface *options;

extern int gamestate;
extern int vol_music, vol_sfx;

extern short unsigned int use_cd;
extern char cd_mode;
extern SDL_CD *cdrom;

extern int mx, my;

extern short unsigned int fullscreen;

extern int gamekeys[];

extern Mix_Music *tunes[];

int options_main()
{
    SDL_Surface *screen = SDL_GetVideoSurface();

    char remapping = -1, slider = 0;

    SDL_Rect center_screen, cursor_rect, check_rect, bar_rect, remap_rect;
    center_screen.x=center_screen.w=center_screen.h=0;
    center_screen.y=0;
    cursor_rect.w=cursor_rect.h=0;

    check_rect.w = check_rect.h = 24;

    bar_rect.h = 28;
    bar_rect.x = 197;

	/* This is the music to play. */
	if (vol_music) {
        playtune(SNG_OPTIONS,-1);
	}

	while ( gamestate == GAMESTATE_OPTIONS )
	{
	    // blank screen and draw options menu
        SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
        SDL_BlitSurface(options,NULL,screen,&center_screen);

        // fill in the checkboxes and sliderbars
        if (use_cd)
        {
            check_rect.x = 417;
            check_rect.y = 163;
            SDL_FillRect(screen,&check_rect, SDL_MapRGB(screen->format,0,0,0));
        }
        if (fullscreen)
        {
            check_rect.x = 589;
            check_rect.y = 286;
            SDL_FillRect(screen,&check_rect, SDL_MapRGB(screen->format,0,0,0));
        }
        bar_rect.w = 2 * vol_music;
        bar_rect.y = 121;
        SDL_FillRect(screen,&bar_rect, SDL_MapRGB(screen->format,0,255,0));
        bar_rect.w = 2 * vol_sfx;
        bar_rect.y = 230;
        SDL_FillRect(screen,&bar_rect, SDL_MapRGB(screen->format,0,255,0));
        if (remapping < 0)
        {
            remap_rect.x = 289;
            remap_rect.h = 21;
            remap_rect.y = 402;
            remap_rect.w = 295;
            SDL_FillRect(screen,&remap_rect, SDL_MapRGB(screen->format,255,255,255));
            remap_rect.y = 422;
            remap_rect.w = 186;
            SDL_FillRect(screen,&remap_rect, SDL_MapRGB(screen->format,255,255,255));
        } else {
            remap_rect.w = 114;
            remap_rect.h = 23;
            remap_rect.x = 56 + ((remapping / 7) > 0 ? 114: 0 );
            remap_rect.y = 300 + (23 * (remapping % 7));
            SDL_FillRect(screen,&remap_rect, SDL_MapRGB(screen->format,255,255,0));
        }

	    cursor_rect.x=mx;
	    cursor_rect.y=my;

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
					{
					    if (remapping >= 0) { remapping = -1; } else {
                            gamestate = GAMESTATE_TITLE;
					    }
                    }
                    if (remapping >= 0) { gamekeys[(unsigned int)remapping] = event.key.keysym.sym; remapping = -1;}
					break;
				case SDL_MOUSEBUTTONUP:
					mx=event.button.x;
					my=event.button.y;
					slider = 0;
					if (mx > 527 && my > 430 && mx < 625 && my < 464) gamestate = GAMESTATE_TITLE;
					else if (mx > 410 && my > 156 && mx < 447 && my < 193) { use_cd = !use_cd; }
					else if (mx > 582 && my > 279 && mx < 619 && my < 316) fullscreen = !fullscreen;
					else if (mx > 56 && my > 300 && mx < 285 && my < 461) {
					    remapping = (my - 300) / 23;
					    if (mx > 168) remapping += 7;
					}
					//else if (mx > 196 && my > )
/*					if (event.button.x > 240 && event.button.x < 300 &&
                        event.button.y > 300 && event.button.y < 315) secret_chars = 1; else secret_chars = 0;*/
//					gamestate = GAMESTATE_SELECT;
					break;
				case SDL_MOUSEBUTTONDOWN:
					mx=event.button.x;
					my=event.button.y;
					if (mx > 196 && mx < 445)
					{

                        if (my > 120 && my < 149) slider = 1;
                        else if (my > 229 && my < 258) slider = 2;
                        else slider = 0;
					}
					else slider = 0;
				case SDL_MOUSEMOTION:
					mx=event.motion.x;
					my=event.motion.y;
					if (slider == 1)
					{
					    int temp = (mx - 190) / 2;
					    if (temp <= 0) vol_music = 0;
					    else if (temp >= MIX_MAX_VOLUME) vol_music = MIX_MAX_VOLUME;
					    else vol_music = temp;
                        Mix_VolumeMusic(vol_music);
					} else if (slider == 2) {
					    int temp = (mx - 190) / 2;
					    if (temp <= 0) vol_sfx = 0;
					    else if (temp >= MIX_MAX_VOLUME) vol_sfx = MIX_MAX_VOLUME;
					    else vol_sfx = temp;
					}
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
