#include <Magick++.h>
#include <curl/curl.h>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <stdio.h>

//using namespace std;
using namespace Magick;

void download();
void prePro();
void prePro2();

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t written;
  written = fwrite(ptr, size, nmemb, stream);
  return written;
}

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

void preProImg() {
    fprintf(stderr, "img ");

  Image screenshot;
  try {
    screenshot.read("/home/pi/cswLED/cache/screenshotNew.jpg");
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

void download() {
  FILE *file;
  CURL *curl = curl_easy_init();

  if(curl) {
    CURLcode res;
    file = fopen("/home/pi/cswLED/cache/screenshotNew.jpg", "wb");

    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.100.4/image.png");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);


    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
      fprintf(stderr, "libcurl error code: %d\n\n", res);
    fclose(file);

    // Check for errors
    if(res != CURLE_OK){

      sleep(15);
      download();
    }
    curl_easy_cleanup(curl);
  }
  preProImg();
}

int main(int argc, char *argv[]) {
    fprintf(stderr, "Main ");
    download();
    return 0;
}

