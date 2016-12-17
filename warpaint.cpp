#include <SDL2/SDL.h>
#undef main
#include <stdio.h>
#include <stdint.h>
#include <math.h> // round

SDL_Rect d = {0, 0, 800, 600};
int last_mouse_x = 0;
int last_mouse_y = 0;
int mouse_x = 0;
int mouse_y = 0;
int mouse_1 = 0;
int mouse_2 = 0;
int mouse_3 = 0;

int pan_x = 0;
int pan_y = 0;
SDL_Rect sd = {(d.w-512)/2+pan_x,(d.h-512)/2+pan_y,512,512};
int surface_pitch = 3*sd.w;

void * surface; // drawing surface
void * surface_stream; // scratch surface for blitting with SDL (jesus christ...)

void maybe_draw(int x, int y)
{
    x -= sd.x;
    y -= sd.y;
    if(x < 0 or y < 0) return;
    if(x >= sd.w or y >= sd.h) return;
    ((char*)surface)[y*surface_pitch + x*3 + 0] = 0;
    ((char*)surface)[y*surface_pitch + x*3 + 1] = 0;
    ((char*)surface)[y*surface_pitch + x*3 + 2] = 0;
}

template <typename T>
int sign(T x)
{
    if(x < 0) return -1;
    if(x > 0) return 1;
    return 0;
}

void draw_line (int x1, int y1, int x2, int y2)
{
    int xdelta = x2-x1;
    int ydelta = y2-y1;
    if(xdelta == 0 and ydelta == 0)
    {
        maybe_draw(x1, y1);
        return;
    }
    if(abs(xdelta) > abs(ydelta)) // y as function of x
    {
        for (int x = 0; x <= abs(xdelta); x++)
        {
            double progress = double(x)/abs(xdelta);
            maybe_draw(x1+x*sign(xdelta), round(y1+progress*ydelta));
        }
    }
    else // x as function of y
    {
        for (int y = 0; y <= abs(ydelta); y++)
        {
            double progress = double(y)/abs(ydelta);
            maybe_draw(round(x1+progress*xdelta), y1+y*sign(ydelta));
        }
    }
}

int throttle = 1;

int main()
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
    
    window   = SDL_CreateWindow  ("Warpaint", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, d.w, d.h, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_ShowCursor(SDL_DISABLE);
    
    texture  = SDL_CreateTexture (renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, sd.w, sd.h);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    surface = malloc(sd.h*surface_pitch);
    memset(surface, 0x7F, sd.h*surface_pitch);
    // surface_stream is allocated etc by SDL
    
    int damage = 2; // 0 = no damage; 1 = low priority damage; 2 = high priority damage
    for(;;)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                exit(0);
            
            if (event.type == SDL_WINDOWEVENT)
            {
                damage = true;
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    d.w = event.window.data1;
                    d.h = event.window.data2;
                    sd.x = (d.w-sd.w)/2+pan_x,
                    sd.y = (d.h-sd.h)/2+pan_y;
                }
                
            }
            
            if (event.type == SDL_RENDER_DEVICE_RESET or event.type == SDL_RENDER_TARGETS_RESET)
            {
                // TODO: don't seem to need to do anything here with my rendering paradigm, at least yet...
            }
            
            if (event.type == SDL_MOUSEMOTION or event.type == SDL_MOUSEBUTTONDOWN or event.type == SDL_MOUSEBUTTONUP or event.type == SDL_MOUSEWHEEL)
            {
                damage = 1;
                last_mouse_x = mouse_x;
                last_mouse_y = mouse_y;
                auto buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
                mouse_1 = buttons & SDL_BUTTON_LMASK;
                mouse_2 = buttons & SDL_BUTTON_RMASK;
                mouse_3 = buttons & SDL_BUTTON_MMASK;
                
                if(mouse_1)
                {
                    draw_line(last_mouse_x, last_mouse_y, mouse_x, mouse_y);
                    damage = 2;
                }
            }
        }
        if(damage)
        {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &d);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderDrawLine(renderer, 0, 0, d.w, d.h);
            
            if (SDL_LockTexture(texture, nullptr, &surface_stream, &surface_pitch) == 0)
            {
                memcpy(surface_stream, surface, sd.h*surface_pitch);
                SDL_UnlockTexture(texture);
            }
            
            SDL_RenderCopy(renderer, texture, NULL, &sd);
            
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderDrawLine(renderer, mouse_x   , mouse_y   , mouse_x+ 2, mouse_y   );
            SDL_RenderDrawLine(renderer, mouse_x   , mouse_y   , mouse_x   , mouse_y- 2);
            SDL_RenderDrawLine(renderer, mouse_x+ 2, mouse_y   , mouse_x+12, mouse_y-10);
            SDL_RenderDrawLine(renderer, mouse_x   , mouse_y- 2, mouse_x+10, mouse_y-12);
            SDL_RenderDrawLine(renderer, mouse_x+12, mouse_y-10, mouse_x+10, mouse_y-12);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderDrawLine(renderer, mouse_x+ 1, mouse_y- 2, mouse_x+10, mouse_y-11);
            SDL_RenderDrawLine(renderer, mouse_x+ 1, mouse_y- 1, mouse_x+10, mouse_y-10);
            SDL_RenderDrawLine(renderer, mouse_x+ 2, mouse_y- 1, mouse_x+11, mouse_y-10);
            
            SDL_RenderPresent(renderer);
            
            if(damage < 2)
                SDL_Delay(throttle);
            
            damage = false;
        }
        else
            SDL_Delay(throttle);
    }
}

