#include "led-matrix.h"
#include "graphics.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cswLogger.h"

// Note this is currently on a pi1, if we move to a newer version we can utilise 1 whole CPU core:
/*
  Add 'isolcpus=3' to the end of the line of /boot/cmdline.txt (needs to be in the same as the other arguments, no newline).
  This will use the last core only to refresh the display then, but it also means, that no other process can utilize it then.
*/

using namespace rgb_matrix;

// do we need this?
static bool FullSaturation(const Color &c) {
    return (c.r == 0 || c.r == 255)
        && (c.g == 0 || c.g == 255)
        && (c.b == 0 || c.b == 255);
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  // Set defaults
  matrix_options.rows = 32;
  matrix_options.chain_length = 2;
//  matrix_options.show_refresh_rate = true;
  runtime_opt.drop_privileges = -1;

  /*
  // Below is removed as we are not using command line args
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
    return 1;
  }
  */

  Color color(255, 255, 0); // set default color to yellow, red = 255,0,0
  Color bg_color(0, 0, 0);  // set bg color to black
  Color outline_color(0,0,0); // IS THIS NEEDED
  const char *bdf_font_file = "/home/pi/rpi-rgb-led-matrix/fonts/9x15B.bdf"; // set default font file
  int x = 5; // centers the text horizontally
  int y = 1; // offsets the text by 1 led vertically = Centered text for 2 lines of text with the 9x15B.bdf font file
  int brightness = 100; // 1-100
  int letter_spacing = 0;

  //Load font. This needs to be a filename with a bdf bitmap font.
  rgb_matrix::Font font;
  font.LoadFont(bdf_font_file);

  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  matrix->SetBrightness(brightness);

  // is this needed?
  const bool all_extreme_colors = (brightness == 100)
      && FullSaturation(color)
      && FullSaturation(bg_color)
      && FullSaturation(outline_color);
  if (all_extreme_colors)
    matrix->SetPWMBits(1);

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();

  // infinte loop
  while(1){
    y = 1; // reset vertical height back to 1
    char a[8] = "A ";
    char b[8] = "B ";

    download(); // get latest screenshot
    imgToStr(&a[2], &b[2]); // extract load numbers and update strings 'a' & 'b'

    offscreen->Clear();
    rgb_matrix::DrawText(offscreen, font, x, y + font.baseline(), color, &bg_color, a, letter_spacing); // draw A
    y += font.height(); // move to next line
    rgb_matrix::DrawText(offscreen, font, x, y + font.baseline(), color, &bg_color, b, letter_spacing); // draw B
    offscreen = matrix->SwapOnVSync(offscreen);

    sleep(10);
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
