#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Entity.h"
#include "Map.h"
#include "Util.h"
#include "Scene.h"
#include "Home.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "HowToPlay.h"
#include "YouWin.h"
#include "YouLose.h"

/*
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5
unsigned int level1_data[] =
{
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    31, 31, 12, 12, 12, 12, 12, 12, 12, 31, 31, 31, 31, 31,
    32, 32, 31, 31, 12, 12, 31, 31, 31, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 12, 12, 32, 32, 32, 32, 32, 32, 32, 32
};
*/


Scene* currentScene;
Scene* sceneList[7];

void SwitchToScene(Scene* scene) {
    currentScene = scene;
    currentScene->Initialize();
}



SDL_Window* displayWindow;
bool gameIsRunning = true;


ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;


Mix_Chunk* bruh;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("P5!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Initialize Game Objects

    // Initialize Player
    /*
    GLuint mapTextureID = Util::LoadTexture("platformPack_tilesheet.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 14, 7);
    */
    sceneList[0] = new Home();
    sceneList[1] = new Level1();
    sceneList[2] = new Level2();
    sceneList[3] = new Level3();
    sceneList[4] = new YouLose();
    sceneList[5] = new YouWin();
    sceneList[6] = new HowToPlay();
    SwitchToScene(sceneList[0]);
   
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music* music;
    music = Mix_LoadMUS("tunak.mp3"); //I'm sorry, its the only mp3 I had handy. Although if I had my way, I'd play Never GOnna Give You Up
    //Mix_PlayMusic(music, -1);

    bruh = Mix_LoadWAV("bounce.wav"); //Aight this also lying around my desktop
}


void ProcessInput() {
    bool controls = false;
    if ((currentScene == sceneList[1] || currentScene == sceneList[2]) || currentScene == sceneList[3])
    {
        controls = true;
    }
    currentScene->state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                break;

            case SDLK_SPACE:
                // Some sort of action
                if (controls)
                {
                    if (currentScene->state.player->collidedBottom)
                    {
                        currentScene->state.player->jump = true;
                        Mix_PlayChannel(-1, bruh, 0);
                        Mix_Volume(-1, MIX_MAX_VOLUME);
                    }
                    break;
                }

                else
                {
                    currentScene->state.nextScene = 0;
                    currentScene->state.lives = 3;
                }

            case SDLK_RETURN:
                if (currentScene == sceneList[0])
                {
                    currentScene->state.nextScene = 1;
                }
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (controls)
    {
        if (keys[SDL_SCANCODE_LEFT]) {
            currentScene->state.player->movement.x = -1.0f;
            currentScene->state.player->animIndices = currentScene->state.player->animLeft;
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            currentScene->state.player->movement.x = 1.0f;
            currentScene->state.player->animIndices = currentScene->state.player->animRight;
        }
        if (glm::length(currentScene->state.player->movement) > 1.0f) {
            currentScene->state.player->movement = glm::normalize(currentScene->state.player->movement);
        }
    }
}


#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        currentScene->Update(FIXED_TIMESTEP);

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

    viewMatrix = glm::mat4(1.0f);
    if (currentScene->state.player->isActive)
    {
        if (currentScene->state.player->position.x > 5 || currentScene->state.player->position.y < 0) {
            viewMatrix = glm::translate(viewMatrix,
                glm::vec3(-(currentScene->state.player->position.x), -(currentScene->state.player->position.y), 0));
        }
        else {
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 0, 0));
        }

        if (currentScene->state.player->position.y < -8.0f)
        {
            currentScene->state.lives -= 1;
            if (currentScene->state.lives <= 0)
            {
                currentScene->state.nextScene = 4;
            }
            else
            {
                currentScene->state.player->position = glm::vec3(2.0f, 0.0f, 0.0f);
            }
        }

        if (currentScene == sceneList[2] || currentScene == sceneList[3])
        {
            if (currentScene->state.player->CheckCollision(currentScene->state.enemies))
            {
                currentScene->state.lives -= 1;
                if (currentScene->state.lives <= 0)
                {
                    currentScene->state.nextScene = 4;
                }
                else
                {
                    currentScene->state.player->position = glm::vec3(2.0f, 0.0f, 0.0f);
                }
            }
        }
    }    

}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetViewMatrix(viewMatrix);

    currentScene->Render(&program);

    currentScene->state.player->Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    int lives = 3;

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();

        lives = currentScene->state.lives;
        if (currentScene->state.nextScene > -1)
        {
            lives = currentScene->state.lives;
            SwitchToScene(sceneList[currentScene->state.nextScene]);
            currentScene->Initialize();
            currentScene->state.lives = lives;
        }
}

    Shutdown();
    return 0;
}