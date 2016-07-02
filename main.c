#include "SDL2/SDL.h"
#include <stdio.h>

typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long i64;
typedef int i32;
typedef short i16;
typedef char i8;

typedef struct Point {
	u32 x;
	u32 y;
	u32 z;
} Point;

typedef enum Direction {
	NORTH,
	SOUTH,
	WEST,
	EAST,
} Direction;

typedef struct GridNode {
	u32 tile_type;
} GridNode;

u32 threed_to_oned(u32 x, u32 y, u32 z, u32 x_max, u32 y_max) {
	return (z * x_max * y_max) + (y * x_max) + x;
}

u32 twod_to_oned(u32 x, u32 y, u32 x_max) {
	return (y * x_max) + x;
}

Point oned_to_twod(u32 idx, u32 x_max) {
	Point p;

	p.x = idx % x_max;
	p.y = idx / x_max;
	p.z = 0;

	return p;
}

Point oned_to_threed(u32 idx, u32 x_max, u32 y_max) {
	u32 z = idx / (x_max * y_max);
	u32 tmp_idx = idx - (z * x_max * y_max);

	Point p = oned_to_twod(tmp_idx, x_max);
	p.z = z;

	return p;
}

// Assumes a 24bit color depth for textures
void blit_surface_to_click_buffer(SDL_Surface *surface, SDL_Rect *screen_rel_rect, u32 *click_map, u32 screen_width, u32 screen_height, u32 tile_num) {
	u32 pchunk_ptr = 0;

	if ((screen_rel_rect->x >= screen_width && screen_rel_rect->x + surface->w >= screen_width) || (screen_rel_rect->y >= screen_height && screen_rel_rect->y + surface->h >= screen_height)) {
		return;
	}

	for (i32 i = 0; i < surface->w * surface->h; i++) {
		u32 pixel = ((u8 *)surface->pixels)[pchunk_ptr + 2] << 16 | ((u8 *)surface->pixels)[pchunk_ptr + 1] << 8 | ((u8 *)surface->pixels)[pchunk_ptr];
		if (pixel != 0) {
			Point pix_pos = oned_to_twod(i, surface->w);
			if ((screen_rel_rect->x + pix_pos.x < screen_width) && (screen_rel_rect->y + pix_pos.y < screen_height)) {
				u32 click_idx = twod_to_oned(screen_rel_rect->x + pix_pos.x, screen_rel_rect->y + pix_pos.y, screen_width);
				if (click_idx < screen_width * screen_height) {
					click_map[click_idx] = tile_num;
				}
			}
		}
		pchunk_ptr += 3;
	}
}

Direction cycle_right(Direction dir) {
	switch (dir) {
		case NORTH: {
			return EAST;
		} break;
		case EAST: {
			return SOUTH;
		} break;
		case SOUTH: {
			return WEST;
		} break;
		case WEST: {
			return NORTH;
		} break;
	}
}

Direction cycle_left(Direction dir) {
	switch (dir) {
		case NORTH: {
			return WEST;
		} break;
		case WEST: {
			return SOUTH;
		} break;
		case SOUTH: {
			return EAST;
		} break;
		case EAST: {
			return NORTH;
		} break;
	}
}

