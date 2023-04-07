#include "game.h"

extern SDL_Surface * cursor;
extern SDL_Surface * arena;
extern SDL_Surface * arena2k;
extern SDL_Surface * csel;

extern SDL_Surface * sprite[NUM_CHARS][NUM_SPRITES];

extern int gamestate;
extern int vol_music, vol_sfx;
extern int mx, my;
extern unsigned char dx_mode;

extern Mix_Music * tunes[];

extern unsigned char p_choice[];

extern int gamekeys[];

extern char cd_mode;
extern SDL_CD * cdrom;

extern Mix_Chunk * sfx_charge;
extern Mix_Chunk * sfx_duck;
extern Mix_Chunk * sfx_jump;
extern Mix_Chunk * sfx_win[2];
extern Mix_Chunk * sfx_hit[5];

int game_main()
{
#define dist (p_rect_dst[1].x - (p_rect_dst[0].x + p_rect_dst[0].w))
	unsigned char dxkeys[14] = {0};
	Uint32 last_ticks;
	SDL_Surface * screen = SDL_GetVideoSurface();
	SDL_Rect center_screen, cursor_rect, p_rect_dst[2], p_rect_src[2], charge_rect;
	SDL_Rect t_rect;
	center_screen.w = center_screen.h = 0;
	center_screen.x = 0;
	cursor_rect.w = cursor_rect.h = 0;
	p_rect_dst[0].w = p_rect_dst[0].h = p_rect_dst[1].w = p_rect_dst[1].h = 0;
	int p_life[2] = {266, 266};
	unsigned char p_act[2] = {0};
	unsigned char p_pos[2] = {0};
	// some dxmode crap
	unsigned int time_to_regen = 0;
	unsigned int cooldown[2] = {0};
	unsigned int stun[2] = {0};
	int jumpin[2] = {0};
	int duckin[2] = {0};
	int chargin[2] = {0};
	int i, turn = 0;
	unsigned char arena_id, playing = 1, winner = 0;

	if (dx_mode) {
		SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);

		if (p_choice[0] != 1 && p_choice[0] != 2 && p_choice[0] != 12
			&& p_choice[1] != 1 && p_choice[1] != 2 && p_choice[1] != 12)
			arena_id = rand() % 2;

		else
			arena_id = 0;
	} else {
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		arena_id = 0;
	}

	if (arena_id)
		center_screen.h = arena2k->h;

	else
		center_screen.h = arena->h;

	center_screen.y = (480 - center_screen.h) / 2;
	p_rect_dst[0].y = p_rect_dst[1].y = center_screen.y + center_screen.h - 128;
	p_rect_dst[0].w = p_rect_dst[1].w = 64;
	p_rect_dst[0].h = p_rect_dst[1].h = 128;
	p_rect_dst[0].x = 0;
	p_rect_dst[1].x = 576;
	p_rect_src[0].x = p_rect_src[0].y = p_rect_src[1].x = p_rect_src[1].y = 0;
	p_rect_src[0].w = p_rect_src[1].w = 64;
	p_rect_src[0].h = p_rect_src[1].h = 128;

	/* This is the music to play. */
	if (vol_music) {
//		music = Mix_LoadMUS("data/music/title.mid");
		int music_id = (rand() % NUM_FIGHTSONGS) + SNG_FIGHT;
		playtune(music_id, -1);
	}

	// start the timer
	last_ticks = SDL_GetTicks();

	while (gamestate == GAMESTATE_GAME) {
		cursor_rect.x = mx;
		cursor_rect.y = my;
		// draw title and cursor to screen
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface((arena_id ? arena2k : arena), NULL, screen, &center_screen);

		// dx-mode only: charge rects
		if (dx_mode) {
			for (i = 0; i < 2; i++) {
				if (chargin[i]) {
					charge_rect.x = p_rect_dst[i].x - chargin[i];

					if (charge_rect.x < 0) charge_rect.x = 0;

					charge_rect.w = 64 + (2 * chargin[i]);

					if (charge_rect.w + charge_rect.x > 640) charge_rect.w = 640 - charge_rect.x;

					charge_rect.y = p_rect_dst[i].y - chargin[i];
					charge_rect.h = 128 + chargin[i];
					SDL_FillRect(screen, &charge_rect, SDL_MapRGB(screen->format, rand() % 256, rand() % 256, rand() % 256));
				}
			}
		}

		SDL_BlitSurface(sprite[p_choice[0]][p_act[0]], &p_rect_src[0], screen, &p_rect_dst[0]);
		SDL_BlitSurface(sprite[p_choice[1]][p_act[1] + 6], &p_rect_src[1], screen, &p_rect_dst[1]);
		// draw health bars
		t_rect.y = center_screen.y + 4;
		t_rect.h = 9;
		t_rect.x = 0;

		if (p_life[0] > 0) {
			t_rect.w = p_life[0] + 1;
			// black outline
			SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 0, 0, 0));
		}

		if (p_life[1] > 0) {
			t_rect.x = 639 - p_life[1];
			t_rect.w = p_life[1] + 1;
			SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 0, 0, 0));
		}

		t_rect.y ++;
		t_rect.h -= 2;
		// colorbars
		int t_life;

		if (p_life[0] > 0) {
			t_life = p_life[0];

			if (t_life > 66) t_life = 66;

			t_rect.w = t_life;
			t_rect.x = 0;
			SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 255, 0, 0));
			t_life = p_life[0] - 66;

			if (t_life > 0) {
				t_rect.w = t_life;
				t_rect.x = 66;
				SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 0, 255, 0));
			}
		}

		if (p_life[1] > 0) {
			t_life = p_life[1];

			if (t_life > 66) t_life = 66;

			t_rect.x = 640 - t_life;
			t_rect.w = t_life;
			SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 255, 0, 0));
			t_life = p_life[1] - 66;

			if (t_life > 0) {
				t_rect.x = 574 - t_life;
				t_rect.w = t_life;
				SDL_FillRect(screen, &t_rect, SDL_MapRGB(screen->format, 0, 255, 0));
			}
		}

