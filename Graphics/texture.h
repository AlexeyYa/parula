
#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL2/SDL.h>

class LTexture
{
public:
    LTexture(int width, int height, SDL_Renderer* renderer);
    ~LTexture();
    void Render() const;

    void* GetPixels();
    void FreePixels();
    int GetPitch();
    void Fill(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
private:
    int m_width;
    int m_height;
    SDL_Renderer* m_renderer;

    SDL_Texture* m_texture;
    void* m_pixels;
    int m_pitch;
};

#endif
