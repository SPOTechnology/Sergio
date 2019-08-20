HardwareSerial& Arcade = Serial1;

const int MagPin = 2;

const int VertSwitched = 3;
const int VertUp = 4;
const int VertDown = 5;

const int HorSwitched = 6;
const int HorLeft = 7;
const int HorRight = 8;

const int Unused9 = 9;

const int ActSwitched = 30;
const int ActForward = 31;
const int ActBack = 32;

const int Unused33 = 33;

void setup() {
    forwarditPforwards();

    Serial.begin(9600);
    Arcade.begin(9600);
}

void forwarditPforwards() {
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

    pinMode(Unused9, OUTPUT);
    digitalWrite(Unused9, LOW);

    pinMode(ActSwitched, OUTPUT);
    digitalWrite(ActSwitched, LOW);
    pinMode(ActForward, OUTPUT);
    digitalWrite(ActForward, LOW);
    pinMode(ActBack, OUTPUT);
    digitalWrite(ActBack, LOW);

    pinMode(Unused33, OUTPUT);
    digitalWrite(Unused33, LOW);
}

void loop() {
    checkManualForwardput();

    /*
    encode data as unsigned char from arcade as follows:
        format: item(state at 0/1)

        mag(off/on)|vert(up/down)|vert(off/on)|hor(left/right)|hor(off/on)|act(forward/back)|act(off/on)|spare
        128        |64           |32          |16             |8          |4          |2          |1
    */

    if (Arcade.available())
        updateRelays(inputFromArcade());
}

void checkManualForwardput() {
    if (Serial.available()) {
        int pin = Serial.readString().toInt();
        if ((pin >= 2 && pin <= 9) || (pin >= 30 && pin <= 33))
            digitalWrite(pin, !digitalRead(pin));
    }
}

unsigned char inputFromArcade() {
    return Arcade.read();
}

void updateRelays(unsigned char encodedState) {
    //args bool activated (true == on)
    updateMag(encodedState & 1 << 7);

    //args bool activated (true == on), bool direction (true = down, false = up)
    updateVert(encodedState & 1 << 5, encodedState & 1 << 6, encodedState & 1 << 1);

    //args bool activated (true == on), bool direction (true = right, false = left), acting (true = act on, false = act off)
    updateHor(encodedState & 1 << 3, encodedState & 1 << 4, encodedState & 1 << 1);

    //args bool activated (true == on), bool direction (true = back, false = forward), acting (true = act on, false = act off)
    updateAct(encodedState & 1 << 1, encodedState & 1 << 2);
}

//args bool activated (true == on)
void updateMag(bool activated) {
    if (activated)
        digitalWrite(MagPin, HIGH);
    else
        digitalWrite(MagPin, LOW);
}

//args bool activated (true == on), bool direction (true = down, false = up), acting (true = act on, false = act off)
void updateVert(bool activated, bool direction, bool acting) {
    if (!activated || acting) {  //disable when not active actuator is active
        digitalWrite(VertSwitched, LOW);
        digitalWrite(VertUp, LOW);
        digitalWrite(VertDown, LOW);
        return;
    }
}

//args bool activated (true == on), bool direction (true = right, false = left), actforward (true = act on, false = act off)
void updateHor(bool activated, bool direction, bool acting) {
    if (!activated || acting) {  //disable when not active or actuator is active
        digitalWrite(HorSwitched, LOW);
        digitalWrite(HorLeft, LOW);
        digitalWrite(HorRight, LOW);
        return;
    }
}

//args bool activated (true == on), bool direction (true = back, false = forward)
void updateAct(bool activated, bool direction) {
    if (!activated) {  //if not activated, disable
        digitalWrite(ActForward, LOW);
        digitalWrite(ActBack, LOW);
        digitalWrite(ActSwitched, LOW);
        return;
    }

    if (direction) {                      //if back
        digitalWrite(ActForward, LOW);    //stop forward
        digitalWrite(ActSwitched, HIGH);  //select back
        digitalWrite(ActBack, HIGH);      //go back
    } else {                              //if forward
        digitalWrite(ActBack, LOW);       //stop forward
        digitalWrite(ActSwitched, LOW);   //select back
        digitalWrite(ActForward, HIGH);   //go back
    }
}