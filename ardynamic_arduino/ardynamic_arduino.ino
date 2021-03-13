
void setup() {
  Serial.begin(9600);
  pinMode(4,OUTPUT);
}

void loop() {
  read_serial();
  UPDATE_PINS();
  PRINT();
  
}
