// SlugFest '97 DX
//  Greg Kennedy 2011
//  with help from Erin Kennedy, Rusty Terwelp

// Shared game functions and defines
#include "game_func.h"

#include <zlib.h>

// Different game screens (title screen, credits, etc)
#include "title.h"
#include "credits.h"
#include "select.h"
#include "game.h"
#include "options.h"

// global variables
//  reference specifically needed ones using "extern" in other files
SDL_Surface * cursor = NULL;
SDL_Surface * arena = NULL;
SDL_Surface * arena2k = NULL;
SDL_Surface * credits = NULL;
SDL_Surface * csel = NULL;
SDL_Surface * title = NULL;
SDL_Surface * mainmenu = NULL;
SDL_Surface * usual_suspects = NULL;
SDL_Surface * options = NULL;

// Music and Sound FX volumes
int vol_music;
int vol_sfx;
Mix_Music * tunes [SNG_FIGHT + NUM_FIGHTSONGS] = {NULL};

Mix_Chunk * sfx_charge = NULL;
Mix_Chunk * sfx_duck = NULL;
Mix_Chunk * sfx_jump = NULL;
Mix_Chunk * sfx_win[2] = {NULL};
Mix_Chunk * sfx_hit[5] = {NULL};

SDL_CD * cdrom = NULL;
short unsigned int use_cd;
char cd_mode;

// What gamestate we are in, changing this flips screens
int gamestate;

int mx, my; // current mouse X / Y location

// character / game related globals
unsigned char dx_mode = 0, secret_chars = 0;
unsigned char p_choice[2];

SDL_Surface * sprite[NUM_CHARS][NUM_SPRITES] = {{NULL}};

short unsigned int fullscreen;
int gamekeys[14];

// SDL_RWops wrapper for zlib
// RWops wrappers for gzip (zlib)
static void sdl_rwops_gzperror(const gzFile file, const char * caller)
{
	int errnum;
	const char * message = gzerror(file, &errnum);
	fprintf(stderr, "zlib error in %s: %s\n", caller, message);

	if (errnum == Z_ERRNO) perror("\tZ_ERRNO detail");
}

static int SDLCALL sdl_rwops_gzread(SDL_RWops * context, void * ptr, int size, int maxnum)
{
//	z_size_t ret = gzfread(ptr, size, maxnum, context->hidden.unknown.data1);
	int ret = gzread(context->hidden.unknown.data1, ptr, size * maxnum) / size;

	if (ret == 0 && !gzeof(context->hidden.unknown.data1))
		sdl_rwops_gzperror(context->hidden.unknown.data1, "gzread");

	return ret;
}

static int SDLCALL sdl_rwops_gzseek(SDL_RWops * context, int offset, int whence)
{
	z_off_t ret = gzseek(context->hidden.unknown.data1, offset, whence);

	if (ret == -1)
		sdl_rwops_gzperror(context->hidden.unknown.data1, "gzseek");

	return ret;
}

static int SDLCALL sdl_rwops_gzclose(SDL_RWops * context)
{
//	int ret = gzclose_r(context->hidden.unknown.data1);
	int ret = gzclose(context->hidden.unknown.data1);

	switch (ret) {
	case Z_STREAM_ERROR:
		fputs("gzclose_r: Z_STREAM_ERROR: file is not valid\n", stderr);
		break;

	case Z_MEM_ERROR:
		fputs("gzclose_r: Z_MEM_ERROR: out of memory\n", stderr);
		break;

	case Z_BUF_ERROR:
		fputs("gzclose_r: Z_BUF_ERROR: last read ended in the middle of a gzip stream\n", stderr);
		break;

	case Z_ERRNO:
		perror("gzclose_r: Z_ERRNO: file operation error");
	}

	SDL_FreeRW(context);
	return ret;
}

