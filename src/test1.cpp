<<<<<<< HEAD
#include "zketch.hpp"
using namespace zketch ;

// this code is tutorial create window

int main() {
	zketch_init() ;

	// create window with ctor with params
	// paramater :
	// - "title"
	// - "width"
	// - "height"
	Window window("Window Demo", 900, 800) ;

	// this method using for showing window
	window.Show() ;

	// Create Event buffer
	Event e ;

	while(Application) {

		// push event to event queue
		while(PollEvent(e)) {

		}
		
		// delay 16 ms equal 60 FPS
		Sleep(16) ;
	}
=======
#include "zketch.hpp"
using namespace zketch ;

// this code is tutorial create window

int main() {
	zketch_init() ;

	// create window with ctor with params
	// paramater :
	// - "title"
	// - "width"
	// - "height"
	Window window("Window Demo", 900, 800) ;

	// this method using for showing window
	window.Show() ;

	// Create Event buffer
	Event e ;

	while(Application) {

		// push event to event queue
		while(PollEvent(e)) {

		}
		
		// delay 16 ms equal 60 FPS
		Sleep(16) ;
	}
>>>>>>> 015feefd91700e70c7ee430b9505f3ab1ca1e0a6
}