#include<Adafruit_SSD1306.h>

Adafruit_SSD1306 dsp(128, 64, &Wire, -1);

#include <EEPROM.h>

#define EEPROM_ADDRESS 0

#define UPPIN     13
#define DWNPIN    9
#define LFTPIN    2
#define RHTPIN    12
#define APIN      8
#define BUZZER    3

#define COIN 1
#define SEEDVILLAGER 2
#define FARMLAND 3
#define FRUITVILLAGER 4
#define HOEVILLAGER 5 // a hoe like in the Minecraft game not like in GTA
#define WATER 6

#define SAVE 100
#define LOAD 99

int8_t xOffset, yOffset;
bool isFacingY = true;
uint16_t money = 50;
uint8_t taxtime = 200;
uint8_t tax;
uint16_t seeds, fruits, hoes;
int lasttime;
uint16_t water = 1000;

uint8_t mapelements[18][10] = {
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 4, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 3, 0, 0, 0, 0},
  {0, 0, 1, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 99, 0, 100, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 6, 0, 5, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

uint8_t treeage[18][10] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

const unsigned char mapstone[] PROGMEM = {
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11101111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111011,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111101, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
};
const unsigned char mapplayery[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b01000010,
  0b11000011,
  0b11000011,
  0b01000010,
  0b00111100,
  0b00000000,
};
const unsigned char mapplayerx[] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01000010,
  0b01000010,
  0b01000010,
  0b01000010,
  0b00111100,
  0b00011000,
};
const unsigned char mapcoin[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b01000010,
  0b01011010,
  0b01011010,
  0b01000010,
  0b00111100,
  0b00000000,
};
const unsigned char mapsvillager[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b01000010,
  0b11000011,
  0b11000011,
  0b01011010,
  0b00111100,
  0b00111100,
};
const unsigned char mapfvillager[] PROGMEM = {
  0b00111100,
  0b11000010,
  0b11000010,
  0b00111100,
  0b00000100,
  0b00111100,
  0b00100100,
  0b00111100,
};
const unsigned char maphvillager[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b01000010,
  0b11000011,
  0b11000011,
  0b10111100,
  0b10000000,
  0b10000000,
};
const unsigned char mapfarmland[] PROGMEM = {
  0b11000100,
  0b01010010,
  0b00100100,
  0b11011010,
  0b01010101,
  0b10110110,
  0b01010110,
  0b11011011,
};
const unsigned char maptree1[] PROGMEM = {
  0b11000100,
  0b00000000,
  0b10000000,
  0b00011000,
  0b00011001,
  0b00000000,
  0b00000000,
  0b01011011,
};
const unsigned char maptree2[] PROGMEM = {
  0b11000100,
  0b00000000,
  0b00011000,
  0b00111100,
  0b00111100,
  0b00011000,
  0b00000000,
  0b01011011,
};
const unsigned char maptree3[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01111110,
  0b00111100,
  0b00000000,
};
const unsigned char mapsave[] PROGMEM = {
  0b00111111,
  0b01001101,
  0b10001101,
  0b10000001,
  0b10000001,
  0b10111101,
  0b10111101,
  0b11111111,
};
const unsigned char mapload[] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01011010,
  0b00011000,
  0b00000000,
  0b11111111,
  0b00000000,
  0b11111111,
};
const unsigned char mapwater[] PROGMEM = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11111111,
  0b10111101,
  0b10000001,
  0b10000001,
  0b01111110,
};

void beep() {
  digitalWrite(BUZZER, 1);
  delay(1);
  digitalWrite(BUZZER, 0);
  delay(1);
}

void interact(uint8_t x, uint8_t y) {
  uint8_t type = mapelements[x][y];
  if (type == SEEDVILLAGER) {
    if (money >= 3) {
      seeds++;
      money -= 3;
    }
  } else if (type == FARMLAND) {
    if (treeage[x][y] == 0) {
      if (seeds > 0) {
        treeage[x][y] = 1;
        seeds--;
      }
    } else if (treeage[x][y] == 3) {
      treeage[x][y] = 0;
      fruits += random(3, 10);
    }
  } else if (type == FRUITVILLAGER) {
    if (fruits >= 3) {
      money += random(3, 7);
      fruits -= 3;
    }
  } else if (type == HOEVILLAGER) {
    if (money >= 10) {
      money -= 10;
      hoes++;
    }
  }
  else if (type == WATER) {
    water += money * 1000;
    money = 0;
  } else if (type == SAVE) {
    struct SavedData {
      uint16_t money;
      uint16_t seeds;
      uint16_t fruits;
      uint16_t hoes;
      uint8_t mapelements[18][10];
      uint8_t treeage[18][10];
      uint8_t taxtime;
      uint8_t tax;
      uint16_t water;
    };
    SavedData save;
    save.money = money;
    save.seeds = seeds;
    save.fruits = fruits;
    save.hoes = hoes;
    memcpy(save.mapelements, mapelements, sizeof(mapelements));
    memcpy(save.treeage, treeage, sizeof(treeage));
    save.taxtime = taxtime;
    save.tax = tax;
    save.water = water;
    EEPROM.put(EEPROM_ADDRESS, save);
  } else if (type == LOAD) {
    struct SavedData {
      uint16_t money;
      uint16_t seeds;
      uint16_t fruits;
      uint16_t hoes;
      uint8_t mapelements[18][10];
      uint8_t treeage[18][10];
      uint8_t taxtime;
      uint8_t tax;
      uint16_t water;
    };
    SavedData save;
    EEPROM.get(EEPROM_ADDRESS, save);
    money = save.money;
    seeds = save.seeds;
    fruits = save.fruits;
    hoes = save.hoes;
    memcpy(mapelements, save.mapelements, sizeof(save.mapelements));
    memcpy(treeage, save.treeage, sizeof(save.treeage));
    taxtime = save.taxtime;
    tax = save.tax;
    water = save.water;
  } else if (type == 0) {
    if (hoes) {
      mapelements[x][y] = FARMLAND;
      hoes--;
    }
  }
}


