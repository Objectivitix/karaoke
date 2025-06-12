#include "pitches.h"

// note duration constants
#define ETH   0.5/4   // eighth
#define QTR   1.0/4   // quarter
#define DQT   1.5/4   // dotted quarter
#define HLF   2.0/4   // half
#define DHF   3.0/4   // dotted half
#define WHL   1       // great big whole note

// light show LED pins
const int LIGHTS_N = 4;
const int LIGHTS[LIGHTS_N] = {2, 3, 4, 5};

// buzzer pin
const int BUZZER = 8;

// music constants
const int BPM = 150;
const int WHOLE_DURATION = 4 * 1000 / (BPM / 60.0);  // convert BPM to seconds per whole note

// song constants
const int NOTES_N = 23;
int pitches[NOTES_N] = 
  {Db4, Gb4, F4, Eb4, Db4, Gb4, F4, Eb4, Db4, Gb4, F4, F4, Eb4, Db4, Eb4, F4, F4, Gb4, Eb4, Eb5, F5, Db5, Db5};
double durations[NOTES_N] =
  {QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR, QTR};
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
int trueDurations[NOTES_N];
int timestamps[NOTES_N + 1];

int noteIndex = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < LIGHTS_N; i++) {
    pinMode(LIGHTS_N, OUTPUT);
  }

  timestamps[0] = START_DELAY;

  for (int i = 0; i < NOTES_N; i++) {
    int trueDuration = WHOLE_DURATION * durations[i];
    trueDurations[i] = trueDuration;

    timestamps[i + 1] = timestamps[i] + trueDuration * 1.3;
  }
}

void loop() {
  int wrappedTime = millis() % timestamps[NOTES_N];
  int prevNoteEndTime = timestamps[noteIndex - 1] - trueDurations[noteIndex] * 0.3;

  if (noteIndex > 0 && wrappedTime > prevNoteEndTime) {
    for (int i = 0; i < LIGHTS_N; i++) {
      digitalWrite(LIGHTS[i], LOW);
    }
  }

  if (wrappedTime < timestamps[NOTES_N - 1] && wrappedTime > timestamps[noteIndex]) {
    for (int i = 0; i < LIGHTS_N; i++) {
      if (lightIsOn[noteIndex][i]) {
        digitalWrite(LIGHTS[i], HIGH);
      }
    }

    Serial.println(noteIndex);
    Serial.println(millis() % timestamps[NOTES_N]);
    tone(BUZZER, pitches[noteIndex], trueDurations[noteIndex]);

    noteIndex++;

    if (noteIndex > NOTES_N) {
      noteIndex = 0;
    }
  }
}
