HardwareSerial& Sergio = Serial1;  //rename Serial1 port to Sergio

//declare pins
const int MAGPIN = 7;

const int VERTUP = 3;
const int VERTDOWN = 4;

const int HORLEFT = 5;
const int HORRIGHT = 6;

const int ACTFORWARD = 10;
const int ACTBACK = 2;

const int RESETPIN = 9;

const int UNUSED10 = 8;

const int UPDATEINTERVAL = 100;  //send commands to Sergio every 100 millis
long prevSendTime = 0;           //record the previous millis() of command sent to Sergio

const int MAGINTERVAL = 2 * 60 * 100;  //timeout the magnet after 2 minutes
long prevMagTime = 0;                  //record the previous millis() the magnet was turned on at

bool autoMode = true;  //manual or automatic control?

unsigned char lastManualState = 0;  //record the state entered manual through Serial

const unsigned char MAGCHAR = 0b10000000;
const unsigned char UPCHAR = 0b01000000;
const unsigned char DOWNCHAR = 0b00100000;
const unsigned char LEFTCHAR = 0b00010000;
const unsigned char RIGHTCHAR = 0b00001000;
const unsigned char FORWARDCHAR = 0b00000100;
const unsigned char BACKCHAR = 0b00000010;
const unsigned char RESETCHAR = 0b00000001;

const unsigned char ALLRELAYSOFF = 0b00000000;

const unsigned char HOMEENCODEDSTATE = 0b01010011;  //set home position to top left, retracted, reset

//---------------------------

void setup() {
    initPins();

    Serial.begin(9600);
    Sergio.begin(9600);
}

//---------------------------

void initPins() {
    pinMode(MAGPIN, INPUT);

    pinMode(VERTUP, INPUT);
    pinMode(VERTDOWN, INPUT);

    pinMode(HORLEFT, INPUT);
    pinMode(HORRIGHT, INPUT);

    pinMode(ACTFORWARD, INPUT);
    pinMode(ACTBACK, INPUT);

    pinMode(RESETPIN, INPUT);

    pinMode(UNUSED10, INPUT);
}

//---------------------------

void loop() {
    if (Serial.available())
        checkManualInput();

    /*
    encode data as unsigned char from arcade as follows:
      format: item (off/on 0/1)

      mag|up|down|left|right|forward|back|reset
      7  |6 |5   |4   |3    |2      |1   |0
  */

    //if it's been longer than the interval, update Sergio
    if ((millis() - prevSendTime >= UPDATEINTERVAL))
        sendCommands(checkControls());
}

//---------------------------

void checkManualInput() {
    String input = Serial.readString();

    if (input == "manual") {
        autoMode = false;
        Serial.println("manual");
    }

    if (input == "auto") {
        Serial.println("auto");
        autoMode = true;
        lastManualState = 0;
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

        lastManualState = encodedState;
        sendCommands(encodedState);
    }
}

//---------------------------

//args unsigned char encodedState - see comment in loop()
void sendCommands(unsigned char encodedState) {
    prevSendTime = millis();
    Sergio.write(encodedState);  //send the encoded state of the controls to Sergio

    //print the encoded state byte
    for (int i = 0; i < 8; ++i) {
        Serial.print((encodedState & (1 << (7 - i))) ? "1" : "0");
    }
    Serial.println();
}

//---------------------------

//check the position of the controls and their legality
unsigned char checkControls() {
    unsigned char encodedState = autoMode ? 0 : lastManualState;

    if (autoMode)
        encodedState = findEncodedState();

    if (checkMagTimeout()) {        //if the mag has not timed out, turn off
        encodedState &= ~(1 << 7);  //set the mag bit to 0
        prevMagTime = millis();
    }

    return encodedState;
}

//---------------------------

bool checkMagTimeout() {
    //if the magnet has been on for longer than the allowed interval, timeout
    if (millis() - prevMagTime >= MAGINTERVAL)
        return true;
    return false;
}

//---------------------------

unsigned char findEncodedState() {
    unsigned long encodedState = 0;

    if (digitalRead(RESETPIN) == HIGH)
        return HOMEENCODEDSTATE;  //if reset is pressed, return to home

    if (digitalRead(MAGPIN) == HIGH)
        encodedState |= MAGCHAR;
    else
        prevMagTime = millis();

    if (digitalRead(VERTUP) == HIGH)
        encodedState |= UPCHAR;
    else if (digitalRead(VERTDOWN) == HIGH)
        encodedState |= DOWNCHAR;

    if (digitalRead(HORLEFT) == HIGH)
        encodedState |= LEFTCHAR;
    else if (digitalRead(HORRIGHT) == HIGH)
        encodedState |= RIGHTCHAR;

    if (digitalRead(ACTFORWARD) == HIGH)
        encodedState |= FORWARDCHAR;
    else if (digitalRead(ACTBACK) == HIGH)
        encodedState |= BACKCHAR;

    return encodedState;
}
