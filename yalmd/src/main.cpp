/* Source code for Yet Another Latency Measuring Device 
 * Written in 2020 by Andreas Schmid (University of Regensburg)
 * Project Website: https://hci.ur.de/projects/end-to-end-latency
 *
 * Yet Another Latency Measuring Device measures end-to-end latency
 * of systems with a button-equipped input device and a visual display.
 * The microcontroller triggers a button of a modified input device with an optocoupler.
 * A program on the device under test changes the display's color from black to white.
 * This change in brightness is detected by a photo resistor attached to the display.
 * The microcontroller measures the time between triggering the click and
 * the measured brightness surpassing a threshold.
 * The result is sent to a PC via the serial port. Therefore, it can either be received
 * with Arduino's or PlatformIO's serial monitor, or with 'cat /dev/ttyACM0 > logfile.txt'.
 *
 * This program is written for the Arduino Micro, but it should also work
 * with other microcontroller platforms after some minor adjustments.
 */

#include <Arduino.h>

#define OFF 0
#define ON 1

// Pins for the components. Change them if using different ones.
#define PIN_SENSOR A3      // photo resistor (has to be ADC pin)
#define PIN_CLICK A4       // optocoupler
#define PIN_STATE 3        // on/off switch
#define PIN_LED_CLICK A1   // indicator LED (turns on when click is triggered)
#define PIN_LED_THRESH A2  // indicator LED (turns on when display turns bright)

int blackValue = -1;
int whiteValue = -1;

int threshold = -1;

bool lastState = OFF;
bool state = OFF;

unsigned long time;

// Calibration is performed on startup and everytime the on/off switch is turned on.
// Measures a brightness value for the dark display,
// then triggers a click (so the display turns white) and measures another brightness value.
// The value in between those measurements is used as a brightness threshold.
int calibrate()
{
    // measure brightness for black display
    blackValue = analogRead(PIN_SENSOR);

    // trigger click and wait
    digitalWrite(PIN_CLICK, HIGH);
    digitalWrite(PIN_LED_CLICK, HIGH);
    delay(500);

    // measure brightness for white display
    whiteValue = analogRead(PIN_SENSOR);

    // release button and wait
    digitalWrite(PIN_CLICK, LOW);
    digitalWrite(PIN_LED_CLICK, LOW);
    delay(500);

    // average value is used as threshold
    threshold = blackValue - ((blackValue - whiteValue) / 2);

    // send values via serial so they are included in the log file as a comment
    Serial.print("# black: ");
    Serial.print(blackValue);
    Serial.println();

    Serial.print("# white: ");
    Serial.print(whiteValue);
    Serial.println();
    
    Serial.print("# threshold: ");
    Serial.print(threshold);
    Serial.println();

    // sanity check for the threshold
    // a too low threshold normally means that there is something wrong with the sensor
    // e.g. a loose connection or too much ambient light
    if(threshold < 100)
    {
        Serial.println("# error: low threshold");
        return 0;
    }
    
    return 1;
}

void setup()
{
    pinMode(PIN_SENSOR, INPUT);
    pinMode(PIN_CLICK, OUTPUT);
    pinMode(PIN_STATE, INPUT);
    pinMode(PIN_LED_CLICK, OUTPUT);
    pinMode(PIN_LED_THRESH, OUTPUT);

    Serial.begin(9600);

    randomSeed(analogRead(A5));

    delay(1000);
}

void loop()
{
    if(digitalRead(PIN_STATE) == HIGH)
    {
        // wait until switch is turned on
        lastState = OFF;
        delay(100);
        return;
    }
    else
    {
        if(lastState == OFF)
        {
            // if device was off and is turned on, perform calibration process
            // only continue if calibration was successful
            while(!calibrate())
            {
                delay(1000);
            }

            lastState = ON;
        }

        // trigger click and log the current timestamp
        digitalWrite(PIN_CLICK, HIGH);
        digitalWrite(PIN_LED_CLICK, HIGH);
        time = micros();

        while(1)
        {
            // poll the photo sensor's value until the threshold is reached
            if(analogRead(PIN_SENSOR) < threshold)
            {
                // log latency
                Serial.println(micros() - time);
                digitalWrite(PIN_LED_THRESH, HIGH);

                // release button
                delay(500);
                digitalWrite(PIN_CLICK, LOW);
                digitalWrite(PIN_LED_CLICK, LOW);

                // make sure we wait until the display is dark again
                while(analogRead(PIN_SENSOR) < threshold) delay(500);
                digitalWrite(PIN_LED_THRESH, LOW);

                break;
            }
        }

        // wait for a random time between 100 and 1000 ms to make sure we don't accidently sync with anything
        delay(random(100, 1000));
    }
}
