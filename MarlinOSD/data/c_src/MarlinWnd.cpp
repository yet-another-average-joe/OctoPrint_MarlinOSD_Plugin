/*

	MarlinWnd.h

*/

#include <math.h>

#include "MarlinWnd.h"
#include "Settings.h"
#include "Errors.h"
#include "Util.h"
#include "MarlinBridge.h"

const uint32_t PRIORITY =  10;
const uint32_t LAYER				= 100;
const VC_IMAGE_TYPE_T IMAGE_TYPE	= VC_IMAGE_ARGB8888; // 32bit + alpha, 8 8 8 8

// source bitmap (overssampled), including margins
uint32_t* CMarlinWnd::bmpScaled		= NULL;
uint32_t CMarlinWnd::bmpSrcSize		= 0;
uint32_t CMarlinWnd::leftMargin		= DEFAULT_MARGIN;
uint32_t CMarlinWnd::rightMargin	= DEFAULT_MARGIN;
uint32_t CMarlinWnd::topMargin		= DEFAULT_MARGIN;
uint32_t CMarlinWnd::bottomMargin	= DEFAULT_MARGIN;
uint32_t CMarlinWnd::widthSrc		= 0;
uint32_t CMarlinWnd::heightSrc		= 0;
uint32_t CMarlinWnd::mul			= 4;	// source bitmap "oversampling" : 4 = low ; overridden by settings
											// tricky : must bee a multiple of 4 and > 4

// dispmanx
DISPMANX_DISPLAY_HANDLE_T	CMarlinWnd::display		= 0;
DISPMANX_RESOURCE_HANDLE_T	CMarlinWnd::res			= 0;
DISPMANX_ELEMENT_HANDLE_T	CMarlinWnd::element		= 0;

// screen
uint32_t CMarlinWnd::widthScreen	= 0;
uint32_t CMarlinWnd::heightScreen	= 0;

// viewport
uint32_t CMarlinWnd::xDst		= 0;
uint32_t CMarlinWnd::yDst		= 0;
uint32_t CMarlinWnd::widthDst	= 0;
uint32_t CMarlinWnd::heightDst	= 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// init dispmanx

