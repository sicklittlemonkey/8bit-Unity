/*
 * Copyright (c) 2018 Anthony Beaucamp.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented * you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 */
 
#include "../unity.h"

// See bitmap.c and sprites.c
extern SCB_REHV_PAL bitmapTGI;
extern LynxSprite spriteSlot[SPRITE_NUM];
extern unsigned char sprEN[SPRITE_NUM];

// Workaround for missing char printing (including palette remapping)
unsigned char textColors[] = { BLACK, RED, PINK, GREY, GREY, GREY, BROWN, ORANGE, YELLOW, LGREEN, GREEN, DRED, PURPLE, BLUE, LBLUE, WHITE };
unsigned char chrCol, chrRow;
void clrscr(void) {
	InitBitmap();
	ClearBitmap();
}
unsigned char textcolor(unsigned char color) {
	inkColor = textColors[color];
}
unsigned char bgcolor(unsigned char color) {
	paperColor = textColors[color];
}
unsigned char bordercolor(unsigned char color) {
	// Do nothing
}
void gotoxy(unsigned char col, unsigned char row) {
	chrCol = col;
	chrRow = row;
}
int cprintf (const char* format, ...) {
	PrintStr(chrCol, chrRow, format);
}

// update display routine
void UpdateDisplay(void)
{
	unsigned char i, j;

	// Wait for previous drawing to complete then reset collisions
	while (tgi_busy()) {}
	
	// Send bitmap and sprites (in reverse order) to Suzy
	tgi_sprite(&bitmapTGI);
	for (j=0; j<SPRITE_NUM; j++) {
		i = (SPRITE_NUM-1) - j;
		if (sprEN[i]) { 
			tgi_sprite(&spriteSlot[i].scb);
		}
	}
	tgi_updatedisplay();
}

// Workaround for missing network
unsigned int  udp_packet;
unsigned char InitNetwork(void) { return 0; }							// Initialize network interface and get IP from DHCP
void InitUDP(unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned int svPort, unsigned int clPort) { }	// Setup UDP connection
void SendUDP(unsigned char* buffer, unsigned char length) { }  // Send UDP packet (of specified length)
unsigned char RecvUDP(unsigned int timeOut) { return 0;}	   // Fetch UDP packet (within time-out period)
