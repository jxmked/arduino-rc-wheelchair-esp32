#include "TimeInterval.h"

#include <Arduino.h>

TimeInterval::TimeInterval(unsigned long interval, unsigned long offset,
                           bool autoUpdate)
  : interval(interval),
    autoUpdate(autoUpdate),
    lastTime(0),
    time(0),
    offset(offset),
    paused(false) {
  update();
  lastTime = time;
}

void TimeInterval::update() {
  time = millis() + offset;

  if (paused) lastTime = time;
}

bool TimeInterval::marked(unsigned long holdMillis) {
  if (autoUpdate) update();

  const auto currentTime = time;
  const unsigned long diff = currentTime - lastTime;

  if (diff >= interval) {
    if (diff - interval >= holdMillis) lastTime = currentTime;
    return true;
  }

  return false;
}

void TimeInterval::reset() {
  if (autoUpdate) update();
  lastTime = time;
}

void TimeInterval::pause() {
  paused = true;
}

void TimeInterval::resume() {
  paused = false;
}