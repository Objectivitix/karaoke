#include <Vector.h>
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

using namespace std;

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
  const int* pitches;
  const float* durations;
  const float* liaisons;
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
  // ME - EE - EE  DOWN!
    Eb5, F5, Db5, Db5
};

const float defyMelodyDurations[] PROGMEM = {
  QTR, ETH, QTR, QTR, QTR, ETH, QTR, ETH, QTR,
  QTR, ETH, QTR, QTR, ETH, STN, STN, QTR, STN, DET,
  HLF, ETH, QTR, HLF
};

const float defyMelodyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0, 0, 0.3, 0, 0, 2,
  0, 0, 0.4, 0
};

const int defyHarmonyPitches[] PROGMEM = {
  // no   wiz - ard  that there is   or  wa - as
    C6, D6, E6, F6, G6, A6, B6, C7
};

const float defyHarmonyDurations[] PROGMEM = {
  WHL, HLF, HLF, HLF, QTR, QTR, QTR, QTR
};

const float defyHarmonyLiaisons[] PROGMEM = {
  0, 0, 0, 0, 0, 0, 0, 0
};

const Track defyTracks[] = {
  { 23, defyMelodyPitches, defyMelodyDurations, defyMelodyLiaisons },
  { 8, defyHarmonyPitches, defyHarmonyDurations, defyHarmonyLiaisons }
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
  {1, 0, 0, 1},  // EE-
  {1, 1, 1, 1}   // DOWN!
};

const Song defy = {
  130,
  2,
  defyTracks,
  defyLights
};

bool finished[MAX_TRACKS_N];
bool allFinished = false;

long beatDuration;

const long START_DELAY = 2000;
long prevTimes[MAX_TRACKS_N];
long currConvertedDurations[MAX_TRACKS_N];
long currTotalDurations[MAX_TRACKS_N];

int noteIndices[MAX_TRACKS_N];

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

  beatDuration = defy.beatDuration();
  Serial.println(String(beatDuration));

  for (int i = 0; i < defy.tracksN; i++) {
    finished[i] = false;
    prevTimes[i] = 0;
    currConvertedDurations[i] = START_DELAY;
    currTotalDurations[i] = START_DELAY;
    noteIndices[i] = 0;
  }
}

void loop() {
  long currTime = millis();

  for (int track = 0; track < defy.tracksN; track++) {
    if (finished[track]) {
      continue;
    }

    long elapsed = currTime - prevTimes[track];

    if (track == 0 && elapsed > currConvertedDurations[0]) {
      for (int i = 0; i < 4; i++) {
        digitalWrite(LIGHTS[i], LOW);
      }
    }

    if (elapsed <= currTotalDurations[track]) {
      continue;
    }

    if (allFinished) {
      noInterrupts();
      while (true);
    }

    if (track == 0) {
      for (int i = 0; i < LIGHTS_N; i++) {
        if (defy.lightIsOn[noteIndices[0]][i]) {
          digitalWrite(LIGHTS[i], HIGH);
        }
      }
    }

    prevTimes[track] = currTime;
    currConvertedDurations[track] = beatDuration * pgm_read_float(&defy.tracks[track].durations[noteIndices[track]]);
    currTotalDurations[track] = currConvertedDurations[track] * (1 + pgm_read_float(&defy.tracks[track].liaisons[noteIndices[track]]));

    Serial.println("track:" + String(track));
    Serial.println("index: " + String(noteIndices[track]));
    Serial.println("dur: " + String(currConvertedDurations[track]));

    tones[track].play(pgm_read_word(&defy.tracks[track].pitches[noteIndices[track]]), currConvertedDurations[track]);

    noteIndices[track]++;

    if (noteIndices[track] == defy.tracks[track].notesN) {
      finished[track] = true;
    }
  }

  for (int track = 0; track < defy.tracksN; track++) {
    if (!finished[track]) {
      return;
    }
  }

  allFinished = true;
}
