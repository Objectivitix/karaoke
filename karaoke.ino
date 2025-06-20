#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Tone.h>
#include "pitches.h"

// note duration constants
#define STN   1.0/4     // sixteenth
#define ETH   2.0/4     // eighth
#define DET   3.0/4     // dotted eighth
#define QTR   1.0       // quarter
#define DQT   1.5       // dotted quarter
#define HLF   2.0       // half
#define DHF   3.0       // dotted half
#define WHL   4.0       // great big whole note

#define LCD_REG 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

using namespace std;

LiquidCrystal_I2C lcd(LCD_REG, LCD_WIDTH, LCD_HEIGHT);
String emptyLine;  // used for LCD's `clearLine` function

// light show LED pins
const int LIGHTS_N = 4;
const int LIGHTS[LIGHTS_N] = {2, 3, 4, 5};

// tone generators and pins
const int MAX_TRACKS_N = 2;
Tone tones[MAX_TRACKS_N];
const int TONE_PINS[MAX_TRACKS_N] = {9, 10};

const int MAX_NOTES_N = 100;
const int MAX_NOTES_N_LONG = 1000;

struct Track {
  int notesN;
  long startDelay;
  int lyricsScrollDelay;
  const int* pitches;
  const float* durations;
  const float* liaisons;
  const char* lyrics;
};

struct Song {
  int bpm;

  int tracksN;
  // melody at index 0 and assumed to be longest track
  const Track* tracks;

  // synced to melody track
  const bool (*lightIsOn)[4];

  // convert BPM to ms per beat
  long beatDuration() const {
    return 60000 / bpm;
  }
};

const int defyMelodyPitches[] PROGMEM = {
  // no   wiz - ard  that there is   or  wa - as
    Db4,  Gb4,   F4, Eb4,  Db4, Gb4, F4, F4, Eb4,
  // is   ev - er  gon - na-  (deco) bring-(deco)
    Db4, Gb4, F4, Eb4, Db4, Eb4, F4, F4, Gb4, Eb4,
  // ME - EE - EE  DOWN --! Look at   her, she's
    Eb5, F5, Db5, Db5, Db5, Bb3, Ab3, Bb3, F3,
  // wick -ed! Bring  me  DOWN --  --   --   --!
    Bb3,  Ab3,  Ab4, Eb5, Eb5, F5, Eb5, Db5, Db5,
  // (war cry)
    Ab4, Db5, Eb5, Db5, Eb5, Db5, Bb4, Db5, Eb5, Bb4, Db5
};

const float defyMelodyDurations[] PROGMEM = {
  QTR, ETH, QTR, QTR, QTR, ETH, QTR, ETH, QTR,
  QTR, ETH, QTR, QTR, ETH, STN, STN, QTR, STN, DET,
  HLF, ETH, QTR, WHL, DHF, ETH, ETH, DQT, ETH,
  ETH, DQT, HLF, HLF, ETH, QTR, STN, WHL, DHF,
  DET, STN, STN, STN, STN, STN, STN, STN, ETH, ETH, WHL
};

const float defyMelodyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0, 0, 0.3, 0, 0, 2,
  0, 0, 0.4, 0, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.6, 0.05, 0.05, 0, 0, 0, 0, 1.7,
  0.1, 0, 0, 0, 0, 0, 0, 0, 0.1, 0.1, 0
};

const char defyMelodyLyrics[] PROGMEM = "            "
  "No  wizard  that  there  is or was      "
  "is    ev-  er    gon-  na   bring               "
  "MEEEEEEEEEEEEE   DOWN-----------------------!       Look at her,  she's"
  " wicked!      Bring   meeeeee  DOWN-------!                                                     "
  "Ah----------------!";

const int defyHarmonyPitches[] PROGMEM = {
  // I  hope you're ha - ppy  --
    Db5, Db5,  C5,  Bb4, Ab4, Ab4,
  // So  we've got  to  bring her
    Bb3,  C4,  Db4, DS4, E4,  FS4
};

const float defyHarmonyDurations[] PROGMEM = {
  QTR, ETH, QTR, QTR, DQT, HLF,
  QTR, QTR, HLF, HLF, HLF, DQT
};

const float defyHarmonyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0, 6.5,
  0.1, 0.1, 0.1, 0.1, 0.1, 0.1
};

