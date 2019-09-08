HardwareSerial& Arcade = Serial1;  //rename Serial1 port to Arcade

//declare pins
const int MAGPIN = 2;

const int VERTSWITCHED = 3;
const int VERTUP = 4;
const int VERTDOWN = 5;

const int HORSWITCHED = 6;
const int HORLEFT = 7;
const int HORRIGHT = 8;

const int ALARMPIN = 9;

const int ACTSWITCHED = 30;
const int ACTFORWARD = 31;
const int ACTBACK = 32;

const int UNUSED33 = 33;

const int ACTRETRACTSWITCH = 10;

const int ACTCONTACTPIN = 34;

const unsigned char MAGCHAR = 0b10000000;
const unsigned char UPCHAR = 0b01000000;
const unsigned char DOWNCHAR = 0b00100000;
const unsigned char LEFTCHAR = 0b00010000;
const unsigned char RIGHTCHAR = 0b00001000;
const unsigned char FORWARDCHAR = 0b00000100;
const unsigned char BACKCHAR = 0b00000010;
const unsigned char RESETCHAR = 0b00000001;

const unsigned char ALLRELAYSOFF = 0b00000000;

bool alarming = false;  //record if alarm should be sounding

long actTime = 0;

//---------------------------

void setup() {
    initPins();

    Serial.begin(9600);
    Arcade.begin(9600);
}

//---------------------------

void initPins() {
    pinMode(MAGPIN, OUTPUT);
    digitalWrite(MAGPIN, LOW);

    pinMode(VERTSWITCHED, OUTPUT);
    digitalWrite(VERTSWITCHED, LOW);
    pinMode(VERTUP, OUTPUT);
    digitalWrite(VERTUP, LOW);
    pinMode(VERTDOWN, OUTPUT);
    digitalWrite(VERTDOWN, LOW);

    pinMode(HORSWITCHED, OUTPUT);
    digitalWrite(HORSWITCHED, LOW);
    pinMode(HORLEFT, OUTPUT);
    digitalWrite(HORLEFT, LOW);
    pinMode(HORRIGHT, OUTPUT);
    digitalWrite(HORRIGHT, LOW);

    pinMode(ALARMPIN, OUTPUT);
    digitalWrite(ALARMPIN, LOW);

    pinMode(ACTSWITCHED, OUTPUT);
    digitalWrite(ACTSWITCHED, LOW);
    pinMode(ACTFORWARD, OUTPUT);
    digitalWrite(ACTFORWARD, LOW);
    pinMode(ACTBACK, OUTPUT);
    digitalWrite(ACTBACK, LOW);

    pinMode(ACTRETRACTSWITCH, INPUT);
    pinMode(ACTCONTACTPIN, INPUT);

    pinMode(UNUSED33, OUTPUT);
    digitalWrite(UNUSED33, LOW);
}

//---------------------------

void loop() {
    checkActuatorExtension();

    checkLose();

    if (Serial.available())
        checkManualInput();

    /*
    encode data as unsigned char from arcade as follows:
      format: item (off/on 0/1)

      mag|up|down|left|right|forward|back|reset
      7  |6 |5   |4   |3    |2      |1   |0
  */

    if (Arcade.available())
        updateRelays(inputFromArcade());
}

//---------------------------

void checkActuatorExtension() {
    if (digitalRead(ACTRETRACTSWITCH) == LOW) {  //disable movement when actuator is out
        digitalWrite(VERTSWITCHED, LOW);
        digitalWrite(VERTUP, LOW);
        digitalWrite(VERTDOWN, LOW);
        digitalWrite(HORSWITCHED, LOW);
        digitalWrite(HORLEFT, LOW);
        digitalWrite(HORRIGHT, LOW);
    }
}

//---------------------------

void checkLose() {  //if the act circuit is open when it's supposed to be close, you lose
    if ((digitalRead(ACTCONTACTPIN) == LOW) && (digitalRead(ACTFORWARD)) && ((millis() - actTime) > 500) && (actTime != 0)) {
        updateRelays(ALLRELAYSOFF);
        alarming = true;
    }
}

//---------------------------

