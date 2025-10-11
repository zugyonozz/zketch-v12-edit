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
}