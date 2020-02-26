/*
  xdsp_11_matrix.ino - Display 8x8 matrix with Parola Library for Tasmota

  Copyright (C) 2020  Michael Sauer and MajicDesign

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  ***************************************************************************

  CPU Clock = 80 Mhz
  max clock of display is 4.545 Mhz (220ns sclk cycle)
  so cpu/18 => 4.44 Mhz should be ok
  HSPI CLK	5	GPIO14
  HSPI /CS	8	GPIO15
  HSPI MOSI	7	GPIO13
*/

#ifdef USE_SPI
#ifdef USE_DISPLAY
#ifdef USE_DISPLAY_MAX7219

#define XDSP_11                    11

#include <SPI.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 12

#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D8

MD_Parola *P;

/*********************************************************************************************/

void MAX7219InitDriver(void)
{
  if (!Settings.display_model) {
    //Settings.display_address[0] = OLED_ADDRESS2;
    //Settings.display_cols[1] = 0;
    //Settings.display_cols[2] = 0;
    Settings.display_model = XDSP_11;
  }

  if (XDSP_11 == Settings.display_model) {
    uint8_t deviceCount = (uint8_t)Settings.display_cols[0];
    if  ((pin[GPIO_SPI_CS]<99) && (pin[GPIO_SPI_MOSI]<99) && (pin[GPIO_SPI_CLK]<99)){
       P = new MD_Parola(HARDWARE_TYPE, pin[GPIO_SPI_CS], (deviceCount <= 0) ? 1 : deviceCount); //Settings.display_cols[0] );
    } else {
      return;
    }
    
    if (Settings.display_cols[0] == 0) {
      Settings.display_cols[0] = MAX_DEVICES;
    }
    if (Settings.display_rows > 1) {
      Settings.display_rows = 1; // Currently only one row is supported
    }

    //TODO: Hardware Type setting

    P->begin();
#ifdef SHOW_SPLASH
    // Welcome text
    P->print("MAX7219");
    delay(1000);
    MAX7219_resetMatrix();
#endif

    if (Settings.display_dimmer > 15) {
      Settings.display_dimmer = 15;
    }
    P->setIntensity(Settings.display_dimmer);
  }
}

/*********************************************************************************************/

void MAX7219_resetMatrix(void) {
  P->displayClear();
  P->displayReset();
}

void MAX7219_matrixOnOff(void)
{
  if (!disp_power) { P->displayClear(); }
}

/*********************************************************************************************/

#ifdef USE_DISPLAY_MODES1TO5

void MAX7219_printTime(void)
{
  char line[24];
  snprintf_P(line, sizeof(line), PSTR(" %02d" D_HOUR_MINUTE_SEPARATOR "%02d %02d" D_MONTH_DAY_SEPARATOR "%02d" D_YEAR_MONTH_SEPARATOR "%04d"), RtcTime.hour, RtcTime.minute, RtcTime.day_of_month, RtcTime.month, RtcTime.year);  // [12:34 01-02-2018]
  P->print(line);
}

void MAX7219_refreshMatrix(void) 
{
  if (Settings.display_mode) {  // Mode 0 is User text
    switch (Settings.display_mode) {
      case 1:  // Time
        MAX7219_printTime();
        break;
      case 2:  // Local
      case 3:  // Local
      case 4:  // Mqtt
      case 5:  // Mqtt
        //SSD1351PrintLog();
        break;
    }
  }
}

/*
void SSD1351PrintLog(void)
{
  disp_refresh--;
  if (!disp_refresh) {
    disp_refresh = Settings.display_refresh;
    if (!disp_screen_buffer_cols) { DisplayAllocScreenBuffer(); }

    char* txt = DisplayLogBuffer('\370');
    if (txt != NULL) {
      uint8_t last_row = Settings.display_rows -1;

      renderer->clearDisplay();
      renderer->setTextSize(Settings.display_size);
      renderer->setCursor(0,0);
      for (byte i = 0; i < last_row; i++) {
        strlcpy(disp_screen_buffer[i], disp_screen_buffer[i +1], disp_screen_buffer_cols);
        renderer->println(disp_screen_buffer[i]);
      }
      strlcpy(disp_screen_buffer[last_row], txt, disp_screen_buffer_cols);
      DisplayFillScreen(last_row);

      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_DEBUG "[%s]"), disp_screen_buffer[last_row]);

      renderer->println(disp_screen_buffer[last_row]);
      renderer->Updateframe();
    }
  }
}

void SSD1351Time(void)
{
  char line[12];

  renderer->clearDisplay();
  renderer->setTextSize(2);
  renderer->setCursor(0, 0);
  snprintf_P(line, sizeof(line), PSTR(" %02d" D_HOUR_MINUTE_SEPARATOR "%02d" D_MINUTE_SECOND_SEPARATOR "%02d"), RtcTime.hour, RtcTime.minute, RtcTime.second);  // [ 12:34:56 ]
  renderer->println(line);
  snprintf_P(line, sizeof(line), PSTR("%02d" D_MONTH_DAY_SEPARATOR "%02d" D_YEAR_MONTH_SEPARATOR "%04d"), RtcTime.day_of_month, RtcTime.month, RtcTime.year);   // [01-02-2018]
  renderer->println(line);
  renderer->Updateframe();
}

void SSD1351Refresh(void)  // Every second
{
  if (Settings.display_mode) {  // Mode 0 is User text
    switch (Settings.display_mode) {
      case 1:  // Time
        SSD1351Time();
        break;
      case 2:  // Local
      case 3:  // Local
      case 4:  // Mqtt
      case 5:  // Mqtt
        SSD1351PrintLog();
        break;
    }
  }
}
*/

#endif  // USE_DISPLAY_MODES1TO5

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xdsp11(uint8_t function) {
  bool result = false;

  if (spi_flg) {
    if (FUNC_DISPLAY_INIT_DRIVER == function) {
      MAX7219InitDriver();
    }
    else if (XDSP_11 == Settings.display_model) {

      switch (function) {
        case FUNC_DISPLAY_MODEL:
          result = true;
          break;
        case FUNC_DISPLAY_INIT:
          //P.resetDisplay();
          //Ili9341Init(dsp_init);
          break;
        case FUNC_DISPLAY_POWER:
          MAX7219_matrixOnOff();
          break;
        case FUNC_DISPLAY_CLEAR:
          MAX7219_resetMatrix();
          break;
        case FUNC_DISPLAY_DRAW_STRING:
          MAX7219_resetMatrix();
          P->print(dsp_str);
          //Ili9341DrawStringAt(dsp_x, dsp_y, dsp_str, dsp_color, dsp_flag);
          break;
        case FUNC_DISPLAY_ONOFF:
          P->displayShutdown(dsp_on);
          //Ili9341DisplayOnOff(dsp_on);
          break;
        case FUNC_DISPLAY_ROTATION:
          //tft->setRotation(Settings.display_rotate);
          break;
        case FUNC_DISPLAY_DIMMER:
          P->setIntensity(Settings.display_dimmer);
          break;
#ifdef USE_DISPLAY_MODES1TO5
        case FUNC_DISPLAY_EVERY_SECOND:
          MAX7219_refreshMatrix();
          //Ili9341Refresh();
          //P.displayAnimate();
          break;
#endif  // USE_DISPLAY_MODES1TO5
      }
    }
  }
  return result;
}

#endif  // USE_DISPLAY_MAX7219
#endif  // USE_DISPLAY
#endif  // USE_SPI
