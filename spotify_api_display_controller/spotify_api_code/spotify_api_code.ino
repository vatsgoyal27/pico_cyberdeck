#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SpotifyArduino.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <AnimatedGIF.h>

#include "swmg.h"   // <-- your GIF header

char ssid[] = "";
char password[] = "";
char clientId[] = "";
char clientSecret[] = "";
#define SPOTIFY_REFRESH_TOKEN ""

#define COUNTRY_CODE ""

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define JOY_X  26
#define JOY_Y  27
#define JOY_SW 22

#define TFT_CS   17
#define TFT_DC   20
#define TFT_RST  21
#define TFT_SCK  18
#define TFT_MOSI 19
#define TFT_MISO 16

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
AnimatedGIF gif;

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

String trackName = "";
String artistName = "";
bool isPlaying = false;
long progressMs = 0;
long durationMs = 0;

// Joystick calibration
int centerX = 2048;
int centerY = 2048;
int deadzone = 150;

unsigned long lastSpotifyUpdate = 0;
unsigned long lastScroll = 0;
unsigned long lastJoyAction = 0;
unsigned long lastLine2Swap = 0;
bool showArtist = false;
int scrollPos = 0;

byte fullBlock[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
byte emptyBlock[8] = {0x00,0x00,0x1F,0x00,0x00,0x00,0x1F,0x00};

void GIFDraw(GIFDRAW *pDraw)
{
  uint16_t lineBuffer[160];

  uint8_t *s = pDraw->pPixels;
  uint16_t *palette = pDraw->pPalette;

  int x = pDraw->iX;
  int y = pDraw->iY + pDraw->y;
  int w = pDraw->iWidth;

  if (y >= 128 || x >= 160) return;

  for (int i = 0; i < w; i++)
  {
    lineBuffer[i] = palette[s[i]];
  }

  tft.startWrite();
  tft.setAddrWindow(x, y, w, 1);
  tft.writePixels(lineBuffer, w, false, false);
  tft.endWrite();
}
void drawProgressBar() {
  lcd.setCursor(0, 1);
  if (durationMs <= 0) {
    lcd.print("                ");
    return;
  }
  long elapsed = progressMs + (millis() - lastSpotifyUpdate);
  if (elapsed > durationMs) elapsed = durationMs;
  int filled = (int)(14.0 * elapsed / durationMs);

  int secsLeft = (durationMs - elapsed) / 1000;
  char timeStr[3];
  if (secsLeft >= 60) {
    snprintf(timeStr, sizeof(timeStr), "%2d", secsLeft / 60);
  } else {
    snprintf(timeStr, sizeof(timeStr), "%2d", secsLeft % 60);
  }

  for (int i = 0; i < 14; i++) {
    if (i < filled) lcd.write((byte)0);
    else lcd.write((byte)1);
  }
  lcd.print(timeStr);
}

void drawArtist() {
  lcd.setCursor(0, 1);
  String artist = artistName.substring(0, 16);
  while (artist.length() < 16) artist += " ";
  lcd.print(artist);
}

void printTrack() {
  lcd.setCursor(0, 0);
  if (trackName.length() > 16) {
    String display = trackName.substring(scrollPos, scrollPos + 16);
    while (display.length() < 16) display += " ";
    lcd.print(display);
    if (millis() - lastScroll > 400) {
      scrollPos++;
      if (scrollPos > (int)trackName.length() - 16) scrollPos = 0;
      lastScroll = millis();
    }
  } else {
    String display = trackName;
    while (display.length() < 16) display += " ";
    lcd.print(display);
  }

  if (showArtist) {
    drawArtist();
  } else {
    drawProgressBar();
  }
}

void saveData(CurrentlyPlaying cp) {
  String newTrack = String(cp.trackName);
  if (newTrack != trackName) {
    trackName = newTrack;
    scrollPos = 0;
  }
  artistName = String(cp.artists[0].artistName);
  isPlaying = cp.isPlaying;
  progressMs = cp.progressMs;
  durationMs = cp.durationMs;
}

void calibrateJoystick() {
  // Average 10 readings for stable center
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 10; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(10);
  }
  centerX = sumX / 10;
  centerY = sumY / 10;
}

void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.createChar(0, fullBlock);
  lcd.createChar(1, emptyBlock);

  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  calibrateJoystick();
  delay(500);

  lcd.setCursor(0, 0);
  lcd.print("Connecting...   ");

  pinMode(JOY_SW, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi failed!    ");
    lcd.setCursor(0, 1);
    lcd.print("Restart Pico    ");
    while(true);
  }

  lcd.setCursor(0, 0);
  lcd.print("WiFi connected!");
  delay(1000);

  client.setInsecure();
  spotify.refreshAccessToken();

  lcd.clear();

  SPI.setRX(TFT_MISO);
  SPI.setTX(TFT_MOSI);
  SPI.setSCK(TFT_SCK);
  SPI.begin();

  tft.initR(INITR_BLACKTAB);  // change if colors wrong
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  gif.begin(LITTLE_ENDIAN_PIXELS);

  if (!gif.open((uint8_t*)swmg, sizeof(swmg), GIFDraw))
  {
    Serial.println("GIF open failed");
    tft.fillScreen(ST77XX_RED);
    while (1);
  }
  Serial.println("GIF started");
  
}

void loop() {
  int xVal = analogRead(JOY_X) - centerX;
  int yVal = analogRead(JOY_Y) - centerY;

  if (millis() - lastJoyAction > 500) {
    if (xVal > deadzone) {
      spotify.nextTrack();
      trackName = "Skipping...";
      artistName = "";
      progressMs = 0;
      durationMs = 0;
      lcd.clear();
      lastJoyAction = millis();
      lastSpotifyUpdate = 0;
    } else if (xVal < -deadzone) {
      spotify.previousTrack();
      trackName = "Previous...";
      artistName = "";
      progressMs = 0;
      durationMs = 0;
      lcd.clear();
      lastJoyAction = millis();
      lastSpotifyUpdate = 0;
    } else if (yVal > deadzone) {
      showArtist = true;
      lastJoyAction = millis();
    } else if (yVal < -deadzone) {
      showArtist = false;
      lastJoyAction = millis();
    }
  }

  if (digitalRead(JOY_SW) == LOW) {
    delay(50);
    if (digitalRead(JOY_SW) == LOW) {
      if (isPlaying) {
        spotify.pause();
      } else {
        spotify.play();
      }
      while (digitalRead(JOY_SW) == LOW);
      delay(200);
      lastSpotifyUpdate = 0;
    }
  }

  if (millis() - lastSpotifyUpdate > 3000) {
    int status = spotify.getCurrentlyPlaying(saveData, COUNTRY_CODE);
    if (status == 200) {
      printTrack();
    } else if (status == 204) {
      lcd.setCursor(0, 0);
      lcd.print("Nothing playing ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    lastSpotifyUpdate = millis();
  }
  gif.playFrame(true, NULL);
  printTrack();
}