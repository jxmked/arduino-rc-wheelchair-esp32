#ifndef Buzzer_h
#define Buzzer_h

class Buzzer {
 public:
  Buzzer(int buzzer_pin);
  void on();
  void off();

 private:
  uint16_t __buzzer_pin;
};

#endif