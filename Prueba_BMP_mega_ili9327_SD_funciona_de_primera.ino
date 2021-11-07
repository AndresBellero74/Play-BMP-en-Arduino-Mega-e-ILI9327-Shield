#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <SD.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#define PIN_SD_CS 5 // D8 
#define PIN_SD_MISO 50 //12
#define PIN_SD_MOSI 51 //11
#define PIN_SD_SCLK 52 //10
File root;
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410
#define NAMEMATCH ""        // "" matches any name
//#define NAMEMATCH "tiger"   // *tiger*.bmp
#define PALETTEDEPTH   0     // do not support Palette modes
//#define PALETTEDEPTH   8     // support 256-colour Palette
uint16_t version = MCUFRIEND_KBV_H_;

void setup(void) {
  Serial.begin(9600);
  Serial.println("Arrancando el display");
  
// Setup the LCD
  uint16_t ID = tft.readID(); //
  if (ID == 0x0404) {
          Serial.println(F("Probably a write-only Mega2560 Shield"));
          Serial.println(F("#define USE_SPECIAL in mcufriend_shield.h"));
          Serial.println(F("#define appropriate SPECIAL in mcufriend_special.h"));
          Serial.println(F("e.g. USE_MEGA_16BIT_SHIELD"));
          Serial.println(F("e.g. USE_MEGA_8BIT_SHIELD"));
          Serial.println(F("Hint.  A Mega2560 Shield has a 18x2 male header"));
          Serial.println(F("Often a row of resistor-packs near the 18x2"));
          Serial.println(F("RP1-RP7 implies 16-bit but it might be 8-bit"));
          Serial.println(F("RP1-RP4 or RP1-RP5 can only be 8-bit"));
  }
  if (ID == 0xD3D3) {
      uint16_t guess_ID = 0x9481; // write-only shield
      Serial.println(F("Probably a write-only Mega2560 Shield"));
      Serial.print(F("Try to force ID = 0x"));
      Serial.println(guess_ID, HEX);
      tft.begin(guess_ID);
  }
  else tft.begin(ID);
  Serial.println(F(""));
  if (tft.width() == 0) {
      Serial.println(F("This ID is not supported"));
      Serial.println(F("look up ID in extras/mcufriend_how_to.txt"));
      Serial.println(F("you may need to edit MCUFRIEND_kbv.cpp"));
      Serial.println(F("to enable support for this ID"));
      Serial.println(F("e.g. #define SUPPORT_8347D"));
      Serial.println(F(""));
      Serial.println(F("New controllers appear on Ebay often"));
      Serial.println(F("If your ID is not supported"));
      Serial.println(F("run LCD_ID_readreg.ino from examples/"));
      Serial.println(F("Copy-Paste the output from the Serial Terminal"));
      Serial.println(F("to a message in Displays topic on Arduino Forum"));
      Serial.println(F("or to Issues on GitHub"));
      Serial.println(F(""));
      Serial.println(F("Note that OPEN-SMART boards have diff pinout"));
      Serial.println(F("Edit the pin defines in LCD_ID_readreg to match"));
      Serial.println(F("Edit mcufiend_shield.h for USE_SPECIAL"));
      Serial.println(F("Edit mcufiend_special.h for USE_OPENSMART_SHIELD_PINOUT"));
     while (1);    //just die
  } else {
      Serial.print(F("PORTRAIT is "));
      Serial.print(tft.width());
      Serial.print(F(" x "));
      Serial.println(tft.height());
      Serial.println(F(""));
      Serial.println(F("Run the examples/graphictest_kbv sketch"));
      Serial.println(F("All colours, text, directions, rotations, scrolls"));
      Serial.println(F("should work.  If there is a problem,  make notes on paper"));
      Serial.println(F("Post accurate description of problem to Forum"));
      Serial.println(F("Or post a link to a video (or photos)"));
      Serial.println(F(""));
      Serial.println(F("I rely on good information from remote users"));
  }
  
  tft.setRotation(3);
  
  // cambio de direccion
  tft.print("Initializing SD card...");
  if (!SD.begin(PIN_SD_CS)){ //, SD_MOSI, SD_MISO, SD_SCLK)) {
    tft.println("failed!");
    while(1);  // stay here
  }
  tft.println("OK!"); 
 
}

void loop() {
  uint8_t ret;
  uint32_t start;
  File root = SD.open("/");  // open SD card main root
 
  while (true) {
    File entry =  root.openNextFile();  // open file
 
    if (! entry) {
      // no more files
      root.close();
      return;
    }
 
    uint8_t nameSize = String(entry.name()).length();  // get file name size
    String str1 = String(entry.name()).substring( nameSize - 4 );  // save the last 4 characters (file extension)
 
    if ( str1.equalsIgnoreCase(".bmp") ) {  // if the file has '.bmp' extension
      tft.fillScreen(BLUE);
      //bmpDraw(entry.name(), 0, 0);        // draw it
      start = millis();
      ret = showBMP(entry.name(), 0, 0);
      switch (ret) {
          case 0:
              Serial.print(millis() - start);
              Serial.println(F("ms"));
              delay(5000);
              break;
          case 1:
              Serial.println(F("bad position"));
              break;
          case 2:
              Serial.println(F("bad BMP ID"));
              break;
          case 3:
              Serial.println(F("wrong number of planes"));
              break;
          case 4:
              Serial.println(F("unsupported BMP format"));
              break;
          case 5:
              Serial.println(F("unsupported palette"));
              break;
          default:
              Serial.println(F("unknown"));
              break;
      }
 
      delay(5000);
    }
    //else if ( str1.equalsIgnoreCase(".jpg") ) {  // if the file has '.bmp' extension
    //  tft.fillScreen(BLUE);
    //  drawJpeg(entry.name(), 0, 0);
    //  delay(5000);
    //  //bmpDraw(entry.name(), 0, 0);        // draw it
    //}
    entry.close();  // close the file
    delay(500);
  }
}


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.
#define BUFFPIXEL 20

uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;      //

    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bmpFile.seek(bmpImageoffset - (4<<bmpDepth)); //54 for regular, diff for colorsimportant
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}
 
// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
 
uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}
 
uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
  
void printDirectory(File dir, int numTabs) {
  while (true) {
 
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
 
