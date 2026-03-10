/*
 * LESSON 1: The Framebuffer — What Rendering Really Is
 * =====================================================
 *
 * This is the foundation of everything we'll build.
 *
 * Key concept: Rendering = writing color values into a flat array of pixels.
 * That array is the "framebuffer." Everything else — 3D math, lighting,
 * textures — is just deciding WHAT color to write WHERE.
 *
 * What this program does:
 *   1. Creates a window with an 800x600 pixel buffer
 *   2. Fills the entire buffer with a dark background (clearing the screen)
 *   3. Plots individual colored pixels
 *   4. Displays the result, ~60 times per second
 *
 * Compile (Linux):
 *   gcc -o lesson01 main.c $(pkg-config --cflags --libs sdl3) -lm
 *
 * Compile (macOS with Homebrew):
 *   gcc -o lesson01 main.c $(pkg-config --cflags --libs sdl3) -lm
 *
 * If pkg-config isn't set up, try:
 *   gcc -o lesson01 main.c -I/usr/include/SDL3 -lSDL3 -lm
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ---------- Screen dimensions ---------- */
/* We choose 800x600 — a comfortable size that's easy to reason about.
 * Wolfenstein 3D ran at 320x200. Quake's default was 320x240.
 * Even at 800x600, we're only dealing with 480,000 pixels per frame. */
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

/* ---------- The Framebuffer ---------- */
/* This is it. This flat array IS the screen.
 * Each uint32_t holds one pixel: 0xAARRGGBB format.
 *
 * We use a global here intentionally — in a real engine this would be
 * part of a renderer struct, but for learning, clarity beats formality.
 */
static uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

/* ---------- Color Helpers ---------- */
/* Pack red, green, blue (each 0-255) into a single uint32_t.
 * Format: 0xAARRGGBB — Alpha is always 0xFF (fully opaque).
 *
 * Think of it as stuffing four 8-bit values into four "slots"
 * inside a 32-bit word, using bit shifts to position each one:
 *
 *   Bits: [31..24] [23..16] [15..8]  [7..0]
 *          Alpha    Red      Green    Blue
 */
static inline uint32_t color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/* ---------- Pixel Plotting ---------- */
/* THE fundamental operation of all rendering.
 * Every line, triangle, texture, and 3D model we'll ever draw
 * ultimately calls something like this.
 *
 * We do a bounds check because — especially when we start doing 3D
 * projection — coordinates WILL land outside the screen. Better to
 * discard them safely than corrupt memory. */
static void put_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
        /*         ^^^^^^^^^^^^^^^^^^^^^^^^
         * Row-major addressing: skip past 'y' complete rows,
         * then index 'x' pixels into the current row.
         *
         * This is the same memory layout as a 2D array
         * uint32_t fb[HEIGHT][WIDTH], just addressed manually.
         */
    }
}

/* ---------- Screen Clearing ---------- */
/* Fill every pixel with a single color.
 * This runs at the START of every frame — we wipe the slate clean
 * and redraw everything. This "redraw the world every frame" approach
 * is how virtually all real-time renderers work.
 *
 * Wolfenstein 3D was clever about NOT clearing — it drew every column
 * of the screen in order, left to right, so nothing needed erasing.
 * But for a general-purpose renderer, clear-then-draw is the standard. */
static void clear_screen(uint32_t color)
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = color;
    }
    /* Note: memset won't work here unless the color happens to have
     * all four bytes identical (like 0x00000000). For arbitrary colors,
     * we need this loop. A production engine might use SIMD here. */
}

/* ---------- The Render Function ---------- */
/* This is where we decide what to draw each frame.
 * Right now it's trivially simple. By Lesson 19, this function
 * will transform 3D models, rasterize triangles, apply textures
 * and lighting, and write tens of thousands of pixels per frame. */
static void render(void)
{
    /* Step 1: Clear to a dark blue-gray background */
    clear_screen(color_rgb(20, 20, 40));

    /* Step 2: Draw some pixels to prove we can!
     *
     * Let's plot a few colored dots. Each one is a single call
     * to put_pixel — the exact same operation that will later
     * fill entire triangles and textured surfaces.
     */

    /* A white pixel near the center */
    put_pixel(400, 300, color_rgb(255, 255, 255));

    /* A red pixel */
    put_pixel(100, 100, color_rgb(255, 0, 0));

    /* A green pixel */
    put_pixel(200, 100, color_rgb(0, 255, 0));

    /* A blue pixel */
    put_pixel(300, 100, color_rgb(0, 0, 255));

    /* Let's draw something slightly more interesting:
     * A small crosshair in the center of the screen,
     * done pixel-by-pixel. Primitive? Yes.
     * But this is genuine rendering. */
    uint32_t yellow = color_rgb(255, 255, 0);
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    for (int i = -20; i <= 20; i++) {
        put_pixel(cx + i, cy, yellow);       /* horizontal bar */
        put_pixel(cx, cy + i, yellow);       /* vertical bar   */
    }

    /* And a little gradient bar along the bottom — demonstrating
     * that we have full control over every pixel's color.
     * Each pixel gets a slightly different red/blue mix. */
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t r = (uint8_t)((x * 255) / SCREEN_WIDTH);
        uint8_t b = (uint8_t)(255 - r);
        put_pixel(x, SCREEN_HEIGHT - 30, color_rgb(r, 0, b));
        put_pixel(x, SCREEN_HEIGHT - 29, color_rgb(r, 0, b));
        put_pixel(x, SCREEN_HEIGHT - 28, color_rgb(r, 0, b));
    }
}

