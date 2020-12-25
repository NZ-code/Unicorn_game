#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {

#include"../szablon2/SDL2-2.0.10/include/SDL.h"
#include"../szablon2/SDL2-2.0.10/include/SDL_main.h"

}

#define SCREEN_WIDTH	1750
#define SCREEN_HEIGHT	900
#define START_POSITION 120
#define MAX_BLOCKS_NUMBER 100
#define BACKGROUND_LENGTH 8
#define FIELD_AVAILABLE 2.3
#define SAVE_POSITION 100
#define JUMP_SPEED_X 0
#define JUMP_SPEED_Y 9
#define delta_distance 10
#define dash_k  5
struct coordinates_t{
	double x=0;
	double y=0;
};

struct sizes_t {
	int width = 0;
	int height = 0;
};
struct game_object{
	coordinates_t position;
	coordinates_t alt_position;
	coordinates_t speed;
	sizes_t sizes;
	SDL_Surface* graphics;
};
struct blocks {
	game_object blocks_arr[MAX_BLOCKS_NUMBER];
	int length = 0;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj�c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj�ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt �rodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1) 
// b�d� poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok�ta o d�ugo�ci bok�w l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

int jump(game_object *player,int * is_jump) {
	player->speed.y -= 0.09;
	if (player->position.y > SCREEN_HEIGHT - player->sizes.height/2) {
		player->position.y = SCREEN_HEIGHT - player->sizes.height/2;
		*is_jump = 0;
		player->speed.y = 0;
		player->speed.x = 0;
		return 0;
	}
	else  return 1;
}

