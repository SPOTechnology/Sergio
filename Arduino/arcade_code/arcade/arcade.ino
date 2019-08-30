HardwareSerial& Sergio = Serial1;  //rename Serial1 port to Sergio

//declare pins
const int MagPin = 2;

const int VertUp = 3;
const int VertDown = 4;

const int HorLeft = 5;
const int HorRight = 6;

const int ActForward = 7;
const int ActBack = 8;

const int ResetPin = 9;

const int Unused10 = 10;

const int UpdateInterval = 100;  //send commands to Sergio every 100 millis
long prevSendTime = 0;           //record the previous millis() of command sent to Sergio

const int MagInterval = 2 * 60 * 100;  //timeout the magnet after 2 minutes
long prevMagTime = 0;                  //record the previous millis() the magnet was turned on at

bool autoMode = true;  //manual or automatic control?

unsigned char lastManualState = 0;  //record the state entered manual through Serial

const unsigned char HomeEncodedState = 0b01010011;  //set home position to top left, retracted, reset

//---------------------------

void setup() {
    initPins();

    Serial.begin(9600);
    Sergio.begin(9600);
}

//---------------------------

void initPins() {
    pinMode(MagPin, INPUT);

    pinMode(VertUp, INPUT);
    pinMode(VertDown, INPUT);

    pinMode(HorLeft, INPUT);
    pinMode(HorRight, INPUT);

    pinMode(ActForward, INPUT);
    pinMode(ActBack, INPUT);

    pinMode(ResetPin, INPUT);

    pinMode(Unused10, INPUT);
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
    if ((millis() - prevSendTime >= UpdateInterval))
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
    if (millis() - prevMagTime >= MagInterval)
        return true;
    return false;
}

//---------------------------

unsigned char findEncodedState() {
    unsigned long encodedState = 0;

    if (digitalRead(ResetPin) == HIGH)
        return HomeEncodedState;  //if reset is pressed, return to home

    if (digitalRead(MagPin) == HIGH)
        encodedState |= (1 << 7);
    else
        prevMagTime = millis();

    if (digitalRead(VertUp) == HIGH)
        encodedState |= (1 << 6);
    else if (digitalRead(VertDown) == HIGH)
        encodedState |= (1 << 5);

    if (digitalRead(HorLeft) == HIGH)
        encodedState |= (1 << 4);
    else if (digitalRead(HorRight) == HIGH)
        encodedState |= (1 << 3);

    if (digitalRead(ActForward) == HIGH)
        encodedState |= (1 << 2);
    else if (digitalRead(ActBack) == HIGH)
        encodedState |= (1 << 1);

    return encodedState;
}