// game over screen
		if (!playing) {
			SDL_Rect button_src_rect;
			button_src_rect.y = 41;
			button_src_rect.w = 96;
			button_src_rect.h = 32;
			// draw "player 1/2" sign
			button_src_rect.x = 421;
			t_rect.y = 200;
			t_rect.x = 270;
			t_rect.w = t_rect.h = 0;
			SDL_BlitSurface(csel, &button_src_rect, screen, &t_rect);
			button_src_rect.x = 569;
			button_src_rect.w = 81;
			t_rect.y = t_rect.x = 275;
			SDL_BlitSurface(csel, &button_src_rect, screen, &t_rect);
			t_rect.y = 241;
			t_rect.x = 308;
			button_src_rect.w = 16;
			button_src_rect.x = 535 + (16 * winner);
			SDL_BlitSurface(csel, &button_src_rect, screen, &t_rect);
		}

		if (!dx_mode) SDL_BlitSurface(cursor, NULL, screen, &cursor_rect);

		SDL_Flip(screen);
		/* Don't run too fast */
		SDL_Delay(1);

// GAME LOGIC GOES HERE
//  for non-dxmode (i.e. classic), this is absurdly simple, because
//  most logic is actually put in the keypress handler.
		if (playing) {
			if (dx_mode) {
				while (last_ticks + 8 <= SDL_GetTicks()) {
					last_ticks += 8;
					time_to_regen ++;

					if (time_to_regen > 25) {
						// Health regeneration for players under 25%
						time_to_regen = 0;

						if (p_life[0] < 66) p_life[0] ++;

						if (p_life[1] < 66) p_life[1] ++;
					}

					// swap turns to make it more fair
					turn = (turn + 1) % 2;
//                    for (i=0; i<2; i++)
					{
						i = turn;

						// jumpin / duckin is uninterruptable
						if (jumpin[i] == 1) {
							p_rect_dst[i].y -= 2;

							if (p_rect_dst[i].y <= center_screen.y + center_screen.h - 160) jumpin[i] = -1;
						} else if (jumpin[i] == -1) {
							p_rect_dst[i].y += 2;

							if (p_rect_dst[i].y >= center_screen.y + center_screen.h - 128) jumpin[i] = 0;
						}

						if (duckin[i] == 1) {
							p_rect_dst[i].y += 2;
							p_rect_dst[i].h -= 2;
							p_rect_src[i].h -= 2;

							if (p_rect_dst[i].y >= center_screen.y + center_screen.h - 96) duckin[i] = -1;
						} else if (duckin[i] == -1) {
							p_rect_dst[i].y -= 2;
							p_rect_src[i].h += 2;
							p_rect_dst[i].h += 2;

							if (p_rect_dst[i].y <= center_screen.y + center_screen.h - 128) duckin[i] = 0;
						}

						// stun can stop all other actions / keypress
						if (stun[i] > 0) stun[i] --;
						else {
							int opp = (i == 1 ? 0 : 1);

							// chargin hit the limit? fire the special
							if (chargin[i] == 1) {
								cooldown[i] = 55;
								p_act[i] = 5;

								if (dist < 16) {
									stun[opp] = 50;
									p_act[opp] = 3;
									p_life[opp] -= 33;
									chargin[opp] = 0;

									if (vol_sfx)
										Mix_PlayChannel(-1, sfx_hit[rand() % 5], 0);
								}
							}

							// decrement chargin
							if (chargin[i]) chargin[i] --;
							else {
								if (jumpin[i] == 0 && duckin[i] == 0) {
									if (dxkeys[0 + (i * 7)]) {
										jumpin[i] = 1;

										if (vol_sfx)
											Mix_PlayChannel(-1, sfx_jump, 0);
									} else if (dxkeys[1 + (i * 7)]) {
										duckin[i] = 1;

										if (vol_sfx)
											Mix_PlayChannel(-1, sfx_duck, 0);
									}
								}

								if (dxkeys[2 + (i * 7)])
									p_rect_dst[i].x -= 2;

								if (dxkeys[3 + (i * 7)])
									p_rect_dst[i].x += 2;

								// cooldown delays actions
								if (cooldown[i] > 0) cooldown[i] --;
								else {
									if (dxkeys[4 + (i * 7)]) {
										cooldown[i] = 30;
										p_act[i] = 1;

										if (duckin[opp] == 0 && dist < 16) {
											stun[opp] = 25;
											p_act[opp] = 3;
											p_life[opp] -= 16;
											chargin[opp] = 0;

											if (vol_sfx)
												Mix_PlayChannel(-1, sfx_hit[rand() % 5], 0);
										}
									} else if (dxkeys[5 + (i * 7)]) {
										cooldown[i] = 30;
										p_act[i] = 2;

										if (jumpin[opp] == 0 && dist < 16) {
											stun[opp] = 25;
											p_act[opp] = 3;
											p_life[opp] -= 16;
											chargin[opp] = 0;

											if (vol_sfx)
												Mix_PlayChannel(-1, sfx_hit[rand() % 5], 0);
										}
									} else if (dxkeys[6 + (i * 7)] && !jumpin[i] && !duckin[i]) {
										// no air specials : )
										if (vol_sfx)
											Mix_PlayChannel(-1, sfx_charge, 0);

										chargin[i] = 32;
									}
								}
							}

							if (cooldown[i] < 15) p_act[i] = 0;
						}
					}

					// check life
					if (p_life[0] <= 0) {
						p_life[0] = 0;
						p_act[0] = 4;
						p_rect_dst[0].w = p_rect_src[0].w = 128;
						p_rect_dst[0].h = p_rect_src[0].h = 64;
						p_rect_dst[0].y = center_screen.y + center_screen.h - 64;
						playing = 0;
						winner = 1;

						if (vol_sfx)
							Mix_PlayChannel(-1, sfx_win[arena_id], 0);
					} else if (p_life[1] <= 0) {
						p_life[1] = 0;
						p_act[1] = 4;
						p_rect_dst[1].w = p_rect_src[1].w = 128;
						p_rect_dst[1].h = p_rect_src[1].h = 64;
						p_rect_dst[1].y = center_screen.y + center_screen.h - 64;
						playing = 0;
						winner = 0;

						if (vol_sfx)
							Mix_PlayChannel(-1, sfx_win[arena_id], 0);
					}

					if (dist < 0) {
						int q = dist / 2;
						p_rect_dst[0].x += q;
						p_rect_dst[1].x -= q;
					}

					// correct x.
					if (p_rect_dst[0].x < 0) p_rect_dst[0].x = 0;

					if (p_rect_dst[0].x > (640 - (p_rect_dst[0].w + p_rect_dst[1].w))) p_rect_dst[0].x = (640 - (p_rect_dst[0].w + p_rect_dst[1].w));

					if (p_rect_dst[1].x < p_rect_dst[0].w) p_rect_dst[1].x = p_rect_dst[0].w;

					if (p_rect_dst[1].x > (640 - (p_rect_dst[1].w))) p_rect_dst[1].x = (640 - (p_rect_dst[1].w));
				}
			} else {
				while (last_ticks + 500 <= SDL_GetTicks()) {
					last_ticks += 500;
					p_life[0] ++;

					if (p_life[0] > 266) p_life[0] = 266;

					p_life[1] ++;

					if (p_life[1] > 266) p_life[1] = 266;
				}
			}
		} else {
		}

		SDL_Event event;

		/* Check for events */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
