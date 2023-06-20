#include <iostream>
#include <SDL2/SDL.h>
#include <cmath>

using namespace std;

bool init();
void kill();
bool loop();

// Pointers to our window, renderer, and texture
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

int main(int argc, char** args) {

	if ( !init() ) return 1;

	while ( loop() ) {
		// wait before processing the next frame
		SDL_Delay(10); 
	}

	kill();
	return 0;
}

bool loop() {

	static const unsigned char* keys = SDL_GetKeyboardState( NULL );

	SDL_Event e;
	SDL_Rect dest;

	// Clear the window to white
	SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
	SDL_RenderClear( renderer );

	// Event loop
	while ( SDL_PollEvent( &e ) != 0 ) {
		switch ( e.type ) {
			case SDL_QUIT:
				return false;
			case SDL_MOUSEMOTION:
				
		}
	}

	

	// Update window
	SDL_RenderPresent( renderer );

	return true;
}

bool init() {
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		return false;
	} 

	window = SDL_CreateWindow( "Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN );
	if ( !window ) {
		cout << "Error creating window: " << SDL_GetError()  << endl;
		system("pause");
		return false;
	}

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( !renderer ) {
		cout << "Error creating renderer: " << SDL_GetError() << endl;
		return false;
	}

	// Load bitmap into surface
	SDL_Surface* buffer = SDL_LoadBMP("test.bmp");
	if ( !buffer ) {
		cout << "Error loading image test.bmp: " << SDL_GetError() << endl;
		return false;
	}

	// Create texture
	texture = SDL_CreateTextureFromSurface( renderer, buffer );
	// Free surface as it's no longer needed
	SDL_FreeSurface( buffer );
	buffer = NULL;
	if ( !texture ) {
		cout << "Error creating texture: " << SDL_GetError() << endl;
		return false;
	}

	return true;
}

void kill() {
	SDL_DestroyTexture( texture );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	texture = NULL;
	window = NULL;
	renderer = NULL;
	SDL_Quit();
}