/* 
 * QR Code generator demo (C++)
 * 
 * Run this command-line program with no arguments. The program computes a bunch of demonstration
 * QR Codes and prints them to the console. Also, the SVG code for one QR Code is printed as a sample.
 * 
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */
#include <windows.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "QrCode.hpp"
#include "SDL2/SDL.h"
#include <bitset>
using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;
const int WINDOW_W = 640;
const int WINDOW_H = 480;
// Function prototypes
static void show_usage(std::string name);
static void doBasicDemo(SDL_Window *window, SDL_Surface *surface, std::string text);
static void printQr(const QrCode &qr);
static void printQr1(const QrCode &qr);
static void printQr2(const QrCode &qr);
static void printQr3(const QrCode &qr, const int border, SDL_Surface * target);

// The main application program.
#undef main
int main(int argc, char** argv) {

/*---- SDL Init ----*/
SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("gradient",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_W, WINDOW_H, 0);
		
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	
	std::bitset<8> mybitset {0000000};
	std::string text = "https:\\\\google.pl\\";
	std::vector<std::string> allArgs(argv, argv + argc);
	for (std::vector<std::string>::const_iterator i = allArgs.begin() + 1; i != allArgs.end(); ++i) {
		std::string temp(*i);
		if (temp == "-l" && (i+1) != allArgs.end() ) {
			text = *(i +1);
		} else if (temp == "-svg" /*&& (i+1)!= allArgs.end()*/ ) {
			mybitset |= 1<<0;
			std::cout <<mybitset;
		} else if (temp == "-bmp") {
			mybitset |= 1<<1;
			std::cout <<mybitset;
		} else if (temp == "-h" || temp == "--help") {
			show_usage(std::string (argv[0]));
			return 0;
		} else if (temp == "-d" || temp == "--demo") {
			std::cout << "Runninng demo link:" << std::endl;
		} else if (temp == "-v" || temp == "--verbose") {
			mybitset |= 1<<2;
			std::cout <<mybitset;
		} else {
			std::cout << "Uknown parameter \"" << *i << "\"" <<std::endl;
			show_usage(std::string (argv[0]));
			return 1;
		}
	}
	doBasicDemo(window, surface, text);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
/*---- Demo suite ----*/

// Creates a single QR Code, then prints it to the console.
static void doBasicDemo(SDL_Window * window, SDL_Surface *surface, std::string text) {
	const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level
	
	const char *text_to_qr = text.c_str();
	// Make and print the QR Code symbol
	const QrCode qr = QrCode::encodeText(text_to_qr, errCorLvl);
	//printQr(qr);
	printQr1(qr);
	//printQr2(qr);
	std::cout << qr.toSvgString(4) << std::endl;
	std::ofstream f("code.svg");
	if(f) f << qr.toSvgString(4);
	
	const int W = qr.getSize()+8;
    const int H = qr.getSize()+8;
	const int W2 = 256;
    const int H2 = 256;
	static SDL_Surface *gradient = SDL_CreateRGBSurface(
		0, W, H, 24, 0xff << 0, 0xff << 8, 0xff << 16, 0);
	
	printQr3(qr, 4, gradient);
	
	SDL_Surface *temp = SDL_CreateRGBSurface(
		0, W2, H, 24 ,0xff << 0, 0xff << 8, 0xff << 16, 0);
	
	uint8_t *pixels = static_cast<uint8_t*>(gradient->pixels);
	uint8_t *pixels2 = static_cast<uint8_t*>(temp->pixels);
	int multipler = H2/H;
	int k = 0;
	for (short i = 0; i<H; i++) {
		k = 0;
		for (short j = 0; j < W; j++) {
			for (short l = 1; l<=multipler; l++) {			
			pixels2[k * 3 + i * temp->pitch + 0] = pixels[j * 3 + i * gradient->pitch + 0];
			pixels2[k * 3 + i * temp->pitch + 1] = pixels[j * 3 + i * gradient->pitch + 1];
			pixels2[k * 3 + i * temp->pitch + 2] = pixels[j * 3 + i * gradient->pitch + 2]; 
			k++;
			}
		}		
	}
	
	SDL_Surface *target = SDL_CreateRGBSurface(
		0, W2, H2, 24 ,0xff << 0, 0xff << 8, 0xff << 16, 0);
	uint8_t *pixels3 = static_cast<uint8_t*>(target->pixels);
	
	for (short i = 0; i<multipler*H; i++) {
		k = 0;
		for (short j = 0; j < W; j++) {
			for (short l = 1; l<=multipler; l++) {			
			pixels3[i * 3 + k * target->pitch + 0] = pixels2[i * 3 + j * temp->pitch + 0];
			pixels3[i * 3 + k * target->pitch + 1] = pixels2[i * 3 + j * temp->pitch + 1];
			pixels3[i * 3 + k * target->pitch + 2] = pixels2[i * 3 + j * temp->pitch + 2]; 
			k++;
			}
		}		
	}
	
	SDL_Rect pos = {
		(WINDOW_W - W2) / 2, (WINDOW_H - H2) / 2, W2, H2};
	
	SDL_BlitSurface(target, NULL, surface, &pos);
	
	SDL_UpdateWindowSurface(window);
	bool shutdown = false;
	while (!shutdown){
	
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if ((event.type == SDL_KEYDOWN && event.key.keysym.sym ==
				SDLK_ESCAPE) || event.type == SDL_QUIT) {
					shutdown = true;
					break;
			}
		}
	}
	SDL_FreeSurface(gradient);
}

/*---- Utilities ----*/

// Prints the given QrCode object to the console.
static void printQr(const QrCode &qr) {
	int border = 4;
	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			std::cout << (qr.getModule(x, y) ? "##" : "  ");
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
static void printQr1(const QrCode &qr) {
	int border = 4;
	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			std::cout << (qr.getModule(x, y) ? "[ 1 ]" : "[ 0 ]");
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
static void printQr2(const QrCode &qr) {
	int border = 4;
	 HDC hdc = GetDC(GetConsoleWindow());
	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			if (!qr.getModule(x, y)) {
			
			SetPixel(hdc, x+ border, y + border, RGB(0, 0, 0));
			}	else  {
			SetPixel(hdc, x+ border, y+ border, RGB(255, 255, 255));
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
static void printQr3(const QrCode &qr, const int border, SDL_Surface * target) {
	
	uint8_t *pixels = static_cast<uint8_t*>(target->pixels);

	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			if (!qr.getModule(x,y)){
				pixels[(x+4) * 3 + (y+4) * target->pitch + 0] = 0xff;
				pixels[(x+4) * 3 + (y+4) * target->pitch + 1] = 0xff;
				pixels[(x+4) * 3 + (y+4) * target->pitch + 2] = 0xff; 
			} else {
				pixels[(x+4) * 3 + (y+4) * target->pitch + 0] = 0x0;
				pixels[(x+4) * 3 + (y+4) * target->pitch + 1] = 0x0;
				pixels[(x+4) * 3 + (y+4) * target->pitch + 2] = 0x0; 
			}
		}
	}
	return;
}

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)> SOURCES\n"
              << "Options:\n"
              << "\t-h,--help\tShow this help message\n"
              << "\t-l,--link LINK\tSpecify the link to convert\n"
			  << "\t-svg\t\tSave output as SVG file\n"
			  << "\t-bmp\t\tSave output as BMP file\n"
			  << "\t-d,--demo\t\tRun demo link\n"
              << std::endl;
}