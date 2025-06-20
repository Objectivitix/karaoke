/**
 * messages.h
 * 
 * Stores constant string messages that are used in Karaoke's UI.
 * All strings stored in PROGMEM to save SRAM on AVR-based boards.
 * 
 * Author: Calo Zheng
 * Current version date: June 20, 2025
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <Arduino.h>

const char starting[] PROGMEM = "Starting up...";

const char useMonitor1[] PROGMEM = "Select song in";
const char useMonitor2[] PROGMEM = "Serial Monitor";

const char select1[] PROGMEM = "Select your song! (Enter its number.)";
const char select2[] PROGMEM = "=====================================";
const char select3[] PROGMEM = "  1 | Defying Gravity";
const char select4[] PROGMEM = "  2 | Mary Had a Little Lamb";
const char select5[] PROGMEM = "  Q [ Quit program ]";

const char invalid[] PROGMEM = "Invalid input. Enter 1, 2, or Q.";

const char goodbye1[] PROGMEM = "Thanks for using";
const char goodbye2[] PROGMEM = "Karaoke. Bye!";
const char goodbyeSerial[] PROGMEM = "Shutting down...";

#endif