#define key(x) (event.key.keysym.sym == gamekeys[x])

			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					gamestate = GAMESTATE_SELECT;
				else if (!dx_mode && playing) {
					if (key(0) || key(1)) {
						p_pos[0] = 0;
						p_rect_dst[0].y = center_screen.y + center_screen.h - 128;
						p_rect_src[0].h = 128;
					} else if (key(4) || key(5) || key(6)) {
						p_act[0] = 0;

						if (p_act[1] == 3)
							p_act[1] = 0;
					} else if (key(7) || key(8)) {
						p_pos[1] = 0;
						p_rect_dst[1].y = center_screen.y + center_screen.h - 128;
						p_rect_src[1].h = 128;
					} else if (key(11) || key(12) || key(13)) {
						p_act[1] = 0;

						if (p_act[0] == 3)
							p_act[0] = 0;
					}
				} else {
					if key(0) dxkeys[0] = 0;
					else if key(1) dxkeys[1] = 0;
					else if key(2) dxkeys[2] = 0;
					else if key(3) dxkeys[3] = 0;
					else if key(4) dxkeys[4] = 0;
					else if key(5) dxkeys[5] = 0;
					else if key(6) dxkeys[6] = 0;
					else if key(7) dxkeys[7] = 0;
					else if key(8) dxkeys[8] = 0;
					else if key(9) dxkeys[9] = 0;
					else if key(10) dxkeys[10] = 0;
					else if key(11) dxkeys[11] = 0;
					else if key(12) dxkeys[12] = 0;
					else if key(13) dxkeys[13] = 0;
				}

				break;

			case SDL_KEYDOWN:

