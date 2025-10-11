<<<<<<< HEAD
#include "zketch.hpp"
using namespace zketch ;

int main() {
	zketch_init() ;

	Font font("Segoe UI", 16) ;
	
	Gdiplus::Font gfont = font ;
    Gdiplus::Graphics graphics(GetDC(NULL));

	RectF boxf = font.GetStringBound(L"Hello World", {0, font.GetAscent()}) ;

    Gdiplus::RectF gboxf;
    graphics.MeasureString(L"Hello World", -1, &gfont, PointF(0, 0), &gboxf);

	logger::info("box\t: {", boxf.x, ", ", boxf.y, ", ", boxf.w, ", ", boxf.h, "}.") ;
	logger::info("gbox\t: {", gboxf.X, ", ", gboxf.Y, ", ", gboxf.Width, ", ", gboxf.Height, "}.") ;

	// output
	/*
	D:\Project\zketch\bin>"D:/Project/zketch/bin/zketch.exe"
	[INFO]  box     : {0, 0, 57.14286, 16}.
	[INFO]  gbox    : {0, 0, 83.731766, 23.28125}.

	D:\Project\zketch\bin>
	*/
=======
#include <unordered_map>

#include "window.hpp"

using namespace zketch ;

static inline bool ISRUNNING = true ;

int main() {
	zketch::AppRegistry::RegisterWindowClass() ;

	std::unordered_map<HWND, std::unique_ptr<Window>> windows ;
	auto w1 = std::make_unique<Window>("Window 1", 300, 200) ;
	auto handle1 = w1->getHandle() ;
	windows[w1->getHandle()] = std::move(w1) ;
	auto w2 = std::make_unique<Window>("Window 2", 300, 200) ;
	auto handle2 = w2->getHandle() ;
	windows[w2->getHandle()] = std::move(w2) ;
	if (!handle1)
		logger::warning("handle is nullptr") ;
	windows[handle1]->Show() ;
	windows[handle2]->Show() ;

	while (ISRUNNING){
		Event e ;
		while (PollEvent(e)) {
			if (e == EventType::KeyDown) {
				if (e.keyCode() == 'Q') {
					windows[e.getHandle()]->Quit() ;
					windows.erase(e.getHandle()) ;
				}
				logger::info(char(e.keyCode()), " pressed from window : ", e.getHandle()) ;
			}
		}

		if (windows.empty()) {
			ISRUNNING = false ;
		}
	}
>>>>>>> 015feefd91700e70c7ee430b9505f3ab1ca1e0a6
}