SDL_RWops * sdl_gzopen(const char * file)
{
	SDL_RWops * rwops = SDL_AllocRW();

	if (!rwops)
		return NULL;

	rwops->hidden.unknown.data1 = gzopen(file, "r");

	if (rwops->hidden.unknown.data1 == NULL) {
		fprintf(stderr, "gzopen(%s) returned NULL: %s\n", file, strerror(errno));
		SDL_FreeRW(rwops);
		return NULL;
	}

	rwops->seek  = sdl_rwops_gzseek;
	rwops->read  = sdl_rwops_gzread;
	rwops->write = NULL;
	rwops->close = sdl_rwops_gzclose;
	return rwops;
}

// Loads an image, in displayformat fmt, from a filename.
//  Pass a 1 for colorkey to make 255,255,255 = transparent.
SDL_Surface * load_image(const char * filename, int colorkey)
{
	SDL_Surface * temp = NULL, *temp2 = NULL;
	temp = IMG_Load(filename);

	if (temp == NULL) {
		printf("IMG_Load: %s\n", IMG_GetError());
		gamestate = GAMESTATE_DONE;
		return NULL;
	}

	if (colorkey) {
		if (SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format, 255, 255, 255))) {
			printf("SDL_SetColorKey: %s\n", SDL_GetError());
			SDL_FreeSurface(temp);
			gamestate = GAMESTATE_DONE;
			return NULL;
		}
	}

	temp2 = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	if (temp2 == NULL) {
		printf("SDL_Display: %s\n", SDL_GetError());
		gamestate = GAMESTATE_DONE;
	}

	return temp2;
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 * p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		} else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}

		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface * surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 * p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;

	case 4:
		return *(Uint32 *)p;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

SDL_Surface * flipHorizontally(SDL_Surface * image, int colorkey)
{
	int x, y;
	// create a copy of the image
	SDL_Surface * flipped_image = SDL_CreateRGBSurface(SDL_HWSURFACE, image->w, image->h, image->format->BitsPerPixel,
			image->format->Rmask, image->format->Gmask, image->format->Bmask, image->format->Amask);

	if (flipped_image == NULL) {
		printf("Unable to flip image: %s\n", SDL_GetError());
		gamestate = GAMESTATE_DONE;
		return NULL;
	}

	// loop through pixels
	for (y = 0; y < image->h; y++) {
		for (x = 0; x < image->w; x++) {
			// copy pixels, but reverse the x pixels!
			putpixel(flipped_image, x, y, getpixel(image, image->w - x - 1, y));
		}
	}

	if (colorkey) {
		if (SDL_SetColorKey(flipped_image, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(flipped_image->format, 255, 255, 255))) {
			printf("SDL_SetColorKey: %s\n", SDL_GetError());
			SDL_FreeSurface(flipped_image);
			gamestate = GAMESTATE_DONE;
		}
	}

	return flipped_image;
}