void setup() {
  pinMode(UPPIN, INPUT_PULLUP);
  pinMode(DWNPIN, INPUT_PULLUP);
  pinMode(LFTPIN, INPUT_PULLUP);
  pinMode(RHTPIN, INPUT_PULLUP);
  pinMode(APIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  dsp.begin(SSD1306_SWITCHCAPVCC,  0x3C);
  dsp.clearDisplay();

  beep();

  dsp.setTextSize(2);
  dsp.setTextColor(1);
  dsp.setCursor(0, 0);
  dsp.cp437(true);
  dsp.write("Mapster");
  dsp.setTextSize(1);
  dsp.write("\n\nBy AzizBgBoss");
  dsp.display();
  delay(1000);
  dsp.clearDisplay();
  lasttime = millis() / 1000;
}

void loop() {
  if (digitalRead(UPPIN) == LOW) {
    isFacingY = true;
    yOffset++;
  } else if (digitalRead(DWNPIN) == LOW) {
    isFacingY = true;
    yOffset--;
  }

  if (digitalRead(LFTPIN) == LOW) {
    isFacingY = false;
    xOffset++;
  } else if (digitalRead(RHTPIN) == LOW) {
    isFacingY = false;
    xOffset--;
  }

  if ((millis() / 1000) - lasttime > 1) {
    for (uint8_t x = 0; x < 18; x++) {
      for (uint8_t y = 0; y < 10; y++) {
        switch (mapelements[x][y]) {
          case FARMLAND:
            if (treeage[x][y] > 0 && treeage[x][y] < 3) {
              if (water > 0) {
                water--;
                if (random(0, 100) == 69) {
                  treeage[x][y]++;
                  beep();
                }
              } else {
                if (random(0, 50) == 21) {
                  treeage[x][y]--;
                }
              }
            }
            break;
        }
      }
    }
    taxtime--;
    lasttime = millis() / 1000;
  }

  if (taxtime < 1) {
    taxtime = 200;
    if (money >= tax) money -= tax;
    else money = 0;
    tax = random(0, 11);
  }

  if (xOffset > 64) xOffset = 64;
  else if (xOffset < -80) xOffset = -80;
  if (yOffset > 32) yOffset = 32;
  else if (yOffset < -48) yOffset = -48;

  dsp.clearDisplay();

  // Draw bitmap at the new position

  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 9; x++) {
      dsp.drawBitmap(x * 16 + xOffset, y * 16 + yOffset, mapstone, 16, 16, WHITE);
    }
  }
  if (isFacingY == true) dsp.drawBitmap(60, 28, mapplayery, 8, 8, BLACK);
  else dsp.drawBitmap(60, 28, mapplayerx, 8, 8, BLACK);

  for (uint8_t x = 0; x < 18; x++) {
    for (uint8_t y = 0; y < 10; y++) {
      if (xOffset <= (x * (-8)) + 64 && xOffset > (x * (-8)) + 57 && yOffset <= (y * (-8)) + 32 && yOffset > (y * (-8)) + 25 ) {
        if (digitalRead(APIN) == LOW) {
          interact(x, y);
        }
        switch (mapelements[x][y]) {
          case COIN: money++; mapelements[x][y] = 0; break;
        }
      }
      switch (mapelements[x][y]) {
        case COIN: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapcoin, 8, 8, BLACK); break;
        case SEEDVILLAGER: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapsvillager, 8, 8, BLACK); break;
        case FRUITVILLAGER: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapfvillager, 8, 8, BLACK); break;
        case HOEVILLAGER: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, maphvillager, 8, 8, BLACK); break;
        case FARMLAND: if (treeage[x][y] == 0) dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapfarmland, 8, 8, BLACK);
          else if (treeage[x][y] == 1) dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, maptree1, 8, 8, BLACK);
          else if (treeage[x][y] == 2) dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, maptree2, 8, 8, BLACK);
          else if (treeage[x][y] == 3) dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, maptree3, 8, 8, BLACK);
          break;
        case SAVE: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapsave, 8, 8, BLACK); break;
        case LOAD: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapload, 8, 8, BLACK); break;
        case WATER: dsp.drawBitmap(x * 8 + xOffset, y * 8 + yOffset, mapwater, 8, 8, BLACK); break;
      }
    }
  }

  dsp.fillRect(1, 1, 127, 17, BLACK);
  dsp.drawRect(0, 0, 128, 19, WHITE);
  dsp.setCursor(2, 2);
  dsp.print(money);
  dsp.write("$ ");
  dsp.print(seeds);
  dsp.write(7);
  dsp.write(" ");
  dsp.print(fruits);
  dsp.write(5);
  dsp.write(" ");
  dsp.print(hoes);
  dsp.write(0xF4);
  dsp.write(" T ");
  dsp.print(tax);
  dsp.write(" ");
  dsp.print(taxtime);
  dsp.write(" ");
  dsp.write(0xF7);
  dsp.print(water);
  dsp.display();

  delay(10);
}