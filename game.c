#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <stdbool.h>

#define WAVPATH "lilypond.wav"
#define FONTPATH "unifont-15.1.05.otf"

#define DEFAULT_PTSIZE 19
#define DEFAULT_TEXT_1 "Pack my box with five dozen liquor jugs."
#define DEFAULT_TEXT "hello world"
#define DELAY 2
#define TITLE_DELAY 1000
#define WINDOW_TITLE "Game"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 80

static int audio_open = 0;
static Mix_Chunk *wave = NULL;

int main(int argc, char *argv[]) {
  int audio_rate;
  Uint16 audio_format;
  int audio_channels;

  SDL_Surface *text = NULL;
  SDL_Texture *text_texture;
  SDL_Color white = {0xFF, 0xFF, 0xFF, 0};
  SDL_Color black = {0x00, 0x00, 0x00, 0};
  SDL_Rect rect = {.x = 0, .y = 20, .w = WINDOW_WIDTH, .h = 100};

  /* Initialize variables */
  audio_rate = MIX_DEFAULT_FREQUENCY;
  audio_format = MIX_DEFAULT_FORMAT;
  audio_channels = MIX_DEFAULT_CHANNELS;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL initialization failed: %s\n",
                 SDL_GetError());
    return 1;
  }

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s\n",
                 SDL_GetError());
    return (255);
  }

  /* Open the audio device */
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't open audio: %s\n",
                 SDL_GetError());
    return (2);
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  }
  audio_open = 1;

  /* Load the requested wave file */
  wave = Mix_LoadWAV(WAVPATH);
  if (wave == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't load %s: %s\n", WAVPATH,
                 SDL_GetError());
    return (2);
  }

  /* Initialize the TTF library */
  if (TTF_Init() < 0) {
    SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
    SDL_Quit();
    return (2);
  }

  /* Open the font file with the requested point size */
  TTF_Font *font = TTF_OpenFont(FONTPATH, 80);
  if (font == NULL) {
    SDL_Log("Couldn't load %d pt font from %s: %s\n", 80, FONTPATH,
            SDL_GetError());
    return (2);
  }

  // Create a window
  SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Window creation failed: %s\n",
                 SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // Create a renderer
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Renderer creation failed: %s\n",
                 SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  text = TTF_RenderUTF8_Solid(font, DEFAULT_TEXT, white);
  text_texture = SDL_CreateTextureFromSurface(renderer, text);

  /* Initialize the IMG library */
  int flags = IMG_INIT_JPG|IMG_INIT_PNG;
  int initted = IMG_Init(flags);
  if(initted != flags) {
    printf("IMG_Init: Failed to init required jpg and png support!\n");
    printf("IMG_Init: %s\n", IMG_GetError());
    return 1;
  }

  // load image
  SDL_Surface *image = IMG_Load("outerpasta.svg");
  if (!image) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "IMG_Load: %s\n", IMG_GetError());
    return 1;
  }
  SDL_Texture *outerpasta_image = SDL_CreateTextureFromSurface(renderer, image);
  if (!outerpasta_image) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create texture: %s\n", SDL_GetError());
  }

  SDL_Rect dstrect;
  dstrect.x = 70;
  SDL_QueryTexture(outerpasta_image, NULL, NULL, &dstrect.w, &dstrect.h);
  SDL_RenderCopy(renderer, outerpasta_image, NULL, &dstrect);
  SDL_RenderPresent(renderer);
  SDL_Delay(TITLE_DELAY);

  // Wait for a quit event
  bool quit = false;
  SDL_Event e;

  float player_x_float = WINDOW_WIDTH / 2.0;
  float player_y_float = 10.0;
  bool player_jump = false;
  float player_y_vel = 0.0;
  float player_x_vel = 1.0;
  float player_y_vel_delta = 0.01;

  while (!quit) {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
        if (strcmp(SDL_GetKeyName(e.key.keysym.sym), "Escape") == 0) {
          quit = true;
        } else if (strcmp(SDL_GetKeyName(e.key.keysym.sym), "Left") == 0) {
          player_x_vel = -1.0;
        } else if (strcmp(SDL_GetKeyName(e.key.keysym.sym), "Right") == 0) {
          player_x_vel = 1.0;
        } else {
          player_x_vel = 0.0;
        }
        break;
      default:
        break;
      }
    }

    if ((int)player_x_float < 0) {
      player_x_vel = 1.0;
    }

    if ((int)player_x_float + PLAYER_WIDTH > WINDOW_WIDTH) {
      player_x_vel = -1.0;
    }

    if ((int)player_y_float < WINDOW_HEIGHT - PLAYER_HEIGHT) {
      player_y_float += 0.01; // Gravity
    } else {
      player_jump = true;
    }

    if (player_jump) {
      player_jump = false;
      player_y_vel = -2.5;

      /* Play and then exit */
      Mix_PlayChannel(0, wave, 0);
    }

    player_y_vel += player_y_vel_delta;
    player_y_float += player_y_vel;

    player_x_float += player_x_vel;

    rectangleRGBA(renderer, (int)player_x_float, (int)player_y_float,
                  (int)player_x_float + PLAYER_HEIGHT,
                  (int)player_y_float + PLAYER_HEIGHT, 255, 0, 255, 255);

    // Update the renderer every frame
    SDL_Delay(DELAY);

    SDL_RenderCopy(renderer, text_texture, NULL, &rect);

    SDL_RenderPresent(renderer);
  }

  // Clean up
  SDL_FreeSurface(text);
  TTF_CloseFont(font);
  SDL_DestroyTexture(text_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  if (wave) {
    Mix_FreeChunk(wave);
    wave = NULL;
  }
  if (audio_open) {
    Mix_CloseAudio();
    audio_open = 0;
  }
  SDL_Quit();

  exit(0);

  return 0;
}
