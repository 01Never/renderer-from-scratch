#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

typedef struct
{
    int x;
    int y;
} point;

typedef struct
{
    float x;
    float y;
    float z;
} vec3;

typedef struct
{
    int x;
    int y;
    int z;
} vert_idx;

typedef struct
{
    vec3 a;
    vec3 b;
    vec3 c;
} face;

typedef struct
{
    float x;
    float y;
} vec2;

static void render(void);
void rotation(vec3 *vert2);
void draw_line(point p1, point p2);

/* ---------- OBJ Loader ---------- */
static vec3  *vert      = NULL;
static face  *facev      = NULL;
static point *verts2d    = NULL;
static int    vert_count = 0;
static int    face_count = 0;

static int load_obj(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Could not open %s\n", path); return 0; }

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == 'v' && line[1] == ' ')
        {
            vec3 v;
            if (sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z) == 3)
            {
                //TODO safly reallocate and make this effiecnt via geometric growth
                vert  = realloc(vert,  (vert_count + 1) * sizeof(vec3));
                verts2d = realloc(verts2d, (vert_count + 1) * sizeof(point));
                vert[vert_count] = v;
                vert[vert_count].y = -1 * vert[vert_count].y; //TODO FLIPING IT UPSIDE UP
                vert_count++;
            }
        }
        else if (line[0] == 'f' && line[1] == ' ') 
        {
          face f;
          vert_idx a;
          vert_idx b;
          vert_idx c;
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z) == 9)
            {
                facev  = realloc(facev,  (face_count + 1) * sizeof(face));
                facev[face_count].a.x = (float)a.x ;
                facev[face_count].a.y = (float)a.y ;
                facev[face_count].a.z = (float)a.z ;

                facev[face_count].b.x = (float)b.x;
                facev[face_count].b.y = (float)b.y;
                facev[face_count].b.z = (float)b.z;

                facev[face_count].c.x = (float)c.x;
                facev[face_count].c.y = (float)c.y;
                facev[face_count].c.z = (float)c.z;
                
                face_count++;
            }  
        }
    }
    fclose(f);
    return vert_count;
}

static inline uint32_t color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static void put_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

static void clear_screen(uint32_t color)
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = color;
    }
    /* Note: memset won't work here unless the color happens to have
     * all four bytes identical (like 0x00000000). For arbitrary colors,
     * we need this loop. */
}

static float angle_y = 0.0f;
static float angle_x = 0.0f;
static float angle_z = 0.0f;

/* ================================================================
 * MAIN — SDL3 Setup and the Render Loop
 * ================================================================*/
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

    bool running = true;
    SDL_Event event;
    load_obj("src/utah_teapot.obj");

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

        render();

        SDL_UpdateTexture
        (
            texture,
            NULL,                               /* update entire texture */
            framebuffer,                      /* our pixel data        */
            SCREEN_WIDTH * sizeof(uint32_t)    /* pitch (bytes per row) */
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

static void render(void)
{
    vec3 vert2[vert_count];
    point p1, p2, p3;
    memset(vert2, 0 , sizeof(vert2));
    rotation(vert2);
    
    /* connecting verts of a tringle */
    for(int i = 0; i < face_count; i++)
    {
        p1 = (point){(int)vert2[(int)facev[i].a.x - 1].x, (int)vert2[(int)facev[i].a.x - 1].y};
        p2 = (point){(int)vert2[(int)facev[i].b.x - 1].x, (int)vert2[(int)facev[i].b.x - 1].y};
        p3 = (point){(int)vert2[(int)facev[i].c.x - 1].x, (int)vert2[(int)facev[i].c.x - 1].y};
        draw_line(p1, p2);
        draw_line(p2, p3);
        draw_line(p3, p1);
    }
}

void draw_line(point p1, point p2)
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

    if((abs(p1.x - p2.x) > abs(p1.y - p2.y)))
    {
        direction =  (p1.x < p2.x) ? 1 : 0;
        major_1 = direction ? &p1.x : &p2.x;
        major_2 = direction ? &p2.x : &p1.x;
        
        minor_1 = direction ? &p1.y : &p2.y;
        minor_2 = direction ? &p2.y : &p1.y;  
        
        delta_major = *major_2 - *major_1;
        delta_minor = abs(*minor_2 - *minor_1);
        slope = ((*minor_2 - *minor_1) > 0 ) ?  1 : -1;
    }
    else 
    {
        direction =  (p1.y < p2.y) ? 1 : 0;
        major_1 = direction ? &p1.y : &p2.y;
        major_2 = direction ? &p2.y : &p1.y;
        
        minor_1 = direction ? &p1.x : &p2.x;
        minor_2 = direction ? &p2.x : &p1.x; 
        
        delta_major = *major_2 - *major_1;
        delta_minor = abs(*minor_2 - *minor_1);
        slope = ((*minor_2 - *minor_1) > 0 ) ?  1 : -1;
    }

while( *major_1 <= *major_2)
    {
    if(error > delta_major)
    {
        *minor_1 += 1*(slope);
        error -= 2*(delta_major);
    }    

    direction ? put_pixel(p1.x, p1.y, color_rgb(255, 0, 0)) : put_pixel(p2.x, p2.y, color_rgb(255, 0, 0)) ;
    (*major_1)++;
    error += 2*(delta_minor);
    }
}

void rotation(vec3 *vert2)
{
    //memset(vert2, 0 , sizeof(vert2)); TODO this should probly be checked for null or un inited
    clear_screen(color_rgb(20, 20, 40));

    angle_y = fmodf(angle_y + 0.005f, 2.0f * (float)M_PI);
    angle_x = fmodf(angle_x + 0.001f, 2.0f * (float)M_PI);
    angle_z = fmodf(angle_z + 0.001f, 2.0f * (float)M_PI);

    float cy = cosf(angle_y), sy = sinf(angle_y);
    float cx = cosf(angle_x), sx = sinf(angle_x);
    float cz = cosf(angle_z), sz = sinf(angle_z);

    for (int i = 0; i < vert_count; i++)
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

        /* of set from camara, focal length, and center on 2d screen */
        vert2[i].x = (int)(rx3 / (rz3 + 5) * 300 + 400);
        vert2[i].y = (int)(ry3 / (rz3 + 5) * 300 + 400);

        /* Ortho aka no depth */
        // vert2[i].x = (int)(rx3 + 400);
        // vert2[i].y = (int)(ry3 + 300);

        // put_pixel((int)(rx3 / (rz3 + 10) * 300 + 400),
        //           (int)(ry3 / (rz3 + 10) * 300 + 300),
        //           color_rgb(0, 255, 0));
    }
}