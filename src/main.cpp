#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>

//Texture wrapper class
class MyTexture {
private:
    //The actual hardware texture
    SDL_Texture* texture;

    //Image dimensions
    int width;
    int height;
public:
    //Initializes variables
    MyTexture();

    //Deallocates memory
    ~MyTexture();

    //Loads image at specified path
    bool load_from_file( const char* path );

    // set color modulation
    void set_color(uint8_t red, uint8_t green, uint8_t blue);

    // alpha modulation stuff
    void set_blend_mode(SDL_BlendMode blending);
    void set_alpha(uint8_t alpha);

    //Deallocates texture
    void free();

    //Renders texture at given point
    void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

    //Gets image dimensions
    int get_width();
    int get_height();
};

enum KeyPressSurfaces
{
    KEY_PRESS_SURFACE_DEFAULT,
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};

// setup
bool init();

// load image
bool load_media();

// tear down
void close();

SDL_Surface* load_surface(const char* path);
SDL_Texture* load_texture(const char* path);

void print_sdl_error(const char* msg) {
    std::cout << msg << SDL_GetError() << std::endl;
}

void print_sdl_image_error(const char* msg) {
    std::cout << msg << IMG_GetError() << std::endl;
}

// gloabal variables
SDL_Window *window = NULL;
SDL_Surface *screen_surface = NULL;
// "images that correspond to a keypress"
SDL_Surface* key_press_surfaces[ KEY_PRESS_SURFACE_TOTAL ];
// current surface
SDL_Surface* current_surface = NULL;
SDL_Surface* png_thing = NULL;

SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;

// textures for the scene with man and bg
MyTexture man_texture;
MyTexture background_texture;

// sprite sheet and clip containers
SDL_Rect sprite_clips[4];
MyTexture sprite_sheet_texture;

// color modulated texture
MyTexture color_mod_texture;

// fade in and out texture
MyTexture fade_in_texture;
MyTexture fade_out_texture;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

// class def

MyTexture::MyTexture() : texture(NULL), width(0), height(0) { }
MyTexture::~MyTexture() {
    this->free();
}

bool MyTexture::load_from_file(const char* path) {
    SDL_Texture* loaded_texture = NULL;

    SDL_Surface* loaded_surface = load_surface(path);
    if (loaded_surface == NULL) {
        return false;
    }

    // color key image
    // pass the surface to color key, the second are is whether or not to enable
    // color keying, and the third is the pixel we want to color key with
    // SDL_MapRGB lets us create a pixel from a color and format these values map to cyan pixels
    SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, 0, 255, 255) );

    // now that we have enabled color keying for the surface
    // we convert it into a texture
    loaded_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (loaded_texture == NULL) {
        std::string err_msg = "unable to create texture from surface with path:";
        err_msg += path;
        print_sdl_error(err_msg.data());
        return false;
    }

    this->width = loaded_surface->w;
    this->height = loaded_surface->h;

    SDL_FreeSurface(loaded_surface);

    this->texture = loaded_texture;
    return true;
}

void MyTexture::free() {
    // only free if there is something to free
    if (this->texture == NULL) {
        return;
    }

    SDL_DestroyTexture(this->texture);
    this->width = 0;
    this->height = 0;
    this->texture = NULL;
}

void MyTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect render_quad = {x, y, this->width, this->height};

    // just me but i prefer the image fit the screen
    // this code is not part of tutorial
    render_quad.w = SCREEN_WIDTH;
    render_quad.h = SCREEN_HEIGHT;

    // if there is a clip apply it
    if (clip != NULL) {
        render_quad.w = clip->w;
        render_quad.h = clip->h;
    }

    // for rotation and fliping it is ez af!
    // just pass shit into the SDL_RenderCopyEx function
    // and SDL will handle the rest for you

    // render to screen
    SDL_RenderCopyEx(renderer, this->texture, clip, &render_quad, angle, center, flip);
}

int MyTexture::get_width() { return width; }
int MyTexture::get_height() { return height; }

void MyTexture::set_color(uint8_t red, uint8_t green, uint8_t blue){
    // so since all pictures have an rgb value heres how to
    // think of color modulation:
    // map 0->255 to 0->1
    // multiply the colors in the texture by these new values
    // so like if red =255= blue and green = 0
    // then all of the green in the texture will disapear
    SDL_SetTextureColorMod(this->texture, red, green, blue);
}

void MyTexture::set_blend_mode(SDL_BlendMode blending){
    SDL_SetTextureBlendMode(this->texture, blending);
}

void MyTexture::set_alpha(uint8_t alpha){
    SDL_SetTextureAlphaMod(this->texture, alpha);
}

// end class def

bool init() {
  // -1 means failure
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    print_sdl_error("failed to init with error: ");
    return false;
  }

  window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  // window is null means failure
  if (window == NULL) {
    print_sdl_error("failed to create window with error: ");
    return false;
  }

  // create renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
      print_sdl_error("could not create renderer");
      return false;
  }

  // set renderer color
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 

  const int img_flags = IMG_INIT_PNG;
  const bool init_worked = IMG_Init(img_flags) & img_flags;
  if (!init_worked) {
      print_sdl_image_error("the sdl_image extension could not init: ");
      return false;
  }

  // if we all good get the surface from of the window from the window
  screen_surface = SDL_GetWindowSurface(window);

  return true;
}

