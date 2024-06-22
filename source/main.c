
#define PC 0 // 1 if building for PC, 0 if for switch

#include <unistd.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#if PC != 1
#include <switch.h>
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define WORLD_WIDTH 32
#define WORLD_HEIGHT 32

#define PLAYER_SIZE 32
#define TILE_SIZE 32

#define JOY_A 0
#define JOY_B 1
#define JOY_X 2
#define JOY_Y 3
#define JOY_PLUS 10
#define JOY_MINUS 11
#define JOY_LEFT 12
#define JOY_UP 13
#define JOY_RIGHT 14
#define JOY_DOWN 15

#define COIN 1
#define SEEDVILLAGER 2
#define FARMLAND 3
#define FRUITVILLAGER 4
#define HOEVILLAGER 5 // a hoe like in the Minecraft game not like in GTA
#define WATER 6

#define SAVE 100
#define LOAD 99

void notice(const char *msg, char *text, int *current_time)
{
    // Copy the text into msg
    snprintf(text, 128, "%s", msg);

    // Get the current Unix timestamp
    *current_time = (int)time(NULL);
}

void setPixel(SDL_Renderer *renderer, int x, int y)
{
    SDL_RenderDrawPoint(renderer, x, y);
}

void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y)
    {
        for (int i = centerX - x; i <= centerX + x; i++)
        {
            setPixel(renderer, i, centerY + y);
            setPixel(renderer, i, centerY - y);
        }
        for (int i = centerX - y; i <= centerX + y; i++)
        {
            setPixel(renderer, i, centerY + x);
            setPixel(renderer, i, centerY - x);
        }
        y++;

        if (radiusError < 0)
        {
            radiusError += 2 * y + 1;
        }
        else
        {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

int main(int argc, char *argv[])
{
#if PC != 1
    romfsInit();
    chdir("romfs:/");
#endif

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Delay(5000);
        SDL_Quit();
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("SDL Rectangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Delay(5000);
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Delay(5000);
        SDL_Quit();
        return 1;
    }

    // Define the seeds variable and initial value
    int money = 10;
    int seeds = 0;
    int hoes = 0;
    int fruits = 0;

    int move_speed = 3;
    int xOffset = 0;
    int yOffset = 0;
    int mapelements[WORLD_WIDTH][WORLD_HEIGHT] = {0};
    int treeage[WORLD_WIDTH][WORLD_HEIGHT] = {0};

    char notice_text[128] = "Welcome to Mapster by AzizBgBoss! Check other versions of Mapster on my GitHub!";
    int notice_time = (int)time(NULL);

    int latest_tick = (int)time(NULL);

    char *tip = "";

    mapelements[0][0] = COIN;
    mapelements[4][1] = COIN;
    mapelements[20][30] = COIN;
    mapelements[5][18] = COIN;
    mapelements[10][17] = SAVE;
    mapelements[11][17] = LOAD;
    mapelements[15][17] = SEEDVILLAGER;
    mapelements[16][17] = HOEVILLAGER;
    mapelements[17][17] = FRUITVILLAGER;

    // Define the top rectangle
    SDL_Rect topRect;
    topRect.x = 16;
    topRect.y = 16;
    topRect.w = SCREEN_WIDTH - 32;
    topRect.h = 35;
    SDL_Rect bottomRect;
    bottomRect.x = 16;
    bottomRect.y = 640;
    bottomRect.w = SCREEN_WIDTH - 32;
    bottomRect.h = 70;

    SDL_InitSubSystem(SDL_INIT_AUDIO);
    Mix_AllocateChannels(5);
    Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);

    Mix_Music *music = Mix_LoadMUS("data/background.ogg");
    Mix_Chunk *sound[2] = {Mix_LoadWAV("data/pop.wav"), Mix_LoadWAV("data/money.wav")};

    if (music)
        Mix_PlayMusic(music, -1);
    // Load a font
    TTF_Font *font = TTF_OpenFont("data/arial.ttf", 24); // Use default font and set size
    if (font == NULL)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Delay(5000);
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    int quit = 0;

    // Event handler
    SDL_Event e;

#if PC != 1
    // Initialize joystick
    SDL_Joystick *joystick = NULL;
    if (SDL_NumJoysticks() > 0)
    {
        joystick = SDL_JoystickOpen(0);
        if (joystick == NULL)
        {
            printf("Failed to open joystick! SDL_Error: %s\n", SDL_GetError());
        }
    }
    else
    {
        printf("No joysticks connected!\n");
    }
#endif

    // Main application loop
    while (!quit)
    {
        // Handle events on the queue
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
#if PC == 1
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_PLUS:
                    quit = 1;
                    break;
                case SDLK_a: // A key for the same action as JOY_A
                {
                    int x = -(xOffset - (TILE_SIZE * WORLD_WIDTH / 2)) / TILE_SIZE;
                    int y = -(yOffset - (TILE_SIZE * WORLD_HEIGHT / 2)) / TILE_SIZE;
                    switch (mapelements[x][y])
                    {
                    case SEEDVILLAGER:
                    {
                        if (money >= 3)
                        {
                            money -= 3;
                            seeds++;
                        }
                        else
                            notice("You got insufficent funds!", notice_text, &notice_time);
                        break;
                    }
                    case HOEVILLAGER:
                    {
                        if (money >= 5)
                        {
                            money -= 5;
                            hoes++;
                        }
                        else
                            notice("You got insufficent funds!", notice_text, &notice_time);
                        break;
                    }
                    case FRUITVILLAGER:
                    {
                        if (fruits)
                        {
                            fruits--;
                            money += 4;
                        }
                        else
                            notice("You got no fruits!", notice_text, &notice_time);
                        break;
                    }
                    case FARMLAND:
                    {
                        if (!treeage[x][y])
                        {
                            if (seeds)
                            {
                                treeage[x][y] = 1;
                                seeds--;
                            }
                            else
                                notice("You got no seeds!", notice_text, &notice_time);
                        }
                        else if (treeage[x][y] == 3)
                        {
                            treeage[x][y] = 0;
                            fruits += (rand() % 100 < 66) ? 1 : 2;
                        }
                        else
                            notice("You must wait until the tree grows!", notice_text, &notice_time);
                        break;
                    }
                    case 0:
                    {
                        if (hoes)
                        {
                            mapelements[x][y] = FARMLAND;
                            hoes--;
                        }
                        else
                            notice("You got no hoes! (no offense)", notice_text, &notice_time);
                        break;
                    }
                    }
                    break;
                }
                case SDLK_UP:
                    yOffset += move_speed;
                    break;
                case SDLK_DOWN:
                    yOffset -= move_speed;
                    break;
                case SDLK_LEFT:
                    xOffset += move_speed;
                    break;
                case SDLK_RIGHT:
                    xOffset -= move_speed;
                    break;
                }
            }
#else
            // this is the code for joystick control
            else if (e.type == SDL_JOYBUTTONDOWN)
            {
                // Check for the + button press
                if (e.jbutton.button == JOY_PLUS)
                {
                    quit = 1;
                }
                if (e.jbutton.button == JOY_A)
                {
                    int x = -(xOffset - (TILE_SIZE * WORLD_WIDTH / 2)) / TILE_SIZE;
                    int y = -(yOffset - (TILE_SIZE * WORLD_HEIGHT / 2)) / TILE_SIZE;
                    switch (mapelements[x][y])
                    {
                    case SAVE:
                    {
                        FILE *fp = fopen("sdmc:/switch/mapster.bin", "wb");
                        if (fp == NULL)
                        {
                            notice("Error while creating save file (mapster.bin)!", notice_text, &notice_time);
                            break;
                        }

                        // Write basic variables
                        fwrite(&money, sizeof(int), 1, fp);
                        fwrite(&seeds, sizeof(int), 1, fp);
                        fwrite(&hoes, sizeof(int), 1, fp);
                        fwrite(&fruits, sizeof(int), 1, fp);
                        fwrite(&latest_tick, sizeof(int), 1, fp);

                        // Write 2D arrays
                        fwrite(mapelements, sizeof(int), WORLD_WIDTH * WORLD_HEIGHT, fp);
                        fwrite(treeage, sizeof(int), WORLD_WIDTH * WORLD_HEIGHT, fp);

                        fclose(fp);
                        notice("File saved!", notice_text, &notice_time);
                        break;
                    }
                    case LOAD:
                    {
                        FILE *fp = fopen("sdmc:/switch/mapster.bin", "rb");
                        if (fp == NULL)
                        {
                            notice("There is no save file!", notice_text, &notice_time);
                            break;
                        }

                        // Read basic variables
                        fread(&money, sizeof(int), 1, fp);
                        fread(&seeds, sizeof(int), 1, fp);
                        fread(&hoes, sizeof(int), 1, fp);
                        fread(&fruits, sizeof(int), 1, fp);
                        fread(&latest_tick, sizeof(int), 1, fp);

                        // Read 2D arrays
                        fread(mapelements, sizeof(int), WORLD_WIDTH * WORLD_HEIGHT, fp);
                        fread(treeage, sizeof(int), WORLD_WIDTH * WORLD_HEIGHT, fp);

                        fclose(fp);
                        notice("File loaded!", notice_text, &notice_time);
                        break;
                    }
                    case SEEDVILLAGER:
                    {
                        if (money >= 3)
                        {
                            money -= 3;
                            seeds++;
                            if (sound[1])
                                Mix_PlayChannel(-1, sound[1], 0);
                        }
                        else
                            notice("You got insufficent funds!", notice_text, &notice_time);
                        break;
                    }
                    case HOEVILLAGER:
                    {
                        if (money >= 5)
                        {
                            money -= 5;
                            hoes++;
                            if (sound[1])
                                Mix_PlayChannel(-1, sound[1], 0);
                        }
                        else
                            notice("You got insufficent funds!", notice_text, &notice_time);
                        break;
                    }
                    case FRUITVILLAGER:
                    {
                        if (fruits)
                        {
                            fruits--;
                            money += 4;
                            if (sound[1])
                                Mix_PlayChannel(-1, sound[1], 0);
                        }
                        else
                            notice("You got no fruits!", notice_text, &notice_time);
                        break;
                    }
                    case FARMLAND:
                    {
                        if (!treeage[x][y])
                        {
                            if (seeds)
                            {
                                treeage[x][y] = 1;
                                seeds--;
                            }
                            else
                                notice("You got no seeds!", notice_text, &notice_time);
                        }
                        else if (treeage[x][y] == 3)
                        {
                            treeage[x][y] = 0;
                            fruits += (rand() % 100 < 66) ? 1 : 2;
                        }
                        else
                            notice("You must wait until the tree grows!", notice_text, &notice_time);
                        break;
                    }
                    case 0:
                    {
                        if (hoes)
                        {
                            mapelements[x][y] = FARMLAND;
                            hoes--;
                        }
                        else
                            notice("You got no hoes! (no offense)", notice_text, &notice_time);
                        break;
                    }
                    }
                }
            }
#endif
        }

