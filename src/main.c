#include <SDL2/SDL.h>
#include <stdio.h>
#include "utils.h" // Assuming this contains v2 struct and helper functions

// Window dimensions
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Square properties
#define SQUARE_SIZE 50
#define PLAYER_SPEED 2.0f  // Increased speed for visible movement

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window
    SDL_Window *window = SDL_CreateWindow(
        "Moving Square", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Square starting position (Centered)
    v2 player_pos = { (SCREEN_WIDTH - SQUARE_SIZE) / 2.0f, (SCREEN_HEIGHT - SQUARE_SIZE) / 2.0f };

    // Debug: Check Initial Position
    printf("Initial Square Position: (%f, %f)\n", player_pos.x, player_pos.y);

    int running = 1;
    SDL_Event event;

    while (running) {
        // Event handling (Only check for quit event)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Get keyboard state
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        // Movement Input Struct
        typedef struct events_info {
            v2 move_vec;
            int exit;
        } events_info;

        events_info events = {0};

        if (keystate[SDL_SCANCODE_ESCAPE]) events.exit = 1;
        if (keystate[SDL_SCANCODE_UP] | keystate[SDL_SCANCODE_W])     events.move_vec.y -= 1.0f;
        if (keystate[SDL_SCANCODE_DOWN] | keystate[SDL_SCANCODE_S])   events.move_vec.y += 1.0f;
        if (keystate[SDL_SCANCODE_LEFT] | keystate[SDL_SCANCODE_A])   events.move_vec.x -= 1.0f;
        if (keystate[SDL_SCANCODE_RIGHT] | keystate[SDL_SCANCODE_D])  events.move_vec.x += 1.0f;

        if (events.exit) break;

        // Apply movement
        v2 move_normalized = v2_normalized(events.move_vec);
        v2 velocity = v2_mult(move_normalized, PLAYER_SPEED);
        player_pos = v2_add(player_pos, velocity);

        // Enforce boundary constraints
        if (player_pos.x < 0) player_pos.x = 0.0f;
        if (player_pos.y < 0) player_pos.y = 0.0f;
        if (player_pos.x > SCREEN_WIDTH - SQUARE_SIZE) player_pos.x = SCREEN_WIDTH - SQUARE_SIZE;
        if (player_pos.y > SCREEN_HEIGHT - SQUARE_SIZE) player_pos.y = SCREEN_HEIGHT - SQUARE_SIZE;

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red square

        // Cast to SDL_Rect to avoid float issues
        SDL_Rect player_rect = { (int)player_pos.x, (int)player_pos.y, SQUARE_SIZE, SQUARE_SIZE };
        SDL_RenderFillRect(renderer, &player_rect);

        SDL_RenderPresent(renderer); // Update screen

        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
