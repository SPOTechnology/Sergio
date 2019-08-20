HardwareSerial& Arcade = Serial1;  //rename Serial1 port to Arcade

//declare pins
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
    initPins();

    Serial.begin(9600);
    Arcade.begin(9600);
}

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
    checkManualInput();

    /*
    encode data as unsigned char from arcade as follows:
        format: item(on/off 0/1)

        mag(off/on)|up(off/on)|down(off/on)|left(off/on)|right(off/on)|forward(off/on)|back(off/on)|spare
        7          |6         |5           |4           |3            |2              |1           |0
    */

    if (Arcade.available())
        updateRelays(inputFromArcade());
}

void checkManualInput() {
    if (Serial.available()) {
        String input = Serial.readString();

        int pin = input.toInt();  //if telling pin number to flip
        if ((pin >= 2 && pin <= 9) || (pin >= 30 && pin <= 33)) {
            digitalWrite(pin, !digitalRead(pin));
            return;
        }

        if (input.length() == 8) {  //if inputing binary encoded state, handle
            //ask dodi for code
        }
    }
}

unsigned char inputFromArcade() {
    return Arcade.read();
}

void updateRelays(unsigned char encodedState) {
    //args bool activated (true == on)
    updateMag(encodedState & 1 << 7);

    bool acting;
    if (encodedState & 1 << 1 || encodedState & 1 << 2)
        acting = true;

    //args bool up, bool down, bool acting
    updateVert(encodedState & 1 << 6, encodedState & 1 << 5, acting);

    //args bool left, bool right, bool acting
    updateHor(encodedState & 1 << 4, encodedState & 1 << 3, acting);

    //args bool forward, bool back
    updateAct(encodedState & 1 << 2, encodedState & 1 << 1);
}

//args bool activated (true == on)
void updateMag(bool activated) {
    if (activated)
        digitalWrite(MagPin, HIGH);
    else
        digitalWrite(MagPin, LOW);
}

void updateVert(bool up, bool down, bool acting) {
    //disable when not active actuator is acting
    digitalWrite(VertSwitched, LOW);
    digitalWrite(VertUp, LOW);
    digitalWrite(VertDown, LOW);
}

void updateHor(bool left, bool right, bool acting) {
    //disable when not active or actuator is acting
    digitalWrite(HorSwitched, LOW);
    digitalWrite(HorLeft, LOW);
    digitalWrite(HorRight, LOW);
}

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