/*
 * Created on 08. March 2025
 * Last modified on 08. March 2025
 * Author: Bondoki
 *
 * Purpose: Rendering of 'random dot stereograms', see https://en.wikipedia.org/wiki/Autostereogram .
 *          It's just for fun and educational purpose. Feel free to modify and use it :)
 *
 * License: This piece of code is (un)licensed to Unlicense according to
 * https://github.com/Bondoki/HexagonGrid/blob/main/LICENSE
 * also attributing Javidx9 for the unique and easy-to-use olc::PixelGameEngine and olcPGEX_TransformedView
 * with the underlying OLC-3 license, see
 * https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/Licencing
 * For more information about Javidx9 and OneLoneCoder please see https://github.com/OneLoneCoder
 *
 *
 * License (Unlicense)
 * ~~~~~~~~~~~~~~~~~~~
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
*/


// Using olc::PixelGameEngine for input and visualisation
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Using a transformed view to handle pan and zoom
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <algorithm>
#include <random>
#include <png.h>
#include <iostream>
#include <cstdlib>
#include <cmath>

// Override base class with your custom functionality
class Stereogram : public olc::PixelGameEngine
{
public:
	Stereogram()
	{		
		sAppName = "Stereogram";
	}

	olc::TransformedView tv;
	std::unique_ptr<olc::Sprite> m_pBackground;

	std::unique_ptr<olc::Sprite> m_pDepthMap;
	std::unique_ptr<olc::Sprite> m_pStereogramShifted; // with shifting algorithm
	std::unique_ptr<olc::Sprite> m_pStereogramTIW; // with algorithm by Thimbleby, Inglis, Witten

	olc::Sprite* m_pDrawImage;


public:
	// Called once at start of application
	bool OnUserCreate() override
	{
		// Prepare Pan & Zoom
		tv.Initialise({ ScreenWidth(), ScreenHeight() });


		// Create two more images with the same dimensions
		m_pBackground = std::make_unique<olc::Sprite>("./BG.png");

		m_pDepthMap = std::make_unique<olc::Sprite>("./DepthMap.png");
		m_pStereogramTIW = std::make_unique<olc::Sprite>(m_pBackground->width, m_pBackground->height);
		m_pStereogramShifted = std::make_unique<olc::Sprite>(m_pBackground->width, m_pBackground->height);

		// Load Test Image
		m_pDrawImage = m_pBackground.get();

		// Calculate the stereograms
		// Stereogram with Shifting
		CreateStereogramShifting(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramShifted.get());

		// Stereogram with algorithm by Thimbleby, Inglis, Witten
		CreateStereogramThimblebyInglisWitten(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramTIW.get());

		return true;
	}

	// Convenience fct to save png with libpng
	void save_png(const char* filename, int width, int height, unsigned char* image_data) {
			FILE *fp = fopen(filename, "wb");
			if (!fp) {
					std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
					return;
			}

			png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			if (!png) {
					std::cerr << "Error: Could not create PNG write struct." << std::endl;
					fclose(fp);
					return;
			}

			png_infop info = png_create_info_struct(png);
			if (!info) {
					std::cerr << "Error: Could not create PNG info struct." << std::endl;
					png_destroy_write_struct(&png, nullptr);
					fclose(fp);
					return;
			}

			if (setjmp(png_jmpbuf(png))) {
					std::cerr << "Error: An error occurred during PNG creation." << std::endl;
					png_destroy_write_struct(&png, &info);
					fclose(fp);
					return;
			}

			png_init_io(png, fp);
			png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
									 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_write_info(png, info);

			// Write image data
			for (int y = 0; y < height; y++) {
					png_bytep row = image_data + y * width * 3; // Assuming RGB format
					png_write_row(png, row);
			}

			png_write_end(png, nullptr);
			png_destroy_write_struct(&png, &info);
			fclose(fp);
	}

