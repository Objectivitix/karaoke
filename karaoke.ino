#include "pitches.h"

// note duration constants
#define STN   0.25/4    // sixteenth
#define ETH   0.50/4    // eighth
#define DET   0.75/4    // dotted eighth
#define QTR   1.00/4    // quarter
#define DQT   1.50/4    // dotted quarter
#define HLF   2.00/4    // half
#define DHF   3.00/4    // dotted half
#define WHL   1         // great big whole note

// light show LED pins
const int LIGHTS_N = 4;
const int LIGHTS[LIGHTS_N] = {2, 3, 4, 5};

// buzzer pin
const int BUZZER = 8;

// music constants
const int BPM = 130;
// convert BPM to ms per whole note
const long WHOLE_DURATION = 4 * 1000 / (BPM / 60.0);

// song constants
const int NOTES_N = 23;
int pitches[NOTES_N] =
  // no   wiz - ard  that there is   or  wa - as
  {  Db4, Gb4,   F4, Eb4,  Db4, Gb4, F4, F4, Eb4,
  // is   ev - er  gon - na-  (deco) bring-(deco)
     Db4, Gb4, F4, Eb4, Db4, Eb4, F4, F4, Gb4, Eb4,
  // ME - EE - EE  DOWN!
     Eb5, F5, Db5, Db5};
double durations[NOTES_N] =
  {QTR, ETH, QTR, QTR, QTR, ETH, QTR, ETH, QTR,
  QTR, ETH, QTR, QTR, ETH, STN, STN, QTR, STN, DET,
  HLF, ETH, QTR, HLF};
double liaisons[NOTES_N] =
  {0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0, 0, 0.3, 0, 0, 2,
  0, 0, 0.4, 0
  };
boolean lightIsOn[NOTES_N][4] = {
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

const int START_DELAY = 2000;
long prevTime = 0;
long currTrueDuration = START_DELAY;
long currTotalDuration = START_DELAY;

int noteIndex = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < LIGHTS_N; i++) {
    pinMode(LIGHTS_N, OUTPUT);
  }
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
      if (lightIsOn[noteIndex][i]) {
        digitalWrite(LIGHTS[i], HIGH);
      }
    }

    prevTime = currTime;
    currTrueDuration = WHOLE_DURATION * durations[noteIndex];
    currTotalDuration = currTrueDuration * (1 + liaisons[noteIndex]);

    Serial.println("index: " + String(noteIndex));
    Serial.println("currTrueDuration: " + String(currTrueDuration));
    tone(BUZZER, pitches[noteIndex], currTrueDuration);

    noteIndex++;

    if (noteIndex == NOTES_N) {
      noteIndex = 0;
    }
  }
}
