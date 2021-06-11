#include "DrawHelper.h"
#include "globals.hpp"
#include "Menu.h"
#include "MenuNavigation.h"
#include <stdarg.h>
#include "esp_bt_main.h"   // Additional BLE functionaity
#include "esp_bt_device.h" // Additional BLE functionaity
#include "Tiny3x3a2pt7b.h"
#include "Storage.h"
namespace FreeTouchDeck
{
  using namespace fs;
  TFT_eSPI tft = TFT_eSPI();
  /* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : const char * message
         const char * module
Output : none
Note   : none
*/
  std::list<std::string> Messages;
  void drawErrorMessage(bool stop, const char *module, const char *fmt, ...)
  {
    displayInit();
    char *message = NULL;
    size_t msg_size = 0;
    if (stop)
    {
      SetActiveScreen("criticalmessage");
    }

    va_list args;
    va_start(args, fmt);
    tft.fillScreen(TFT_BLACK);
    SetSmallestFont(1);
    tft.setTextSize(1);
    tft.setCursor(0, tft.fontHeight() + 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    msg_size = vsnprintf(NULL, 0, fmt, args) + 1;
    message = (char *)malloc_fn(msg_size);

    if (!message)
    {
      ESP_LOGE(module, "Could not allocate %d bytes of memory to display message on screen", msg_size);
      tft.println(fmt);
      stop = true;
    }
    else
    {
      vsprintf(message, fmt, args);
      LOC_LOGE(module, "%s", message);
      tft.println(message);
    }
    va_end(args);
    free(message);
    uint16_t t_x;
    uint16_t t_y;
    while (stop)
    {
      if(getTouch(&t_x,&t_y))
      {
        tft.println("Restarting...");
        delay(1000);
        ESP.restart();
      }
      yield();
    }
    // hold a bit to let user read message
    // if it's not a full stop
    delay(5000);
  }
  /* ------------- Print an error message the TFT screen  ---------------- 
Purpose: This function prints an message to the TFT screen on a black 
         background. 
Input  : String message
Output : none
Note   : none
*/

  void drawErrorMessage(String message)
  {

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);
    SetSmallestFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(message);
  }

  /**
* @brief This function gets the Bluetooth device address and prints it to the serial monitor
         and the TFT screen in a 6 byte format, seperating each byte by ":".
*
* @param none
*
* @return none
*
* @note e.g. 00:11:22:33:22:EE
*/

  void printDeviceAddress()
  {

    const uint8_t *point = esp_bt_dev_get_address();

    for (int i = 0; i < 6; i++)
    {

      char str[3];

      sprintf(str, "%02X", (int)point[i]);
      //Serial.print(str);
      tft.print(str);

      if (i < 5)
      {
        // Serial.print(":");
        tft.print(":");
      }
    }
  }
  std::list<const GFXfont *> FontsList;
  const GFXfont *CurrentFont = LABEL_FONT;
  const GFXfont *DefaultFont = LABEL_FONT;
  const GFXfont *GetCurrentFont()
  {
    return CurrentFont;
  }
  bool SetFont(const GFXfont *newFont)
  {
    CurrentFont = newFont;
    tft.setFreeFont(newFont);
  }
  bool SetDefaultFont()
  {
    SetFont(DefaultFont);
  }
  void InitFontsTable()
  {
    //  FontsList.push_back(&Tiny3x3a2pt7b);
    FontsList.push_back(&TomThumb);
    LOC_LOGV(module, "Pushed font TomThumb to list with address %d ", &TomThumb);
    // FontsList.push_back(&FreeSans9pt7b);
    // LOC_LOGV(module,"Pushed font FreeSans9pt7b to list with address %d ",&FreeSans9pt7b);
    FontsList.push_back(&FreeSansBold9pt7b);
    LOC_LOGV(module, "Pushed font FreeSansBold9pt7b to list with address %d ", &FreeSansBold9pt7b);
    // FontsList.push_back(&FreeSans12pt7b);
    // LOC_LOGV(module,"Pushed font FreeSans12pt7b to list with address %d ",&FreeSans12pt7b);
    FontsList.push_back(&FreeSansBold12pt7b);
    LOC_LOGV(module, "Pushed font FreeSansBold12pt7b to list with address %d ", &FreeSansBold12pt7b);
    SetDefaultFont();
  }
  bool SetSmallestFont(int whichOne)
  {
    std::list<const GFXfont *>::iterator it = FontsList.begin();
    std::advance(it, whichOne);
    if (it != FontsList.end())
    {
      SetFont(*it);
    }
  }
  bool SetLargestFont()
  {
    SetFont(FontsList.back());
  }
  template <typename iter, typename t>
  iter iterator_from_ptr(iter first, iter last, t ptr)
  {
    while (first != last)
    {
      if (*first == ptr)
      {
        LOC_LOGV(module, "%d == %d. Returning iterator at this position", (uint32_t)*first, (uint32_t)ptr);
        return first;
      }
      LOC_LOGV(module, "%d != %d. Getting next", (uint32_t)*first, (uint32_t)ptr);
      ++first;
    }
    LOC_LOGV(module, "Returning last pointer ");
    return last;
  }
  bool SetSmallerFont()
  {
    auto newFont = iterator_from_ptr(FontsList.begin(), FontsList.end(), CurrentFont);

    if (newFont != FontsList.end())
    {
      LOC_LOGV(module, "Found current font. Getting the next one");
      std::advance(newFont, -1);
    }
    if (newFont != FontsList.end())
    {
      LOC_LOGD(module, "Setting new font");
      SetFont(*newFont);
      return true;
    }
    else
    {
      LOC_LOGD(module, "Already using smallest font");
    }
    return false;
  }
  bool SetLargerFont()
  {
    auto newFont = iterator_from_ptr(FontsList.begin(), FontsList.end(), CurrentFont);

    if (newFont != FontsList.end())
    {
      LOC_LOGV(module, "Found current font. Getting the next one");
      newFont++;
    }
    if (newFont != FontsList.end())
    {
      LOC_LOGD(module, "Setting new font");
      SetFont(*newFont);
      return true;
    }
    else
    {
      LOC_LOGD(module, "Already using largest font");
    }

    return false;
  }

