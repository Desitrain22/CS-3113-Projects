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


Mix_Chunk* bounce;
Mix_Chunk* success;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("P6: Claw Machine Adventure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
    music = Mix_LoadMUS("kahootaudio.mp3"); 
    Mix_PlayMusic(music, -1);

    bounce = Mix_LoadWAV("boing_x.wav");
    success = Mix_LoadWAV("chime.wav");
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
                break;

            case SDLK_RIGHT:
                break;

            case SDLK_SPACE:
                for (int i = 0; i < 5; i++)
                {
                    if (currentScene->state.objects[i].token);
                    {
                        currentScene->state.objects[i].token = false;
                        currentScene->state.player->token = false;
                    }
                }
            case SDLK_RETURN:
                if (currentScene == sceneList[0])
                {
                    currentScene->state.nextScene = 1;
                }
            }
            break;
        }
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (controls)
	{
		if (keys[SDL_SCANCODE_LEFT]) {
			currentScene->state.player->movement.x = -1.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT]) {
			currentScene->state.player->movement.x = 1.0f;
		}
		if (keys[SDL_SCANCODE_DOWN]) {
			currentScene->state.player->movement.y = -1.0f;
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
        currentScene->Update(FIXED_TIMESTEP);

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

    viewMatrix = glm::mat4(1.0f);
    if (currentScene->state.player->isActive)
    {
        if (currentScene->state.player->position.x > 5 || currentScene->state.player->position.y < 0) {
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-(currentScene->state.player->position.x), -(currentScene->state.player->position.y), 0));
        }

        else {
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 0, 0));
        }


        if (currentScene->state.player->CheckCollision(currentScene->state.enemies))
        {
            //currentScene->Initialize();
            currentScene->state.player->position = glm::vec3(2.0f, -1.0f, 0.0f);
            for (int j = 0; j < 5; j++)
            {
                currentScene->state.objects[j].token = false;
            }
            currentScene->state.player->token = false;
        }

        if (currentScene->state.time <= 0)
        {
            currentScene->state.nextScene = 4;
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

    float time = 180;

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();

        time = currentScene->state.time;
        if (currentScene->state.nextScene > -1)
        {
            time = currentScene->state.time;
            SwitchToScene(sceneList[currentScene->state.nextScene]);
            Mix_PlayChannel(-1, success, 0);
            Mix_Volume(-1, MIX_MAX_VOLUME);
            currentScene->Initialize();
            currentScene->state.time = time;
        }
}

    Shutdown();
    return 0;
}