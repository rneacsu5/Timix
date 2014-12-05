/*

	colortext library
	Copyright (c) 2014 Neacsu Razvan

	A simple library that allows you to print colored text in your console with ease.
	For now just Windows is supported. 

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define COLORTEXT_IMPLEMENTATION

	Note: For now this only works on Windows
	Note: Everything prefixed with "ctxtp_" or "CTXTP_" (stands for "colortext private") should not be used or else problems might occur.
*/

#ifndef CTXT_HEADER_INCLUDED

#define CTXT_HEADER_INCLUDED

#ifdef _WIN32

#include <stdio.h>
#include <Windows.h>

// All the colors
#define CTXT_FOREGROUND_BLACK					0
#define CTXT_FOREGROUND_BLUE					FOREGROUND_BLUE
#define CTXT_FOREGROUND_GREEN											FOREGROUND_GREEN
#define CTXT_FOREGROUND_AQUA					FOREGROUND_BLUE		|	FOREGROUND_GREEN
#define CTXT_FOREGROUND_RED																		FOREGROUND_RED
#define CTXT_FOREGROUND_PURPLE					FOREGROUND_BLUE								|	FOREGROUND_RED
#define CTXT_FOREGROUND_YELLOW											FOREGROUND_GREEN	|	FOREGROUND_RED
#define CTXT_FOREGROUND_WHITE					FOREGROUND_BLUE		|	FOREGROUND_GREEN	|	FOREGROUND_RED

#define CTXT_FOREGROUND_GRAY																						FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_BLUE 				FOREGROUND_BLUE													|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_GREEN 									FOREGROUND_GREEN						|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_AQUA 				FOREGROUND_BLUE		|	FOREGROUND_GREEN						|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_RED 																FOREGROUND_RED	|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_PURPLE 			FOREGROUND_BLUE								|	FOREGROUND_RED	|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_LIGHT_YELLOW 									FOREGROUND_GREEN	|	FOREGROUND_RED	|	FOREGROUND_INTENSITY
#define CTXT_FOREGROUND_BRIGHT_WHITE 			FOREGROUND_BLUE		|	FOREGROUND_GREEN	|	FOREGROUND_RED	|	FOREGROUND_INTENSITY

#define CTXT_BACKGROUND_BLACK					0
#define CTXT_BACKGROUND_BLUE					BACKGROUND_BLUE
#define CTXT_BACKGROUND_GREEN											BACKGROUND_GREEN
#define CTXT_BACKGROUND_AQUA					BACKGROUND_BLUE		|	BACKGROUND_GREEN
#define CTXT_BACKGROUND_RED																		BACKGROUND_RED
#define CTXT_BACKGROUND_PURPLE					BACKGROUND_BLUE								|	BACKGROUND_RED
#define CTXT_BACKGROUND_YELLOW											BACKGROUND_GREEN	|	BACKGROUND_RED
#define CTXT_BACKGROUND_WHITE					BACKGROUND_BLUE		|	BACKGROUND_GREEN	|	BACKGROUND_RED

#define CTXT_BACKGROUND_GRAY																						BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_BLUE 				BACKGROUND_BLUE													|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_GREEN 									BACKGROUND_GREEN						|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_AQUA 				BACKGROUND_BLUE		|	BACKGROUND_GREEN						|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_RED 																BACKGROUND_RED	|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_PURPLE 			BACKGROUND_BLUE								|	BACKGROUND_RED	|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_LIGHT_YELLOW 									BACKGROUND_GREEN	|	BACKGROUND_RED	|	BACKGROUND_INTENSITY
#define CTXT_BACKGROUND_BRIGHT_WHITE 			BACKGROUND_BLUE		|	BACKGROUND_GREEN	|	BACKGROUND_RED	|	BACKGROUND_INTENSITY

// Main color changing function. Use any of the colors constants from this library. You can combine foreground and background colors with |
void ctxt_ChangeColor(WORD color);

// Restore original color set when the application started
void ctxt_RestoreColor(void);

// Implementation
#ifdef COLORTEXT_IMPLEMENTATION

static HANDLE ctxtp_consoleHandle = 0;
static WORD ctxtp_originalColor;
static int ctxtp_firstUsed = 1;

// Get handle and original color
static void ctxtp_init(void)
{
	ctxtp_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(ctxtp_consoleHandle, &consoleInfo);
	ctxtp_originalColor = consoleInfo.wAttributes;
	ctxtp_firstUsed = 0;
}

// Main color changing function. Use any of the colors constants from this library. You can combine foreground and background colors with |
void ctxt_ChangeColor(WORD color)
{
	if (ctxtp_firstUsed) ctxtp_init();
	SetConsoleTextAttribute(ctxtp_consoleHandle, color);
}

// Restore original color set when the application started
void ctxt_RestoreColor(void)
{
	if (ctxtp_firstUsed) ctxtp_init();
	SetConsoleTextAttribute(ctxtp_consoleHandle, ctxtp_originalColor);
}

#endif // CTXT_IMPLEMENTATION

#endif // _WIN32

#endif // !CTXT_HEADER_INCLUDED
