/*

	MarlinWnd.h

*/

#pragma once

#include "bcm_host.h"

#include "Settings.h"

class CMarlinWnd // static class : ovrlayed window displaying Marlin UI
{
private:
	// dispmanx
	static DISPMANX_DISPLAY_HANDLE_T	display;
	static DISPMANX_RESOURCE_HANDLE_T	res;
	static DISPMANX_ELEMENT_HANDLE_T	element;

	// screen
	static uint32_t widthScreen;
	static uint32_t heightScreen;

	// viewport
	static uint32_t xDst;
	static uint32_t yDst;
	static uint32_t widthDst;
	static uint32_t heightDst;

	// oversampled source bitmap that will be bitblited ; margins included
	static uint32_t* bmpScaled;
	static uint32_t bmpSrcSize;
	static uint32_t widthSrc;
	static uint32_t heightSrc;
	static uint32_t leftMargin;
	static uint32_t rightMargin;
	static uint32_t topMargin;
	static uint32_t bottomMargin;
	static uint32_t mul;	// oversampling factor (<=> quality)

	static void updateDimensions();
	static void updateBmpSrc(); // update oversampled source bitmap bifore bitbliting

public:

	static void init();
	static void paint(); // bitblt
	static void close();
	static bool isOpen() { return bmpScaled != NULL; }
};

