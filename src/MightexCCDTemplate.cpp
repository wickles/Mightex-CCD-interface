/* Star_Tracker.cpp
Mightex Interface by Alexander Wickes
July 6, 2011

This program connects to a Mightex CCD Camera, model CGE-B013-U, in order to capture
images.

Settings are loaded from settings.txt.
*/

#include "stdafx.h"

#include <cstdio>
#include <cstdarg>
#include <direct.h>
#include "camera.h"

using namespace std;

// Compile Settings
#define DEBUG_TEXT
#define SUBTRACT_DARK

// Primary structure
MightexInterface Interface;

const char* settings_file = "settings.txt";
bool fault = false;

// Callback function for handling 
void FrameCallBack( TProcessedDataProperty* Attributes, unsigned char* BytePtr )
{
	int idx = Attributes->CameraID; // The working camera.
	camera_s* cam = &Interface.Cameras[idx-1];
	int width = cam->Settings.ResHoriz;
	int height = cam->Settings.ResVert;

	SYSTEMTIME localtime;
	GetLocalTime( &localtime );
	/*
	printf( "Grabbing frame: %04d.%02d.%02d : %02d:%02d:%02d.%03d\n", systime.wYear, systime.wMonth, systime.wDay,
						systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds );
						*/

	//printf( "BytePtr at %08X\n", (unsigned int)BytePtr);

	if ( BytePtr == NULL )
		return;

	// If in 12 bit mode, fix the messed up bit ordering.
	if ( Interface.BitMode == 12 )
	{
		int i;
		unsigned short* BytePtr16 = (unsigned short*)BytePtr;
		for ( i = 0; i < width*height; i++ )
			BytePtr16[i] = (BytePtr[2*i] << 4) | BytePtr[2*i+1];
	}

	if (cam->Settings.DumpImages)
	{
		// Save image to file w/ timestamp
		char buffer[512];

		// get system timestamp (millisecond accuracy)
		sprintf( buffer, "./%s/image_%04d%02d%02d_%02d%02d%02d%03d.raw", cam->DumpDirectory, localtime.wYear, localtime.wMonth, localtime.wDay,
						localtime.wHour, localtime.wMinute, localtime.wSecond, localtime.wMilliseconds );

		FILE* file = fopen(buffer, "wb");
		if ( file == NULL )
		{
			printf( "Error: Could not open file.\n" );
		}
		else
		{
			int pixel_bytes = ( Interface.BitMode == 8 ? 1 : 2 );
			fwrite( BytePtr, width*height*pixel_bytes, 1, file );

			fclose(file);
			//printf( "Wrote to file successfully.\n" );
		}
	}
		
	int bytes_pp = (Interface.BitMode == 12 ? 2 : 1);
	unsigned short* BytePtr16 = (unsigned short*)BytePtr;
	// If Binning is enabled, average pixels horizontally (camera only bins vertically)
	if ( cam->Settings.BinMode > 1 )
	{
		unsigned int sum;
		int x, y, i;
		for ( y = 0; y < height; y++ )
		{
			for ( x = 0; x < width; x++ )
			{
				sum = 0;
				for ( i = 0; i < cam->Settings.BinMode; i++ )
				{
					if ( Interface.BitMode == 12 )
						sum += BytePtr16[y*width + x*cam->Settings.BinMode+i];
					else
						sum += BytePtr[y*width + x*cam->Settings.BinMode+i];
				}
				if ( Interface.BitMode == 12 )
					BytePtr16[y*width + x] = sum / cam->Settings.BinMode;
				else
					BytePtr[y*width + x] = sum / cam->Settings.BinMode;
			}
		}
	}

	if ( cam->DarkImage.data != NULL )
	{
		// Subtract the dark image.
		int i;
		short* BytePtr16s = (short*)BytePtr;
		unsigned short* BytePtr16 = (unsigned short*)BytePtr;
		unsigned short* DarkPtr16 = (unsigned short*)cam->DarkImage.data;
		for ( i = 0; i < width*height; i++ )
			BytePtr16s[i] = short( BytePtr16[i] - cam->Settings.DarkCoeff * DarkPtr16[i] );
	}
}

// Called when there is a problem with the camera
void CameraFaultCallBack( int ImageType )
{
	printf( "Error: Camera fault.\n" );
	fault = true;
}

int main(int argc, char* argv[])
{
	MSG msg; // Required for camera to interface with windows

	InitCameraSettings( Interface, FrameCallBack, CameraFaultCallBack, settings_file );

	printf( "Entering main loop.\n" );
	while ( !fault )
	{
		// The following is to let camera engine to be active..it needs message loop.
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				goto exit;
			}
			else if ( msg.message == WM_TIMER )
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if ( Interface.TriggerMode == false )
		{
			// If trigger mode is off, program is on a user-set timer.
			unsigned long long ticks = GetTickCount();	// milliseconds
			//printf( "TickCount: %llu\n", ticks );

			if ( Interface.Timer < 0 )
				Interface.Timer = 0;
			else
				Interface.Timer += ticks - Interface.PrevTimeStamp;

			Interface.PrevTimeStamp = ticks;

			if ( Interface.Timer >= Interface.CaptureDelay )
			{
				// reset timer
				Interface.Timer %= Interface.CaptureDelay;
				BUFCCDUSB_StartFrameGrab( 1 );
			}
		}
	}

exit:
	printf( "Cleaning up.\n" );

	CleanupCameraSettings( Interface );

	printf( "Press ENTER to exit.\n" );
	getchar();
    return 0;
}