void checkManualInput() {
    String input = Serial.readString();

    int pin = input.toInt();  //if telling pin number to flip
    if ((pin >= 2 && pin <= 9) || (pin >= 30 && pin <= 33)) {
        digitalWrite(pin, !digitalRead(pin));
        return;
    }

    if (input.length() == 8) {  //if inputing binary encoded state, handle
        unsigned char encodedState = 0;
        for (int i = 0; i < 8; ++i) {
            int x = input.charAt(i) - '0';
            if (x != 0 && x != 1) {
                return;
            }
            if (x == 1) {
                encodedState |= (1 << (7 - i));
            }
        }

        //could run checks on data in the future

        updateRelays(encodedState);
    }
}

//---------------------------

unsigned char inputFromArcade() {
    return Arcade.read();

    //could run checks on data in the future
}

//---------------------------

void updateRelays(unsigned char encodedState) {
    //print the encoded state byte
    // for (int i = 0; i < 8; ++i) {
    //     Serial.print((encodedState & (1 << (7 - i))) ? "1" : "0");
    // }
    // Serial.println();

    //args bool activated (true == on)
    updateMag(encodedState & MAGCHAR);

    //args bool reset
    updateAlarm(encodedState & RESETCHAR);

    if (alarming && !(encodedState & RESETCHAR))  //if alarming and reset button is not pressed, do not allow motion
        return;

    bool acting = ((encodedState & FORWARDCHAR) || (digitalRead(ACTRETRACTSWITCH) == LOW)) ? true : false;

    //args bool up, bool down, bool acting
    updateVert(encodedState & UPCHAR, encodedState & DOWNCHAR, acting);

    //args bool left, bool right, bool acting
    updateHor(encodedState & LEFTCHAR, encodedState & RIGHTCHAR, acting);

    //args bool forward, bool back
    updateAct(encodedState & FORWARDCHAR, encodedState & BACKCHAR);
}

//---------------------------

//args bool activated (true == on)
void updateMag(bool activated) {
    if (activated)
        digitalWrite(MAGPIN, HIGH);
    else
        digitalWrite(MAGPIN, LOW);
}

//---------------------------

void updateVert(bool up, bool down, bool acting) {
    if (up && !acting) {
        digitalWrite(VERTSWITCHED, LOW);
        digitalWrite(VERTUP, HIGH);
        digitalWrite(VERTDOWN, LOW);
        return;
    }

    if (down && !acting) {
        digitalWrite(VERTSWITCHED, HIGH);
        digitalWrite(VERTUP, LOW);
        digitalWrite(VERTDOWN, HIGH);
        return;
    }

    //disable when not active actuator is acting
    digitalWrite(VERTSWITCHED, LOW);
    digitalWrite(VERTUP, LOW);
    digitalWrite(VERTDOWN, LOW);
}

//---------------------------

void updateHor(bool left, bool right, bool acting) {
    if (left && !acting) {
        digitalWrite(HORLEFT, HIGH);
        digitalWrite(HORRIGHT, LOW);
        digitalWrite(HORSWITCHED, LOW);
        return;
    }

    if (right && !acting) {
        digitalWrite(HORLEFT, LOW);
        digitalWrite(HORRIGHT, HIGH);
        digitalWrite(HORSWITCHED, HIGH);
        return;
    }

    //disable when not active or actuator is acting
    digitalWrite(HORSWITCHED, LOW);
    digitalWrite(HORLEFT, LOW);
    digitalWrite(HORRIGHT, LOW);
}

//---------------------------

void updateAct(bool forward, bool back) {
    if (back) {
        digitalWrite(ACTFORWARD, LOW);    //stop forward
        digitalWrite(ACTSWITCHED, HIGH);  //select back
        digitalWrite(ACTBACK, HIGH);      //go back

        actTime = 0;

        return;
    }

    if (forward) {
        if (actTime == 0) {
            actTime = millis();
        }

        digitalWrite(ACTBACK, LOW);      //stop forward
        digitalWrite(ACTSWITCHED, LOW);  //select back
        digitalWrite(ACTFORWARD, HIGH);  //go back
        return;
    }

    actTime = 0;

    //if not activated, disable
    digitalWrite(ACTFORWARD, LOW);
    digitalWrite(ACTBACK, LOW);
    digitalWrite(ACTSWITCHED, LOW);
}

//---------------------------

void updateAlarm(bool reset) {
    if (alarming && !reset) {
        digitalWrite(ALARMPIN, HIGH);  //sound alarm
        return;
    }

    digitalWrite(ALARMPIN, LOW);
    alarming = false;
}
