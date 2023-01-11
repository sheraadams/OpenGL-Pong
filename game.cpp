/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <algorithm>
#include <sstream>
#include <iostream>

#include <learnopengl/filesystem.h>

#include <irrklang/irrKlang.h>
using namespace irrklang;

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"


// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
GameObject        *Player2;
GameObject        *Pause;
GameObject        *Fruit;
GameObject        *Fruit2;
GameObject        *Fruit3;
GameObject        *Fruit4;
GameObject        *Fruit5;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;
ISoundEngine      *SoundEngine = createIrrKlangDevice();
TextRenderer      *Text;

float ShakeTime = 0.0f;
int points;
int keycounter;
int points2;


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(1), Points()
{ 

}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Player2;
    delete Fruit;
    delete Fruit2;
    delete Fruit3;
    delete Fruit4;
    delete Fruit5;
    delete Pause;
    delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();
}

void Game::Init()
{
    SoundEngine->play2D("resources/audio/boobass.mp3", true);
    // load shaders
    ResourceManager::LoadShader("sprite.vs", "sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("post_processing.vs", "post_processing.fs", nullptr, "postprocessing");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    // load textures
    ResourceManager::LoadTexture("resources/textures/background3.jpg", false, "background");
    ResourceManager::LoadTexture("resources/textures/awesomeface2.png", true, "face");
    ResourceManager::LoadTexture("resources/textures/block.png", false, "block");
    ResourceManager::LoadTexture("resources/textures/block_solid5.jpg", false, "block_solid");
    ResourceManager::LoadTexture("resources/textures/fox2.png", true, "fox");
    ResourceManager::LoadTexture("resources/textures/beaver.png", true, "beaver");
    ResourceManager::LoadTexture("resources/textures/particle2.png", true, "particle");
    ResourceManager::LoadTexture("resources/textures/powerup_speed3.png", true, "powerup_speed");
    ResourceManager::LoadTexture("resources/textures/powerup_sticky3.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("resources/textures/powerup_increase3.png", true, "powerup_increase");
    ResourceManager::LoadTexture("resources/textures/powerup_confuse3.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("resources/textures/powerup_chaos3.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("resources/textures/powerup_passthrough3.png", true, "powerup_passthrough");
    ResourceManager::LoadTexture("resources/textures/pause.png", true, "pause");
    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("resources/fonts/OCRAEXT.TTF", 24);
    // load levels
    GameLevel one; one.Load("resources/levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("resources/levels/two.lvl", this->Width, this->Height /2 );
    GameLevel three; three.Load("resources/levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("resources/levels/four.lvl", this->Width, this->Height / 2);
   
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;


    // configure game objects
    
    //players
    glm::vec2 playerPos = glm::vec2(PLAYER_SIZE.x / 15.0f, this->Height /2.0f - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("fox"));
    glm::vec2 player2Pos = glm::vec2(this->Width - PLAYER2_SIZE.x * 2.0f  / 2.0f, this->Height /2.0f - PLAYER2_SIZE.y);
    Player2 = new GameObject(player2Pos, PLAYER2_SIZE, ResourceManager::GetTexture("beaver"));

    //fruit
    glm::vec2 FruitPos = glm::vec2(this->Width /2.0f - FRUIT_SIZE.x, this->Height/ 10.0f - FRUIT_SIZE.y);
    Fruit = new GameObject(FruitPos, FRUIT_SIZE, ResourceManager::GetTexture("powerup_speed"));
    glm::vec2 Fruit2Pos = glm::vec2(this->Width / 5.0f - FRUIT_SIZE.x * 3.0f, this->Height / 10.0f - FRUIT_SIZE.y);
    Fruit2 = new GameObject(Fruit2Pos, FRUIT_SIZE, ResourceManager::GetTexture("powerup_increase"));
    glm::vec2 Fruit3Pos = glm::vec2(this->Width / 4.0f + FRUIT_SIZE.x * 2.0f, this->Height / 10.0f - FRUIT_SIZE.y);
    Fruit3 = new GameObject(Fruit3Pos, FRUIT_SIZE, ResourceManager::GetTexture("powerup_chaos"));
    glm::vec2 Fruit4Pos = glm::vec2(this->Width / 3.0f - FRUIT_SIZE.x, this->Height / 10.0f - FRUIT_SIZE.y);
    Fruit4 = new GameObject(Fruit4Pos, FRUIT_SIZE, ResourceManager::GetTexture("powerup_confuse"));
    glm::vec2 Fruit5Pos = glm::vec2(this->Width - FRUIT_SIZE.x * 3, this->Height / 10.0f - FRUIT_SIZE.y);
    Fruit5 = new GameObject(Fruit5Pos, FRUIT_SIZE, ResourceManager::GetTexture("powerup_sticky"));

    // pause buttom
    glm::vec2 PausePos = glm::vec2(this->Width / 2.0f - 200, this->Height / 2.0f - 170);
    Pause = new GameObject(PausePos, PAUSE_SIZE, ResourceManager::GetTexture("pause"));
  
    // ball
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
    //     glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

}

void Game::Update(float dt)
{

    int points = this->Points;
    // update objects
    Ball->Move(dt, this->Width);
    // check for collisions
    this->DoCollisions();
    // update particles
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
    // update PowerUps
    this->UpdatePowerUps(dt);
    // reduce shake time
    if (ShakeTime > 0.0f)
    {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f)
            Effects->Shake = false;
    }

    if (Ball->Position.y >=this->Height) // did ball reach bottom edge?
    {
        if (this->State == GAME_PAUSE)

        {
            Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
        }
        if ((this->State == GAME_ACTIVE) || (this->State == GAME_AI))
        {
            Effects->Chaos = true;
            points -= 1000;
            // did the player lose all his lives? : game over
            if (this->Lives == 0)
            {
                this->ResetLevel();
                this->State = GAME_MENU;
            }
            this->ResetPlayer();
        }
    }

    // check loss condition
    if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
    {
        Effects->Chaos = true;
        this->Lives = this->Lives -1;
        // did the player lose all his lives? : game over
        if (this->Lives == 0)
        {
            this->ResetLevel();
            this->State = GAME_MENU;
        }
        this->ResetPlayer();
    }
    // check win condition
    if (this->State == GAME_ACTIVE && IsCompleted() && (points > 3000))
    {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = true;
        this->State = GAME_WIN;
    }
    if (this->State == GAME_ACTIVE && IsCompleted() && (points < 3000))
    {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = true;
        this->State = GAME_LOSE;
    }

}


void Game::ProcessInput(float dt)
{
    float velocity = PLAYER_VELOCITY * dt;
    bool aiPlayer = false;


    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
        // choose whether or not to play game or have 2 ai players

        if (this->Keys[GLFW_KEY_A] && !this->KeysProcessed[GLFW_KEY_A])
        {
            this->State = GAME_AI;
            aiPlayer = true;
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
    }
    if (this->State == GAME_WIN)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;


            this->State = GAME_MENU;
            points = 0;
            points2 = 0;
        }
    }
    if (this->State == GAME_LOSE)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;


            this->State = GAME_MENU;
            points = 0;
            points2 = 0;
        }
    }
    if (this->State == GAME_AI)
    {
        
        if (this->Keys[GLFW_KEY_P] && !this->KeysProcessed[GLFW_KEY_P])
        {
            int pausepoints = points;
            points = pausepoints;
            this->State = GAME_PAUSE;
            this->KeysProcessed[GLFW_KEY_P] = true;
        }
        //  random effects
        if (keycounter > 200) { Effects->Chaos = true; }
        if (keycounter > 470) { Effects->Chaos = false; }
        if (keycounter > 600) { Effects->Confuse = true; }
        if (keycounter > 800) { Effects->Confuse = false;}
        if (keycounter > 810) { Effects->Shake = false; }
        if (keycounter > 880) { Effects->Shake = false; }

        Ball->Stuck = false;
        float velocity = PLAYER_VELOCITY * dt;
        if (Player->Position.y <= Ball->Position.y)
        {
            keycounter += 1;
            Player->Position.y += velocity;

            // keep player 2 in the upper bounds of the screen

            if (Player->Position.y >= this->Height - Player->Size.y)
            {
                Player->Position.y = this->Height - Player->Size.y;
            }
        }
        if (Player->Position.y >= Ball->Position.y)
        {
            // keep player 2 in the lower bounds of the screen
            if (Player->Position.y <= 0)
            {
                Player->Position.y = 0;
            }
            Player->Position.y -= velocity;

        }
        // AI player 2 is auto
        if (Player2->Position.y <= Ball->Position.y)
        {

            Player2->Position.y += velocity;

            // keep player 2 in the upper bounds of the screen

            if (Player2->Position.y >= this->Height - Player2->Size.y)
            {
                Player2->Position.y = this->Height - Player2->Size.y;
            }
        }
        if (Player2->Position.y >= Ball->Position.y)
        {
            // keep player 2 in the lower bounds of the screen
            if (Player2->Position.y <= 0)
            {
                Player2->Position.y = 0;
            }
            Player2->Position.y -= velocity;

        }
        // AI player 2 vertical

        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;

        // fruit drops
        Fruit->Position.y += velocity / 2.0f;
        Fruit2->Position.y += velocity / 1.1f;
        Fruit3->Position.y += velocity / 3.0f;
        Fruit4->Position.y += velocity;
        Fruit5->Position.y += velocity;

    }
    if (this->State == GAME_ACTIVE)
    {
        if (this->Keys[GLFW_KEY_P] && !this->KeysProcessed[GLFW_KEY_P])
        {
            int pausepoints = points;
            points = pausepoints;
            this->State = GAME_PAUSE;
            this->KeysProcessed[GLFW_KEY_P] = true;
        }
        //  random effects
        if (keycounter > 200) {Effects->Chaos = true;}
        if (keycounter > 470){ Effects->Chaos = false;}
        if (keycounter > 600){ Effects->Confuse = true;}
        if (keycounter > 800){ Effects->Confuse = false;}
        if (keycounter > 810){ Effects->Shake = false; }
        if (keycounter > 880){ Effects->Shake = false; }

        // scoring points
        if (Ball->Position.x <= 0)
        {
            ShakeTime = 0.02f;
            Effects->Shake = true;

            points2 += 1000;
        }
        if (Ball->Position.x >= this->Width)
        {
            ShakeTime = 0.02f;
            Effects->Shake = true;
            points += 1000;
        }
        Ball->Stuck = false;

        // fruit drops
        Fruit->Position.y += velocity/ 2.0f;
        Fruit2->Position.y += velocity/ 1.1f;
        Fruit3->Position.y += velocity/ 3.0f;
        Fruit4->Position.y += velocity;
        Fruit5->Position.y += velocity;

        // move playerboard
        if (this->Keys[GLFW_KEY_A])
        {

            if (Player->Position.x >= 0.0f)
            {

                Player->Position.x -= velocity;
                if (Player->Position.x <= 0)
                    Player->Position.x = 0;

                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }

        if (this->Keys[GLFW_KEY_D])
        {

            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        // add additional left right option 
        if (this->Keys[GLFW_KEY_LEFT])
        {

            if (Player->Position.x >= 0.0f)
            {
                Player->Position.x -= velocity;
                // keep in screen bound left
                if (Player->Position.x <= 0)
                    Player->Position.x = 0;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_RIGHT])
        {

            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_UP])
        {
            keycounter += 1;
            if (Player->Position.y >= 0.0f)
            {
                Player->Position.y -= velocity;
                if (Ball->Stuck)
                    Ball->Position.y -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_W])
        {
            keycounter += 1;
            if (Player->Position.y >= 0.0f)
            {
                Player->Position.y -= velocity;
                if (Ball->Stuck)
                    Ball->Position.y -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_DOWN])
        {
            keycounter += 1;
            if (Player->Position.y <= this->Height - Player->Size.y)
            {
                Player->Position.y += velocity;
                if (Ball->Stuck)
                    Ball->Position.y += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_S])
        {
            keycounter += 1;
            if (Player->Position.y <= this->Height - Player->Size.y)
            {
                Player->Position.y += velocity;
                if (Ball->Stuck)
                    Ball->Position.y += velocity;
            }
        }

        // AI player 2 is auto
        if (Player2->Position.y <= Ball->Position.y)
        {
            keycounter += 1;
            Player2->Position.y += velocity;


            // keep player 2 in the upper bounds of the screen
            if (Player2->Position.y >= this->Height - Player2->Size.y)
            {
                Player2->Position.y = this->Height - Player2->Size.y;
            }
        }
        if (Player2->Position.y >= Ball->Position.y)
        {
            // keep player 2 in the lower bounds of the screen
            if (Player2->Position.y <=  0)
            {
                Player2->Position.y = 0;
            }
            Player2->Position.y -= velocity;

        }
            // AI player 2 vertical
        
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }
    if (this->State == GAME_PAUSE)
    {
        if (this->Keys[GLFW_KEY_P] && !this->KeysProcessed[GLFW_KEY_P])
        {
            int pausepoints = points;
            points = pausepoints;
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_P] = true;
        }
    }
}