void CMarlinWnd::init()
{
	mul = 4 * (settings[GFX_QUALITY].val + 1);

	leftMargin		= DEFAULT_MARGIN;
	rightMargin		= DEFAULT_MARGIN;
	topMargin		= DEFAULT_MARGIN;
	bottomMargin	= DEFAULT_MARGIN;

	if (graphics_get_display_size(0, &widthScreen, &heightScreen) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// Marlin mode ; preserves aspect ratio ; equation =
	// (topMargin + LCD_HEIGHT + bottomMargin) / (DEFAULT_MARGIN + LCD_WIDTH + DEFAULT_MARGIN) = heightScreen / widthScreen
	
	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		topMargin = bottomMargin = ((heightScreen * (LCD_WIDTH + 2 * DEFAULT_MARGIN) / widthScreen - LCD_HEIGHT) / 2) + 2; // +2 = rounding adjustments
		leftMargin = rightMargin = DEFAULT_MARGIN;
	}

	widthSrc = (LCD_WIDTH + leftMargin + rightMargin) * mul;
	heightSrc = (LCD_HEIGHT + topMargin + bottomMargin) * mul;

	bmpSrcSize = sizeof(uint32_t) * widthSrc * heightSrc;

	// allocate or reallocate source bitmap
	if (bmpScaled == NULL && (bmpScaled = (uint32_t*)malloc(bmpSrcSize)) == NULL)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if((display = vc_dispmanx_display_open(0)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// source and destination rectangles
	updateDimensions();

	// initialize dispmanx resources and elements
	VC_DISPMANX_ALPHA_T alpha;
	alpha.flags = (DISPMANX_FLAGS_ALPHA_T)(DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_MIX); // DISPMANX_FLAGS_ALPHA_MIX -> Window opacity
	alpha.mask = 0;

	alpha.opacity = 0xFF;

	DISPMANX_UPDATE_HANDLE_T update;
	VC_RECT_T src_rect;
	VC_RECT_T dst_rect;
	uint32_t img_ptr;

	if((res = vc_dispmanx_resource_create(IMAGE_TYPE, widthSrc, heightSrc, &img_ptr)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if((update = vc_dispmanx_update_start(PRIORITY)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	vc_dispmanx_rect_set(&src_rect, 0, 0, widthSrc << 16, heightSrc << 16);
	//vc_dispmanx_rect_set(&dst_rect, xDst, yDst, widthDst, heightDst);
	vc_dispmanx_rect_set(&dst_rect, xDst, yDst, ALIGN_UP(widthDst, 32), heightDst);
	element = vc_dispmanx_element_add(update, display, LAYER, &dst_rect, res, &src_rect,
											DISPMANX_PROTECTION_NONE, &alpha, NULL, DISPMANX_NO_ROTATE);
	if(element == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if(vc_dispmanx_update_submit_sync(update) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate the bitmap dimensions MarlinUI will be copied to before bitbliting

void CMarlinWnd::updateDimensions()
{
	// background rectangle
	switch (settings[DISPLAY_MODE].val)
	{
	case DISPLAY_MODE_CUSTOM:
		widthDst = (uint32_t)((float)widthScreen * (float)settings[SIZE].val / 100.0);
		heightDst = widthDst / 2; // Marlin UI aspect ratio = 2:1 ; margins effect on real aspect ratio is neglected
		break;

	case DISPLAY_MODE_MARLIN:
	default: // should not happen...
		widthDst = widthScreen;
		heightDst = heightScreen;
		xDst = 0;
		yDst = 0;
		break;
	}

	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_CUSTOM)
	{
		switch (settings[POSITION].val)
		{
		case POS_TOP_LEFT:
			xDst = 0;
			yDst = 0;
			break;
		case POS_TOP_CENTER:
			xDst = (widthScreen - widthDst) / 2;
			yDst = 0;
			break;
		case POS_TOP_RIGHT:
			xDst = widthScreen - widthDst;
			yDst = 0;
			break;
		case POS_CENTER_LEFT:
			xDst = 0;
			yDst = (heightScreen - heightDst) / 2;
			break;
		case POS_CENTER_CENTER:
			xDst = (widthScreen - widthDst) / 2;
			yDst = (heightScreen - heightDst) / 2;
			break;
		case POS_CENTER_RIGHT:
			xDst = widthScreen - widthDst;
			yDst = (heightScreen - heightDst) / 2;
			break;
		case POS_BOTTOM_LEFT:
			xDst = 0;
			yDst = heightScreen - heightDst;
			break;
		case POS_BOTTOM_CENTER:
			xDst = (widthScreen - widthDst) / 2;
			yDst = heightScreen - heightDst;
			break;
		case POS_BOTTOM_RIGHT:
			xDst = widthScreen - widthDst;
			yDst = heightScreen - heightDst;
			break;
		default:
			exitError(ERROR_DISPMANX, __FILE__, __LINE__);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// terminate the overlay window

void CMarlinWnd::close()
{
	DISPMANX_UPDATE_HANDLE_T update;

	if ((update = vc_dispmanx_update_start(PRIORITY)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// foreground
	if (vc_dispmanx_element_remove(update, element) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if (vc_dispmanx_resource_delete(res) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// display
	if (vc_dispmanx_update_submit_sync(update) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if (vc_dispmanx_display_close(display) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// source bitmap
	if (bmpScaled == NULL)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	free(bmpScaled);
	bmpScaled = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prepare a new (oversampled) bitmap for bitbliting

void CMarlinWnd::updateBmpSrc()
{
	uint32_t* dst = (uint32_t*)bmpScaled;
	const uint8_t* src = CMarlinBridge::spiBuffer;

	// top margin
	size_t n;

	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		n = widthSrc * (topMargin - DEFAULT_MARGIN) * mul;
		do { *dst++ = SOLID_BLACK; n--; } while (n);
	}

	n = widthSrc * DEFAULT_MARGIN * mul;
	do { *dst++ = bkgndColor; n--; } while (n);

	// "iterators"
	size_t mul_leftMargin = mul * leftMargin;
	size_t mul_rightMargin = mul * rightMargin;

	for (uint32_t y = 0; y < LCD_HEIGHT; y++, src += LCD_WIDTH / 8) // src = 8 pixels / byte
	{
		for (uint32_t iLine = 0; iLine < mul; iLine++) // repeat line mul times
		{
			size_t n = mul_leftMargin;
			do { *dst++ = bkgndColor; n--; } while (n); // left margin

			const uint8_t* tmp = src + 1;
			uint8_t srcByte = *src;

			for (uint32_t b = 0; b < LCD_WIDTH / 8; b++, srcByte = *tmp++)
				for (int32_t j = 7; j >= 0; j--) // 8 pixels
					for (uint32_t k = 0; k < mul; k++) // reapeat pixel mul times
						*dst++ = srcByte & (1 << j) ? fgndColor : bkgndColor;

			size_t m = mul_rightMargin;
			do { *dst++ = bkgndColor; m--; } while (m); // right margin
		}
	}

	n = widthSrc * DEFAULT_MARGIN * mul;
	do { *dst++ = bkgndColor; n--; } while (n);

	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		n = widthSrc * (bottomMargin - DEFAULT_MARGIN) * mul;
		do { *dst++ = SOLID_BLACK; n--; } while (n);
	}

	if (dst - bmpScaled != (int)(widthSrc * heightSrc))
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bitblt

void CMarlinWnd::paint()
{
	updateBmpSrc();

	// bitblits bmpScaled to screen :

	DISPMANX_UPDATE_HANDLE_T update;
	int32_t pitch;
	VC_RECT_T rect;

	if((update = vc_dispmanx_update_start(PRIORITY)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// foreground
	pitch = ALIGN_UP(widthSrc * 4, 32);

	vc_dispmanx_rect_set(&rect, 0, 0, widthSrc, heightSrc);

	if((vc_dispmanx_resource_write_data(res, IMAGE_TYPE, pitch, bmpScaled, &rect)) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if ((vc_dispmanx_update_submit_sync(update)) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);
}
