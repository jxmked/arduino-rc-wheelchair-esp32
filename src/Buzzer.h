#ifndef Buzzer_h
#define Buzzer_h

class Buzzer {
public:
  Buzzer(int buzzer_pin);
  void on();
  void off();

private:
  int __buzzer_pin;
};

#endif