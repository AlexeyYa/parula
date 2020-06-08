
#include "Graphics/texture.h"

LTexture::LTexture(int width, int height, SDL_Window* window, SDL_Renderer* renderer) :
    m_width(width),
    m_height(height),
    m_renderer(renderer),
    m_pixels(nullptr),
    m_pitch(0)
{
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
    if (!m_texture)
    {
        // Falied to create
        throw;
    }
    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    Fill(255, 255, 255, 0);
}

LTexture::~LTexture()
{
    SDL_DestroyTexture(m_texture);
}

void LTexture::Render() const
{
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
}

void LTexture::Fill(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    uint32_t* ptr = (uint32_t*)GetPixels();
    if (ptr != nullptr)
    {
        for (int i = 0; i < m_width * m_height; i++)
        {
            // SDL ARGB coloring
            ptr[i] = ((r * 256 + g) * 256 + b) * 256 + a;
        }
    }

    FreePixels();
}

void* LTexture::GetPixels()
{
    if (m_pixels == nullptr)
    {
        if (SDL_LockTexture(m_texture, nullptr, &m_pixels, &m_pitch) == 0)
            return m_pixels;
        else
            // Unable to lock
            throw;
    }
    else
    {
        // Already locked
        return nullptr;
    }
}

void LTexture::FreePixels()
{
    if (m_pixels != nullptr)
    {
        SDL_UnlockTexture(m_texture);
        m_pixels = nullptr;
        m_pitch = 0;
    }
    else
    {
        // Texture is not locked
        throw;
    }
}

int LTexture::GetPitch()
{
    return m_pitch;
}