/* ================================================================
 * MAIN — SDL3 Setup and the Render Loop
 * ================================================================
 * SDL3 does three things for us:
 *   1. Creates a window (OS-level windowing)
 *   2. Gives us a texture we can stream pixel data into
 *   3. Handles input events (keyboard, mouse, quit)
 *
 * Think of SDL3 as the modern equivalent of setting VGA Mode 13h —
 * it gives us a rectangle of pixels to fill, and handles getting
 * them onto the actual display hardware.
 *
 * SDL3 API notes (vs SDL2, for reference):
 *   - SDL_Init() returns bool (true = success), not int
 *   - SDL_CreateWindow() no longer takes x,y position params
 *   - SDL_CreateRenderer() takes (window, name) — no index param
 *   - VSync set via SDL_SetRenderVSync() after renderer creation
 *   - Event constants renamed: SDL_EVENT_QUIT, SDL_EVENT_KEY_DOWN
 *   - Key field: event.key.key (not event.key.keysym.sym)
 *   - SDL_RenderCopy() is now SDL_RenderTexture()
 */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;  /* Suppress unused parameter warnings */

    /* Initialize SDL's video subsystem */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Create window and renderer together.
     * SDL3 docs recommend SDL_CreateWindowAndRenderer() over creating
     * them separately, to avoid initial window flicker. */
    SDL_Window *window = NULL;
    SDL_Renderer *sdl_renderer = NULL;

    if (!SDL_CreateWindowAndRenderer(
            "Lesson 01 — The Framebuffer",   /* title bar text    */
            SCREEN_WIDTH,                     /* width in pixels   */
            SCREEN_HEIGHT,                    /* height in pixels  */
            0,                                /* window flags      */
            &window,                          /* out: window ptr   */
            &sdl_renderer))                   /* out: renderer ptr */
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer failed: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* Enable VSync — sync presentation to monitor refresh rate.
     * In SDL3, this is a separate call after renderer creation
     * (SDL2 used a flag during creation). */
    SDL_SetRenderVSync(sdl_renderer, 1);

    /* Create a streaming texture — this is the bridge between our
     * framebuffer (a plain array in RAM) and what SDL displays.
     *
     * SDL_PIXELFORMAT_ARGB8888 means: 0xAARRGGBB byte ordering.
     * This matches our color_rgb() function above.
     *
     * SDL_TEXTUREACCESS_STREAMING means: we intend to update this
     * texture's pixels every single frame (which we do). */
    SDL_Texture *texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* ---- THE RENDER LOOP ----
     *
     * This is the heartbeat of any real-time renderer:
     *
     *   while (running) {
     *       process_input();
     *       update();          // game logic, physics, etc.
     *       render();          // fill the framebuffer
     *       present();         // show it on screen
     *   }
     *
     * Wolfenstein, Doom, Quake, and every game since runs this loop.
     * At 60 FPS, everything inside executes in under ~16.6 milliseconds.
     */
    bool running = true;
    SDL_Event event;

    while (running) {
        /* --- Process Input --- */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        /* --- Render the frame --- */
        render();

        /* --- Present: copy our framebuffer to the screen ---
         *
         * SDL_UpdateTexture copies our uint32_t array into the
         * SDL texture. The "pitch" is the number of BYTES per row:
         * SCREEN_WIDTH pixels × 4 bytes per pixel.
         *
         * Then SDL_RenderTexture blits that texture to the window,
         * and SDL_RenderPresent flips it to the display.
         *
         * This is conceptually identical to what Doom did:
         * render into an off-screen buffer, then copy to VGA memory.
         */
        SDL_UpdateTexture(
            texture,
            NULL,                               /* update entire texture */
            framebuffer,                         /* our pixel data        */
            SCREEN_WIDTH * sizeof(uint32_t)      /* pitch (bytes per row) */
        );
        SDL_RenderClear(sdl_renderer);
        SDL_RenderTexture(sdl_renderer, texture, NULL, NULL);
        SDL_RenderPresent(sdl_renderer);
    }

    /* --- Cleanup --- */
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}