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
#include <math.h> 
//#include <cstdint>
//#include <cstdlib>
//#include <cstring>
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

enum Dimension {
	Columns = 0,
	Rows
};
// Function prototypes
static void show_usage(std::string name);
static void save_bmp(SDL_Surface * sfc);
static void save_svg(const QrCode qr, int border);
static void doBasicDemo(SDL_Window *window, SDL_Surface *surface, std::string text, int margin);
static void printQr(const QrCode &qr, int margin);
static void printQr1(const QrCode &qr, int margin);
static void printQr3(const QrCode &qr, const int border, SDL_Surface * target);
static void resize_one_dimension(Dimension dim, SDL_Surface * source, int multipler, int width, int hight, SDL_Surface * dest);

// The main application program.
#undef main
int main(int argc, char** argv) {

/*---- SDL Init ----*/
SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("gradient",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_W, WINDOW_H, 0);
		
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 0, 0));
	
	std::bitset<8> mybitset {0000000};
	std::string text = "https://google.pl/";
	int margin{0};
	std::vector<std::string> allArgs(argv, argv + argc);
	for (std::vector<std::string>::const_iterator i = allArgs.begin() + 1; i != allArgs.end(); ++i) {
		std::string temp(*i);
		if ((temp == "-l" || temp == "-link" )&& (i+1) != allArgs.end() ) {
			text = *(++i);
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
		} else if ((temp == "-m" || temp == "--margin") && (i+1) != allArgs.end()) {
			if (isdigit(margin) && margin >=0 && margin <=10)
				margin = std::stoi(*(++i));
			else {
				std::cout << "Invalid margin thickness <0;10> or not a number!!!"<< std::endl;
				return 1;
			}
		} else if (temp == "-v" || temp == "--verbose") {
			mybitset |= 1<<2;
			std::cout <<mybitset;
		} else {
			std::cout << "Uknown parameter \"" << *i << "\"" <<std::endl;
			show_usage(std::string (argv[0]));
			return 1;
		}
	}
	doBasicDemo(window, surface, text, margin);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
/*---- Demo suite ----*/

// Creates a single QR Code, then prints it to the console.
static void doBasicDemo(SDL_Window * window, SDL_Surface *surface, std::string text, int margin) {
	const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level
	
	const char *text_to_qr = text.c_str();
	// Make and print the QR Code symbol
	const QrCode qr = QrCode::encodeText(text_to_qr, errCorLvl);
	//printQr(qr, margin);
	printQr1(qr, margin);
	std::cout << qr.toSvgString(margin) << std::endl;
	save_svg(qr, margin);
	const int W = qr.getSize()+2*margin;
    const int H = qr.getSize()+2*margin;
	std::cout << Dimension::Columns << Dimension::Rows;
	const int W2 = W*8;
    const int H2 = H*8;
	const int multipler = floor(H2/H);
	static SDL_Surface *gradient = SDL_CreateRGBSurface(
		0, W, H, 24, 0xff << 0, 0xff << 8, 0xff << 16, 0);
	
	printQr3(qr, margin, gradient);
	
	SDL_Surface *temp = SDL_CreateRGBSurface(
		0, W2, H, 24 ,0xff << 0, 0xff << 8, 0xff << 16, 0);
	
	resize_one_dimension(Dimension::Columns, gradient, multipler, W, H, temp);

	
	static SDL_Surface *target = SDL_CreateRGBSurface(
		0, W2, H2, 24 ,0xff << 0, 0xff << 8, 0xff << 16, 0);

	
	resize_one_dimension(Dimension::Rows, temp, multipler, W, H, target);

	SDL_Rect pos = {
		(WINDOW_W - W2) / 2, (WINDOW_H - H2) / 2, W2, H2};
	
	SDL_BlitSurface(target, NULL, surface, &pos);
	
	SDL_UpdateWindowSurface(window);
	
	save_bmp(target);
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
static void printQr(const QrCode &qr, int border) {
	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			std::cout << (qr.getModule(x, y) ? "##" : "  ");
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
static void printQr1(const QrCode &qr, int border) {
	for (int y = -border; y < qr.getSize() + border; y++) {
		for (int x = -border; x < qr.getSize() + border; x++) {
			std::cout << (qr.getModule(x, y) ? "[ 1 ]" : "[ 0 ]");
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
				pixels[(x+border) * 3 + (y+border) * target->pitch + 0] = 0xff;
				pixels[(x+border) * 3 + (y+border) * target->pitch + 1] = 0xff;
				pixels[(x+border) * 3 + (y+border) * target->pitch + 2] = 0xff; 
			} else {
				pixels[(x+border) * 3 + (y+border) * target->pitch + 0] = 0x0;
				pixels[(x+border) * 3 + (y+border) * target->pitch + 1] = 0x0;
				pixels[(x+border) * 3 + (y+border) * target->pitch + 2] = 0x0; 
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

static void save_bmp(SDL_Surface * sfc) {
	
	if (SDL_SaveBMP(sfc, "code.bmp") != 0) {
		std:: cout << " Failed to save .bmp file" <<std::endl;
	}
}
static void save_svg(const QrCode qr, int border) {
	std::ofstream f("code.svg");
	if(f) f << qr.toSvgString(border);
}
static void resize_one_dimension(Dimension dim, SDL_Surface * source, int multipler, int width, int hight, SDL_Surface * dest) {
	
	uint8_t *pixels_src = static_cast<uint8_t*>(source->pixels);
	uint8_t *pixels_dest = static_cast<uint8_t*>(dest->pixels);
	int k = 0;
	
	if (dim == 0){
		for (short i = 0; i<hight; i++) {
			k = 0;
			for (short j = 0; j < width; j++) {
				for (short l = 1; l<=multipler; l++) {			
					pixels_dest[k * 3 + i * dest->pitch + 0] = pixels_src[j * 3 + i * source->pitch + 0];
					pixels_dest[k * 3 + i * dest->pitch + 1] = pixels_src[j * 3 + i * source->pitch + 1];
					pixels_dest[k * 3 + i * dest->pitch + 2] = pixels_src[j * 3 + i * source->pitch + 2]; 
					k++;
				}
			}		
		}
	} else {
		for (short i = 0; i<multipler*hight; i++) {
			k = 0;
			for (short j = 0; j < width; j++) {
				for (short l = 1; l<=multipler; l++) {			
					pixels_dest[i * 3 + k * dest->pitch + 0] = pixels_src[i * 3 + j * source->pitch + 0];
					pixels_dest[i * 3 + k * dest->pitch + 1] = pixels_src[i * 3 + j * source->pitch + 1];
					pixels_dest[i * 3 + k * dest->pitch + 2] = pixels_src[i * 3 + j * source->pitch + 2]; 
					k++;
				}
			}		
		}
	}
}
