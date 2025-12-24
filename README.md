# arduino-rc-wheelchair-esp32


> This project will be compiled and push to the ESP32 WROOM-32 using Arduino IDE by the client.
>
> That is the reason why all files remain in the `./src` folder

> For remote for this machine, please click this dedicated [repository](https://github.com/jxmked/arduino-rc-wheelchair-esp32-glove) for that. __Salamuch__ 💖.

## Adjustment and configurations

#### Adjusting Motor Max Power
-> Goto file `constants.h` -> and change `MOTOR_MAX_POWER` from `0` to `255` integer value

> Note: If the `USE_MOTOR_DRIVER` is set to 0 or if the `USE_MOTOR_PWM_CHANNEL` is set to 0 the `MOTOR_MAX_POWER` will not work. Since, the code will force to switch to use the `digitalWrite` instead of using `analogWrite`. Maybe using relay instead of high current motor driver.

#### Adjusting distance threshold for Distance Sensor
-> Goto file `constants.h` -> and change `OBSTACLE_DISTANCE_THRESHOLD`

> Note: The actual distance is not __60%__ and above accurate ;).

#### How to turn on and connect to the remote?

1. Turn on the remote first
2. Turn on the wheelchair and wait for the built-in blue light
   from ESP32 to turn on. (Means, it's connected. No more steps if this happens)
3. If the remote shows green light while the Wheelchair built-in blue light remains off, Turn off the wheelchair and wait for the remote to change its light from green to red.
4. Go back to step 2 :).

## Modules & Hardwares
 - ESP32 WROOM-32
 - Push Button
 - LED (4 Pieces w/ unique colors)
 - 5v Active Buzzer
 - Sharp 2Y0A02 - Distance Sensor
 - MY1016Z Brush Motor - 24v300watts (2 Pieces)
 - LM2596 DC-DC Step-down Converter
 - IBT-2 - BTS7960 module (2 Pieces)
 - 12v 10Ah battery 

## Wirings and connnections


| ESP32 Pin | Type | Output Pin | Mode |
|:---:|:---:|:---:|:---:|
| 13 | Analog | Sharp 2Y0A02 Data | Input |
| 21 | Digital | Override Push Btn | Input-Pulldown |
| 22 | Digital | Buzzer + | Output |
| 16 | Digital | LED RED + | Output |
| 17 |Digital | LED GREEN + | Output |
| 18 | Digital | LED YELLOW + | Output |
| 19 | Digital | LED BLUE + | Output |
| 4 | Digital | (1 & 2, Left & Rigt) IBT-2 Enable Pin | Output | 
| 25 | Analog | (1) IBT-2 LPWM | Output |
| 26 | Analog | (1) IBT-2 RPWM | Output |
| 34 | Analog | (1) IBT-2 L_IS | Input |
| 35 | Analog | (1) IBT-2 R_IS | Input |
| 32 | Analog | (2) IBT-2 LPWM | Output |
| 33 | Analog | (2) IBT-2 RPWM | Output |
| 36 | Analog | (2) IBT-2 L_IS | Input |
| 39 | Analog | (2) IBT-2 R_IS | Input |

| Section | IBT-2 BTS7960 Pin | Wired In |
|:---:|:---:|:---:|
| 1 | M- | (1) Motor (Black) Negative Contact |
| 1 | M+ | (1) Motor (Red) Positive Contact |
| 1 | B+ | Positive (+) 12v 10Ah Battery |
| 1 | B- | Negative (-) 12v 10Ah Battery |
| 2 | M- | (2) Motor (Black) Negative Contact |
| 2 | M+ | (2) Motor (Red) Positive Contact |
| 2 | B+ | Positive (+) 12v 10Ah Battery |
| 2 | B- | Negative (-) 12v 10Ah Battery |

## Circuit Diagram

![Wheelchair Circuit Diagram](./schematic/diagram%20-%20wire%20colored.svg)

This is the circuit diagram with black and white only.

![Wheelchair Circuit Diagram](./schematic/diagram%20-%20b&w.svg)

