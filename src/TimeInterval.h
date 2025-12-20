#ifndef TimeInterval_h
#define TimeInterval_h

class TimeInterval {
public:
  TimeInterval(unsigned long interval, unsigned long offset = 0, bool autoUpdate = false);
  void update();
  bool marked(unsigned long holdMillis = 0);
  void reset();
  void pause();
  void resume();

private:
  unsigned long interval;
  bool autoUpdate;
  unsigned long lastTime;
  unsigned long time;
  unsigned long offset;
  bool paused;
};

#endif
