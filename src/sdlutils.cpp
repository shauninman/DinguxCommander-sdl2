#include <iostream>
#include "sdlutils.h"
#include <SDL_image.h>
#include "def.h"
#include "resourceManager.h"

// Load an image using SDL_image
SDL_Surface *SDL_utils::loadImage(const std::string &p_filename)
{
    INHIBIT(std::cout << "SDL_utils::loadImage(" << p_filename << ")" << std::endl;)
    // Load image
    SDL_Surface* l_img = IMG_Load(p_filename.c_str());
    SDL_Surface* l_img2 = NULL;
    if(l_img != NULL)
    {
        // Optimize the image
        // l_img2 = SDL_DisplayFormat(l_img);
        l_img2 = SDL_ConvertSurfaceFormat(l_img, SDL_GetWindowPixelFormat(Globals::g_sdlwindow), 0);
        // Free the first image
        SDL_FreeSurface(l_img);
        // Set color key
        if (l_img2 != NULL)
            SDL_SetColorKey(l_img2, SDL_TRUE, SDL_MapRGB(l_img2->format, COLOR_KEY));
            // SDL_SetColorKey(l_img2, SDL_SRCCOLORKEY, SDL_MapRGB(l_img2->format, COLOR_KEY));
    }
    // Check errors
    if (l_img2 == NULL)
        std::cerr << "SDL_utils::loadImage: " << SDL_GetError() << std::endl;
    return l_img2;
}

void SDL_utils::applySurface(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_source, SDL_Surface* p_destination, SDL_Rect *p_clip)
{
    // Rectangle to hold the offsets
    SDL_Rect l_offset;
    // Set offsets
    l_offset.x = p_x;
    l_offset.y = p_y;
    // l_offset.w = SCREEN_WIDTH;
    // l_offset.h = SCREEN_HEIGHT;
    //Blit the surface
    SDL_BlitSurface(p_source, p_clip, p_destination, &l_offset);

     
}

TTF_Font *SDL_utils::loadFont(const std::string &p_font, const int p_size)
{
    INHIBIT(std::cout << "SDL_utils::loadFont(" << p_font << ", " << p_size << ")" << std::endl;)
    TTF_Font *l_font = TTF_OpenFont(p_font.c_str(), p_size);
    if (l_font == NULL)
        std::cerr << "SDL_utils::loadFont: " << SDL_GetError() << std::endl;
    return l_font;
}

SDL_Surface *SDL_utils::renderText(TTF_Font *p_font, const std::string &p_text, const SDL_Color &p_fg)
{
    return TTF_RenderUTF8_Solid(p_font, p_text.c_str(), p_fg);
}

