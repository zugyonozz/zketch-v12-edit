<<<<<<< HEAD
#pragma once

#include <objidl.h>
#include <gdiplus.h>

namespace zketch {

	class GDISession__ {
	private :
		ULONG_PTR token_ ;

	public:
		GDISession__() {
			Gdiplus::GdiplusStartupInput input ;
			Gdiplus::GdiplusStartup(&token_, &input, nullptr) ;
		}

		~GDISession__() { 
			Gdiplus::GdiplusShutdown(token_) ;
		}
	} ;

	static GDISession__ gdi_session_init__ ;
=======
#pragma once

#include <objidl.h>
#include <gdiplus.h>

namespace zketch {

	class GDISession__ {
	private :
		ULONG_PTR token_ ;

	public:
		GDISession__() {
			Gdiplus::GdiplusStartupInput input ;
			Gdiplus::GdiplusStartup(&token_, &input, nullptr) ;
		}

		~GDISession__() { 
			Gdiplus::GdiplusShutdown(token_) ;
		}
	} ;

	static GDISession__ gdi_session_init__ ;
>>>>>>> 015feefd91700e70c7ee430b9505f3ab1ca1e0a6
}