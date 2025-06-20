/**
 * songs.h
 * 
 * Stores musical data for songs used in Karaoke.
 * Includes pitches, durations, liaisons, lyrics, lighting
 * patterns, and metadata like title, BPM, and track structure.
 * 
 * Author: Calo Zheng
 * Current version date: June 20, 2025
 */

#ifndef SONGS_H
#define SONGS_H

#include <Arduino.h>
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

/**
 * A sequence of notes in a song, to be played solo or in
 * polyphony with other such sequences. Also stores info
 * about the corresponding lyrics, if this is a vocal track.
 */
struct Track {
  int notesN;
  long startDelay;
  int lyricsScrollDelay;
  const int* pitches;
  const float* durations;
  const float* liaisons;
  const char* lyrics;  // can be null, in which case it's ignored
};

/**
 * A song consists of several tracks, a light show program
 * synced to the melody track, and a title. It also needs to
 * know about its BPM, and a convenience method is included
 * to convert BPM to ms per beat.
 */
struct Song {
  int bpm;

  int tracksN;
  const Track* tracks;  // melody at index 0 and is longest track

  const bool (*lightIsOn)[4] PROGMEM;

  // broken into multiple lines due to LCD width constraint
  const char* const* title;

  long beatDuration() const {
    return 60000 / bpm;
  }
};

const char defyTitleLineOne[] PROGMEM = "Defying Gravity";
const char defyTitleLineTwo[] PROGMEM = "";
const char* const defyTitle[] = { defyTitleLineOne, defyTitleLineTwo };

const int defyMelodyPitches[] PROGMEM = {
  // No   wiz - ard  that there is   or  wa - as
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
  "MEEEEEEEEEEEEE   DOWN------------------------!      Look at her,  she's"
  " wicked!      Bring   meeeeee  DOWN-------!                                                      "
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

const bool defyLights[][4] PROGMEM = {
  {1, 0, 0, 0},  // No
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

const char maryTitleLineOne[] PROGMEM = "Mary Had a";
const char maryTitleLineTwo[] PROGMEM = "Little Lamb";
const char* const maryTitle[] = { maryTitleLineOne, maryTitleLineTwo };

const int maryMelodyPitches[] PROGMEM = {
  E4, D4, C4, D4, E4, E4, E4,
  D4, D4, D4, E4, G4, G4,
  E4, D4, C4, D4, E4, E4, E4,
  E4, D4, D4, E4, D4, C4
};

const float maryMelodyDurations[] PROGMEM = {
  QTR, QTR, QTR, QTR, QTR, QTR, HLF,
  QTR, QTR, HLF, QTR, QTR, HLF,
  QTR, QTR, QTR, QTR, QTR, QTR, QTR,
  QTR, QTR, QTR, QTR, QTR, WHL
};

const float maryMelodyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3
};

const char maryMelodyLyrics[] PROGMEM = "            "
  "Mar-  y   had   a   litt-le   lamb      "
  "Litt-le  lamb,        litt-le  lamb       "
  "Mar-  y   had   a   litt-le  lamb  "
  "Its fleece was white as snow.";

const int maryHarmonyPitches[] PROGMEM = {
  G4, F4, E4, F4, G4, G4, G4,
  F4, F4, F4, G4, C5, C5,
  G4, F4, E4, F4, G4, G4, G4,
  G4, F4, F4, G4, F4, E4
};

const float maryHarmonyDurations[] PROGMEM = {
  QTR, QTR, QTR, QTR, QTR, QTR, HLF,
  QTR, QTR, HLF, QTR, QTR, HLF,
  QTR, QTR, QTR, QTR, QTR, QTR, QTR,
  QTR, QTR, QTR, QTR, QTR, WHL
};

const float maryHarmonyLiaisons[] PROGMEM = {
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3,
  0.3, 0.3, 0.3, 0.3, 0.3, 0.3
};

const Track maryTracks[] = {
  { 26, 2000, 200, maryMelodyPitches, maryMelodyDurations, maryMelodyLiaisons, maryMelodyLyrics },
  { 26, 2000, 200, maryHarmonyPitches, maryHarmonyDurations, maryHarmonyLiaisons }
};

const bool maryLights[][4] PROGMEM = {
  {0, 1, 0, 0},  // Mar-
  {0, 0, 1, 0},  // y
  {1, 0, 0, 0},  // had
  {0, 0, 0, 1},  // a
  {1, 0, 0, 0},  // litt-
  {0, 1, 0, 0},  // le
  {0, 1, 0, 0},  // lamb/
  {1, 0, 0, 0},  // Litt-
  {0, 1, 0, 0},  // le
  {0, 1, 0, 0},  // lamb,
  {1, 0, 0, 0},  // litt-
  {0, 0, 0, 1},  // le
  {0, 0, 0, 1},  // lamb/
  {0, 1, 1, 0},  // Mar-
  {1, 1, 0, 0},  // y
  {0, 1, 1, 0},  // had
  {0, 0, 1, 1},  // a
  {1, 0, 0, 0},  // litt-
  {0, 1, 0, 0},  // le
  {0, 1, 0, 0},  // lamb/
  {0, 0, 1, 0},  // Its
  {0, 1, 0, 0},  // fleece
  {0, 1, 0, 0},  // was
  {0, 0, 0, 1},  // white
  {1, 0, 0, 0},  // as
  {0, 1, 0, 0}   // snow.
};

const Song songs[] = {
  { 130, 2, defyTracks, defyLights, defyTitle },
  { 150, 2, maryTracks, maryLights, maryTitle }
};

const int songsN = sizeof(songs) / sizeof(songs[0]);

#endif