int check_collision(game_object object1, game_object object2,int save_distance_x) {
	if (
		(object1.position.x + object1.sizes.width/2 + save_distance_x) - (object2.position.x - object2.sizes.width/2) >= 0
		&& !((object1.position.x - object1.sizes.width/2) - (object2.position.x + object2.sizes.width / 2) >= 0)
		&& (object1.position.y + object1.sizes.height/2) - (object2.position.y - object1.sizes.height / 2)>= 0
		&& !((object1.position.y - object1.sizes.height/2) - (object2.position.y + object2.sizes.height/2) >= 0)

		)
	{
		return 1;

	}
	else {
		return 0;
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif


void move_gameobject(game_object * object) {
	object->position.x += object->speed.x;
	object->position.y -= object->speed.y;
}
void draw_gameobject(SDL_Surface* screen, game_object object) {
	DrawSurface(screen, object.graphics,
		object.position.x,
		object.position.y);
}
void block_movement(blocks *blocks, SDL_Surface* screen,float a) {
	//block movement
	for (int i = 0; i < blocks->length; i++)
	{
		blocks->blocks_arr[i].speed.x -= a;

		move_gameobject(&blocks->blocks_arr[i]);
		draw_gameobject(screen, blocks->blocks_arr[i]);
	}
}
void free_and_destroy(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
int main(int argc, char **argv) {

	int games = 1;
	while (games > 0) {

		int t1, t2, quit, frames, rc;
		double delta, worldTime, fpsTimer, fps, distance, backSpeed;
		SDL_Event event;
		SDL_Surface* screen, * charset;
		SDL_Surface* background_spr;
		SDL_Surface* player_sprite;
		SDL_Surface* block_sprite;
		SDL_Texture* scrtex;
		SDL_Window* window;
		SDL_Renderer* renderer;
		
		int game_mode = 0; // default
		// okno konsoli nie jest widoczne, jezeli chcemy zobaczyc
		// komunikaty wypisywane printf-em trzeba w opcjach:
		// project -> szablon2 properties -> Linker -> System -> Subsystem
		// zmienic na "Console"
		// console window is not visible, to see the printf output
		// the option:
		// project -> szablon2 properties -> Linker -> System -> Subsystem
		// must be changed to "Console"
		printf("wyjscie printfa trafia do tego okienka\n");
		printf("printf output goes here\n");

		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			printf("SDL_Init error: %s\n", SDL_GetError());
			return 1;
		}

		// tryb pe�noekranowy / fullscreen mode
	//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	//	                                 &window, &renderer);
		rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
			&window, &renderer);
		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			return 1;
		};

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


		screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);


		// wylaczenie widocznosci kursora myszy
		SDL_ShowCursor(SDL_DISABLE);

		// wczytanie obrazka cs8x8.bmp
		charset = SDL_LoadBMP("../szablon2/cs8x8.bmp");
		if (charset == NULL) {
			printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};
		SDL_SetColorKey(charset, true, 0x000000);

		background_spr = SDL_LoadBMP("../szablon2/background.bmp");
		if (background_spr == NULL) {
			printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};

		player_sprite = SDL_LoadBMP("../szablon2/player.bmp");
		if (player_sprite == NULL) {
			printf("SDL_LoadBMP(player.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};

		block_sprite = SDL_LoadBMP("../szablon2/block.bmp");
		if (block_sprite == NULL) {
			printf("SDL_LoadBMP(block.bmp) error: %s\n", SDL_GetError());
			free_and_destroy(screen, scrtex, window, renderer);
			return 1;
		};
		char text[128];
		int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
		int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
		int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
		int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

		t1 = SDL_GetTicks();

		frames = 0;
		fpsTimer = 0;
		fps = 0;
		quit = 0;
		worldTime = 0;
		distance = 0;
		backSpeed = 1;
		// player 
		game_object player;
		player.graphics = player_sprite;
		player.speed.x = 0;
		player.speed.y = 0;
		player.sizes.width = 200 ;
		player.sizes.height = 200;
		player.position.x = player.sizes.width / 2;
		player.position.y = SCREEN_HEIGHT - player.sizes.height / 2 ;
		
		game_object background;
		background.graphics = background_spr;
		background.speed.x = -3;
		background.speed.y = 0;
		background.sizes.width = 5*SCREEN_WIDTH; // 5*SCREEN_WIDTH
		background.sizes.height = SCREEN_HEIGHT;
		background.position.x =  SCREEN_WIDTH * 2.5,
		background.position.y = SCREEN_HEIGHT/ 2;

		blocks blocks;
		for (int i = 0; i < MAX_BLOCKS_NUMBER; i++)
		{
			game_object block;
			block.graphics = block_sprite;
			block.sizes.width = 200;
			block.sizes.height = 200;
			block.speed.x = -3;
			block.speed.y = 0;
			block.position.x = SCREEN_WIDTH + block.sizes.width + i * SCREEN_WIDTH  ;
			block.position.y = SCREEN_HEIGHT - block.sizes.height/2;
			blocks.blocks_arr[i] = block;
			blocks.length++;
		}

		

		int is_jump = 0;
		int is_jumping = 0;
		int is_double_jump = 0;
		int is_dash = 0;
		int block_index = -1;
		int time_dash;
		float a = 0.0001;// acceleration
		while (!quit) {
			t2 = SDL_GetTicks();

			// w tym momencie t2-t1 to czas w milisekundach,
			// jaki uplyna� od ostatniego narysowania ekranu
			// delta to ten sam czas w sekundach
			// here t2-t1 is the time in milliseconds since
			// the last screen was drawn
			// delta is the same time in seconds

			delta = (t2 - t1) * 0.001;
			t1 = t2;
			worldTime += delta;
			if (distance + backSpeed * delta > 0) {
				distance += backSpeed * delta;

				//printf("%f\n", player.position.y);
			}
			

			SDL_FillRect(screen, NULL, zielony);
			/*
			
			// drawing background
			if (distance > BACKGROUND_LENGTH) {
				distance = 0;
			}
			*/
			// background movement
			if (background.position.x < -background.sizes.width/3.5) {
				background.position.x = SCREEN_WIDTH * 2.5;
			}
			move_gameobject(&background);
			draw_gameobject(screen, background);
			if (!is_dash) {
				move_gameobject(&player);
			}
			
			// drawing player
			draw_gameobject(screen, player);

			background.speed.x -= a;
			//block movement
			block_movement(&blocks, screen, a);
			// check jump
			if (game_mode == 0) {
				for (int i = 0; i < blocks.length; i++)
				{
					
					if (check_collision(player, blocks.blocks_arr[i], SAVE_POSITION) && is_jumping == 0 && game_mode == 0) {
						is_jump = 1;
						player.speed.x = JUMP_SPEED_X;
						player.speed.y = JUMP_SPEED_Y;
						is_jumping = 1;
						block_index = i;
					}
				}
				// jumping 
				if (is_jump) {
					is_jumping = jump(&player,&is_jump);
				}
			}
			if (game_mode == 1 && is_jump) {
				is_jumping = jump(&player, &is_jump);
				if (!is_jumping) {
					is_double_jump = 0;
				}
			}
			for (int i = 0; i < blocks.length; i++)
			{
				//check collision
				if (check_collision(player, blocks.blocks_arr[i],0)) {
					quit = 1;
					games++;
				}
				
			}
			if (is_dash) {
				int time_dash1 = SDL_GetTicks();
				if (time_dash1 - time_dash > 200) {
					background.speed.x /= dash_k;
					is_dash = 0;
					for (int i = 0; i < blocks.length; i++)
					{
						blocks.blocks_arr[i].speed.x /= dash_k;
					}
				}
			}


			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};

			// tekst informacyjny / info text
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
			sprintf(text, "UNICORN ATTACK, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			//	      "Esc - exit, \030 - faster, \031 - slower"
			sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			//		SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			// obs�uga zdarze� (o ile jakie� zasz�y) / handling of events (if there were any)
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						quit = 1;
						games --;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT) backSpeed = 3.0;
					else if (event.key.keysym.sym == SDLK_LEFT) backSpeed = -3.0;
					else if (event.key.keysym.sym == 'n')
					{
						printf("n -pressed\n");
						quit = 1;
						games++;
					}
					else if (event.key.keysym.sym == 'd')
					{
						printf("d -pressed\n");
						if (game_mode == 0)
							game_mode = 1;
						else
							game_mode = 0;
					}
					else if (event.key.keysym.sym == 'z' && game_mode == 1)
					{		
						if (!is_double_jump) {
							if (is_jumping) {
								is_double_jump = 1;
							}
							is_jump = 1;
							player.speed.x = JUMP_SPEED_X;
							player.speed.y = JUMP_SPEED_Y;
							is_jumping = 1;
						}
					}
					else if (event.key.keysym.sym == 'x' && game_mode == 1)
					{	
						
						if (!is_dash) {
							time_dash = SDL_GetTicks();
							background.speed.x *= dash_k;
							is_dash = 1;
							for (int i = 0; i < blocks.length; i++)
							{
								blocks.blocks_arr[i].speed.x *= dash_k;
							}
						}
			
						
						

					}
					break;
				case SDL_KEYUP:
					backSpeed = 1; // bylo 1.0 
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
			frames++;
		};

		// zwolnienie powierzchni / freeing all surfaces
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();

		games--;
	}
	return 0;
	};