void Game::Render()
{

    if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN || this->State == GAME_AI || this->State == GAME_LOSE || this->State== GAME_PAUSE)
    {
        // begin rendering to postprocessing framebuffer
        Effects->BeginRender();

            // draw background
            Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
            // draw level
            this->Levels[this->Level].Draw(*Renderer);
            // draw player

            Player->Draw(*Renderer);
            Player2->Draw(*Renderer);


            // draw PowerUps
            for (PowerUp &powerUp : this->PowerUps)
                if (!powerUp.Destroyed)
                    powerUp.Draw(*Renderer);
            // draw particles	
            Particles->Draw();
            // draw ball
            Ball->Draw(*Renderer);            
        // end rendering to postprocessing framebuffer
        Effects->EndRender();
        // render postprocessing quad
        Effects->Render(glfwGetTime());
        // render text (don't include in postprocessing)

        // player 1 points
        std::stringstream sp; sp << points;
        // player 2 points
        std::stringstream pp; pp << points2;

  
        // player 1 points
        Text->RenderText("Player 1:" + sp.str(), 5.0f, 5.0f, 1.0f);
        // player 2 points
        Text->RenderText("Player 2:" + pp.str(), 5.0f, 25.0f, 1.0f);
    }
    if (this->State == GAME_PAUSE)
    {
        Pause->Draw(*Renderer);
    }
    if (this->State == GAME_ACTIVE || this->State == GAME_AI || this->State == GAME_WIN)
    {
        Fruit->Draw(*Renderer);
        Fruit2->Draw(*Renderer);
        Fruit3->Draw(*Renderer);
        Fruit4->Draw(*Renderer);
        Fruit5->Draw(*Renderer);
    }
    
    if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", this->Width / 2.0f - 160.0f, this->Height / 2.0f, 1.0f);
        Text->RenderText("Press A to let AI Players Play", this->Width /2.0f - 180.0f, this->Height / 2.0f + 20.0f, 0.75f);

    }
    if (this->State == GAME_WIN)
    {
        Text->RenderText("You WON!!!", this->Width / 2.0f - 70.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        Text->RenderText("Press ENTER to retry or ESC to quit", this->Width / 2.0f - 240.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
    if (this->State == GAME_LOSE)
    {
        Text->RenderText("You lost!!!", this->Width / 2.0f - 70.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        Text->RenderText("Press ENTER to retry or ESC to quit", this->Width / 2.0f - 240.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
}


void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);

}

void Game::ResetPlayer()
{
    --this->Lives; 
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(PLAYER_SIZE.x / 15.0f, this->Height / 2.0f - PLAYER_SIZE.y);

    // player2
    Player2->Size = PLAYER2_SIZE;
    Player2->Position = glm::vec2(this->Width - PLAYER2_SIZE.x , this->Height / 2.0f - PLAYER2_SIZE.y);

    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

    // also disable all active powerups
    Effects->Chaos = Effects->Confuse = false;
    Ball->PassThrough = Ball->Sticky = false;
    Player->Color = glm::vec3(1.0f);
    Player2->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f);
}


// powerups
bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type);

void Game::UpdatePowerUps(float dt)
{

    for (PowerUp &powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated)
        {
            points + 100;
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f)
            {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // deactivate effects
                if (powerUp.Type == "sticky")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                    }
                }
            }
        }
    }
    // Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
    // Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}