int main() {
    u16 screen_width = 640;
    u16 screen_height = 480;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, 0);


	if (window == NULL) {
		printf("!window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	u32 *click_map = malloc(screen_width * screen_height * sizeof(u32));
	memset(click_map, 0, screen_width * screen_height * sizeof(u32));

	SDL_Surface *wall_bmp = SDL_LoadBMP("assets/wall.bmp");
	SDL_Surface *brick_bmp = SDL_LoadBMP("assets/brick.bmp");
	SDL_Surface *grass_bmp = SDL_LoadBMP("assets/grass.bmp");
	SDL_Surface *wood_wall_bmp = SDL_LoadBMP("assets/wood_wall.bmp");
	SDL_Surface *north_door_bmp = SDL_LoadBMP("assets/north_door.bmp");
	SDL_Surface *west_door_bmp = SDL_LoadBMP("assets/west_door.bmp");
	SDL_Surface *roof_bmp = SDL_LoadBMP("assets/roof.bmp");
	SDL_Surface *cylinder_bmp = SDL_LoadBMP("assets/cylinder.bmp");
	SDL_Surface *white_brick_bmp = SDL_LoadBMP("assets/white_brick.bmp");

	SDL_SetColorKey(wall_bmp, SDL_TRUE, SDL_MapRGB(wall_bmp->format, 0, 0, 0));
	SDL_SetColorKey(brick_bmp, SDL_TRUE, SDL_MapRGB(brick_bmp->format, 0, 0, 0));
	SDL_SetColorKey(grass_bmp, SDL_TRUE, SDL_MapRGB(grass_bmp->format, 0, 0, 0));
	SDL_SetColorKey(wood_wall_bmp, SDL_TRUE, SDL_MapRGB(wood_wall_bmp->format, 0, 0, 0));
	SDL_SetColorKey(north_door_bmp, SDL_TRUE, SDL_MapRGB(north_door_bmp->format, 0, 0, 0));
	SDL_SetColorKey(west_door_bmp, SDL_TRUE, SDL_MapRGB(west_door_bmp->format, 0, 0, 0));
	SDL_SetColorKey(roof_bmp, SDL_TRUE, SDL_MapRGB(roof_bmp->format, 0, 0, 0));
	SDL_SetColorKey(cylinder_bmp, SDL_TRUE, SDL_MapRGB(cylinder_bmp->format, 0, 0, 0));
	SDL_SetColorKey(white_brick_bmp, SDL_TRUE, SDL_MapRGB(white_brick_bmp->format, 0, 0, 0));

	SDL_Texture *wall_tex = SDL_CreateTextureFromSurface(renderer, wall_bmp);
	SDL_Texture *brick_tex = SDL_CreateTextureFromSurface(renderer, brick_bmp);
	SDL_Texture *white_brick_tex = SDL_CreateTextureFromSurface(renderer, white_brick_bmp);
	SDL_Texture *grass_tex = SDL_CreateTextureFromSurface(renderer, grass_bmp);
	SDL_Texture *wood_wall_tex = SDL_CreateTextureFromSurface(renderer, wood_wall_bmp);
	SDL_Texture *north_door_tex = SDL_CreateTextureFromSurface(renderer, north_door_bmp);
	SDL_Texture *west_door_tex = SDL_CreateTextureFromSurface(renderer, west_door_bmp);
	SDL_Texture *roof_tex = SDL_CreateTextureFromSurface(renderer, roof_bmp);
	SDL_Texture *cylinder_tex = SDL_CreateTextureFromSurface(renderer, cylinder_bmp);

	FILE *fp = fopen("assets/map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " "));
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	u8 map[map_width * map_depth * map_height];
	memset(map, 0, sizeof(map));

	fgets(line, 256, fp);

	for (u8 z = 0; z < map_depth; z++) {
		for (u8 y = 0; y < map_height; y++) {
			u8 new_line = 1;
			for (u8 x = 0; x < map_width; x++) {
				char *bit;
				if (new_line == 1) {
					bit = strtok(line, " ");
					new_line = 0;
				} else {
					bit = strtok(NULL, " ");
				}

				u8 tile_id = atoi(bit);
				map[threed_to_oned(x, y, z, map_width, map_height)] = tile_id;
			}
			fgets(line, 256, fp);
		}
		fgets(line, 256, fp);
	}

    u32 player_x = 0;
	u32 player_y = 0;
	u32 player_z = 1;
	map[threed_to_oned(player_x, player_y, player_z, map_width, map_height)] = 7;

	i32 camera_x = -35;
	i32 camera_y = screen_height / 2;
	u32 scale = 1;

	Direction direction = NORTH;
	SDL_Surface *dir_door_bmp = north_door_bmp;
	SDL_Texture *dir_door_tex = north_door_tex;

	u8 running = 1;
    while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_a: {
							if ((player_x > 0) && map[threed_to_oned(player_x - 1, player_y, player_z, map_width, map_height)] == 0) {
								map[threed_to_oned(player_x, player_y, player_z, map_width, map_height)] = 0;
								map[threed_to_oned(player_x - 1, player_y, player_z, map_width, map_height)] = 7;
								player_x -= 1;
							}
						} break;
						case SDLK_d: {
							if ((player_x < map_width - 1) && map[threed_to_oned(player_x + 1, player_y, player_z, map_width, map_height)] == 0) {
								map[threed_to_oned(player_x, player_y, player_z, map_width, map_height)] = 0;
								map[threed_to_oned(player_x + 1, player_y, player_z, map_width, map_height)] = 7;
								player_x += 1;
							}
						} break;
						case SDLK_s: {
							if ((player_y < map_height - 1) && map[threed_to_oned(player_x, player_y + 1, player_z, map_width, map_height)] == 0) {
								map[threed_to_oned(player_x, player_y, player_z, map_width, map_height)] = 0;
								map[threed_to_oned(player_x, player_y + 1, player_z, map_width, map_height)] = 7;
								player_y += 1;
							}
						} break;
						case SDLK_w: {
							if ((player_y > 0) && map[threed_to_oned(player_x, player_y - 1, player_z, map_width, map_height)] == 0){
								map[threed_to_oned(player_x, player_y, player_z, map_width, map_height)] = 0;
								map[threed_to_oned(player_x, player_y - 1, player_z, map_width, map_height)] = 7;
								player_y -= 1;
							}
						} break;
						case SDLK_e: {
							direction = cycle_right(direction);
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_q: {
							direction = cycle_left(direction);
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_1: {
							scale = 1;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_2: {
							scale = 2;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_3: {
							scale = 3;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_UP: {
							camera_y -= 10 * scale;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_DOWN: {
							camera_y += 10 * scale;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_LEFT: {
							camera_x -= 10 * scale;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
						case SDLK_RIGHT: {
							camera_x += 10 * scale;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
						} break;
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					i32 mouse_x, mouse_y;
					SDL_GetMouseState(&mouse_x, &mouse_y);
					Point p = oned_to_threed(click_map[twod_to_oned(mouse_x, mouse_y, screen_width)], map_width, map_height);
					printf("screen: (%d, %d) | grid: (%u, %u, %u)\n", mouse_x, mouse_y, p.x, p.y, p.z);
				} break;
				case SDL_QUIT: {
					running = 0;
				} break;
			}
		}

		SDL_RenderClear(renderer);

		i32 box_width, box_height;
		SDL_QueryTexture(wall_tex, NULL, NULL, &box_width, &box_height);

		SDL_Rect dest;
		dest.w = box_width * scale;
		dest.h = box_height * scale;

		for (u32 z = 0; z < map_depth; z++) {
			for (u32 x = 0; x < map_width; x++) {
				for (u32 y = 0; y < map_height; y++) {


					u32 adj_y;
					u32 adj_x;

					u32 cam_adj_x;
					u32 cam_adj_y;


					switch (direction) {
						case NORTH: {
							adj_y = y;
							adj_x = x;

							cam_adj_x = x;
							cam_adj_y = map_height - y;

							dir_door_bmp = north_door_bmp;
							dir_door_tex = north_door_tex;
						} break;
						case EAST: {
							adj_y = map_height - y - 1;
							adj_x = x;

							cam_adj_x = y;
							cam_adj_y = map_width - x;
						} break;
						case SOUTH: {
							adj_y = map_height - y - 1;
							adj_x = map_width - x - 1;

							cam_adj_x = x;
							cam_adj_y = map_height - y;
						} break;
						case WEST: {
							adj_x = map_width - x - 1;
							adj_y = y;

							cam_adj_x = y;
							cam_adj_y = map_width - x;

							dir_door_bmp = west_door_bmp;
							dir_door_tex = west_door_tex;
						} break;
					}

					dest.x = ((cam_adj_x + cam_adj_y) * 16 * scale) + camera_x;
					dest.y = ((cam_adj_x - cam_adj_y) * 8 * scale) - (16 * scale * z) + camera_y;

					u8 tile_id = map[threed_to_oned(adj_x, adj_y, z, map_width, map_height)];
					switch (tile_id) {
						case 1: {
							blit_surface_to_click_buffer(wall_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, wall_tex, NULL, &dest);
						} break;
						case 2: {
							blit_surface_to_click_buffer(grass_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, grass_tex, NULL, &dest);
						} break;
						case 3: {
							blit_surface_to_click_buffer(brick_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, brick_tex, NULL, &dest);
						} break;
						case 4: {
							blit_surface_to_click_buffer(wood_wall_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, wood_wall_tex, NULL, &dest);
						} break;
						case 5: {
							blit_surface_to_click_buffer(dir_door_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, dir_door_tex, NULL, &dest);
						} break;
						case 6: {
							blit_surface_to_click_buffer(roof_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, roof_tex, NULL, &dest);
						} break;
						case 7: {
							blit_surface_to_click_buffer(cylinder_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, cylinder_tex, NULL, &dest);
						} break;
						case 8: {
							blit_surface_to_click_buffer(white_brick_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
							SDL_RenderCopy(renderer, white_brick_tex, NULL, &dest);
						} break;
					}
				}
			}
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
