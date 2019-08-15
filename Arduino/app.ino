void setup()
{
    // put your setup code here, to run once:
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    Serial.begin(9600);
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (Serial.available())
    {
        int pin = Serial.readString().toInt();
        if (pin >= 2 && pin <= 9)
            digitalWrite(pin, !digitalRead(pin));
    }
}