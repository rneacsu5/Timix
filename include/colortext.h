/*

	colortext library
	Copyright (c) 2014 Neacsu Razvan

	A simple library that allows you to print colored (and styled) text in your console with ease.

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define COLORTEXT_IMPLEMENTATION

	Note: Everything prefixed with "ctxtp_" or "CTXTP_" (stands for "colortext private") should not be used or else problems might occur.
	Note: Some styles or colors might not work. This means your console doesn't support them.
	Note: Styling does not work on Windows.
*/

#ifndef CTXT_HEADER_INCLUDED

#define CTXT_HEADER_INCLUDED

#ifdef _WIN32

#include <Windows.h>

#endif // _WIN32

// Bit codes
#define CTXTP_D			0x0001 // Default
#define CTXTP_B			0x0002 // Contains blue
#define CTXTP_G			0x0004 // Contains green
#define CTXTP_R			0x0008 // Contains red
#define CTXTP_L			0x0010 // Lighter color


// All colors
#define CTXT_DEFAULT		CTXTP_D
#define CTXT_BLACK			0
#define CTXT_RED			CTXTP_R
#define CTXT_GREEN							CTXTP_G
#define CTXT_YELLOW			CTXTP_R		|	CTXTP_G
#define CTXT_BLUE											CTXTP_B
#define CTXT_MAGENTA		CTXTP_R						|	CTXTP_B
#define CTXT_CYAN							CTXTP_G		|	CTXTP_B
#define CTXT_LIGHT_GRAY		CTXTP_R		|	CTXTP_G		|	CTXTP_B

#define CTXT_DARK_GRAY														CTXTP_L
#define CTXT_LIGHT_RED		CTXTP_R										|	CTXTP_L
#define CTXT_LIGHT_GREEN					CTXTP_G						|	CTXTP_L
#define CTXT_LIGHT_YELLOW	CTXTP_R		|	CTXTP_G						|	CTXTP_L
#define CTXT_LIGHT_BLUE										CTXTP_B		|	CTXTP_L
#define CTXT_LIGHT_MAGENTA	CTXTP_R						|	CTXTP_B		|	CTXTP_L
#define CTXT_LIGHT_CYAN						CTXTP_G		|	CTXTP_B		|	CTXTP_L
#define CTXT_WHITE			CTXTP_R		|	CTXTP_G		|	CTXTP_B		|	CTXTP_L

// Special styles
#define CTXT_BOLD			111 // 1 is already used by CTXT_DEFAULT
#define CTXT_DIM			2
#define CTXT_UNDERLINED		4
#define CTXT_BLINK			5
#define CTXT_REVERSE		7
#define CTXT_HIDDEN			8

// Structure that holds the foreground, background color and special styles
typedef struct {
	unsigned short foreColor;
	unsigned short backColor;
	unsigned short special;
}ctxt_Color;

// Main color changing function. Use ctxt_CreateColor to create the color
void ctxt_ChangeColor(ctxt_Color color);

// Restore original/default color
void ctxt_RestoreColor(void);

// Print the given text to the console in the given color
void ctxt_PrintColored(ctxt_Color color, const char* format, ...);

// Returns a ctxt_Color structure. Use CTXT_* constants. Combine them with |. Use 1 for console default
ctxt_Color ctxt_CreateColor(unsigned short foregroundColor, unsigned short backgroundColor, unsigned short special);

// Implementation
#ifdef COLORTEXT_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32 // Windows is a bit diffrent

static HANDLE ctxtp_consoleHandle = 0;
static WORD ctxtp_textAttributes;
static int ctxtp_firstUsed = 1;

// Get handle and original color
static void ctxtp_init(void)
{
	ctxtp_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(ctxtp_consoleHandle, &consoleInfo);
	ctxtp_textAttributes = consoleInfo.wAttributes;
	ctxtp_firstUsed = 0;
}

// Main color changing function. Use ctxt_CreateColor to create the color
void ctxt_ChangeColor(ctxt_Color color)
{
	if (ctxtp_firstUsed) ctxtp_init();

	short newAttr = ctxtp_textAttributes;
	if (!(color.foreColor & CTXT_DEFAULT)) {
		// Set new foreground
		newAttr = newAttr / (1 << 4) + (color.foreColor >> 1);
	}
	if (!(color.backColor & CTXT_DEFAULT)) {
		// Set new background
		newAttr = newAttr / (1 << 8) + ((color.backColor >> 1) << 4) + newAttr % (1 << 4);
	}

	SetConsoleTextAttribute(ctxtp_consoleHandle, newAttr);
}

// Restore original/default color
void ctxt_RestoreColor(void)
{
	if (ctxtp_firstUsed) ctxtp_init();
	SetConsoleTextAttribute(ctxtp_consoleHandle, ctxtp_textAttributes);
}

#else // Use ANSI color codes

// Main color changing function. Use ctxt_CreateColor to create the color
void ctxt_ChangeColor(ctxt_Color color)
{
	// Get foreground color code
	int foreCode = 30; // Black
	if (color.foreColor & CTXTP_R) foreCode += 1;
	if (color.foreColor & CTXTP_G) foreCode += 2;
	if (color.foreColor & CTXTP_B) foreCode += 4;
	foreCode += (color.foreColor & CTXTP_L) ? 60 : 0;
	if (color.foreColor & CTXTP_D) foreCode = 39;

	// Get background color code
	int backCode = 40; // Black
	if (color.backColor & CTXTP_R) backCode += 1;
	if (color.backColor & CTXTP_G) backCode += 2;
	if (color.backColor & CTXTP_B) backCode += 4;
	backCode += (color.backColor & CTXTP_L) ? 60 : 0;
	if (color.backColor & CTXTP_D) backCode = 49;

	// Get special code
	int specialCode = color.special;
	if (specialCode == 1) specialCode = 0;
	if (specialCode == CTXT_BOLD) specialCode = 1;

	printf("\e[%d;%d;%dm", specialCode, foreCode, backCode);
}

// Restore original/default color
void ctxt_RestoreColor(void)
{
	printf("\e[0m");
}

#endif // _WIN32

// Print the given text to the console in the given color
void ctxt_PrintColored(ctxt_Color color, const char* format, ...)
{
	// Change color
	ctxt_ChangeColor(color);

	// Print text
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	// Restore color
	ctxt_RestoreColor();
}

// Returns a ctxt_Color structure. Use CTXT_* constants. Combine them with |. Use 1 for console default
ctxt_Color ctxt_CreateColor(unsigned short foregroundColor, unsigned short backgroundColor, unsigned short special) {
	ctxt_Color newColor;
	newColor.foreColor = foregroundColor;
	newColor.backColor = backgroundColor;
	newColor.special = special;
	return newColor; 
}

#endif // CTXT_IMPLEMENTATION

#endif // !CTXT_HEADER_INCLUDED
