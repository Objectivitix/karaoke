#include <vector>
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

// buzzer pin
const int BUZZER = 8;

struct Song {
  int bpm;
  int notesN;
  vector<int> pitches;
  vector<double> durations;
  vector<double> liaisons;
  vector<vector<bool>> lightIsOn;

  // convert BPM to ms per beat
  long beatDuration() const {
    return 1000 * 60 / bpm;
  }
};

Song defy;

long currBeatDuration;

const int START_DELAY = 2000;
long prevTime = 0;
long currTrueDuration = START_DELAY;
long currTotalDuration = START_DELAY;

int noteIndex = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < LIGHTS_N; i++) {
    pinMode(LIGHTS[i], OUTPUT);
  }

  // set up each song
  defy.bpm = 130;
  defy.notesN = 23;
  defy.pitches =
    // no   wiz - ard  that there is   or  wa - as
    {  Db4, Gb4,   F4, Eb4,  Db4, Gb4, F4, F4, Eb4,
    // is   ev - er  gon - na-  (deco) bring-(deco)
      Db4, Gb4, F4, Eb4, Db4, Eb4, F4, F4, Gb4, Eb4,
    // ME - EE - EE  DOWN!
      Eb5, F5, Db5, Db5};
  defy.durations =
    {QTR, ETH, QTR, QTR, QTR, ETH, QTR, ETH, QTR,
    QTR, ETH, QTR, QTR, ETH, STN, STN, QTR, STN, DET,
    HLF, ETH, QTR, HLF};
  defy.liaisons =
    {0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
    0.3, 0.3, 0.3, 0.3, 0, 0, 0.3, 0, 0, 2,
    0, 0, 0.4, 0
    };
  defy.lightIsOn = {
    {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false},
    {false, false, false, true},
      {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false},
    {false, false, false, true},
      {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false},
    {false, false, false, true},
      {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false},
    {false, false, false, true},
      {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false},
    {false, false, false, true},
      {true, false, false, false},
    {false, true, false, false},
    {false, false, true, false}};

  currBeatDuration = defy.beatDuration();
}

void loop() {
  long currTime = millis();
  long elapsed = currTime - prevTime;

  if (noteIndex > 0 && elapsed > currTrueDuration) {
    for (int i = 0; i < LIGHTS_N; i++) {
      digitalWrite(LIGHTS[i], LOW);
    }
  }

  if (elapsed > currTotalDuration) {
    for (int i = 0; i < LIGHTS_N; i++) {
      if (defy.lightIsOn[noteIndex][i]) {
        digitalWrite(LIGHTS[i], HIGH);
      }
    }

    prevTime = currTime;
    currTrueDuration = currBeatDuration * defy.durations[noteIndex];
    currTotalDuration = currTrueDuration * (1 + defy.liaisons[noteIndex]);

    Serial.println("index: " + String(noteIndex));
    Serial.println("currTrueDuration: " + String(currTrueDuration));
    tone(BUZZER, defy.pitches[noteIndex], currTrueDuration);

    noteIndex++;

    if (noteIndex == defy.notesN) {
      noteIndex = 0;
    }
  }
}