	void CreateStereogramThimblebyInglisWitten(const olc::Sprite* pBackground, const olc::Sprite* pDepthMap, olc::Sprite* pStereogram)
	{
			// this algorithm was published by
			// Thimbleby, Harold W., Stuart Inglis, and Ian H. Witten.
			// "Displaying 3D images: Algorithms for single-image random-dot stereograms."
			// Computer 27.10 (1994): 38-48
			// Link: https://core.ac.uk/download/pdf/29194616.pdf
			// or
			// JavaScript: https://github.com/tony-pizza/Stereogram.js

			 uint32_t width = pBackground->width;
			 uint32_t height = pBackground->height;

			 float dpi = 72; // assuming output of 72 dots per inch
			 float eyeSep = std::round(2.5 * dpi); // eye separation assumed to be 2.5 inches
			 float mu = (1 / 3.0); // depth of field (fraction of viewing distance)
			 // basically mu corresponds to 'viewing little parts of image mu ~ 1, or whole image mu << 1)

			 // // Random number generator
			 std::default_random_engine generator;
			 std::uniform_int_distribution<int> distribution{0, 255};

			 // The destination image is primed with the source image as the pixel
			 // values become altered as the algorithm executes
			 std::copy(pBackground->pColData.begin(), pBackground->pColData.end(), pStereogram->pColData.begin());

			 // for each row
			 for (uint32_t y = 0; y < height; y++) {

					 int32_t* same = new int32_t[width]; // Points to a pixel to the right

					 for (int32_t x = 0; x < width; x++) {
							 same[x] = x; // each pixel is initially linked with itself
					 }

					 // for each column
					 for (int32_t x = 0; x < width; x++) {

							 float z = pDepthMap->GetPixel(x,y).g / 255.0; // Gives the zdepth of the pixel as a value between 0.0 and 1.0

							 // stereo separation corresponding to z
							 int32_t sep = std::round((1.0 - (mu * z)) * eyeSep / (2.0 - (mu * z)));

							 // x-values corresponding to left and right eyes
							 int32_t left = std::round(x - ((sep + (sep & y & 1)) / 2));
							 //If sep & y & 1 is 1 (meaning the result of sep & y is odd), then sep + 1 is computed.
							 //If sep & y & 1 is 0 (meaning the result of sep & y is even), then sep + 0 (which is just sep) is computed.
							 int32_t right = left + sep;

							 if (0 <= left && right < width) {
									 // remove hidden surfaces
									 bool visible; // First, perform hidden-surface removal
									 int32_t t = 1; // We will check the points (x-t,y) and (x+t,y)
									 float zt; // z-coord of ray at these two points

									 do {
											 zt = z + (2 * (2 - (mu * z)) * t / (mu * eyeSep));

											 visible = ((pDepthMap->GetPixel(x-t,y).g / 255.0) < zt) && ((pDepthMap->GetPixel(x+t,y).g / 255.0) < zt); // false if obscured
											 t++;
									 } while (visible && zt < 1);

									 if (visible) {

											 int32_t k;

											 // record that left and right pixels are the same
											 for (k = same[left]; k != left && k != right; k = same[left]) {
													 if (k < right) {
															 left = k;
													 } else {
															 left = right;
															 right = k;
													 }
											 }
											 same[left] = right;
									 }
							 }
					 }

					 for (int32_t x = (width - 1); x >= 0; x--) {

							 if (same[x] == x) {
									 // set random color but not used here
									 //pStereogram->SetPixel(x,y,  olc::Pixel(distribution(generator), distribution(generator), distribution(generator)) );
									 // or try to copy the background
								   //pStereogram->SetPixel(x,y,  pBackground->GetPixel(same[x], y ) );
									 ;
							 } else {
									 // constrained pixel, obey constraint
									 pStereogram->SetPixel(x,y, pStereogram->GetPixel(same[x], y ));
							 }
					 }

					 // Deallocate memory
					 delete[] same;

			 }
	}
	
	void CreateStereogramShifting(olc::Sprite* pBackground, const olc::Sprite* pDepthMap, olc::Sprite* pStereogram)
	{
		// see also: https://bensimonds.com/2013/06/07/magic-eye-pictures-with-processing-and-blender/
		// see also: https://openprocessing.org/sketch/106591

		// create repeating background with slice (panelSz, pBackground->height)
		olc::Sprite* pBG = pBackground->Duplicate();

		int panelSz = 64;

		// copy initial panel across window
				for (int step = 0; step < pBG->width; step+=panelSz) {
										// use for loops to get pixel location
										for (int i = 0; i < panelSz; i++) {
												for (int j = 0; j < pBG->height; j++) {
														pBG->SetPixel(step+i,j, pBG->GetPixel(i,j) );
												}
										}
								}

			std::copy(pBG->pColData.begin(), pBG->pColData.end(), pStereogram->pColData.begin());

		// Iterate through each pixel from top left to bottom right, compare the pixel
	 	olc::vi2d vPixel;

		float shiftrange = 0.3; // Proportion of the pattern width to use for shifting to create depth.


		for (vPixel.x = 0; vPixel.x < pBG->width; vPixel.x++)
			for (vPixel.y = 0; vPixel.y < pBG->height; vPixel.y++)
				{
						int loc = vPixel.x; //+ (vPixel.y*pBG->width); // Gives the pixels index based on it's x and y coordinates.
						float zdepth = pDepthMap->GetPixel(vPixel.x,vPixel.y).g / 255.0; // Gives the zdepth of the pixel as a value between 0.0 and 1.0
						int shift = int(panelSz - (zdepth*(panelSz * shiftrange))); // Calculates the amount of shift for the pixel.

						// If in first col, ignore shift (nothing to repeat yet):
						int locshifted = loc;
						if (vPixel.x > panelSz) {
								locshifted = (vPixel.x-shift); // Gives the index of the pixel to repeat.
						}
						// Now set the new pixel colour to that of the pixel to repeat.
						pStereogram->SetPixel(vPixel, pStereogram->GetPixel(locshifted, vPixel.y ));//olc::Pixel(tmp,tmp,tmp));
				}
	}

