/*
    This code uses a variety of chunks from devkitPro's example code as a jumping-off point.
    https://github.com/devkitPro/3ds-examples

	Other parts of the code written by:
	- MaxxBMD (Jan.17 2025)
*/

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE/30)
#define BYTESPERSAMPLE 4

#define clrOffWhite		C2D_Color32(0xFA, 0xFA, 0xFA, 0xFF)
#define clrOffWhite2	C2D_Color32(0xD0, 0xD0, 0xD0, 0xFF)
#define clrOffBlack		C2D_Color32(0x30, 0x30, 0x30, 0xFF)
#define clrOffBlack2	C2D_Color32(0x11, 0x25, 0x25, 0xFF)
#define clrClear		C2D_Color32(0xFF, 0xD8, 0xB0, 0x68)
#define clrMangoTango	C2D_Color32(0xff, 0x82, 0x43, 0xff)
#define clrDimGray		C2D_Color32(0x69, 0x69, 0x69, 0xff)

#define clrOnWhite clrMangoTango
#define clrOnBlack clrMangoTango

struct Button {
	int x, y;	//top left corner (px)
	int z;		//depth
	int w, h; 	//scale (px)(extending to bottom right)
	u32 clrOff;
	u32 clrOn;
	float frequency;	//Hz of sound it makes
	float volVelocity;	//the "velocity" of the sound's volume
	bool isActive;
};
enum waveShape {sine, square, triangle, sawtooth};

bool doesBoxOverlapPoint(int bx, int by, int bw, int bh, int px, int py) {
	//(0,0) = screen's top left corner
	//bx, by, bw, bh track onto Button's .x, .y, .w, .h respectively.
	//px, py are the (x,y) of point on screen
	return (
		px >= bx &&
		px < bx + bw &&
		py >= by &&
		py < by + bh
	);
}

//TODO make sure these things work
float xToSquare(float x) {
	//like a sine wave, but for a square wave
	if (sin(x)) {return 1.0;}
	return -1.0;
}
/* TODO: change these so they're float-friendly
float xToSaw (float x) {
	//like a sine wave, but for a sawtooth wave
	if (x < 0) {x *= -1;}
	return (x % 2) - 1;
}
float xToTriangle (float x) {
	//like a sine wave, but for a triangle wave
	if (x < 0) {x *= -1;}
	return abs(x % 2) - 1;
}
*/

// audioBuffer is stereo PCM16
void fill_buffer(void* audioBuffer, size_t offset, size_t size, struct Button* btnToPlay, enum waveShape soundShape) {
	u32* dest = (u32*) audioBuffer;

	if (!btnToPlay) {return;}

	//const float TARGET_VELOCITY = 1.0;
	const float AMPLITUDE_MAX = 0.3;
		
	/* TODO(?): fade in/out
	I dunno if it'll fully be an ASDR thing,
	but I'd like to have the sound fade in/out.
	Perhaps it work like this:
		- each tick/frame, each button will keep track of its' velocity.
		- any button that has >0 velocity will play a sound
			- as in, fill_buffer() will be called.
				- hmm... does this function allow multiple sounds to play at once...?
			- the sound will be louder in proportion to the velocity (with limits)
		- buttons will gain/lose velocity based on if they're being pressed or not
	*/
	
	for (int i = 0; i < size; i++) {
		// This is a simple sine wave, with a frequency of `frequency` Hz.

		s16 sample = AMPLITUDE_MAX * 0x7FFF;

		//there's probably an easier way to do this.
		if (soundShape == square) {
			sample *= xToSquare(btnToPlay->frequency * (2 * M_PI) * (offset + i) / SAMPLERATE);
		}
		/*	TODO: fix the xToSaw and xToTriangle functions before re-enabling these
		else if (soundShape == sawtooth) {
			sample *= xoToSaw(btnToPlay->frequency * (2 * M_PI) * (offset + i) / SAMPLERATE);
		}
		else if (soundShape == triangle) {
			sample *= xToTriangle(btnToPlay->frequency * (2 * M_PI) * (offset + i) / SAMPLERATE);
		}
		*/
		else {
			//sine, or if soundShape is unspecified
			sample *= sin(btnToPlay->frequency * (2 * M_PI) * (offset + i) / SAMPLERATE);
		}

		// Stereo samples are interleaved: left and right channels.
		dest[i] = (sample << 16) | (sample & 0xffff);
	}

	DSP_FlushDataCache(audioBuffer, size);
}

