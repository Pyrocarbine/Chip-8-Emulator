//
// Created by Aaron Han on 2024-06-15.
//

#pragma once

#include <cstdint>
#include <SDL.h>


class Platform
{
    friend class Imgui;

public:
    Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
    ~Platform();
    void Update(void const* buffer, int pitch);
    bool ProcessInput(uint8_t* keys);

private:
    SDL_Window* window{};
    SDL_GLContext gl_context{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
};