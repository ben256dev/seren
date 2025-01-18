#include <SDL2/SDL.h>
#include <stdio.h>
#include "utils.h" // Assuming this contains v2 struct and helper functions

// Window dimensions
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Square properties
#define SQUARE_SIZE 50
#define PLAYER_SPEED 2.0f  // Increased speed for visible movement

#define WHITE 0xD9D0DE
#define PINK  0xBC8DA0
#define RED   0xA22C29
#define BLACK 0x0C1713

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

typedef struct gamestate {
    playerstate player;
} gamestate;

void sdl_init(sdl_instance* sdl) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    sdl->window = SDL_CreateWindow(
        "Moving Square", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
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
}

void render_player(SDL_Renderer* renderer, const playerstate* player) {
    sdl_set_color(renderer, RED);
    SDL_Rect player_rect = { (int)player->pos.x, (int)player->pos.y, SQUARE_SIZE, SQUARE_SIZE };
    SDL_RenderFillRect(renderer, &player_rect);
}

void gamestate_render(SDL_Renderer* renderer, const gamestate* state) {
    render_clear(renderer, BLACK);
    render_player(renderer, &state->player);
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
}

int main(int argc, char *argv[]) {
    sdl_instance sdl;
    sdl_init(&sdl);

    gamestate state;
    gamestate_init(&state);

    events_info events = {0};

    while (poll_events(&events)) {
        gamestate_update(&state, &events);
        gamestate_render(sdl.renderer, &state);
    }

    sdl_destroy(&sdl);

    return 0;
}