  void displayInit()
  {
    static bool initialized = false;
    if (initialized)
      return;
    initialized = true;
    // --------------- Init Display -------------------------
    LOC_LOGI(module, "Initializing display");
    // Initialise the TFT screen
    tft.init();
    // Set the rotation
    tft.setRotation(generalconfig.screenrotation);
    // Clear the screen
    tft.fillScreen(TFT_BLACK);
    // Setup the Font used for plain text
    InitFontsTable();
    SetDefaultFont();
    tft.setCursor(0, tft.fontHeight() + 1);
    LOC_LOGI(module, "Screen size is %dx%d", tft.width(), tft.height());
  }

  void PrintScreenMessage(const char *message)
  {
    SetSmallestFont(1);
    tft.setTextSize(1);
    tft.setCursor(0, tft.fontHeight() + 1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(message);
  }

  void DrawSplash()
  {
    LOC_LOGD(module, "Loading splash screen bitmap.");
    ImageWrapper *splash = FreeTouchDeck::GetImage("freetouchdeck_logo.bmp");
    if (splash)
    {
      LOC_LOGD(module, "splash screen bitmap loaded. Drawing");
      splash->Draw(0, 0, false);
    }
    else
    {
      LOC_LOGW(module, "Unable to draw the splash screen.");
    }
  }
  void CacheBitmaps()
  {
    // Preload all bitmaps
    File root = ftdfs->open("/", FILE_READ);
    File file = root.openNextFile();
    PrintMemInfo();
    while (file)
    {
      String FileName = file.name();
      if (FileName.endsWith(".bmp"))
      {
        int start = FileName.lastIndexOf("/") + 1;
        FileName = FileName.substring(start);
        LOC_LOGV(module, "Caching bitmap from file %s, with name %s", file.name(), FileName.c_str());
        FreeTouchDeck::ImageWrapper *image = FreeTouchDeck::GetImage(FileName.c_str());
        LOC_LOGV(module, "Adding menu completed. Getting next file");
      }
      file = root.openNextFile();
    }
  }

  /**
* @brief This functions accepts a HTML including the # colour code 
         (eg. #FF00FF)  and returns it in RGB888 format.
*
* @param *html char (including #)
*
* @return unsigned long
*
* @note none
*/
  unsigned long convertHTMLtoRGB888(const char *html)
  {
    const char *hex = html; // remove the #
    if (ISNULLSTRING(html))
    {
      LOC_LOGE(module, "Invalid color.  Null string passed");
      return 0;
    }
    if (html[0] == '#')
    {
      hex += 1;
    }
    return (unsigned long)strtoul(hex, NULL, 16);
  }

  /**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
  unsigned int convertRGB888ToRGB565(unsigned long rgb)
  {
    return (((rgb & 0xf80000) >> 8) | ((rgb & 0xfc00) >> 5) | ((rgb & 0xf8) >> 3));
  }

  /**
* @brief This function converts RGB888 to RGB565.
*
* @param rgb unsigned long
*
* @return unsigned int
*
* @note none
*/
  unsigned int convertHTMLRGB888ToRGB565(const char *html)
  {
    unsigned long rgb = convertHTMLtoRGB888(html);
    return convertRGB888ToRGB565(rgb);
  }

  unsigned long convertRGB656ToRGB888(unsigned long rgb565)
  {
    return (unsigned long)(rgb565 & 0xF800 << 8) | (rgb565 & 0x7E0 << 5) | (rgb565 & 0x1F << 3);
  }
  const char *convertRGB656ToHTMLRGB888(unsigned long rgb565)
  {
    static char buffer[8] = {0};
    snprintf(buffer, sizeof(buffer), "#%06x", convertRGB656ToRGB888(rgb565));
    return buffer;
  }
  const char *convertRGB888oHTMLRGB888(unsigned long rgb888)
  {
    static char buffer[8] = {0};
    snprintf(buffer, sizeof(buffer), "#%06x", rgb888);
    return buffer;
  }
}