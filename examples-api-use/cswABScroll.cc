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
  int y = 1; // centers the text vertically
  int pos = x; // set pos to the horizzontal start position
  int pos_a = x; // set pos to the horizzontal start position
  int pos_b = x; // set pos to the horizzontal start position
  int len_a = 0;
  int len_b = 0;
  int brightness = 100; // 1-100
  int letter_spacing = 0;
  char c[100] = "LOADING";
//  char d[100] = "longer scrolling text here one line";
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

//  sleep(5); // to give us time to get outside
  int a_state = 1;
  int b_state = 1;

  // infinte loop
  while(1){
    y = 1; // reset vertical height back to 1
    pos = x; // set pos to the horizzontal start position
    char a[8] = "A ";
    char b[8] = "B ";

    download(); // get latest screenshot
    imgToStr(&a[2], &b[2]); // extract load numbers and update strings 'a' & 'b'

    if(a_state==1 || b_state==1){
      // 'A: ' = 27 && 'A:' = 18
      while( (--pos + 64) > 25){ // todo get length of "loading" this relys on A & B having the same length scrolling message
	y=1;
        offscreen_canvas->Clear(); // clear canvas

        if(a_state==1){
          len_a = rgb_matrix::DrawText(offscreen_canvas, font, pos, y + font.baseline(), color, &bg_color, c, letter_spacing); // scrolling text
          rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, "A ", letter_spacing); // draw A
        }
	else
	  rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, a, letter_spacing); // draw A

        y += font.height(); // move to next line

        if(b_state==1){
          len_b = rgb_matrix::DrawText(offscreen_canvas, font, pos, y + font.baseline(), color, &bg_color, c, letter_spacing); // scrolling text
          rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, "B ", letter_spacing); // draw B
        }
	else
	  rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, b, letter_spacing); // draw B

        usleep(20000);
        // Swap the offscreen_canvas with canvas on vsync, avoids flickering
        offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);

      } // close while
    } // close if

    y = 1; // reset vertical height back to 1
    offscreen_canvas->Clear();
    rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, a, letter_spacing); // draw A
    y += font.height(); // move to next line
    rgb_matrix::DrawText(offscreen_canvas, font, 0, y + font.baseline(), color, &bg_color, b, letter_spacing); // draw B
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
    // end of load number texr

    // sleep(3); // dont need to sleep as it takes 10 seconds to update

  } // close while

  canvas->Clear();
  delete canvas;   // turn off the LEDs

  return 0;
}
