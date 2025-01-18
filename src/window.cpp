#include <iostream>
#include "window.h"
#include "def.h"
#include "sdlutils.h"

#define KEYHOLD_TIMER_FIRST   6
#define KEYHOLD_TIMER         2

CWindow::CWindow(void):
    m_timer(0),
    m_lastPressed(SDLK_0),
    m_retVal(0)
{
    // Add window to the lists for render
    Globals::g_windows.push_back(this);
}

CWindow::~CWindow(void)
{
    // Remove last window
    Globals::g_windows.pop_back();
}

const int CWindow::execute(void)
{
    m_retVal = 0;
    Uint32 l_time(0);
    SDL_Event l_event;
    bool l_loop(true);
    bool l_render(true);
    // Main loop
    while (l_loop)
    {
        l_time = SDL_GetTicks();
        // Handle key press
        while (SDL_PollEvent(&l_event))
        {
            if (l_event.type == SDL_KEYDOWN)
            {
				printf("key:%d\n",l_event.key.keysym.sym); fflush(stdout);
				l_event.key.keysym.sym = l_event.key.keysym.sym==MYKEY_MENU?MYKEY_SYSTEM:l_event.key.keysym.sym;
                l_render = this->keyPress(l_event);
                if (m_retVal)
                    l_loop = false;
            }
            else if (l_event.type == SDL_QUIT)
            {
                // Re-insert event so we exit from nested menus
                SDL_PushEvent(&l_event);
                l_loop = false;
                break;
            }
            else if(l_event.type == SDL_JOYBUTTONDOWN)
            {
                printf("joy:%d\n",l_event.jbutton.button); fflush(stdout);
                SDL_Event key_event;
				key_event.key.keysym.sym = l_event.jbutton.button==MYKEY_MENU?MYKEY_SYSTEM:l_event.jbutton.button;
                l_render = this->keyPress(key_event); // always returns false
				l_render = true;
                if (m_retVal)
                    l_loop = false;
            }
			else if (l_event.type==SDL_JOYHATMOTION) {
				printf("hat:%d\n",l_event.jhat.value); fflush(stdout);
				int hat = l_event.jhat.value;
				SDL_Event key_event;
				key_event.key.keysym.sym = 0;
				if (hat==SDL_HAT_UP) key_event.key.keysym.sym = MYKEY_UP;
				if (hat==SDL_HAT_DOWN) key_event.key.keysym.sym = MYKEY_DOWN;
				if (hat==SDL_HAT_LEFT) key_event.key.keysym.sym = MYKEY_LEFT;
				if (hat==SDL_HAT_RIGHT) key_event.key.keysym.sym = MYKEY_RIGHT;
				
				if (key_event.key.keysym.sym) {
	                l_render = this->keyPress(key_event); // always returns false
					l_render = true;
	                if (m_retVal)
	                    l_loop = false;
				}
			}
        }
        // Handle key hold
        if (l_loop)
            l_render = this->keyHold() || l_render;
        // Render if necessary
        if (l_render && l_loop)
        {
            SDL_utils::renderAll();
            // Flip twice to avoid graphical glitch on Dingoo
            // SDL_Flip(Globals::g_screen);
            // SDL_Flip(Globals::g_screen);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);

            l_render = false;
            INHIBIT(std::cout << "Render time: " << SDL_GetTicks() - l_time << "ms"<< std::endl;)
        }
        // Cap the framerate
        l_time = MS_PER_FRAME - (SDL_GetTicks() - l_time);
        if (l_time <= MS_PER_FRAME) SDL_Delay(l_time);
    }
    return m_retVal;
}

const bool CWindow::keyPress(const SDL_Event &p_event)
{
    // Reset timer if running
    if (m_timer)
        m_timer = 0;
    m_lastPressed = p_event.key.keysym.sym;
    return false;
}

const bool CWindow::keyHold(void)
{
    // Default behavior
    return false;
}

const bool CWindow::tick(const Uint8 p_held)
{
    bool l_ret(false);
    if (p_held)
    {
        if (m_timer)
        {
            --m_timer;
            if (!m_timer)
            {
                // Trigger!
                l_ret = true;
                // Timer continues
                m_timer = KEYHOLD_TIMER;
            }
        }
        else
        {
            // Start timer
            m_timer = KEYHOLD_TIMER_FIRST;
        }
    }
    else
    {
        // Stop timer if running
        if (m_timer)
            m_timer = 0;
    }
    return l_ret;
}

const int CWindow::getReturnValue(void) const
{
    return m_retVal;
}

const bool CWindow::isFullScreen(void) const
{
    // Default behavior
    return false;
}
