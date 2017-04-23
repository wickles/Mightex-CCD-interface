/**
 * Mightex CCD Interface
 * Copyright 2011 Alexander Wickes
 * This work is licensed under the MIT license,
 * subject to all terms as reproduced in the included LICENSE file.
 */

//#include "BUF_USBCCDCamera_SDK.h"

// Useful macros
#define CCD_FRAME_RAW			0
#define CCD_FRAME_BMP			1
#define CCD_MODE_NORMAL			0
#define CCD_MODE_TRIGGER		1

#define FRAME_TYPE				CCD_FRAME_RAW
#define CAMERA_MODE(trigger)	( (trigger) ? CCD_MODE_TRIGGER : CCD_MODE_NORMAL )
#define BIN_MODE(bin)			( (bin) == 1 ? 0 : 0x81 + ((bin)-2) )

struct image_s {
	int width;
	int height;
	int bitmode;
	void* data;
};

// Settings loaded from file about how the program should run
struct settings_s {
	int ResHoriz;		// Pixels
	int ResVert;
	int BinMode;		// 1, 2, 3, 4
	int ExposureTime;	// milliseconds (1 - 200,000)
	int RedGain;		//
	int GreenGain;		// dB (6 - 41)
	int BlueGain;		//
	float DarkCoeff;
	bool DumpImages;
};

// CCD Camera settings
struct camera_s {
	char ModuleNo[32];
	char SerialNo[32];

	settings_s Settings;
	image_s DarkImage;
	char DumpDirectory[512];
};

struct MightexInterface {
	int NumberOfCameras;
	bool TriggerMode;
	int BitMode;		// 8 or 12
	int CaptureDelay;	// milliseconds

	unsigned long long Timer;
	unsigned long long PrevTimeStamp;

	camera_s* Cameras;
};

void InitCameraSettings( MightexInterface& ifc, FrameDataCallBack FrameHook, DeviceFaultCallBack CameraFaultCallBack, const char* filename );
void CleanupCameraSettings( MightexInterface& ifc );