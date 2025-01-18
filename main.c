#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "seren.h" // Assuming this contains v2 struct and helper functions

// Window dimensions
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Square properties
#define SQUARE_SIZE 128
#define PLAYER_SPEED 10.0f
#define PLAYER_SIZE (v2){ SQUARE_SIZE, SQUARE_SIZE }

#define WHITE 0xD9D0DE
#define PINK  0xBC8DA0
#define RED   0xA22C29
#define BLACK 0x0C1713

#define ORIGIN_CENTERED (v2){ 0.5f, 0.5f }

u64 time = 0ULL;

void sdl_set_color(SDL_Renderer* renderer, u32 hex) {
    u8 r = (hex >> 16) & 0xFF;
    u8 g = (hex >> 8 ) & 0xFF;
    u8 b =  hex        & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
}

typedef struct events_info {
    v2 move_vec;
    int exit;
    SDL_Event sdl;
} events_info;

typedef struct sdl_instance {
    SDL_Renderer* renderer;
    SDL_Window* window;
} sdl_instance;

typedef struct playerstate {
    v2 pos;
} playerstate;

typedef struct camerastate {
    v2 pos;
    v2 size;
    v2 origin; // 0.0f - 1.0f
} camerastate;

v2 player_get_origin(const playerstate* player) {
    return v2_add( player->pos, v2_mult_v2(PLAYER_SIZE, ORIGIN_CENTERED) );
}

v2 camera_get_offset(const camerastate* camera) {
    return v2_add( v2_mult(camera->pos, -1.0f) , v2_mult_v2(camera->size, camera->origin) );
}

typedef struct gamestate {
    camerastate camera;
    playerstate player;
} gamestate;

typedef struct renderres {
    SDL_Texture* leaf;
} renderres;

void res_load(SDL_Renderer* renderer, renderres* res) {
    res->leaf = IMG_LoadTexture(renderer, "res/leaf.png");
    if (!res->leaf) {
        printf("Failed to load image! IMG_Error: %s\n", IMG_GetError());
        //fuschia
    }
}

void sdl_init(sdl_instance* sdl, renderres* res) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (!IMG_Init(IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    sdl->window = SDL_CreateWindow(
        "seren", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (sdl->window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl->renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl->window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    res_load(sdl->renderer, res);
}

void sdl_destroy(sdl_instance* sdl) {
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

void render_clear(SDL_Renderer* renderer, u32 color) {
    sdl_set_color(renderer, color);
    SDL_RenderClear(renderer);
}

void render_present(SDL_Renderer* renderer, int delay) {
    SDL_RenderPresent(renderer);
    SDL_Delay(delay);
    time += delay;
}

void render_player(SDL_Renderer* renderer, const camerastate* camera, const playerstate* player) {
    sdl_set_color(renderer, WHITE);
    v2 player_transformed = v2_add(player->pos, camera_get_offset(camera));
    SDL_Rect player_rect = {
        (int)(player_transformed.x), (int)(player_transformed.y),
        SQUARE_SIZE, SQUARE_SIZE
    };
    SDL_RenderFillRect(renderer, &player_rect);
}

void render_leaf(SDL_Renderer* renderer, const camerastate* camera, const renderres* res, v2 world_pos) {
    SDL_RenderCopy(renderer, res->leaf, NULL, NULL);
}

void gamestate_render(SDL_Renderer* renderer, const renderres* res, const gamestate* state) {
    render_clear(renderer, BLACK);
    render_player(renderer, &state->camera, &state->player);
    render_leaf(renderer, &state->camera, res, ORIGIN_CENTERED);
    render_present(renderer, 16);
}

const events_info* poll_events(events_info* events)
{
    while (SDL_PollEvent(&events->sdl)) {
        if (events->sdl.type == SDL_QUIT) {
            events->exit = 1;
        }
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    events->move_vec.x = 0.f, events->move_vec.y = 0.f;
    if (keystate[SDL_SCANCODE_ESCAPE]) events->exit = 1;
    if (keystate[SDL_SCANCODE_UP] | keystate[SDL_SCANCODE_W])     events->move_vec.y -= 1.0f;
    if (keystate[SDL_SCANCODE_DOWN] | keystate[SDL_SCANCODE_S])   events->move_vec.y += 1.0f;
    if (keystate[SDL_SCANCODE_LEFT] | keystate[SDL_SCANCODE_A])   events->move_vec.x -= 1.0f;
    if (keystate[SDL_SCANCODE_RIGHT] | keystate[SDL_SCANCODE_D])  events->move_vec.x += 1.0f;

    if (events->exit)
        return NULL;
    return events;
}

void gamestate_init(gamestate* state) {
    state->camera.size.x = SCREEN_WIDTH;
    state->camera.size.y = SCREEN_HEIGHT;
    state->camera.origin = ORIGIN_CENTERED;
    state->camera.pos = camera_get_offset(&state->camera);

    state->player.pos.x = (SCREEN_WIDTH - SQUARE_SIZE) / 2.0f;
    state->player.pos.y = (SCREEN_HEIGHT - SQUARE_SIZE) / 2.0f;
}

void gamestate_update(gamestate* state, const events_info* events) {
    v2 move_normalized = v2_normalized(events->move_vec);
    v2 velocity = v2_mult(move_normalized, PLAYER_SPEED);
    state->player.pos = v2_add(state->player.pos, velocity);

    if (state->player.pos.x < 0) state->player.pos.x = 0.0f;
    if (state->player.pos.y < 0) state->player.pos.y = 0.0f;
    if (state->player.pos.x > SCREEN_WIDTH - SQUARE_SIZE) state->player.pos.x = SCREEN_WIDTH - SQUARE_SIZE;
    if (state->player.pos.y > SCREEN_HEIGHT - SQUARE_SIZE) state->player.pos.y = SCREEN_HEIGHT - SQUARE_SIZE;

    state->camera.pos = v2_lerp(state->camera.pos, player_get_origin(&state->player), 0.05f);
}

int main(int argc, char *argv[]) {
    sdl_instance sdl;
    renderres res;
    sdl_init(&sdl, &res);

    gamestate state;
    gamestate_init(&state);

    events_info events = {0};

    while (poll_events(&events)) {
        gamestate_update(&state, &events);
        gamestate_render(sdl.renderer, &res, &state);
    }

    sdl_destroy(&sdl);

    return 0;
}
