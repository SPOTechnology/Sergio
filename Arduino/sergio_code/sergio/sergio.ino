HardwareSerial& Arcade = Serial1;  //rename Serial1 port to Arcade

//declare pins
const int MagPin = 2;

const int VertSwitched = 3;
const int VertUp = 4;
const int VertDown = 5;

const int HorSwitched = 6;
const int HorLeft = 7;
const int HorRight = 8;

const int AlarmPin = 9;

const int ActSwitched = 30;
const int ActForward = 31;
const int ActBack = 32;

const int Unused33 = 33;

const int ActRetractSwitch = 10;

const int ActContactPin = A0;

bool alarming = false;  //record if alarm should be sounding

const unsigned char AllRelaysOff = 0b00000000;

//---------------------------

void setup() {
    initPins();

    Serial.begin(9600);
    Arcade.begin(9600);
}

//---------------------------

void initPins() {
    pinMode(MagPin, OUTPUT);
    digitalWrite(MagPin, LOW);

    pinMode(VertSwitched, OUTPUT);
    digitalWrite(VertSwitched, LOW);
    pinMode(VertUp, OUTPUT);
    digitalWrite(VertUp, LOW);
    pinMode(VertDown, OUTPUT);
    digitalWrite(VertDown, LOW);

    pinMode(HorSwitched, OUTPUT);
    digitalWrite(HorSwitched, LOW);
    pinMode(HorLeft, OUTPUT);
    digitalWrite(HorLeft, LOW);
    pinMode(HorRight, OUTPUT);
    digitalWrite(HorRight, LOW);

    pinMode(AlarmPin, OUTPUT);
    digitalWrite(AlarmPin, LOW);

    pinMode(ActSwitched, OUTPUT);
    digitalWrite(ActSwitched, LOW);
    pinMode(ActForward, OUTPUT);
    digitalWrite(ActForward, LOW);
    pinMode(ActBack, OUTPUT);
    digitalWrite(ActBack, LOW);

    pinMode(ActRetractSwitch, INPUT);

    pinMode(Unused33, OUTPUT);
    digitalWrite(Unused33, LOW);
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
    if (digitalRead(ActRetractSwitch) == LOW) {  //disable movement when actuator is out
        digitalWrite(VertSwitched, LOW);
        digitalWrite(VertUp, LOW);
        digitalWrite(VertDown, LOW);
        digitalWrite(HorSwitched, LOW);
        digitalWrite(HorLeft, LOW);
        digitalWrite(HorRight, LOW);
    }
}

//---------------------------

void checkLose() {  //if the act circuit is open when it's supposed to be close, you lose
    if ((analogRead(ActContactPin) < 100) && (digitalRead(ActForward))) {
        updateRelays(AllRelaysOff);
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
    for (int i = 0; i < 8; ++i) {
        Serial.print((encodedState & (1 << (7 - i))) ? "1" : "0");
    }
    Serial.println();

    //args bool activated (true == on)
    updateMag(encodedState & (1 << 7));

    //args bool reset
    updateAlarm(encodedState & (1 << 0));

    if (alarming && !(encodedState & (1 << 0)))  //if alarming and reset button is not pressed, do not allow motion
        return;

    bool acting = (encodedState & (1 << 1)) || (encodedState & (1 << 2) || (digitalRead(ActRetractSwitch) == LOW)) ? true : false;

    //args bool up, bool down, bool acting
    updateVert(encodedState & (1 << 6), encodedState & (1 << 5), acting);

    //args bool left, bool right, bool acting
    updateHor(encodedState & (1 << 4), encodedState & (1 << 3), acting);

    //args bool forward, bool back
    updateAct(encodedState & (1 << 2), encodedState & (1 << 1));
}

//---------------------------

//args bool activated (true == on)
void updateMag(bool activated) {
    if (activated)
        digitalWrite(MagPin, HIGH);
    else
        digitalWrite(MagPin, LOW);
}

//---------------------------

void updateVert(bool up, bool down, bool acting) {
    if (up && !acting) {
        digitalWrite(VertSwitched, LOW);
        digitalWrite(VertUp, HIGH);
        digitalWrite(VertDown, LOW);
        return;
    }

    if (down && !acting) {
        digitalWrite(VertSwitched, HIGH);
        digitalWrite(VertUp, LOW);
        digitalWrite(VertDown, HIGH);
        return;
    }

    //disable when not active actuator is acting
    digitalWrite(VertSwitched, LOW);
    digitalWrite(VertUp, LOW);
    digitalWrite(VertDown, LOW);
}

//---------------------------

void updateHor(bool left, bool right, bool acting) {
    if (left && !acting) {
        digitalWrite(HorLeft, HIGH);
        digitalWrite(HorRight, LOW);
        digitalWrite(HorSwitched, LOW);
        return;
    }

    if (right && !acting) {
        digitalWrite(HorLeft, LOW);
        digitalWrite(HorRight, HIGH);
        digitalWrite(HorSwitched, HIGH);
        return;
    }

    //disable when not active or actuator is acting
    digitalWrite(HorSwitched, LOW);
    digitalWrite(HorLeft, LOW);
    digitalWrite(HorRight, LOW);
}

//---------------------------

void updateAct(bool forward, bool back) {
    if (back) {
        digitalWrite(ActForward, LOW);    //stop forward
        digitalWrite(ActSwitched, HIGH);  //select back
        digitalWrite(ActBack, HIGH);      //go back
        return;
    }

    if (forward) {
        digitalWrite(ActBack, LOW);      //stop forward
        digitalWrite(ActSwitched, LOW);  //select back
        digitalWrite(ActForward, HIGH);  //go back
        return;
    }

    //if not activated, disable
    digitalWrite(ActForward, LOW);
    digitalWrite(ActBack, LOW);
    digitalWrite(ActSwitched, LOW);
}

//---------------------------

void updateAlarm(bool reset) {
    if (alarming && !reset) {
        digitalWrite(AlarmPin, HIGH);  //sound alarm
        return;
    }

    digitalWrite(AlarmPin, LOW);
    alarming = false;
}
