#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <string>

const int WIDTH = 800;
const int HEIGHT = 600;
const float GRAVITY = 0.8f;
const float JUMP_FORCE = -16.0f;
const float PLAYER_SPEED = 6.0f;

struct Player { float x, y, w, h, velocityY; bool onGround; };
struct Platform { int x, y, w, h; };
struct Enemy { float x, y, w, h, speed; int direction; float minX, maxX; };
struct Coin { int x, y, w, h; bool collected; };

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    SDL_Window* window = SDL_CreateWindow("VOID ENGINE - SCORE: 0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // --- SETUP ---
    Player player = { 50, 450, 40, 40, 0, false };
    int score = 0;
    bool gameWon = false;

    // Platformlar
    std::vector<Platform> platforms = {
        { 0, 550, 800, 50 },    // Zemin
        { 200, 400, 400, 20 },  // Orta
        { 50, 250, 100, 20 },   // Sol Ust
        { 650, 250, 100, 20 },  // Sag Ust
        { 350, 150, 100, 20 }   // En Tepe
    };

    // Dusmanlar
    std::vector<Enemy> enemies = {
        { 300, 360, 40, 40, 3.0f, 1, 200, 560 }, // Orta kat
        { 500, 510, 40, 40, 5.0f, -1, 400, 750 } // Zemin
    };

    // Altinlar
    std::vector<Coin> coins = {
        { 70, 210, 20, 20, false },
        { 670, 210, 20, 20, false },
        { 390, 110, 20, 20, false },
        { 400, 510, 20, 20, false }
    };
    int totalCoins = coins.size();

    bool isRunning = true;
    SDL_Event event;

    // --- GAME LOOP ---
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && player.onGround) {
                player.velocityY = JUMP_FORCE;
                player.onGround = false;
            }
        }

        if (!gameWon) {
            // Hareket
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_LEFT]) player.x -= PLAYER_SPEED;
            if (keys[SDL_SCANCODE_RIGHT]) player.x += PLAYER_SPEED;

            // Fizik
            player.velocityY += GRAVITY;
            player.y += player.velocityY;
            player.onGround = false;

            // Oyuncu Karesini Hesapla (Carpismalar icin)
            SDL_Rect playerRect = { (int)player.x, (int)player.y, (int)player.w, (int)player.h };

            // 1. Platform Carpismasi
            for (auto& plat : platforms) {
                SDL_Rect platRect = { plat.x, plat.y, plat.w, plat.h };
                if (SDL_HasIntersection(&playerRect, &platRect)) {
                    if (player.velocityY > 0 && player.y + player.h - 10 < plat.y + plat.h) {
                        player.y = plat.y - player.h;
                        player.velocityY = 0;
                        player.onGround = true;
                        // Konum degistigi icin playerRect guncelle
                        playerRect.y = (int)player.y; 
                    }
                }
            }

            // 2. Dusman Carpismasi
            for (auto& enemy : enemies) {
                enemy.x += enemy.speed * enemy.direction;
                if (enemy.x > enemy.maxX || enemy.x < enemy.minX) enemy.direction *= -1;

                SDL_Rect enemyRect = { (int)enemy.x, (int)enemy.y, (int)enemy.w, (int)enemy.h };
                if (SDL_HasIntersection(&playerRect, &enemyRect)) {
                    // OLUM
                    player.x = 50; player.y = 450; player.velocityY = 0;
                    score = 0;
                    for(auto& c : coins) c.collected = false;
                    SDL_SetWindowTitle(window, "VOID ENGINE - OLDUN!");
                }
            }

            // 3. Altin Toplama
            for (auto& coin : coins) {
                if (!coin.collected) {
                    SDL_Rect coinRect = { coin.x, coin.y, coin.w, coin.h };
                    if (SDL_HasIntersection(&playerRect, &coinRect)) {
                        coin.collected = true;
                        score++;
                        std::string title = "VOID ENGINE - SCORE: " + std::to_string(score) + " / " + std::to_string(totalCoins);
                        SDL_SetWindowTitle(window, title.c_str());

                        if (score >= totalCoins) {
                            gameWon = true;
                            SDL_SetWindowTitle(window, "VOID ENGINE - KAZANDIN! (Muhendis Onayli)");
                        }
                    }
                }
            }
        }

        // --- RENDER (CIZIM) ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        if (gameWon) {
            SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // YESIL EKRAN
            SDL_RenderClear(renderer);
        } else {
            // Platformlar
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            for (auto& plat : platforms) { SDL_Rect r = { plat.x, plat.y, plat.w, plat.h }; SDL_RenderFillRect(renderer, &r); }

            // Dusmanlar
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            for (auto& enemy : enemies) { SDL_Rect r = { (int)enemy.x, (int)enemy.y, (int)enemy.w, (int)enemy.h }; SDL_RenderFillRect(renderer, &r); }

            // Altinlar
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            for (auto& coin : coins) { 
                if(!coin.collected) { SDL_Rect r = { coin.x, coin.y, coin.w, coin.h }; SDL_RenderFillRect(renderer, &r); }
            }

            // Oyuncu
            SDL_SetRenderDrawColor(renderer, 50, 150, 255, 255);
            // DÜZELTME BURADA: Cizmeden once oyuncu karesini tekrar tanimladik
            SDL_Rect drawRect = { (int)player.x, (int)player.y, (int)player.w, (int)player.h }; 
            SDL_RenderFillRect(renderer, &drawRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}