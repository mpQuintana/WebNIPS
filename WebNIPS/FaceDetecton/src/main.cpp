#include "emscripten.h"
#include "violajones.h"


// Define common image buffer and return parameters
unsigned char* buffer;		   	// Common buffer where input image is held
unsigned short   rect[4];	  	// Rectangle defining the face bounding box (left, top, width, height)
float 		   expression[7];  	// Vector defining the intensity for each facial expression

// Define objects to be used
ViolaJones*    faceDetector;   	// Face detection object

// ********************************************************
// ** ASM.JS EXTERNAL CALLS
// ********************************************************

// Allocate buffer into memory
extern "C"{
	// Allocate image buffer (and do other initializations) and return a pointer to it
	unsigned char* capture_buffer(int w, int h){
		buffer = (unsigned char *)new unsigned char[w*h*4];
		faceDetector = new ViolaJones(w, h);

		// Return input image buffer
		return buffer;
	}

	// Detect face and return bounding box (image is in buffer)
    unsigned short* detect_face(){
		faceDetector->detect(buffer, rect);
		return rect;
	}

	// Track face and return bounding box (image is in buffer)
    unsigned short* track_face(){
        faceDetector->detect(buffer, rect);

        float scale = 0.7;
        unsigned int dif = (1-scale)*rect[2];
        /*
        unsigned short roi2track[4];
        if(rect[2]==0 || rect[3]==0)
        {
            // If no detection set roi to whole frame
            roi2track[0] = 0;
            roi2track[1] = 0;
            roi2track[2] = faceDetector->getW();
            roi2track[3] = faceDetector->getH();
        }
        else
        {
            // Define roi
            Rect roi(rect[0]-dif/2, rect[1]-dif/2, rect[2]+dif, rect[3]+dif);
            Rect wholeFrame = Rect(0, 0, faceDetector->getW(), faceDetector->getH());
            roi.limitTo(wholeFrame);

            // Look around previous detection
            roi2track[0] = roi.getX();
            roi2track[1] = roi.getY();
            roi2track[2] = roi.getWidth();
            roi2track[3] = roi.getHeight();
        }

        faceDetector->track(buffer, roi2track, rect);
        */

        rect[0] = rect[0]+dif/2;
        rect[1] = rect[1]+dif/2;
        rect[2] = rect[2]-dif;
        rect[3] = rect[3]-dif;

		return rect;
	}

	// Recognize expression and return bounding box (image is in buffer)
	float* recognize_expression(){
		// TODO
		return expression;
	}
}