bool load_media(){
    const char* file_names[] = {"press.bmp", "up.bmp", "down.bmp", "left.bmp", "right.bmp"};
    // he does not use a for loop in the tutorial but jesus christ i had to
    for(int i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
        std::string filename = "images/"; // didnt feel like typing images 5 times
        filename+= file_names[i];
        key_press_surfaces[(KeyPressSurfaces)i] = load_surface(filename.data());
        if (key_press_surfaces[(KeyPressSurfaces)i] == NULL) {
            return false;
        }
    }

    // load le png
    png_thing = load_surface("images/loaded.png");
    if(png_thing == NULL) {
        return false;
    }

    // load le png le texture
    texture = load_texture("images/loaded.png");
    if(texture == NULL) {
        return false;
    }

    // load png's into the classes
    if (!man_texture.load_from_file("images/foo.png")) {
        std::cout << "could not load foo.png as texture" << std::endl;
        return false;
    }

    if (!background_texture.load_from_file("images/background.png")) {
        std::cout << "could not load background.png as texture" << std::endl;
        return false;
    }

    // load sprite sheet texture
    if (!sprite_sheet_texture.load_from_file("images/dots.png")) {
        return false;
    }

    // top left
    sprite_clips[0].x = 0;
    sprite_clips[0].y = 0;
    sprite_clips[0].w = 100;
    sprite_clips[0].h = 100;

    // top right
    sprite_clips[1].x = 100;
    sprite_clips[1].y = 0;
    sprite_clips[1].w = 100;
    sprite_clips[1].h = 100;

    // bottom left
    sprite_clips[2].x = 0;
    sprite_clips[2].y = 100;
    sprite_clips[2].w = 100;
    sprite_clips[2].h = 100;

    // bottom right
    sprite_clips[3].x = 100;
    sprite_clips[3].y = 100;
    sprite_clips[3].w = 100;
    sprite_clips[3].h = 100;

    // load color mod texture
    if (!color_mod_texture.load_from_file("images/colors.png")) {
        return false;
    }

    if(!fade_out_texture.load_from_file("images/fadeout.png")){
        return false;
    }

    fade_out_texture.set_blend_mode(SDL_BLENDMODE_BLEND);

    if(!fade_in_texture.load_from_file("images/fadein.png")){
        return false;
    }

    return true;
}

void close(){
    // free surfaces
    for(int i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
        SDL_FreeSurface(key_press_surfaces[i]);
        key_press_surfaces[i] = NULL;
    }

    // free png
    SDL_FreeSurface(png_thing);

    // free texture
    SDL_DestroyTexture(texture);

    // whatever it pointed to was already freed
    current_surface = NULL;
    
    // destroy window (also gets rid of window surface)
    SDL_DestroyWindow(window);
    window = NULL;
    screen_surface = NULL;

    // destroy renderer
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    
    // quit
    IMG_Quit();
    SDL_Quit();
}

// you are responsible for this pointer!
SDL_Surface* load_surface(const char* path){
    // final optimized image
    SDL_Surface* optimized_surface = NULL;

    SDL_Surface* loaded_surface = IMG_Load( path );
    if (loaded_surface == NULL) {
        std::string err_msg = "could not load image with path: ";
        err_msg += path;
        print_sdl_image_error(err_msg.data());
    }

    optimized_surface = SDL_ConvertSurface(loaded_surface, screen_surface->format, 0);
    if (optimized_surface == NULL) {
        std::string err_msg ="unable to optimze surface with path: "; 
        err_msg += path;
        print_sdl_error(err_msg.data());
    }

    SDL_FreeSurface(loaded_surface);

    return optimized_surface;
}

SDL_Texture* load_texture(const char* path) {
    SDL_Texture* loaded_texture = NULL;

    // load image at path 
    SDL_Surface* loaded_surface = load_surface(path);
    if (loaded_surface == NULL) {
        print_sdl_image_error("unable to load surface");
        return NULL;
    }

    loaded_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (loaded_texture == NULL) {
        print_sdl_error("unable to create texture from surface");
        return NULL;
    }

    SDL_FreeSurface(loaded_surface);

    return loaded_texture;
}