	// Called every frame
	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle Pan (but no Zoom) using defaults middle mouse button
		tv.HandlePanAndZoom(0, 0.1f, true, false);

		// Erase previous frame
		Clear(olc::BLACK);

		// Draw Source Image
		if (GetKey(olc::Key::Q).bPressed)//.bHeld)
		{
			// Background
			m_pDrawImage = m_pBackground.get();
		}
		else if (GetKey(olc::Key::W).bPressed)//.bHeld)
		{
			// DepthMap
			m_pDrawImage = m_pDepthMap.get();
		}
		else if (GetKey(olc::Key::A).bPressed)//.bHeld)
		{
			// Stereogram with Shifting
			//CreateStereogramShifting(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramShifted.get());
			m_pDrawImage = m_pStereogramShifted.get();
		}
		else if (GetKey(olc::Key::S).bPressed)//.bHeld)
		{
			// Stereogram with Shifting
			//CreateStereogramThimblebyInglisWitten(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramTIW.get());
			m_pDrawImage = m_pStereogramTIW.get();
		}
		else if (GetKey(olc::Key::SPACE).bPressed)//.bHeld)
		{
			//CreateStereogramShifting(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramShifted.get());
		  //CreateStereogramThimblebyInglisWitten(m_pBackground.get(), m_pDepthMap.get(), m_pStereogramTIW.get());
			// Toggle between Stereograms
			m_pDrawImage = (m_pDrawImage == m_pStereogramTIW.get()) ?   m_pStereogramShifted.get() : m_pStereogramTIW.get();
		}
		else if (GetKey(olc::Key::ENTER).bPressed)//.bHeld)
		{
			// save the Stereogram to png file

			// Create a dynamic array using raw pointers - to lazy to figure out the typedefs in libpng
			unsigned char* image_data = new unsigned char[m_pDrawImage->width * m_pDrawImage->height * 3]; // For RGB

			// Fill the image with a gradient
			for (int y = 0; y < m_pDrawImage->height; y++) {
				for (int x = 0; x < m_pDrawImage->width; x++) {
					image_data[(y * m_pDrawImage->width + x) * 3 + 0] = (unsigned char) m_pDrawImage->GetPixel(x, y).r; // Red
					image_data[(y * m_pDrawImage->width + x) * 3 + 1] = (unsigned char) m_pDrawImage->GetPixel(x, y).g; // Green
					image_data[(y * m_pDrawImage->width + x) * 3 + 2] = (unsigned char) m_pDrawImage->GetPixel(x, y).b; // Blue
				}
			}

			save_png("output.png",  m_pDrawImage->width,  m_pDrawImage->height,  image_data);

			// Deallocate memory
			delete[] image_data;
		}

		// CenterToScreen
		tv.DrawSprite({ (ScreenWidth()-m_pDrawImage->width)/2, (ScreenHeight()-m_pDrawImage->height)/2}, m_pDrawImage);
		DrawString({10, 10}, "Use key Q for background image", olc::DARK_CYAN, 1);
		DrawString({10, 20}, "Use key W for depth map", olc::DARK_CYAN, 1);
		DrawString({10, 30}, "Use key A for shifted stereogram ", olc::DARK_CYAN, 1);
		DrawString({10, 40}, "Use key S for stereogram with algorithm by Thimbleby,Inglis,Witten", olc::DARK_CYAN, 1);
		DrawString({10, 50}, "Use SPACE (toggle stereograms) and ENTER (save view to 'output.png')", olc::DARK_CYAN, 1);


		return !GetKey(olc::Key::ESCAPE).bPressed;
	}
};

int main()
{
	Stereogram demo;
	if (demo.Construct(720, 720, 1, 1))
		demo.Start();
	return 0;
}
