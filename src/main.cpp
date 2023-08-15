#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

// on loadFromRenderedText method impl

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

    // loads from font string
    bool load_from_rendered_text(const char* texture_text, SDL_Color text_color);

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

void print_sdl_ttf_error(const char* msg) {
    std::cout << msg << TTF_GetError() << std::endl;
}

// gloabal variables
SDL_Window *window = NULL;

SDL_Renderer* renderer = NULL;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

TTF_Font* font = NULL;

MyTexture text_texture;

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

bool MyTexture::load_from_rendered_text(const char* text, SDL_Color text_color) {
    this->free();

    // render text surface
    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, text_color);
    if (text_surface == NULL) {
        print_sdl_ttf_error("unable to render text as surface!");
        return false;
    }

    // create texture from surface pixels
    this->texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (this->texture == NULL) {
        print_sdl_error("unable to create texture from surface");
        return false;
    }

    this->width = text_surface->w;
    this->height = text_surface->h;

    SDL_FreeSurface(text_surface);

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
    // render_quad.w = SCREEN_WIDTH;
    // render_quad.h = SCREEN_HEIGHT;

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

  return true;
}

bool load_media(){
    if (!arrow.load_from_file("images/arrow.png")) {
        return false;
    }

    return true;
}

void close(){
    SDL_DestroyWindow(window);
    window = NULL;

    // destroy renderer
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    
    // quit
    IMG_Quit();
    SDL_Quit();
}

// you are responsible for this pointer!
SDL_Surface* load_surface(const char* path){
    SDL_Surface* loaded_surface = IMG_Load( path );
    if (loaded_surface == NULL) {
        std::string err_msg = "could not load image with path: ";
        err_msg += path;
        print_sdl_image_error(err_msg.data());
    }

    return loaded_surface;
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

    double degrees = 0.0;
    SDL_RendererFlip flip_type = SDL_FLIP_NONE;
    while(!done) {
        while( SDL_PollEvent( &e ) != 0) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_x:
                        degrees -= 20;
                        break;

                    case SDLK_c:
                        degrees += 20;
                        break;

                    case SDLK_h:
                        flip_type = SDL_FLIP_HORIZONTAL;
                        break;

                    case SDLK_n:
                        flip_type = SDL_FLIP_NONE;
                        break;

                    case SDLK_v:
                        flip_type = SDL_FLIP_VERTICAL;
                        break;
                }
            }
        }

        // render arrow
        arrow.render(( SCREEN_WIDTH - arrow.get_width() ) / 2, ( SCREEN_HEIGHT - arrow.get_height() ) / 2, NULL, degrees, NULL, flip_type);

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    }
    close();
}