// L, R, U, D, P, K, S
				if (!dx_mode) {
					if (playing) {
						if key(0) {
							p_pos[0] = 1;
							p_rect_dst[0].y = center_screen.y + center_screen.h - 160;
							p_rect_src[0].h = 128;
						} else if key(1) {
							p_pos[0] = 2;
							p_rect_dst[0].y = center_screen.y + center_screen.h - 96;
							p_rect_src[0].h = 96;
						} else if key(2)
							p_rect_dst[0].x -= 8;
						else if key(3)
							p_rect_dst[0].x += 8;
						else if key(4) {
							p_act[0] = 1;

							if (p_pos[1] != 2 && dist < 16) {
								p_life[1] -= 4;
								p_act[1] = 3;
							}
						} else if key(5) {
							p_act[0] = 2;

							if (p_pos[1] != 1 && dist < 16) {
								p_life[1] -= 4;
								p_act[1] = 3;
							}
						} else if (key(6) && (p_choice[0] > 9)) {
							p_act[0] = 5;

							if (dist < 16) {
								p_life[1] -= 8;
								p_act[1] = 3;
							}
						} else if key(7) {
							p_pos[1] = 1;
							p_rect_dst[1].y = center_screen.y + center_screen.h - 160;
							p_rect_src[1].h = 128;
						} else if key(8) {
							p_pos[1] = 2;
							p_rect_dst[1].y = center_screen.y + center_screen.h - 96;
							p_rect_src[1].h = 96;
						} else if key(9)
							p_rect_dst[1].x -= 8;
						else if key(10)
							p_rect_dst[1].x += 8;
						else if key(11) {
							p_act[1] = 1;

							if (p_pos[0] != 2 && dist < 16) {
								p_life[0] -= 4;
								p_act[0] = 3;
							}
						} else if key(12) {
							p_act[1] = 2;

							if (p_pos[0] != 1 && dist < 16) {
								p_life[0] -= 4;
								p_act[0] = 3;
							}
						} else if (key(13) && (p_choice[1] > 9)) {
							p_act[1] = 5;

							if (dist < 16) {
								p_life[0] -= 8;
								p_act[0] = 3;
							}
						}
					}

					// check life
					if (p_life[0] <= 0) {
						p_life[0] = 0;
						p_act[0] = 4;
						p_rect_dst[0].w = p_rect_src[0].w = 128;
						p_rect_dst[0].h = p_rect_src[0].h = 64;
						p_rect_dst[0].y = center_screen.y + center_screen.h - 64;
						playing = 0;
						winner = 1;
					}

					if (p_life[1] <= 0) {
						p_life[1] = 0;
						p_act[1] = 4;
						p_rect_dst[1].w = p_rect_src[1].w = 128;
						p_rect_dst[1].h = p_rect_src[1].h = 64;
						p_rect_dst[1].y = center_screen.y + center_screen.h - 64;
						playing = 0;
						winner = 0;
					}

					// bump apart
					while (dist < 0) {
						p_rect_dst[0].x -= 4;
						p_rect_dst[1].x += 4;
					}

					// correct x
					if (p_rect_dst[0].x < 0) p_rect_dst[0].x = 0;

					if (p_rect_dst[0].x > (640 - (p_rect_dst[0].w + p_rect_dst[1].w))) p_rect_dst[0].x = (640 - (p_rect_dst[0].w + p_rect_dst[1].w));

					if (p_rect_dst[1].x < p_rect_dst[0].w) p_rect_dst[1].x = p_rect_dst[0].w;

					if (p_rect_dst[1].x > (640 - (p_rect_dst[1].w))) p_rect_dst[1].x = (640 - (p_rect_dst[1].w));
				} else {
					if key(0) dxkeys[0] = 1;
					else if key(1) dxkeys[1] = 1;
					else if key(2) dxkeys[2] = 1;
					else if key(3) dxkeys[3] = 1;
					else if key(4) dxkeys[4] = 1;
					else if key(5) dxkeys[5] = 1;
					else if (key(6) && p_choice[0] > 9) dxkeys[6] = 1;
					else if key(7) dxkeys[7] = 1;
					else if key(8) dxkeys[8] = 1;
					else if key(9) dxkeys[9] = 1;
					else if key(10) dxkeys[10] = 1;
					else if key(11) dxkeys[11] = 1;
					else if key(12) dxkeys[12] = 1;
					else if (key(13) && p_choice[1] > 9) dxkeys[13] = 1;
				}

				break;

			case SDL_MOUSEBUTTONUP:
				mx = event.motion.x;
				my = event.motion.y;

				if (!dx_mode && !playing) gamestate = GAMESTATE_SELECT;

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