const char defyHarmonyLyrics[] PROGMEM = "              "
  "I hope you're happ-  y                                                                           "
  "So  we've  got   to      bring    her";

const Track defyTracks[] = {
  { 48, 2000, 200, defyMelodyPitches, defyMelodyDurations, defyMelodyLiaisons, defyMelodyLyrics },
  { 12, 13000, 200, defyHarmonyPitches, defyHarmonyDurations, defyHarmonyLiaisons, defyHarmonyLyrics }
};

const bool defyLights[][4] = {
  {1, 0, 0, 0},  // no
  {0, 1, 0, 0},  // wiz-
  {0, 0, 1, 0},  // ard
  {0, 0, 0, 1},  // that
  {1, 0, 0, 0},  // there
  {0, 1, 0, 0},  // is
  {0, 0, 1, 0},  // or
  {0, 0, 1, 0},  // wa-
  {0, 0, 0, 1},  // as
  {1, 0, 0, 0},  // is
  {0, 1, 0, 0},  // ev-
  {0, 0, 1, 0},  // er
  {0, 0, 0, 1},  // gon-
  {1, 0, 0, 1},  // na-
  {1, 1, 0, 1},  // (deco)
  {0, 0, 1, 0},  // (deco)
  {0, 0, 1, 0},  // bring-
  {0, 1, 0, 0},  // (deco)
  {0, 0, 0, 1},  // (deco)
  {1, 0, 0, 1},  // ME-
  {0, 1, 1, 0},  // EE-
  {0, 0, 0, 0},  // EE-
  {1, 1, 1, 1},  // DOWN--
  {1, 1, 1, 1},  // --!
  {0, 1, 0, 0},  // Look
  {0, 0, 1, 0},  // at
  {1, 0, 0, 1},  // her,
  {0, 1, 1, 0},  // she's
  {0, 0, 0, 1},  // wick-
  {0, 0, 1, 1},  // ed!
  {1, 0, 0, 0},  // Bring
  {1, 1, 0, 0},  // me
  {1, 1, 1, 1},  // DOWN--
  {1, 1, 1, 0},  // --
  {1, 1, 0, 0},  // --
  {1, 0, 0, 0},  // --
  {1, 0, 0, 0},  // --!
  {1, 0, 0, 0},  // (war cry)
  {0, 0, 1, 0},  //
  {1, 0, 0, 1},  //
  {0, 1, 0, 0},  //
  {0, 0, 0, 1},  //
  {0, 0, 1, 0},  //
  {0, 1, 0, 0},  //
  {1, 0, 1, 0},  //
  {0, 1, 0, 1},  //
  {0, 0, 0, 0},  //
  {1, 1, 1, 1},  //
  {1, 1, 1, 1},  //
};

const int maryMelodyPitches[] PROGMEM = {
  Ab4, Db5, Eb5, Db5, Eb5, Db5, Bb4, Db5, Eb5, Bb4, Db5
};

const float maryMelodyDurations[] PROGMEM = {
  DET, STN, STN, STN, STN, STN, STN, STN, ETH, ETH, WHL
};

const float maryMelodyLiaisons[] PROGMEM = {
  0.1, 0, 0, 0, 0, 0, 0, 0, 0.1, 0.1, 0
};

const char maryMelodyLyrics[] PROGMEM = "Mary";

const int maryHarmonyPitches[] PROGMEM = {
  // I  hope you're ha - ppy  --
    Db5, Db5,  C5,  Bb4, Ab4, Ab4
};

const float maryHarmonyDurations[] PROGMEM = {
  QTR, ETH, QTR, DQT, DQT, HLF
};

const float maryHarmonyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0, 0
};

const Track maryTracks[] = {
  { 11, 2000, 200, maryMelodyPitches, maryMelodyDurations, maryMelodyLiaisons, maryMelodyLyrics },
  // { 6, 13000, maryHarmonyPitches, maryHarmonyDurations, maryHarmonyLiaisons }
};

const bool maryLights[][4] = {
  {1, 0, 0, 0},  // no
  {0, 1, 0, 0},  // wiz-
  {0, 0, 1, 0},  // ard
  {0, 0, 0, 1},  // that
  {1, 0, 0, 0},  // there
  {0, 1, 0, 0}   // is
};

const Song songs[] = {
  { 130, 2, defyTracks, defyLights },
  { 130, 1, maryTracks, maryLights }
};

bool finished[MAX_TRACKS_N];
bool allFinished = false;

