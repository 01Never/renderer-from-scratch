#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

struct point
{
    int x;
    int y;
};

struct vec3
{
    float x;
    float y;
    float z;
};

void draw_line(int *x1, int *y1, int *x2, int *y2);


/* ---------- Color Helpers ---------- */
static inline uint32_t color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/* ---------- Pixel Plotting ---------- */
static void put_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

/* ---------- Screen Clearing ---------- */
static void clear_screen(uint32_t color)
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = color;
    }
    /* Note: memset won't work here unless the color happens to have
     * all four bytes identical (like 0x00000000). For arbitrary colors,
     * we need this loop. */
}

/* cube verts — centered at origin */
static const struct vec3 vert[8] = {
    {-100,  100, -100},  /* 0: left  top    front */
    { 100,  100, -100},  /* 1: right top    front */
    { 100, -100, -100},  /* 2: right bottom front */
    {-100, -100, -100},  /* 3: left  bottom front */
    {-100,  100,  100},  /* 4: left  top    back  */
    { 100,  100,  100},  /* 5: right top    back  */
    { 100, -100,  100},  /* 6: right bottom back  */
    {-100, -100,  100},  /* 7: left  bottom back  */
};

static float angle_y = 0.0f;
static float angle_x = 0.0f;
static float angle_z = 0.0f;

/* ---------- The Render Function ---------- */
static void render(void)
{
    clear_screen(color_rgb(20, 20, 40));

    angle_y = fmodf(angle_y + 0.010f, 2.0f * (float)M_PI);
    angle_x = fmodf(angle_x + 0.005f, 2.0f * (float)M_PI);
    angle_z = fmodf(angle_z + 0.007f, 2.0f * (float)M_PI);

    float cy = cosf(angle_y), sy = sinf(angle_y);
    float cx = cosf(angle_x), sx = sinf(angle_x);
    float cz = cosf(angle_z), sz = sinf(angle_z);

    for (int i = 0; i < 8; i++)
    {
        /* rotate Y */
        float rx  = vert[i].x * cy - vert[i].z * sy;
        float ry  = vert[i].y;
        float rz  = vert[i].x * sy + vert[i].z * cy;

        /* rotate X */
        float rx2 = rx;
        float ry2 = ry * cx - rz * sx;
        float rz2 = ry * sx + rz * cx;

        /* rotate Z */
        float rx3 = rx2 * cz - ry2 * sz;
        float ry3 = rx2 * sz + ry2 * cz;
        float rz3 = rz2;

        put_pixel((int)(rx3 / (rz3 + 300) * 300 + 400),
                  (int)(ry3 / (rz3 + 300) * 300 + 300),
                  color_rgb(0, 255, 0));
    }


    // /* All 8 octants radiating from center (400, 300) */
    // int cx = 400, cy = 300;
    // int x, y, x2, y2;

    // /* Octant 1: shallow right-down  (~18.43°)  */
    // x = cx; y = cy; x2 = 700; y2 = 400;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 2: steep right-down    (~71.57°)  */
    // x = cx; y = cy; x2 = 500; y2 = 600;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 3: steep left-down     (~108.43°) */
    // x = cx; y = cy; x2 = 300; y2 = 600;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 4: shallow left-down   (~161.57°) */
    // x = cx; y = cy; x2 = 100; y2 = 400;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 5: shallow left-up     (~198.43°) */
    // x = cx; y = cy; x2 = 100; y2 = 200;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 6: steep left-up       (~251.57°) */
    // x = cx; y = cy; x2 = 300; y2 = 0;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 7: steep right-up      (~288.43°) */
    // x = cx; y = cy; x2 = 500; y2 = 0;
    // draw_line(&x, &y, &x2, &y2);

    // /* Octant 8: shallow right-up    (~341.57°) */
    // x = cx; y = cy; x2 = 700; y2 = 200;
    // draw_line(&x, &y, &x2, &y2);

}

/* ================================================================
 * MAIN — SDL3 Setup and the Render Loop
 * ================================================================
 * SDL3 does three things for us:
 *   1. Creates a window (OS-level windowing)
 *   2. Gives us a texture we can stream pixel data into
 *   3. Handles input events (keyboard, mouse, quit)
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

    /* Create window and renderer together. */
    SDL_Window *window = NULL;
    SDL_Renderer *sdl_renderer = NULL;

    if (!SDL_CreateWindowAndRenderer(
            "MY FIRST CUBE",                 /* title bar text    */
            SCREEN_WIDTH,                    /* width in pixels   */
            SCREEN_HEIGHT,                  /* height in pixels  */
            0,                        /* window flags      */
            &window,                        /* out: window ptr   */
            &sdl_renderer))               /* out: renderer ptr */
    {
        fprintf(stderr, "SDL_CreateWindowAndRenderer failed: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* Enable VSync — sync presentation to monitor refresh rate.*/
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


void draw_line(int *x1, int *y1, int *x2, int *y2)
{

    int *major_1;
    int *major_2;
    int delta_major;

    int *minor_1;
    int *minor_2;
    int delta_minor;

    int direction;
    int slope;
    int error = 0;

    if((abs(*x1 - *x2) > abs(*y1 -*y2)))
    {
        direction =  (*x1 < *x2) ? 1 : 0;
        major_1 = direction ? x1 : x2;
        major_2 = direction ? x2 : x1;
        
        minor_1 = direction ? y1 : y2;
        minor_2 = direction ? y2 : y1;  
        
        delta_major = *major_2 - *major_1;
        delta_minor = abs(*minor_2 - *minor_1);
        slope = ((*minor_2 - *minor_1) > 0 ) ?  1 : -1;
    }
    else 
    {
        direction =  (*y1 < *y2) ? 1 : 0;
        major_1 = direction ? y1 : y2;
        major_2 = direction ? y2 : y1;
        
        minor_1 = direction ? x1 : x2;
        minor_2 = direction ? x2 : x1; 
        
        delta_major = *major_2 - *major_1;
        delta_minor = abs(*minor_2 - *minor_1);
        slope = ((*minor_2 - *minor_1) > 0 ) ?  1 : -1;
    }

while( *major_1 <= *major_2)
    {
    (*major_1)++;
    error += 2*(delta_minor);
    if(error > delta_major)
    {
        *minor_1 += 1*(slope);
        error -= 2*(delta_major);
    }    

    direction ? put_pixel(*x1, *y1, color_rgb(255, 0, 0)) : put_pixel(*x2, *y2, color_rgb(255, 0, 0)) ;
    }
}