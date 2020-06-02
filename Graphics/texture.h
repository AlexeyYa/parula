
#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL2/SDL.h>

class LTexture
{
public:
    LTexture(int width, int height, SDL_Window* window, SDL_Renderer* renderer);
    ~LTexture();
    void Render() const;

    void* GetPixels();
    void FreePixels();
    int GetPitch();
private:
    int m_width;
    int m_height;
    SDL_Renderer* m_renderer;

    SDL_Texture* m_texture;
    void* m_pixels;
    int m_pitch;
};

#endif
