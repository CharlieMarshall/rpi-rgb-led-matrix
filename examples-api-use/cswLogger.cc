#include <Magick++.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <curl/curl.h>
#include <unistd.h>
#include "cswLogger.h"
#include <iostream>
#include <ctime>
#include <stdio.h>

//using namespace std;
using namespace Magick;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t written;
  written = fwrite(ptr, size, nmemb, stream);
  return written;
}

void download() {
  FILE *file;
  CURL *curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    file = fopen("/home/pi/cswLED/cache/screenshot.jpg", "wb");
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.100.4/image.png");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
    fclose(file);

    // Check for errors
    if(res != CURLE_OK){
/*
      // current date/time based on current system
      time_t now = time(0);
      // convert now to string form
      char* dt = ctime(&now);

      // comment out logging for now
      fprintf(stderr, "%s:curl_easy_perform() failed: %s. libcurl error code: %d\n\n", dt, curl_easy_strerror(res), res);
*/
      // try again in 15 seconds
      sleep(15);
      download();
    }
    curl_easy_cleanup(curl);
  }
  prePro();
}

/*
void prePro() {
  Image screenshot;
  try {
    screenshot.read("/home/pi/cswLED/cache/screenshot.jpg");
    screenshot.crop(Geometry(443,20, 357, 160)); // Crop the image (width, height, xOffset, yOffset)

    screenshot.type(GrayscaleType);
    screenshot.write("/home/pi/cswLED/cache/1gray.png");

    screenshot.normalize();
    screenshot.write("/home/pi/cswLED/cache/2normal.png");

    screenshot.resolutionUnits(PixelsPerInchResolution);
    screenshot.density("300x300");
    screenshot.write("/home/pi/cswLED/cache/3dpi.png");

    screenshot.resize("250%");
    screenshot.write("/home/pi/cswLED/cache/4resize.png");

    screenshot.borderColor("white");
    screenshot.border("20x20+0+0");
    screenshot.write("/home/pi/cswLED/cache/5border.png");

    screenshot.adaptiveThreshold(25,25,10);
    screenshot.write("/home/pi/cswLED/cache/6lat.png");
  }
  catch(Exception &error_) {
    fprintf(stderr, "Caught exception: %s\n", error_.what());
    sleep(15);
    download();
  }
}
*/

void prePro() {
  Image screenshot;
  try {
    screenshot.read("/home/pi/cswLED/cache/screenshot.jpg");
    screenshot.crop(Geometry(443,20, 357, 160)); // Crop the image (width, height, xOffset, yOffset)
    screenshot.resize("250%");
    screenshot.write("/home/pi/cswLED/cache/cropped.png");
  }
  catch(Exception &error_) {
    fprintf(stderr, "Caught exception: %s\n", error_.what());
    sleep(15);
    download();
  }
}

// a & b are pointers to elements a[2] & b[2]
void imgToStr(char *a, char *b) {
  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  // Initialize tesseract-ocr with English, without specifying tessdata path
  if (api->Init(NULL, "eng")) {
      fprintf(stderr, "Could not initialize tesseract.\n");
      exit(1);
  }

  // Open input images with leptonica library
  Pix *image = pixRead("/home/pi/cswLED/cache/cropped.png");
  api->SetImage(image);

  // Restrict recognition to a sub-rectangle of the image call, SetRectangle(left, top, width, height) after SetImage.
  api->SetRectangle(0, 0, 100, 46);	// gets A
  char *outA = api->GetUTF8Text();	// Get OCR result

  api->SetRectangle(1005, 0, 100, 46);	// gets B
  char *outB = api->GetUTF8Text();	// Get OCR result

//  for(int y=0; outA[y] != '\0'; y++)
//    printf("Char %d: %c\n", y, outA[y]);

//  for(int y=0; outB[y] != '\0'; y++)
//    printf("Char %d: %c\n", y, outB[y]);

  int i=0;
  for(i=0; outA[i] != '\0'; i++) {
      if(i>4 || outA[i] == '\n')
	break;
      a[i] = outA[i];
  }
  a[i] = '\0';

  for(i=0; outB[i] != '\0'; i++){
      if(i>4 || outB[i] == '\n')
        break;
      b[i]=outB[i];
  }
  b[i] = '\0';

// try this instead // doesnt seem to work on the *a++ = *outA++ line
/*
  int i;
  for(i=0; *outA != '\n' && *outA != '\0' && i<5; i++){
    printf("outA is: %c\n", *outA); // correct
    *a++ = *outA++;
    printf("set char at a to : %c\n", a);
  }

  for(i=0; *outB != '\n' && *outB != '\0' && i<5; i++)
    *b++ = *outB++;

  *a = '\0'; // set the last chars to null terminators
  *b = '\0'; // set the last chars to null terminators
*/

  // Destroy used object and release memory
  api->End();
  delete [] outA;
  delete [] outB;
  pixDestroy(&image);
}