#if PC != 1
        // Handle joystick axis movement
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_UP] || (SDL_JoystickGetButton(joystick, JOY_UP)))
        {
            yOffset += move_speed;
        }
        if (state[SDL_SCANCODE_DOWN] || (SDL_JoystickGetButton(joystick, JOY_DOWN)))
        {
            yOffset -= move_speed;
        }
        if (state[SDL_SCANCODE_LEFT] || (SDL_JoystickGetButton(joystick, JOY_LEFT)))
        {
            xOffset += move_speed;
        }
        if (state[SDL_SCANCODE_RIGHT] || (SDL_JoystickGetButton(joystick, JOY_RIGHT)))
        {
            xOffset -= move_speed;
        }
#endif

        int x = -(xOffset - (TILE_SIZE * WORLD_WIDTH / 2)) / TILE_SIZE;
        int y = -(yOffset - (TILE_SIZE * WORLD_HEIGHT / 2)) / TILE_SIZE;
        switch (mapelements[x][y])
        {
        case COIN:
        {
            mapelements[x][y] = 0;
            money++;
            break;
        }
        case SAVE:
        {
            tip = "Save progress";
            break;
        }
        case LOAD:
        {
            tip = "Load previous progress";
            break;
        }
        case SEEDVILLAGER:
        {
            tip = "1 seed for 3 coins";
            break;
        }
        case HOEVILLAGER:
        {
            tip = "1 hoe for 5 coins";
            break;
        }
        case FRUITVILLAGER:
        {
            tip = "4 coins for 1 fruit";
            break;
        }
        default:
        {
            tip = "";
            break;
        }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Set the drawing color (red, green, blue, alpha)
        SDL_SetRenderDrawColor(renderer, 0x00, 0x99, 0x00, 0xFF);

        // Draw the rectangle
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            for (int x = 0; x < WORLD_WIDTH; x++)
            {
                SDL_Rect rect;
                rect.x = 128 + x * TILE_SIZE + xOffset;
                rect.y = -152 + y * TILE_SIZE + yOffset;
                rect.w = TILE_SIZE;
                rect.h = TILE_SIZE;
                SDL_SetRenderDrawColor(renderer, 0x00, 0x99, 0x00, 0xFF);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 0x00, 0x55, 0x00, 0xFF);
                SDL_RenderDrawRect(renderer, &rect);
                if (mapelements[x][y])
                {
                    // Create a buffer large enough to hold the final string
                    char filename[128]; // Adjust size as needed

                    // Format the string with the integer
                    sprintf(filename, "%s%d%s", "data/sprite", mapelements[x][y], ".png");

                    SDL_Surface *sprite = IMG_Load(filename);
                    if (sprite)
                    {
                        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, sprite);
                        SDL_FreeSurface(sprite); // Free the surface to avoid memory leak

                        if (texture)
                        {
                            SDL_Rect rect;
                            rect.x = 128 + x * TILE_SIZE + xOffset;
                            rect.y = -152 + y * TILE_SIZE + yOffset;
                            rect.w = sprite->w;
                            rect.h = sprite->h;

                            // Render the texture (assuming you have a renderer and a proper rendering loop)
                            SDL_RenderCopy(renderer, texture, NULL, &rect);

                            // Free the texture when it's no longer needed
                            SDL_DestroyTexture(texture);
                        }
                    }
                    if (mapelements[x][y] == FARMLAND && treeage[x][y])
                    {
                        // Create a buffer large enough to hold the final string
                        char filename[128]; // Adjust size as needed

                        // Format the string with the integer
                        sprintf(filename, "%s%d%s", "data/tree", treeage[x][y], ".png");

                        SDL_Surface *sprite = IMG_Load(filename);
                        if (sprite)
                        {
                            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, sprite);
                            SDL_FreeSurface(sprite); // Free the surface to avoid memory leak

                            if (texture)
                            {
                                SDL_Rect rect;
                                rect.x = 128 + x * TILE_SIZE + xOffset;
                                rect.y = -152 + y * TILE_SIZE + yOffset;
                                rect.w = sprite->w;
                                rect.h = sprite->h;

                                // Render the texture (assuming you have a renderer and a proper rendering loop)
                                SDL_RenderCopy(renderer, texture, NULL, &rect);

                                // Free the texture when it's no longer needed
                                SDL_DestroyTexture(texture);
                            }
                        }
                    }
                }
            }
        }

        if (xOffset > (TILE_SIZE * WORLD_WIDTH / 2) - PLAYER_SIZE / 2)
            xOffset = (TILE_SIZE * WORLD_WIDTH / 2) - PLAYER_SIZE / 2;
        else if (xOffset < -(TILE_SIZE * WORLD_WIDTH / 2) + PLAYER_SIZE / 2)
            xOffset = -(TILE_SIZE * WORLD_WIDTH / 2) + PLAYER_SIZE / 2;

        if (yOffset > (TILE_SIZE * WORLD_HEIGHT / 2) - PLAYER_SIZE / 2)
            yOffset = (TILE_SIZE * WORLD_HEIGHT / 2) - PLAYER_SIZE / 2;
        else if (yOffset < -(TILE_SIZE * WORLD_HEIGHT / 2) + PLAYER_SIZE / 2)
            yOffset = -(TILE_SIZE * WORLD_HEIGHT / 2) + PLAYER_SIZE / 2;

        SDL_SetRenderDrawColor(renderer, 0x99, 0, 0, 255); // Black color for the circle
        drawCircle(renderer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, PLAYER_SIZE / 2);

        // Draw the top rectangle with white border
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &topRect);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDrawRect(renderer, &topRect);

        // Render the seeds text
        char seedText[128];
        sprintf(seedText, "Money: %d dollars | Seeds: %d | Hoes: %d | Fruits: %d", money, seeds, hoes, fruits);
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, seedText, (SDL_Color){255, 255, 255, 255});
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        // Get the text width and height for proper centering
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_Rect textRect = {(topRect.w + (topRect.x * 2) - textWidth) / 2, (topRect.h + (topRect.y * 2) - textHeight) / 2, textWidth, textHeight};

        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);

        if ((int)time(NULL) - notice_time <= 5)
        {
            // Draw the top rectangle with white border
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &bottomRect);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderDrawRect(renderer, &bottomRect);

            // Render the notice text
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, notice_text, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            // Get the text width and height for proper centering
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;
            SDL_Rect textRect = {(bottomRect.w + (bottomRect.x * 2) - textWidth) / 2, (bottomRect.h + (bottomRect.y * 2) - textHeight) / 2, textWidth, textHeight};

            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }

        if (strlen(tip))
        {
            // Render the notice text
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, tip, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            // Get the text width and height for proper centering
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;

            SDL_Rect tipRect;
            tipRect.x = 635 - (textWidth / 2);
            tipRect.y = 295 - (textHeight / 2);
            tipRect.w = textWidth + 10;
            tipRect.h = textHeight + 10;

            SDL_Rect textRect = {(tipRect.w + (tipRect.x * 2) - textWidth) / 2, (tipRect.h + (tipRect.y * 2) - textHeight) / 2, textWidth, textHeight};

            // Draw the top rectangle with white border
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xAA);
            SDL_RenderFillRect(renderer, &tipRect);

            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }

        while ((int)time(NULL) > latest_tick)
        {
            for (int y = 0; y < WORLD_HEIGHT; y++)
            {
                for (int x = 0; x < WORLD_WIDTH; x++)
                {
                    if (treeage[x][y] > 0 && treeage[x][y] < 3)
                    {
                        if (rand() % 1000 < 16)
                        {
                            treeage[x][y]++;
                            if (sound[0])
                                Mix_PlayChannel(-1, sound[0], 0);
                        }
                    }
                }
            }
            latest_tick++;
        }

        // Update the screen
        SDL_RenderPresent(renderer);
    }

#if PC != 1
    // Clean up joystick
    if (joystick != NULL)
    {
        SDL_JoystickClose(joystick);
    }
#endif

    // Destroy the font
    TTF_CloseFont(font);

    // Destroy the window and renderer
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Quit SDL_ttf and SDL subsystems
    printf("Quitting, goodbye!");
    TTF_Quit();
    SDL_Quit();

    return 0;
}