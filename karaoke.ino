/**
 * karaoke.ino
 * 
 * Entry point for Karaoke, an Arduino-based karaoke player with
 * a LCD lyrics prompter, synchronized light shows, and multi-track
 * audio playback using `Tone.h` library. (On most AVR boards only
 * 2 tracks are supported because we have only 3 timers at our
 * disposal, and one is used for general scheduling with `millis`.)
 * Handles input via Serial Monitor.
 * 
 * Author: Calo Zheng
 * Current version date: June 20, 2025
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Tone.h>
#include "songs.h"
#include "messages.h"

// LCD constants
#define LCD_ADDR 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_WIDTH, LCD_HEIGHT);

// light show LED pins
const int LIGHTS_N = 4;
const int LIGHTS[LIGHTS_N] = {2, 3, 4, 5};

// tone generators and pins
const int MAX_TRACKS_N = 2;
Tone tones[MAX_TRACKS_N];
const int TONE_PINS[MAX_TRACKS_N] = {9, 10};

// state vars: track whether note sequence exhausted
bool trackFinished[MAX_TRACKS_N];
bool songFinished = false;

// state vars: schedule note playing and light show
long prevTimes[MAX_TRACKS_N];
long currConvertedDurations[MAX_TRACKS_N];
long currTotalDurations[MAX_TRACKS_N];
int noteIndices[MAX_TRACKS_N];

// state vars: schedule lyrics scrolling on prompter
long scrollPrevTimes[LCD_HEIGHT];
int lyricsIndices[LCD_HEIGHT];

// current song's index and beat duration
Song currSong;
long beatDuration;

/**
 * Turns off the LCD prompter.
 */
void offLCD() {
  lcd.clear();
  lcd.noBacklight();
}

/**
 * Finds the length of a string in PROGMEM.
 */
int getPStringLength(const char* strPtr) {
  // read byte by byte until we reach end of string
  int len = 0;
  for (; pgm_read_byte_near(strPtr + len) != '\0'; len++);
  return len;
}

/**
 * Reads a string from PROGMEM.
 */
String readPString(const char* strPtr) {
  String result;

  // read byte by byte until we reach end of string
  char c;
  for (int i = 0;; i++) {
    c = pgm_read_byte_near(strPtr + i);
    if (c == '\0') break;
    result += c;
  }

  return result;
}

/**
 * Prints to LCD a substring whose length is `LCD_WIDTH`, given a
 * `startIndex`. Pads with spaces if substring would go past
 * original string length. This is a subroutine for `updateScroll`.
 */
void printSubstring(const char* strPtr, int startIndex) {
  char buffer[LCD_WIDTH + 1];
  int len = getPStringLength(strPtr);

  for (int i = 0; i < LCD_WIDTH; i++) {
    int index = startIndex + i;
    if (index < len) {
      buffer[i] = pgm_read_byte_near(strPtr + index);
    } else {
      buffer[i] = ' ';  // pad with spaces
    }
  }
  buffer[LCD_WIDTH] = '\0';
  lcd.print(buffer);  // print at current cursor
}

/**
 * Updates LCD prompter by checking time and printing
 * advanced substrings if necessary. Called in `loop`.
 */
void updateScroll(const Song& song, long currTime, int skip) {
  for (int i = 0; i < min(song.tracksN, LCD_HEIGHT); i++) {
    const Track& track = song.tracks[i];

    // Skip if lyrics is null or first character is null terminator
    if (!track.lyrics || pgm_read_byte(track.lyrics) == '\0') {
      continue;
    }

    if (currTime - scrollPrevTimes[i] > track.lyricsScrollDelay) {
      // print current "frame" of lyrics for this track
      lcd.setCursor(0, i);
      printSubstring(track.lyrics, lyricsIndices[i]);

      // schedule next frame
      scrollPrevTimes[i] = currTime;
      lyricsIndices[i] += skip;
    }
  }
}

/**
 * Gets a song selection from the user, or quits the program
 * if that is what the user chooses.
 */
int selectSong() {
  Serial.println(readPString(select1));
  Serial.println(readPString(select2));
  Serial.println(readPString(select3));
  Serial.println(readPString(select4));
  Serial.println(readPString(select5));

  // keep looping until user enters valid input
  while (true) {
    if (Serial.available()) {
      delay(300);  // make sure entire message has been sent

      String input = Serial.readString();

      input.toUpperCase();  // for matching with "Q"
      input.trim();  // remove any leading or trailing whitespace

      if (input == "Q") {
        // show shut-off messages on both LCD and Serial Monitor
        printTwoLines(readPString(goodbye1), readPString(goodbye2));
        Serial.println(readPString(goodbyeSerial));
        delay(2000);

        // turn off LCD and halt
        offLCD();
        while (true);
      }

      // parse string as integer and store final character
      char* end;
      int num = strtol(input.c_str(), &end, 10);

      // check if final character was indeed end of string,
      // and check if user-entered integer is within range
      if (*end == '\0' && (num >= 1 && num <= songsN)) {
        return num - 1;  // user has entered valid input; return index
      } else {
        Serial.println(readPString(invalid));
      }
    }
  }
}

