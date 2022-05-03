/*

	MarlinWnd.h

*/

#include <math.h>

#include "MarlinWnd.h"
#include "Settings.h"
#include "Errors.h"
#include "Util.h"
#include "MarlinBridge.h"

const uint32_t PRIORITY				= 10;
const uint32_t LAYER				= 100;
const VC_IMAGE_TYPE_T IMAGE_TYPE	= VC_IMAGE_ARGB8888; // 32bit + alpha, 8 8 8 8

// source bitmap (overssampled), including margins
uint32_t* CMarlinWnd::bmpSrc		= NULL;
uint32_t CMarlinWnd::bmpSrcSize		= 0;
uint32_t CMarlinWnd::leftMargin		= DEFAULT_MARGIN;
uint32_t CMarlinWnd::rightMargin	= DEFAULT_MARGIN;
uint32_t CMarlinWnd::topMargin		= DEFAULT_MARGIN;
uint32_t CMarlinWnd::bottomMargin	= DEFAULT_MARGIN;
uint32_t CMarlinWnd::widthSrc		= 0;
uint32_t CMarlinWnd::heightSrc		= 0;
uint32_t CMarlinWnd::mul			= 1; // source bitmap "oversampling" : medium

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
// inits dispmanx

void CMarlinWnd::init()
{
	mul = 4 * (settings[IMG_QUALITY].val + 1);

	leftMargin		= DEFAULT_MARGIN;
	rightMargin		= DEFAULT_MARGIN;
	topMargin		= DEFAULT_MARGIN;
	bottomMargin	= DEFAULT_MARGIN;

	if (graphics_get_display_size(0, &widthScreen, &heightScreen) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// Marlin mode ; preserves aspect ratio
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
	if (bmpSrc == NULL && (bmpSrc = (uint32_t*)malloc(bmpSrcSize)) == NULL)
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
// calculates the bitmap dimensions MarlinUI will be copied to before bitbliting

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
// terminates the overlay window

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
	if (bmpSrc == NULL)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	free(bmpSrc);
	bmpSrc = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prepares a new bitmap for bitbliting

void CMarlinWnd::updateBmpSrc()
{
	uint32_t* dst = (uint32_t*)bmpSrc;
	const uint8_t* src = CMarlinBridge::spiBuffer;

	uint32_t fgnd_clr;
	uint32_t bkgnd_clr;

	// top margin
	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		themeDict::getColors(settings[THEME].val, fgnd_clr, bkgnd_clr);

		for (uint32_t i = 0; i < widthSrc * (topMargin - DEFAULT_MARGIN) * mul; i++)
			*dst++ = SOLID_BLACK;

		for (uint32_t i = 0; i < widthSrc * DEFAULT_MARGIN * mul; i++)
			*dst++ = bkgnd_clr;
	}
	else
	{
		fgnd_clr = settings[FGND_COLOR].val | (percentToByte(settings[FGND_OPACITY].val) << 24);
		bkgnd_clr = settings[BKGND_COLOR].val | (percentToByte(settings[BKGND_OPACITY].val) << 24);

		for (uint32_t i = 0; i < widthSrc * topMargin * mul; i++)
			*dst++ = bkgnd_clr;
	}

	for (uint32_t y = 0; y < LCD_HEIGHT; y++, src += LCD_WIDTH / 8) // src = 8 pixels / byte
	{
		for (uint32_t iLine = 0; iLine < mul; iLine++) // repeat line mul times
		{
			for (uint32_t i = 0; i < mul * leftMargin; i++) // left margin
				*dst++ = bkgnd_clr;

			const uint8_t* tmp = src;

			for (uint32_t b = 0; b < LCD_WIDTH / 8; b++)
			{
				uint8_t srcByte = *tmp++;

				for (int32_t j = 7; j >= 0; j--) // 8 pixels
					for (uint32_t k = 0; k < mul; k++) // reapeat pixel mul times
						*dst++ = srcByte & (1 << j) ? fgnd_clr : bkgnd_clr;
			}

			for (uint32_t i = 0; i < mul * rightMargin; i++) // right margin
				*dst++ = bkgnd_clr;
		}
	}

	// botom margin
	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		for (uint32_t i = 0; i < widthSrc * DEFAULT_MARGIN * mul; i++)
			*dst++ = bkgnd_clr;

		for (uint32_t i = 0; i < widthSrc * (bottomMargin - DEFAULT_MARGIN) * mul; i++)
			*dst++ = SOLID_BLACK;
	}
	else
	{
		for (uint32_t i = 0; i < widthSrc * bottomMargin * mul; i++)
			*dst++ = bkgnd_clr;
	}

	if (dst - bmpSrc != (int)(widthSrc * heightSrc))
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bitblt

void CMarlinWnd::paint()
{
	updateBmpSrc();

	// bitblits bmpSrc to screen :

	DISPMANX_UPDATE_HANDLE_T update;
	int32_t pitch;
	VC_RECT_T rect;

	if((update = vc_dispmanx_update_start(PRIORITY)) == 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	// foreground
	pitch = ALIGN_UP(widthSrc * 4, 32);

	vc_dispmanx_rect_set(&rect, 0, 0, widthSrc, heightSrc);

	if((vc_dispmanx_resource_write_data(res, IMAGE_TYPE, pitch, bmpSrc, &rect)) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);

	if ((vc_dispmanx_update_submit_sync(update)) != 0)
		exitError(ERROR_DISPMANX, __FILE__, __LINE__);
}