glm::vec2 ShouldSpawn(unsigned int chance)
{
    int randx = chance * rand() % 5;
    int randy = chance * rand() % 5;
    float a = randx * 1.0f;
    glm::vec2 ball2(a, a);
    return ball2;
}
void Game::SpawnPowerUps(glm::vec2 ball)
{
    int chance = 1;
    if (points > 1000)
    {
        int randx = chance * rand() % 5;
        int randy = chance * rand() % 5;
        float a = randx * 1.0f;
        glm::vec2 ball2(a, a);
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(0.0f, 0.5f, 0.0f), 20.0f, ball2, ResourceManager::GetTexture("powerup_sticky"))); // pear
    }

    if (points > 2000)

    {
        int randx = chance * rand() % 5;
        int randy = chance * rand() % 5;
        float a = randx * 1.0f;
        glm::vec2 ball2(a, a);
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(1.5f, 1.5f, 0.0f), 0.0f, ball2, ResourceManager::GetTexture("powerup_speed"))); // banana
    }
    if (points > 3000)

    {
        int randx = chance * rand() % 5;
        int randy = chance * rand() % 5;
        float a = randx * 1.0f;
        glm::vec2 ball2(a, a);
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(1.0f, .50f, 0.0f), 10.0f, ball2, ResourceManager::GetTexture("powerup_passthrough"))); // orange
    }
    if (points > 4000)

    {
        int randx = chance * rand() % 5;
        int randy = chance * rand() % 5;
        float a = randx * 1.0f;
        glm::vec2 ball2(a, a);
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.5f, 1.5f, 0.0), 0.0f, ball2, ResourceManager::GetTexture("powerup_increase"))); // lemon
    }
    if (points > 5000)

    {
        int randx = chance * rand() % 5;
        int randy = chance * rand() % 5;
        float a = randx * 1.0f;
        glm::vec2 ball2(a, a);
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(0.64f, 0.0f, 1.0f), 15.0f, ball2, ResourceManager::GetTexture("powerup_confuse"))); // grape
        PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.0f, 0.0f), 15.0f, ball2, ResourceManager::GetTexture("powerup_chaos"))); // cherry
    }

}