/**
 * Resets the state variables seen at the top of this file
 * to prepare a clean slate for the next song.
 */
void resetSongStates() {
  long startTime = millis();

  songFinished = false;

  for (int i = 0; i < currSong.tracksN; i++) {
    long delay = currSong.tracks[i].startDelay;

    trackFinished[i] = false;
    prevTimes[i] = startTime;
    currConvertedDurations[i] = delay;
    currTotalDurations[i] = delay;
    noteIndices[i] = 0;
  }

  for (int i = 0; i < LCD_HEIGHT; i++) {
    Track t = currSong.tracks[i];

    scrollPrevTimes[i] = startTime + t.startDelay - t.lyricsScrollDelay;
    lyricsIndices[i] = 0;
  }
}

/**
 * Shows the home menu in Serial Monitor, and a prompt to use
 * the Serial Monitor on the LCD. If a song is selected, shows
 * its title, calculates its beat duration, and resets song states.
 */
void home() {
  printTwoLines(readPString(useMonitor1), readPString(useMonitor2));
  currSong = songs[selectSong()];

  lcd.clear();

  // show title for 1.5 s
  String line1 = readPString(currSong.title[0]);
  String line2 = readPString(currSong.title[1]);
  printTwoLines(line1, line2);
  delay(1500);

  lcd.clear();
  delay(500);

  beatDuration = currSong.beatDuration();
  resetSongStates();
}

/**
 * Clear and print two strings on top and bottom lines.
 */
void printTwoLines(String a, String b) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(a);
  lcd.setCursor(0, 1);
  lcd.print(b);
}

void setup() {
  Serial.begin(9600);
  Serial.println(readPString(starting));

  // init tone pins and tone players
  for (int i = 0; i < MAX_TRACKS_N; i++) {
    pinMode(TONE_PINS[i], OUTPUT);
    tones[i].begin(TONE_PINS[i]);
  }

  // init LED pins
  for (int i = 0; i < LIGHTS_N; i++) {
    pinMode(LIGHTS[i], OUTPUT);
  }

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  // show home menu on Serial Monitor
  home();
}

void loop() {
  long currTime = millis();

  updateScroll(currSong, currTime, 2);

  for (int trackIndex = 0; trackIndex < currSong.tracksN; trackIndex++) {
    long elapsed = currTime - prevTimes[trackIndex];
    Track currTrack = currSong.tracks[trackIndex];

    // lights off between notes of melody track
    if (trackIndex == 0 && elapsed > currConvertedDurations[0]) {
      for (int i = 0; i < 4; i++) {
        digitalWrite(LIGHTS[i], LOW);
      }

      // after lights off, check if whole song done;
      // if so go back to menu
      if (songFinished) {
        home();
        return;
      }
    }

    // if not yet time to play next note, check next track
    if (elapsed <= currTotalDurations[trackIndex]) {
      continue;
    }

    // skip this track if already finished
    if (trackFinished[trackIndex]) {
      continue;
    }

    // light show only follows melody track
    if (trackIndex == 0) {
      for (int i = 0; i < LIGHTS_N; i++) {
        if (pgm_read_byte(&(currSong.lightIsOn[noteIndices[0]][i]))) {
          digitalWrite(LIGHTS[i], HIGH);
        }
      }
    }

    // schedule next note
    prevTimes[trackIndex] = currTime;
    currConvertedDurations[trackIndex] = beatDuration * pgm_read_float(
      &currTrack.durations[noteIndices[trackIndex]]);
    currTotalDurations[trackIndex] = currConvertedDurations[trackIndex] * (
      1 + pgm_read_float(&currTrack.liaisons[noteIndices[trackIndex]]));  // spacing between notes

    // play next note
    tones[trackIndex].play(
      pgm_read_word(&currTrack.pitches[noteIndices[trackIndex]]),
      currConvertedDurations[trackIndex]
    );

    // advance this track's note index
    noteIndices[trackIndex]++;

    if (noteIndices[trackIndex] == currTrack.notesN) {
      trackFinished[trackIndex] = true;
    }
  }

  // if any track remains unfinished, go to the next
  // iteration of `loop` (keep playing)
  for (int track = 0; track < currSong.tracksN; track++) {
    if (!trackFinished[track]) {
      return;
    }
  }

  // this flag is only set after all tracks finish; then on
  // the next `loop` iteration we'll go back to main menu
  songFinished = true;
}
