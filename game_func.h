// shared game functions and other nonsense
#ifndef GAME_FUNC_H_
#define GAME_FUNC_H_

// C includes
#include <stdlib.h>
#include <time.h>

// SDL includes
#include <SDL/SDL.h>

#ifdef __APPLE__
#include "SDL_mixer/SDL_mixer.h"
#include "SDL_image/SDL_image.h"

#include "CoreFoundation/CoreFoundation.h"
#else
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_image.h"
#endif

// Gamestates
#define GAMESTATE_DONE 0
#define GAMESTATE_TITLE 1
#define GAMESTATE_CREDITS 2
#define GAMESTATE_SELECT 3
#define GAMESTATE_GAME 4
#define GAMESTATE_OPTIONS 5

#define GAMESTATE_LOAD_CHARS 100
#define GAMESTATE_UNLOAD_CHARS 101

// default chunksize should probably be in sdl_mixer, oh well.
#define MIX_DEFAULT_CHUNKSIZE   4096

#define playtune(tracknum,repeat) if (cd_mode == 0) { \
            Mix_PlayMusic(tunes[tracknum],repeat); \
            Mix_VolumeMusic(vol_music); \
	    } else if (cd_mode == 1) { \
	        SDL_CDPlayTracks(cdrom,tracknum+1,0,1,0); \
	    }

#define stoptune         if (cd_mode == 0) \
        { \
            Mix_HaltMusic(); \
        } else if (cd_mode == 1) { \
	        SDL_CDStop(cdrom); \
	    }

#define SNG_TITLE 0
#define SNG_OPTIONS SNG_TITLE + 1
#define SNG_CREDITS SNG_OPTIONS + 1
#define SNG_DXCREDITS SNG_CREDITS + 1
#define SNG_SELECT SNG_DXCREDITS + 1
#define SNG_DXSELECT SNG_SELECT + 1
#define SNG_S_SELECT SNG_DXSELECT + 1
#define SNG_S_DXSELECT SNG_S_SELECT + 1

#define SNG_FIGHT SNG_S_DXSELECT + 1
#define NUM_FIGHTSONGS 39

// 14 total chars in game
#define NUM_CHARS   14

// 12 sprites / char
#define NUM_SPRITES 12

#define CI_STAND    0
#define CI_PUNCH    1
#define CI_KICK     2
#define CI_HIT      3
#define CI_DEAD     4
#define CI_SPECIAL  5

#define CF_DX       0x80
#define CF_SECRET   0x40

#endif