void SDL_utils::applyText(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_destination, TTF_Font *p_font, const std::string &p_text, const SDL_Color &p_fg, const T_TEXT_ALIGN p_align)
{
    SDL_Surface *l_text = renderText(p_font, p_text, p_fg);
    switch (p_align)
    {
        case T_TEXT_ALIGN_LEFT:
            applySurface(p_x, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_RIGHT:
            applySurface(p_x - l_text->w, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_CENTER:
            applySurface(p_x - l_text->w / 2, p_y, l_text, p_destination);
            break;
        default:
            break;
    }
    SDL_FreeSurface(l_text);
}

SDL_Surface *SDL_utils::createImage(const int p_width, const int p_height, const Uint32 p_color)
{
    // Create image in the same format as the screen
    SDL_Surface *l_ret = SDL_CreateRGBSurface(SURFACE_FLAGS, p_width, p_height, Globals::g_screen->format->BitsPerPixel, Globals::g_screen->format->Rmask, Globals::g_screen->format->Gmask, Globals::g_screen->format->Bmask, Globals::g_screen->format->Amask);
    if (l_ret == NULL)
        std::cerr << "SDL_utils::createImage: " << SDL_GetError() << std::endl;
    // Fill image with the given color
    SDL_FillRect(l_ret, NULL, p_color);
    return l_ret;
}

void SDL_utils::renderAll(void)
{
    if (Globals::g_windows.empty())
        return;
    // First window to draw is the last fullscreen
    unsigned int l_i = Globals::g_windows.size() - 1;
    while (l_i && !Globals::g_windows[l_i]->isFullScreen())
        --l_i;
    // Draw windows
    for (std::vector<CWindow *>::iterator l_it = Globals::g_windows.begin() + l_i; l_it != Globals::g_windows.end(); ++l_it)
        (*l_it)->render(l_it + 1 == Globals::g_windows.end());

#if defined(PLATFORM_MY355)
    SDL_Rect rect{0,0,640,480};
	SDL_BlitScaled(Globals::g_screen, NULL, Globals::g_screen_real,&rect);
#elif defined(PLATFORM_TG5040)
	int sw = Globals::g_screen_real->w;
	int sh = Globals::g_screen_real->h;
	int sx = (sw / SCREEN_WIDTH);
	int sy = (sh / SCREEN_HEIGHT);
	int s = sx<sy?sx:sy;
	int w = s * SCREEN_WIDTH;
	int h = s * SCREEN_HEIGHT;
	int x = (sw - w) / 2;
	int y = (sh - h) / 3;
	SDL_Rect rect{x,y,w,h};
	SDL_BlitScaled(Globals::g_screen, NULL, Globals::g_screen_real,&rect);
#else
	// rotate 90° and scale 320x240 to 480x640
	uint32_t* src = (uint32_t*)Globals::g_screen->pixels;
	uint32_t* dst = (uint32_t*)Globals::g_screen_real->pixels;
	int sw,sh,dw,dh,dx,dy,i;
	sw = 320;
	sh = 240;
	dw = 480;
	dh = 640;
	uint32_t c,a,r,g,b;
	for (int y=0; y<sh; y++) {
		for (int x=0; x<sw; x++) {
			c = src[y*sw+x];
		    a = (c & 0xFF000000);
		    r = (c & 0x00FF0000);
		    g = (c & 0x0000FF00);
		    b = (c & 0x000000FF);
			c = a | (b << 16) | g | (r >> 16);

#if defined(PLATFORM_MY282)
			dx = y*2;
			dy = (sw-x-1)*2;
#else // PLATFORM_ZERO28
			dx = (sh-y-1)*2;
			dy = x*2;
#endif
			i = (dy)*dw+dx;
			dst[i] = c;
			dst[i+1] = c;
			dst[i+dw] = c;
			dst[i+dw+1] = c;
		}
	}
#endif
}

void SDL_utils::hastalavista(void)
{
    // Destroy all dialogs except the first one (the commander)
    while (Globals::g_windows.size() > 1)
        delete Globals::g_windows.back();
    // Free resources
    CResourceManager::instance().sdlCleanup();
    // Quit SDL
    TTF_Quit();
    SDL_Quit();
}

void SDL_utils::pleaseWait(void)
{

    SDL_Surface *l_surfaceTmp = renderText(CResourceManager::instance().getFont(), "Please wait...", Globals::g_colorTextNormal);
    SDL_Rect l_rect;
    l_rect.x = (SCREEN_WIDTH - (l_surfaceTmp->w + 2 * DIALOG_MARGIN + 2 * DIALOG_BORDER)) >> 1;
    l_rect.y = (SCREEN_HEIGHT - (l_surfaceTmp->h + 9)) >> 1;
    l_rect.w = l_surfaceTmp->w + 2 * DIALOG_MARGIN + 2 * DIALOG_BORDER;
    l_rect.h = l_surfaceTmp->h + 9;
    SDL_FillRect(Globals::g_screen, &l_rect, SDL_MapRGB(Globals::g_screen->format, COLOR_BORDER));
    l_rect.x += DIALOG_BORDER;
    l_rect.y += DIALOG_BORDER;
    l_rect.w -= 2 * DIALOG_BORDER;
    l_rect.h -= 2 * DIALOG_BORDER;
    SDL_FillRect(Globals::g_screen, &l_rect, SDL_MapRGB(Globals::g_screen->format, COLOR_BG_1));
    applySurface((SCREEN_WIDTH - l_surfaceTmp->w) >> 1, (SCREEN_HEIGHT - l_surfaceTmp->h) >> 1, l_surfaceTmp, Globals::g_screen);
    SDL_FreeSurface(l_surfaceTmp);

    SDL_UpdateWindowSurface(Globals::g_sdlwindow);
    SDL_UpdateWindowSurface(Globals::g_sdlwindow);
    // SDL_Flip(Globals::g_screen);
    // SDL_Flip(Globals::g_screen);
}