// Loads all the data/ from disk.
//  This is all the title screens, character sprites, MIDI music, WAV files, ...
void load_data()
{
	int i;
	char buffer[80];
	const char * fight_music[NUM_FIGHTSONGS] = {"Bassoon",
			"Battle",
			"Beautiful Sorrow",
			"BOB!!",
			"Bottles",
			"boyblob",
			"cave",
			"ClueSong",
			"Doodlehead",
			"DUde",
			"Duet for Studel and Lauren",
			"duet",
			"erinsong",
			"Every Little Note",
			"gq_title",
			"hammer2",
			"hitcatjz",
			"IDONCARE",
			"Jamin'",
			"Lauren's Wonderful Wok",
			"lvl1",
			"marching",
			"Medieval",
			"modest",
			"OldGuy",
			"party",
			"SPECIAL",
			"sd_title",
			"Some Scary Story",
			"spelbook",
			"Strudle",
			"tetmix",
			"title",
			"vampires",
			"Victory!",
			"HnS",
			"Tweets",
			"fisher",
			"snowfall"
		};
#define QUICKLOAD_UI(NAME,TRANS) if( ! ( NAME = load_image("data/img/ui/" #NAME ".png",TRANS))) return;
	QUICKLOAD_UI(cursor, 1);
	QUICKLOAD_UI(arena, 0);
	QUICKLOAD_UI(arena2k, 0);
	QUICKLOAD_UI(credits, 0);
	QUICKLOAD_UI(csel, 0);
	QUICKLOAD_UI(title, 0);
	QUICKLOAD_UI(mainmenu, 0);
	QUICKLOAD_UI(usual_suspects, 0);
	QUICKLOAD_UI(options, 0);
	tunes[SNG_TITLE] = Mix_LoadMUS("data/music/ui/intro.mid");

	if (!tunes[SNG_TITLE]) {
		printf("Mix_LoadMUS(data/music/ui/intro.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_OPTIONS] = Mix_LoadMUS("data/music/ui/slowsong.mid");

	if (!tunes[SNG_OPTIONS]) {
		printf("Mix_LoadMUS(data/music/ui/slowsong.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_CREDITS] = Mix_LoadMUS("data/music/ui/Theme.mid");

	if (!tunes[SNG_CREDITS]) {
		printf("Mix_LoadMUS(data/music/ui/Theme.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_DXCREDITS] = Mix_LoadMUS("data/music/ui/cchitcat.mid");

	if (!tunes[SNG_DXCREDITS]) {
		printf("Mix_LoadMUS(data/music/ui/cchitcat.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_SELECT] = Mix_LoadMUS("data/music/ui/coldness.mid");

	if (!tunes[SNG_SELECT]) {
		printf("Mix_LoadMUS(data/music/ui/coldness.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_DXSELECT] = Mix_LoadMUS("data/music/ui/select.mid");

	if (!tunes[SNG_DXSELECT]) {
		printf("Mix_LoadMUS(data/music/ui/select.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_S_SELECT] = Mix_LoadMUS("data/music/ui/scared so.mid");

	if (!tunes[SNG_S_SELECT]) {
		printf("Mix_LoadMUS(data/music/ui/scared so.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	tunes[SNG_S_DXSELECT] = Mix_LoadMUS("data/music/ui/not1.mid");

	if (!tunes[SNG_S_DXSELECT]) {
		printf("Mix_LoadMUS(data/music/ui/not1.mid): %s\n", Mix_GetError());
		gamestate = GAMESTATE_DONE;
		return;
	}

	for (i = 0; i < NUM_FIGHTSONGS; i++) {
		sprintf(buffer, "data/music/fight/%s.mid", fight_music[i]);
		tunes[i + SNG_FIGHT] = Mix_LoadMUS(buffer);

		if (!tunes[i + SNG_FIGHT]) {
			printf("Mix_LoadMUS(%s): %s\n", buffer, Mix_GetError());
			gamestate = GAMESTATE_DONE;
			return;
		}
	}

	// load sfx
#define QUICKLOAD_WAV(NAME,FILE) if( ! ( NAME = Mix_LoadWAV_RW(sdl_gzopen("data/sfx/" FILE ".wav.gz"),1))) { printf("Mix_LoadWAV(%s): %s\n", FILE, Mix_GetError()); gamestate=GAMESTATE_DONE; return; }
	QUICKLOAD_WAV(sfx_charge, "charge");
	QUICKLOAD_WAV(sfx_duck, "duck");
	QUICKLOAD_WAV(sfx_jump, "jump");
	QUICKLOAD_WAV(sfx_win[0], "win1");
	QUICKLOAD_WAV(sfx_win[1], "win2");
	QUICKLOAD_WAV(sfx_hit[0], "hit1");
	QUICKLOAD_WAV(sfx_hit[1], "hit2");
	QUICKLOAD_WAV(sfx_hit[2], "hit3");
	QUICKLOAD_WAV(sfx_hit[3], "hit4");
	QUICKLOAD_WAV(sfx_hit[4], "hit5");
}

void load_chars()
{
	int i, k;
	char fname[64];
	const char * c_names[NUM_CHARS] = {"Black",
			"Box",
			"Hippie",
			"Harry",
			"Bob",
			"Itchy",
			"Mafia",
			"MM",
			"Smiley",
			"Spam",
			"Dragon",
			"Pigboy",
			"Dot",
			"Blippo"
		};
	const unsigned char c_flags[NUM_CHARS] = {(1 << CI_PUNCH) | (1 << CI_KICK),  // Mr. Black has prerendered rev punch/kick
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			(1 << CI_PUNCH),          // Mafia has pre-rev punch
			0x00,
			0x00,
			CF_DX,                // Spam is a DX-mode char
			CF_SECRET,        // secret characters
			CF_SECRET,
			CF_SECRET,
			0xFF                  // Blippo has it all.
		};
	const char * c_img_suffix[] = {"1", "2", "3", "H", "D"};

	for (i = 0; i < NUM_CHARS; i++) {
		for (k = 0; k < 5; k++) {
			sprintf(fname, "data/img/char/%s%s.png", c_names[i], c_img_suffix[k]);

			if (!(sprite[i][k] = load_image(fname, dx_mode))) return;
		}

		if (c_flags[i] & CF_SECRET) {
			sprintf(fname, "data/img/char/%s4.png", c_names[i]);

			if (!(sprite[i][5] = load_image(fname, dx_mode))) return;
		} else
			sprite[i][5] = NULL;

		// load horiz-flipped sprites
		for (k = 0; k < 5; k++) {
			if (c_flags[i] & (1 << k)) {
				sprintf(fname, "data/img/char/%s%sR.png", c_names[i], c_img_suffix[k]);

				if (!(sprite[i][k + 6] = load_image(fname, dx_mode))) return;
			} else
				sprite[i][k + 6] = flipHorizontally(sprite[i][k], dx_mode);
		}

		if (c_flags[i] & CF_SECRET) {
			if (c_flags[i] & CI_SPECIAL) {
				sprintf(fname, "data/img/char/%s4R.png", c_names[i]);

				if (!(sprite[i][11] = load_image(fname, dx_mode))) return;
			} else
				sprite[i][11] = flipHorizontally(sprite[i][5], dx_mode);
		} else
			sprite[i][11] = NULL;
	}
}

#define QUICKFREE(NAME) if (NAME != NULL) { SDL_FreeSurface (NAME); NAME = NULL; }
void free_chars()
{
	int i, j;

	for (i = 0; i < NUM_CHARS; i++) {
		for (j = 0; j < NUM_SPRITES; j++)
			QUICKFREE(sprite[i][j]);
	}
}

void free_data()
{
	int i;
	free_chars();
	QUICKFREE(cursor);
	QUICKFREE(arena);
	QUICKFREE(arena2k);
	QUICKFREE(credits);
	QUICKFREE(csel);
	QUICKFREE(title);
	QUICKFREE(mainmenu);
	QUICKFREE(usual_suspects);
	QUICKFREE(options);
#define CHUNKFREE(NAME) if (NAME != NULL) { Mix_FreeChunk (NAME); NAME = NULL; }
	CHUNKFREE(sfx_charge);
	CHUNKFREE(sfx_duck);
	CHUNKFREE(sfx_jump);
	CHUNKFREE(sfx_win[0]);
	CHUNKFREE(sfx_win[1]);
	CHUNKFREE(sfx_hit[0]);
	CHUNKFREE(sfx_hit[1]);
	CHUNKFREE(sfx_hit[2]);
	CHUNKFREE(sfx_hit[3]);
	CHUNKFREE(sfx_hit[4]);

	for (i = 0; i < NUM_FIGHTSONGS + SNG_FIGHT; i++) {
		if (tunes[i] != NULL) {
			Mix_FreeMusic(tunes[i]);
			tunes[i] = NULL;
		}
	}
}
int main(int argc, char ** argv)
{
	SDL_Surface * screen = NULL;
	FILE * inifile;
	// audio params
	int frequency;
	Uint16 format;
	int channels;
	int chunksize;
	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];

	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) {
		// error!
	}

	CFRelease(resourcesURL);
	chdir(path);
	//std::cout << "Current Path: " << path << std::endl;
#endif
	// ----------------------------------------------------------------------------
	// Read from INI file.  We do this _before_ opening the mixer or
	//  opening the screen object.
	inifile = fopen("config.ini", "r");

	if (inifile == NULL) {
		vol_sfx = MIX_MAX_VOLUME;
		vol_music = MIX_MAX_VOLUME;
		fullscreen = 1;
		frequency = MIX_DEFAULT_FREQUENCY;
		format = MIX_DEFAULT_FORMAT;
		channels = MIX_DEFAULT_CHANNELS;
		chunksize = MIX_DEFAULT_CHUNKSIZE;
		use_cd = 1;
		gamekeys[0] = SDLK_w;
		gamekeys[1] = SDLK_s;
		gamekeys[2] = SDLK_a;
		gamekeys[3] = SDLK_d;
		gamekeys[4] = SDLK_e;
		gamekeys[5] = SDLK_q;
		gamekeys[6] = SDLK_c;
		gamekeys[7] = SDLK_i;
		gamekeys[8] = SDLK_k;
		gamekeys[9] = SDLK_j;
		gamekeys[10] = SDLK_l;
		gamekeys[11] = SDLK_u;
		gamekeys[12] = SDLK_o;
		gamekeys[13] = SDLK_m;
	} else {
		if (fscanf(inifile, "vol_sfx=%d\nvol_music=%d\nfullscreen=%hu\nmix_frequency=%d\nmix_format=%hu\nmix_channels=%d\nmix_chunksize=%d\nuse_cd=%hu\n"
				"key_p1_jump=%d\nkey_p1_duck=%d\nkey_p1_left=%d\nkey_p1_right=%d\nkey_p1_punch=%d\nkey_p1_kick=%d\nkey_p1_special=%d\n"
				"key_p2_jump=%d\nkey_p2_duck=%d\nkey_p2_left=%d\nkey_p2_right=%d\nkey_p2_punch=%d\nkey_p2_kick=%d\nkey_p2_special=%d\n"
				, &vol_sfx, &vol_music, &fullscreen, &frequency, &format, &channels, &chunksize, &use_cd,
				&gamekeys[0], &gamekeys[1], &gamekeys[2], &gamekeys[3], &gamekeys[4], &gamekeys[5], &gamekeys[6],
				&gamekeys[7], &gamekeys[8], &gamekeys[9], &gamekeys[10], &gamekeys[11], &gamekeys[12], &gamekeys[13]
			) != 22) {
			vol_sfx = MIX_MAX_VOLUME;
			vol_music = MIX_MAX_VOLUME;
			fullscreen = 1;
			frequency = MIX_DEFAULT_FREQUENCY;
			format = MIX_DEFAULT_FORMAT;
			channels = MIX_DEFAULT_CHANNELS;
			chunksize = MIX_DEFAULT_CHUNKSIZE;
			use_cd = 1;
			gamekeys[0] = SDLK_w;
			gamekeys[1] = SDLK_s;
			gamekeys[2] = SDLK_a;
			gamekeys[3] = SDLK_d;
			gamekeys[4] = SDLK_e;
			gamekeys[5] = SDLK_q;
			gamekeys[6] = SDLK_c;
			gamekeys[7] = SDLK_i;
			gamekeys[8] = SDLK_k;
			gamekeys[9] = SDLK_j;
			gamekeys[10] = SDLK_l;
			gamekeys[11] = SDLK_u;
			gamekeys[12] = SDLK_o;
			gamekeys[13] = SDLK_m;
		}

		fclose(inifile);
	}

	// initialize SDL video
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | (use_cd ? SDL_INIT_CDROM : 0)) < 0) {
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);
	// create a new window
#ifdef __APPLE__
	screen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
#else
	screen = SDL_SetVideoMode(640, 480, (fullscreen ? 16 : 0), SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
#endif

	if (screen == NULL)
		printf("Unable to set 640x480 video: %s\n", SDL_GetError());

	SDL_WM_SetCaption("SlugFest '97 DX", NULL);
	SDL_ShowCursor(SDL_DISABLE);
	cd_mode = 0;

	if (SDL_CDNumDrives() > 0) {    // At least one CD drive?
		cdrom = SDL_CDOpen(0);      // open it

		if (cdrom == NULL)          // if it's null then turn off CD audio
			use_cd = 0;

		else {
			if (CD_INDRIVE(SDL_CDStatus(cdrom))) {      // check if CD inserted
				if (cdrom->numtracks == 1 + SNG_FIGHT + NUM_FIGHTSONGS)
					cd_mode = 1; // pressed CD!
				else {
					// not our mastered CD, just play start to finish.
					SDL_CDPlayTracks(cdrom, 0, 0, 0, 0);
					cd_mode = 2;
				}
			}   // no CD inserted.  Leave use_cd alone but cd_mode=0 means we get midi
		}
	} else {    // No optical drives, turn off CD audio.
		cdrom = NULL;
		use_cd = 0;
	}

	// Initialize helper libraries
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
		printf("IMG_Init: Failed to init required png support!\n");
		printf("IMG_Init: %s\n", IMG_GetError());
		return 1;
	}

	//Initialize SDL_mixer
	if (Mix_OpenAudio(frequency, format, channels, chunksize) == -1) {
		printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
		vol_sfx = 0;
		vol_music = 0;
	} else if (!Mix_QuerySpec(&frequency, &format, &channels)) {
		printf("Error querying Mix spec: %s\n", Mix_GetError());
		vol_sfx = 0;
		vol_music = 0;
		Mix_CloseAudio(); // An error querying the spec may mean bigger problems, so just shut us down now.
	}

	if (vol_sfx) Mix_Volume(-1, vol_sfx);

	// Libraries initialized.  Set us to the title screen.
	gamestate = GAMESTATE_TITLE;
	// Load ALL game data.
	load_data();
	// seed RNG
	srand((unsigned int)time(NULL));
	// get initial sane values for mx / my
	SDL_GetMouseState(&mx, &my);

	// loop
	while (gamestate != GAMESTATE_DONE) {
		switch (gamestate) {
		case GAMESTATE_TITLE:
			gamestate = title_main();
			break;

		case GAMESTATE_LOAD_CHARS:
			gamestate = GAMESTATE_CREDITS;  // credits would be next
			load_chars();                   // an error here can override GAMESTATE
			break;

		case GAMESTATE_UNLOAD_CHARS:
			gamestate = GAMESTATE_TITLE;
			free_chars();
			break;

		case GAMESTATE_OPTIONS:
			gamestate = options_main();
			break;

		case GAMESTATE_CREDITS:
			gamestate = credits_main();
			break;

		case GAMESTATE_SELECT:
			gamestate = select_main();
			break;

		case GAMESTATE_GAME:
			gamestate = game_main();
			break;

		default:
			printf("Game reached unknown gamestate %d!", gamestate);
			gamestate = GAMESTATE_DONE;
		}
	}

	// free all game data.
	free_data();
	// write config options back out
	inifile = fopen("config.ini", "w");

	if (inifile != NULL) {
		fprintf(inifile, "vol_sfx=%d\nvol_music=%d\nfullscreen=%hu\nmix_frequency=%d\nmix_format=%hu\nmix_channels=%d\nmix_chunksize=%d\nuse_cd=%hu\n"
			"key_p1_jump=%d\nkey_p1_duck=%d\nkey_p1_left=%d\nkey_p1_right=%d\nkey_p1_punch=%d\nkey_p1_kick=%d\nkey_p1_special=%d\n"
			"key_p2_jump=%d\nkey_p2_duck=%d\nkey_p2_left=%d\nkey_p2_right=%d\nkey_p2_punch=%d\nkey_p2_kick=%d\nkey_p2_special=%d\n",
			vol_sfx, vol_music, fullscreen, frequency, format, channels, chunksize, use_cd,
			gamekeys[0], gamekeys[1], gamekeys[2], gamekeys[3], gamekeys[4], gamekeys[5], gamekeys[6],
			gamekeys[7], gamekeys[8], gamekeys[9], gamekeys[10], gamekeys[11], gamekeys[12], gamekeys[13]
		);
		fclose(inifile);
	}

	// turn on the cursor
	SDL_ShowCursor(SDL_ENABLE);

	// cd spinning? stop it : )
	if (cd_mode != 0)
		SDL_CDStop(cdrom);

	// cdrom drive opened? close it
	if (cdrom != NULL)
		SDL_CDClose(cdrom);

	// shut down audio library
	Mix_CloseAudio();
	// shut down SDL
	SDL_Quit();
	return 0;
}