void ActivatePowerUp(PowerUp &powerUp)
{
    if (powerUp.Type == "speed")
    {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        Player->Size.y += 20;
        Player->Size.x += 20;  // fixme todo here
    } 
    else if (powerUp.Type == "confuse")
    {
        if (!Effects->Chaos)
            Effects->Confuse = true; // only activate if chaos wasn't already active
    }
    else if (powerUp.Type == "chaos")
    {
        if (!Effects->Confuse)
            Effects->Chaos = true;
    }
}

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
    // Check if another PowerUp of the same type is still active
    // in which case we don't disable its effect (yet)
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}


// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{

    // also check collisions on PowerUps and if so, activate them
    for (PowerUp &powerUp : this->PowerUps)
    {
        if (!powerUp.Destroyed)
        {
            // first check if powerup passed bottom edge, if so: keep as inactive and destroy
            if (powerUp.Position.y >= this->Height)
            {
                //powerUp.Destroyed = true;

            }
            if (CheckCollision(*Player, powerUp))
            {	// collided with player, now activate powerup
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
                SoundEngine->play2D("resources/audio/powerup.wav", false);

            }
        }
    }

    // and finally check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);

        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        // 6 is the direction of the velocity from the left should be increased if screen size is larger
        Ball->Velocity.x = 6.0f * abs(Ball->Velocity.y);
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky fox
        Ball->Velocity.y = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        SoundEngine->play2D("resources/audio/bleep.wav", false);
    }



    Collision result2 = CheckCollision(*Ball, *Player2);
    if (!Ball->Stuck && std::get<0>(result2))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player2->Position.x + Player2->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player2->Size.x / 2.0f);

        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        // -6 is the direction of the velocity from the right. this should be increased if screen size is larger (-4 to -6 is good number for most screen sizes)
        Ball->Velocity.x = -6.0f * abs(Ball->Velocity.y);
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky fox
        Ball->Velocity.y = INITIAL_BALL_VELOCITY.x* percentage* strength;
        SoundEngine->play2D("resources/audio/bleep.wav", false);
    }
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)

    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);

    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;

    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;
    
    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
  
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}
bool Game::IsCompleted()
{
    if (this->Lives = 0)
        return true;
    if ((points > 3000) || (points2 > 3000))
        return true;
    else
        return false;
}