int main() {
    if (!init()) {
        return 1;
    }

    if (!load_media()) {
        return 1;
    }

    bool done = false;
    SDL_Event e;
    current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT];
    bool toggle_renderer = false;
    bool toggle_geometry = false;
    bool toggle_viewport = false;
    bool toggle_color_key_scene = false;
    bool toggle_sprite_sheet = false;
    bool toggle_color_modulation = false;
    bool toggle_alpha_modulation = false;

    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    uint8_t a = 255;
    while(!done) {
        while( SDL_PollEvent( &e ) != 0) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                toggle_renderer = false;
                toggle_geometry = false;
                toggle_viewport = false;
                toggle_color_key_scene = false;
                toggle_sprite_sheet = false;
                toggle_color_modulation = false;
                toggle_alpha_modulation = false;
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        current_surface = key_press_surfaces[KEY_PRESS_SURFACE_UP];
                        break;
                    case SDLK_DOWN:
                        current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DOWN];
                        break;
                    case SDLK_LEFT:
                        current_surface = key_press_surfaces[KEY_PRESS_SURFACE_LEFT];
                        break;
                    case SDLK_RIGHT:
                        current_surface = key_press_surfaces[KEY_PRESS_SURFACE_RIGHT];
                        break;
                    case SDLK_p:
                        current_surface = png_thing;
                        break;
                    case SDLK_r:
                        toggle_renderer = true;
                        break;
                    case SDLK_g:
                        toggle_geometry = true;
                        break;
                    case SDLK_v:
                        toggle_viewport = true;
                        break;
                    case SDLK_c:
                        toggle_color_key_scene = true;
                        break;
                    case SDLK_s:
                        toggle_sprite_sheet = true;
                        break;
                    case SDLK_m:
                        toggle_color_modulation = true;
                        break;
                    case SDLK_a:
                        toggle_alpha_modulation = true;
                        break;
                    default:
                        current_surface = key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT];
                        break;
                }
            }
        }

        if (toggle_renderer) {
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
        else if(toggle_geometry) {
            // clear screen
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            // render red filled quad (solid rectangle)?
            // x, y, width, height
            SDL_Rect fill_rect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // fill rect draws the entire rectangle
            SDL_RenderFillRect(renderer, &fill_rect);

            // green rect
            SDL_Rect outlined_rect = {SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3};
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            // draw rectangle only draws the perimeter
            SDL_RenderDrawRect(renderer, &outlined_rect);

            // draw blue horizontal line
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);

            // draw verticle dotted line
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
                SDL_RenderDrawPoint(renderer, SCREEN_WIDTH / 2, i);
            }

            SDL_RenderPresent(renderer);
        }
        else if (toggle_viewport) {
            // x, y, w, h
            SDL_Rect top_left_vp = {0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
            // so something something all viewports have the same coordinate system???
            // WHY WOULD YOU MAKE IT LIKE THAT
            // so like if the SCREEN_HEIGHT is 600 (which it is)
            // the bottom of this viewport is at y = 600
            // even though the viewport is only 300 (SCREEN_HEIGHT / 2) pixels
            // tall
            // so sort of like it maps the screen onto a smaller screen?
            SDL_RenderSetViewport(renderer, &top_left_vp);

            SDL_RenderCopy(renderer, texture, NULL, NULL);

            SDL_RenderPresent(renderer);

            // x, y, w, h
            SDL_Rect top_right_vp = {SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
            SDL_RenderSetViewport(renderer, &top_right_vp);

            SDL_RenderCopy(renderer, texture, NULL, NULL);

            // x, y, w, h
            SDL_Rect bottom_vp = {0, SCREEN_WIDTH / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
            SDL_RenderSetViewport(renderer, &bottom_vp);

            SDL_RenderCopy(renderer, texture, NULL, NULL);

            // update display
            SDL_RenderPresent(renderer);

            // update display
            SDL_RenderPresent(renderer);
        }
        else if (toggle_color_key_scene) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            background_texture.render(0, 0);
            man_texture.render(240, 190);

            SDL_RenderPresent(renderer);
        }
        else if (toggle_sprite_sheet) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            // render top left
            sprite_sheet_texture.render(0, 0, &sprite_clips[0]);

            // render top right
            sprite_sheet_texture.render(SCREEN_WIDTH - sprite_clips[1].w, 0, &sprite_clips[1]);
            
            // render bottom left
            sprite_sheet_texture.render(0, SCREEN_HEIGHT - sprite_clips[2].h, &sprite_clips[2]);
            
            // render bottom right
            sprite_sheet_texture.render(SCREEN_WIDTH - sprite_clips[3].w, SCREEN_HEIGHT - sprite_clips[3].h, &sprite_clips[3]);
            
            SDL_RenderPresent(renderer);
        }
        else if (toggle_color_modulation) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            if (r != 0) {
                r --;
            }

            color_mod_texture.set_color(r, g, b);
            color_mod_texture.render(0, 0);

            SDL_RenderPresent(renderer);
        }
        else if (toggle_alpha_modulation) {
            if(a != 0) {
                a--;
            }
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            fade_in_texture.render(0, 0);

            // render front blended
            fade_out_texture.set_alpha(a);
            fade_out_texture.render(0, 0);

            SDL_RenderPresent(renderer);
        }
        else {
            // apply the image stretched
            SDL_Rect stretch_rect;
            stretch_rect.x = 0;
            stretch_rect.y = 0;
            stretch_rect.w = SCREEN_WIDTH;
            stretch_rect.h = SCREEN_HEIGHT;
            // fits image to screen
            //             surface to draw        destination     rect to scale to/by
            SDL_BlitScaled(current_surface, NULL, screen_surface, &stretch_rect);

            SDL_UpdateWindowSurface(window);
        }
    }

    close();
}