void fill_buttonArr(struct Button* keyboard) {

	//we will write to the first 16 slots of this array
	//we aren't currently checking if there is room to fit.
	//make sure there is room or we will clobber other data!!!

	//initialize buttons
	//.z should init to 0, but that should be fine.
	//.volvelocity should init to 0.
	//.isActive should init to false.
	struct Button C4 = {
		//middle C
		.x = 20, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite, .clrOn = clrOnWhite,
		.frequency = 261.6256,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Cs4 = {
		.x = 35, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack, .clrOn = clrOnBlack,
		.frequency = 277.1826,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button D4 = {
		.x = 50, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite2, .clrOn = clrOnWhite,
		.frequency = 293.6648,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Ds4 = {
		.x = 65, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack2, .clrOn = clrOnBlack,
		.frequency = 311.1270,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button E4 = {
		.x = 80, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite, .clrOn = clrOnWhite,
		.frequency = 329.6276,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button F4 = {
		.x = 110, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite2, .clrOn = clrOnWhite,
		.frequency = 349.2282,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Fs4 = {
		.x = 125, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack, .clrOn = clrOnBlack,
		.frequency = 369.9944,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button G4 = {
		.x = 140, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite, .clrOn = clrOnWhite,
		.frequency = 391.9954,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Gs4 = {
		.x = 155, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack2, .clrOn = clrOnBlack,
		.frequency = 415.3047,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button A4 = {
		.x = 170, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite2, .clrOn = clrOnWhite,
		.frequency = 440.0000,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button As4 = {
		.x = 185, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack, .clrOn = clrOnBlack,
		.frequency = 466.1638,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button B4 = {
		.x = 200, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite, .clrOn = clrOnWhite,
		.frequency = 493.8833,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button C5 = {
		.x = 230, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite2, .clrOn = clrOnWhite,
		.frequency = 523.2511,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Cs5 = {
		.x = 245, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack, .clrOn = clrOnBlack,
		.frequency = 554.3653,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button D5 = {
		.x = 260, .y = 90,
		.w = 30, .h = 120, .z = 0,
		.clrOff = clrOffWhite, .clrOn = clrOnWhite,
		.frequency = 587.3295,
		.volVelocity = 0,
		.isActive = false
	};
	struct Button Ds5 = {
		.x = 275, .y = 60,
		.w = 30, .h = 60, .z = 0,
		.clrOff = clrOffBlack2, .clrOn = clrOnBlack,
		.frequency = 622.2540,
		.volVelocity = 0,
		.isActive = false
	};

	//black keys go in first (so they'll end up on top)
	keyboard[0] = Cs4;
	keyboard[1] = Ds4;
	keyboard[2] = Fs4;
	keyboard[3] = Gs4;
	keyboard[4] = As4;
	keyboard[5] = Cs5;
	keyboard[6] = Ds5;

	keyboard[7] = C4;
	keyboard[8] = D4;
	keyboard[9] = E4;
	keyboard[10] = F4;
	keyboard[11] = G4;
	keyboard[12] = A4;
	keyboard[13] = B4;
	keyboard[14] = C5;
	keyboard[15] = D5;
	
}

int main(int argc, char **argv)
{
	//----- ----- GRAPHICS SETUP ----- -----

	//Initialize citro libraries/screens
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	C3D_RenderTarget* bot = C2D_CreateScreenTarget (GFX_BOTTOM, GFX_LEFT);
	
	//Initialize console printing
	//top screen has 30 rows and 50 columns for text
	//bottom screen has 30 rows and 40 columns for text
	PrintConsole topScreen;
	consoleInit(GFX_TOP, &topScreen);
	
	//print some text (it will remain constant)
	consoleSelect(&topScreen);
	printf("\x1b[3;17HVirtual Stylophone");
	printf("\x1b[4;16HPress Start to exit.");
	printf("\x1b[29;0HCurrent frequency (Hz):");
	printf("\x1b[30;0HTouch Screen position (px):");

	//the colors are now defined at the top of the page as a bunch of #define statements. -Maxx

	//----- ----- DATA STRUCTURE(s) SETUP ----- -----

	struct Button keyboard[16];
	//careful! this function assumes keyboard has 16+ open spaces!
	fill_buttonArr(keyboard);

	//----- ----- AUDIO SETUP ----- -----

	//initialize ndsp stuff
	u32 *audioBuffer = (u32*) linearAlloc(SAMPLESPERBUF * BYTESPERSAMPLE * 2);
	bool fillBlock = false;
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(0,NDSP_INTERP_LINEAR);
	ndspChnSetRate(0,SAMPLERATE);
	ndspChnSetFormat(0,NDSP_FORMAT_STEREO_PCM16);

	// We set up two wave buffers and alternate between the two,
	// effectively streaming an infinitely long sine wave.
	ndspWaveBuf waveBuf[2];
	memset(waveBuf,0,sizeof(waveBuf));
	waveBuf[0].data_vaddr = &audioBuffer[0];
	waveBuf[0].nsamples = SAMPLESPERBUF;
	waveBuf[1].data_vaddr = &audioBuffer[SAMPLESPERBUF];
	waveBuf[1].nsamples = SAMPLESPERBUF;

	size_t stream_offset = 0;
	stream_offset += SAMPLESPERBUF;
	ndspChnWaveBufAdd(0, &waveBuf[0]);
	ndspChnWaveBufAdd(0, &waveBuf[1]);

	// ----- ----- MAIN LOOP ----- -----
	while (aptMainLoop())
	{
		//----- ----- BEGIN PHASE: INPUT ------ -----

		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) break; // break in order to return to hbmenu

		touchPosition touch;
		hidTouchRead(&touch);
		
		//----- ----- BEGIN PHASE: PROCESSING ----- -----

		/*	Iterate through every button (make sure that the keys
			on-top appear first in the list before others!!!).
			Only one key is allowed to be active at a time.
			All other keys will need to be de-activated.
			Once we've found topmost activated key, don't check others.
			Also, keep track of the activated key's pitch.
		*/
		bool isTargetLocated = false;
		struct Button *selectedBtn = NULL;

		for (int i = 0; i < sizeof(keyboard)/sizeof(keyboard[0]); i++) {
			#define crBtn keyboard[i]

			crBtn.isActive = false;
			
			if (isTargetLocated) {continue;}

			bool isCurrBtnTouched = false;
			isCurrBtnTouched = doesBoxOverlapPoint
			(
				crBtn.x, crBtn.y, crBtn.w, crBtn.h,
				touch.px, touch.py
			);
			if (isCurrBtnTouched) {
				isTargetLocated = true;
				selectedBtn = &crBtn;
				crBtn.isActive = true;
			}		
		}
		
		//----- ----- BEGIN PHASE: OUTPUT ----- -----

		//begin the render
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(bot, clrDimGray);
		C2D_SceneBegin(bot);

		//Print frequency and touch coords to top screen (if pointer is real)
		if (selectedBtn) {
			printf("\x1b[29;25H%.4f", selectedBtn->frequency);
		} else {
			printf("\x1b[29;25H        ");
		}
		printf("\x1b[30;28H%03d; %03d", touch.px, touch.py);

		//draw each button in reverse order (so the first items appear on top)
		for (int i = sizeof(keyboard)/sizeof(keyboard[0]) - 1; i >= 0; i--) {
			#define crBtn keyboard[i]

			//current color is determined by if button is active
			u32 clrCurr = clrClear;
			if (crBtn.isActive) {
				clrCurr = crBtn.clrOn;
			}
			else clrCurr = crBtn.clrOff;

			C2D_DrawRectSolid(crBtn.x, crBtn.y, crBtn.z, crBtn.w, crBtn.h, clrCurr);
		}

		//play the audio
		bool doPlayAudio = false;

		//safety - make sure selectedBtn is real and has a valid frequency.
		if (selectedBtn) {doPlayAudio = (selectedBtn->frequency > 0);}

		if (waveBuf[fillBlock].status == NDSP_WBUF_DONE && doPlayAudio) {

			fill_buffer
			(
				waveBuf[fillBlock].data_pcm16, 
				stream_offset, 
				waveBuf[fillBlock].nsamples, 
				selectedBtn,
				sine
			);
			ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);
			stream_offset += waveBuf[fillBlock].nsamples;
			fillBlock = !fillBlock;
		}

		C3D_FrameEnd(0);
    }
    
	// Deinit libs
	ndspExit();
	linearFree(audioBuffer);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}