#include <conio.h>         // for textmode(), kbhit() and getch()
#include <go32.h>          // for _dos_ds (access VRAM directly)
#include <sys/farptr.h>    // for _farsetsel() and _farnspokeb() (access VRAM far pointer)
#include <stdlib.h>        // for rand()
#include <time.h>          // for pseudorandom number seed
#include <dos.h>           // for delay()
#include <stdio.h>         // for printf()

const bool Wind = true;

namespace PC
{
	const unsigned W = 320, H = 70; // The screen is 320x200, but i will only use
									// 320x75. It's enough for the fire, since the
									// rest of the screen will be black.

	const unsigned Palette[20] = {
		0,  111, 111, 111, 41,
		41, 41,  42,  42,  42,
		43, 43,  43,  44,  44,
		44, 14,  14,  14,  15
	};

	unsigned PixelBuffer[W*H]; // Stores indexes for the color palette (0-19)

	void CleanBuffer() { for (unsigned x : PixelBuffer) x = 0; }

	void
	Init()
	{
		_farsetsel(_dos_ds);
		textmode(0x13); // 320x200, 256-color mode
		CleanBuffer();

		// Create fire source (white line in the bottom)
		for (unsigned x = W*H - W; x < W*H; x++)
			PixelBuffer[x] = 19; // White
	}    

	void
	Render()
	{
		// This code i'm going to write now is pretty much
		// irrelevant. It's just a colorful scrolling footer 
		// in the screen with my name. It's just for fun.
		union REGS iregs, oregs;
		iregs.h.ah = 0x0f;
		int386(0x10, &iregs, &oregs);
		iregs.h.ah = 0x02;
		iregs.h.bh = oregs.h.bh;
		iregs.h.dl = (char)2  - 1;
		iregs.h.dh = (char)25 - 1;
		int386(0x10, &iregs, &oregs);

		const int Colors[12] = {
			31, 32, 33, 34, 35, 36,
			37, 42, 43, 45, 46, 47
		};

		const char *Bottom = "\033[1;%dmA\033[0m\033[1;%dmp\033[0m\033[1;%dmo\033[0m"
							   "\033[1;%dmc\033[0m\033[1;%dma\033[0m\033[1;%dml\033[0m"
							   "\033[1;%dmi\033[0m\033[1;%dmp\033[0m\033[1;%dms\033[0m"
							   "\033[1;%dme\033[0m\033[1;34m (C) - 2021\033[0m\r\n";

		printf(Bottom, Colors[rand()%13], Colors[rand()%13], Colors[rand()%13],
					   Colors[rand()%13], Colors[rand()%13], Colors[rand()%13],
					   Colors[rand()%13], Colors[rand()%13], Colors[rand()%13], 
					   Colors[rand()%13]);

		// Fire rendering
		while (!kbhit()) {
			// Fire intensity decays per row by small random number
			for (unsigned x=0; x<W; x++)
				for (unsigned y=0; y<H; y++) {
					unsigned PixelIndex = x+(W*y);
					unsigned BelowPixelIndex = PixelIndex+W;
					unsigned BelowPixelIntensity, NewFireIntensity;
					unsigned Decay = rand()%2;

					if (BelowPixelIndex >= W*H)
						continue;

					BelowPixelIntensity = PixelBuffer[BelowPixelIndex];

					if (BelowPixelIntensity==0) {
						PixelBuffer[PixelIndex] = 0;
						continue;
					}

					NewFireIntensity = BelowPixelIntensity-Decay;
					
					if (NewFireIntensity < 0) NewFireIntensity = 0;

					if (Wind) PixelBuffer[PixelIndex-(rand()&1)] = NewFireIntensity;
					else      PixelBuffer[PixelIndex] 			 = NewFireIntensity;
				}

			for (unsigned y=0; y<H; y++)
				for (unsigned p=y*W, x=0; x<W; x++, p++)
					_farnspokeb(0xA0000+p+W*110, Palette[PixelBuffer[p]]);    
		
			// New frame each 25 milliseconds (approximately 40 FPS)
			delay(25);
		}
	}

	void
	Close()
	{
		textmode(C80); // Default DOS text-mode    
	}
}

int
main()
{
	srand(time(NULL));

	PC::Init();
	PC::Render();
	getch();

	PC::Close();

	return 0;
}
