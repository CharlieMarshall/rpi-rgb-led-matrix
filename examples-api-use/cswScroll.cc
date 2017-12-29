#include "led-matrix.h"
#include "graphics.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cswLogger.h"

/* TOD Add binaries & objects to Makefile $BINARIES $OBJECTS */

using namespace rgb_matrix;

static bool FullSaturation(const Color &c) {
    return (c.r == 0 || c.r == 255)
        && (c.g == 0 || c.g == 255)
        && (c.b == 0 || c.b == 255);
}

int main(int argc, char *argv[]) {
  if(argc != 2){
    printf("Usage: cswScroll [text]\n");
    return -1;
  }
  // printf("Displaying: %s\n", argv[1]);

  // set defaults. Note rows should default to 32
  RGBMatrix::Options matrix_options;
  matrix_options.rows = 32;
  matrix_options.chain_length = 2;

  rgb_matrix::RuntimeOptions runtime_opt;
  runtime_opt.drop_privileges = -1;

  /*
  // Below is removed as we are not using command line args
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
    return 1;
  }
  */

  Color color(255, 255, 0); // set default color to yellow, red = 255,0,0
  Color bg_color(0, 0, 0);
  Color outline_color(0,0,0);
  const char *bdf_font_file = "/home/pi/rpi-rgb-led-matrix/fonts/9x15B.bdf"; // set default font file
  int x = (matrix_options.chain_length * matrix_options.rows) + 5; // horizontal start position, 5 is just a buffer to create a space
  int y = 7; // centers the text vertically
  int pos = x; // set pos to the horizzontal start position
  int len = 0;
  int brightness = 100; // 1-100
  int letter_spacing = 0;

  //Load font. This needs to be a filename with a bdf bitmap font.
  rgb_matrix::Font font;
  font.LoadFont(bdf_font_file);

  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  RGBMatrix *canvas = rgb_matrix::CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (canvas == NULL)
    return 1;

  canvas->SetBrightness(brightness);

  const bool all_extreme_colors = (brightness == 100)
      && FullSaturation(color)
      && FullSaturation(bg_color)
      && FullSaturation(outline_color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();

  sleep(5); // to give us time to get outside

  // infinte loop
  while(1){
    offscreen_canvas->Clear(); // clear canvas
    // len = holds how many pixels we advanced on the screen, athe no of pixels our text takes up
    len = rgb_matrix::DrawText(offscreen_canvas, font, pos, y + font.baseline(), color, &bg_color, argv[1], letter_spacing);

    if (--pos + len < 0)
      pos = x;

    usleep(20000);
    // Swap the offscreen_canvas with canvas on vsync, avoids flickering
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
  }

  canvas->Clear();
  delete canvas;   // turn off the LEDs

  return 0;
}