long beatDuration;

long prevTimes[MAX_TRACKS_N];
long currConvertedDurations[MAX_TRACKS_N];
long currTotalDurations[MAX_TRACKS_N];

int noteIndices[MAX_TRACKS_N];

long scrollPrevTimes[LCD_HEIGHT];
int lyricsIndices[LCD_HEIGHT];

void offLCD() {
  lcd.clear();
  lcd.noBacklight();
}

void clearLine() {
  lcd.print(emptyLine);
}

int progmemStrlen(const char* s) {
  int len = 0;
  while (pgm_read_byte_near(s + len) != '\0') {
    len++;
  }
  return len;
}

void printSubstring(const char* s, int startIndex) {
  char buffer[LCD_WIDTH + 1];
  int len = progmemStrlen(s);

  for (int i = 0; i < LCD_WIDTH; i++) {
    int index = startIndex + i;
    if (index < len) {
      buffer[i] = pgm_read_byte_near(s + index);
    } else {
      buffer[i] = ' ';
    }
  }
  buffer[LCD_WIDTH] = '\0';
  lcd.print(buffer);
}

void updateScroll(const Song& song, long currTime, int skip) {
  for (int i = 0; i < LCD_HEIGHT; i++) {
    const Track& track = song.tracks[i];

    if (currTime - scrollPrevTimes[i] > track.lyricsScrollDelay) {
      scrollPrevTimes[i] = currTime;
      lcd.setCursor(0, i);
      printSubstring(track.lyrics, lyricsIndices[i]);
      lyricsIndices[i] += skip;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("hi?");

  for (int i = 0; i < MAX_TRACKS_N; i++) {
    pinMode(TONE_PINS[i], OUTPUT);
    tones[i].begin(TONE_PINS[i]);
  }

  for (int i = 0; i < LIGHTS_N; i++) {
    pinMode(LIGHTS[i], OUTPUT);
  }

  beatDuration = songs[0].beatDuration();
  Serial.println(String(beatDuration));

  for (int i = 0; i < songs[0].tracksN; i++) {
    finished[i] = false;
    prevTimes[i] = 0;
    currConvertedDurations[i] = songs[0].tracks[i].startDelay;
    currTotalDurations[i] = songs[0].tracks[i].startDelay;
    noteIndices[i] = 0;
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  for (int i = 0; i < LCD_WIDTH; i++) {
    emptyLine += " ";
  }

  for (int i = 0; i < LCD_HEIGHT; i++) {
    scrollPrevTimes[i] = songs[0].tracks[i].startDelay;
    lyricsIndices[i] = 0;
  }
}

void loop() {
  long currTime = millis();

  updateScroll(songs[0], currTime, 2);

  for (int track = 0; track < songs[0].tracksN; track++) {
    long elapsed = currTime - prevTimes[track];

    if (track == 0 && elapsed > currConvertedDurations[0]) {
      for (int i = 0; i < 4; i++) {
        digitalWrite(LIGHTS[i], LOW);
      }

      if (allFinished) {
        offLCD();
        while (true);
      }
    }

    if (elapsed <= currTotalDurations[track]) {
      continue;
    }

    if (finished[track]) {
      continue;
    }

    if (track == 0) {
      for (int i = 0; i < LIGHTS_N; i++) {
        if (songs[0].lightIsOn[noteIndices[0]][i]) {
          digitalWrite(LIGHTS[i], HIGH);
        }
      }
    }

    prevTimes[track] = currTime;
    currConvertedDurations[track] = beatDuration * pgm_read_float(&songs[0].tracks[track].durations[noteIndices[track]]);
    currTotalDurations[track] = currConvertedDurations[track] * (1 + pgm_read_float(&songs[0].tracks[track].liaisons[noteIndices[track]]));

    Serial.println("track:" + String(track));
    Serial.println("index: " + String(noteIndices[track]));
    Serial.println("prev: " + String(prevTimes[track]));

    tones[track].play(pgm_read_word(&songs[0].tracks[track].pitches[noteIndices[track]]), currConvertedDurations[track]);

    noteIndices[track]++;

    if (noteIndices[track] == songs[0].tracks[track].notesN) {
      finished[track] = true;
    }
  }

  for (int track = 0; track < songs[0].tracksN; track++) {
    if (!finished[track]) {
      return;
    }
  }

  allFinished = true;
}
