#pragma once

#include "Color.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <string>


namespace Renderer {

    /**
     * renders a string 
     * @author retucio
     * @param r pointer to SDL_Renderer object
     * @param font point to loaded TTF_Font* object
     * @param text string to draw on screen
     * @param x, y coordinates
     * @param color color to use when drawing the text
     */
    void renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, Color c);

    /**
     * renders a filled rectangle
     * @author retucio
     * @param r pointer to SDL_Renderer object
     * @param x, y coordinates
     * @param w, h size of the rect to draw
     * @param c color
     */
    void fillRect(SDL_Renderer* r, int x, int y, int w, int h, Color c);

    /**
     * renders the outline of a rectangle
     * @author retucio
     * @param r pointer to SDL_Renderer object
     * @param x, y coordinates
     * @param w, h size of the rect to draw
     * @param c color
     * @param thickness thickness of the outline's lines
     */
    void outlineRect(SDL_Renderer* r, int x, int y, int w, int h, Color c, int thickness);

    /**
     * renders a button-like object, by rendering a rectangle and text on top
     * @author retucio
     * @return an SDL_Rect for hit-checking
     * @param r pointer to SDL_Renderer object
     * @param font pointer to loaded TTF_Font object
     * @param text label of the button
     * @param x, y coordinates
     * @param w, h size of the button
     * @param bg background (rect) color
     * @param fg foreground (text) color
     */
    SDL_Rect renderButton(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, int w, int h, Color bg, Color fg);

    /**
     * renders a filled circle given its coordinates and radius
     * @author retucio, stolen from Gumichan01's gist: https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
     * @param renderer pointer to SDL_Renderer object
     * @param x, y center coordinates of the filled circle
     * @param r radius of filled circle
     * @param color color of filled circle
    */
    int fillCircle(SDL_Renderer* renderer, int x, int y, int r, Color color);
};