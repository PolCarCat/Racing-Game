#ifndef __j1FONTS_H__
#define __j1FONTS_H__

#include "Module.h"
#include "SDL\include\SDL_pixels.h"
#include "SDL_image\include\SDL_image.h"
#include "Label.h"

#define DEFAULT_FONT "fonts/open_sans/OpenSans-Regular.ttf"
#define DEFAULT_FONT_SIZE 12

struct SDL_Texture;
struct _TTF_Font;

class j1Fonts : public Module
{
public:

	j1Fonts(Application* app, bool start_enabled = true);

	// Destructor
	virtual ~j1Fonts();

	// Called before render is available
	bool Init();

	// Called before quitting
	bool CleanUp();

	// Load Font
	_TTF_Font* const Load(const char* path, int size = 12);
	bool Unload(_TTF_Font* font);

	// Create a surface from text
	SDL_Texture* Print(const char* text, SDL_Color color = { 255, 255, 255, 255 }, _TTF_Font* font = NULL, Label::RenderMode render_mode = Label::RenderMode::BLENDED, SDL_Color bg_color = { 0, 0, 0, 0 });

	bool CalcSize(const char* text, int& width, int& height, _TTF_Font* font = NULL) const;

public:

	p2List<_TTF_Font*>	fonts;
	_TTF_Font*			default;
};


#endif // __j1FONTS_H__