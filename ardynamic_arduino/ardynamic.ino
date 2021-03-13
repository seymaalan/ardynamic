#define MAX_DATA_LENGTH 58
#define MAX_VALUE_LENGTH 50
#define MAX_TYPE_LENGTH 999
#define MAX_STRINGS 10
#define MAX_PIN 50
#define MAX_VARIABLE 50
#define PRINT_INTERVAL_MS 500
char data[80]; //Set zero at call

//VIRTUAL_REGISTERS
// if ram space is an issue, convert convert some of those variables to byte
//deneme
//deneme2
//deneme3
char STRING_DATA_REGISTER[MAX_STRINGS][MAX_VALUE_LENGTH];//records strings
int  STRING_LENGTH_REGISTER[MAX_STRINGS];                //
int  READ_REGISTER[MAX_PIN];                              //
int  VARIABLE_REGISTER[MAX_VARIABLE];                    //records variables
int  PRINT_READ_REGISTER[MAX_PIN];                       //
int  PRINT_VARIABLE_REGISTER[MAX_PIN];                   //
int  PRINT_MODE = 0; //0:DO NOT PRINT; 1:print once than change it to 0; 2:print continiously

int PIN_FUNCTION_REGISTER[MAX_PIN];   // 0:continue; 1:digital_read; 2:digital_write; 3:analog_read; 4:analog_write; 5:fake_analog_write
int PIN_MODE_REGISTER[MAX_PIN];       // 0:input ; 1:input_pullup ;2:output
int PIN_STATE_REGISTER[MAX_PIN];      // 0:low; 1:high
int PIN_PERIOD_REGISTER[MAX_PIN];     // [0,32767]
int PIN_DUTY_REGISTER[MAX_PIN];       // [0,100]
int PIN_ON_TIME_REGISTER[MAX_PIN];    // PIN_ON_TIME_REGISTER[pin_no] = (PIN_PERIOD_REGISTER[pin_no] * PIN_DUTY_REGISTER[pin_no]/100)

const char start_del = '#';
const char end_del = '$';
const char seperator_del = ':';
const char minus_del = '-';
const char plus_del = '+';

boolean accept_data = false; //Set false at call
int counter = 0; //Set zero at call

void ARDYNAMIC_RUN(){
  read_serial();
  UPDATE_PINS();
  PRINT();
}
void read_serial() {
  while (Serial.available()) {
    char c = Serial.read();
    if ( !isNumeric_or_Letter_or_Delimeter(c) )return;
    //START-----------------------------------------------------
    if (c == start_del) {
      accept_data = true;
      counter = 0;
      for ( int i = 0; i < sizeof(data);  i++ ) data[i] = '0';
    }
    //END------------------------------------------------------
    else if (c == end_del) {
      accept_data = false;
      try_to_use_serial_data();
    }
    //MID------------------------------------------------------
    else if (accept_data) {
      data[counter] = c;
      counter ++;
    }
  }
}
void try_to_use_serial_data() { // used in read_serial()
  int value_length = -1;
  int value_type   = -1;
  //VALUE TYPE----------------------------------------------------------------------
  if (isValueNumeric(0, 3)) {
    value_type = (int)(data[0] - '0') * 100 + (int)(data[1] - '0') * 10 + (int)(data[2] - '0')  ;
  } else return;
  //CHECK
  if (value_type > MAX_TYPE_LENGTH || value_type < 0 || data[3] != ':') return;

  //VALUE LENGTH---------------------------------------------------------------------
  if (isValueNumeric(4, 3)) {
    value_length = (int)(data[4] - '0') * 100 + (int)(data[5] - '0') * 10 + (int)(data[6] - '0')  ;
  }
  //CHECK
  if (value_length > MAX_VALUE_LENGTH || value_length < 0 || data[7] != ':') return;

  //------------------------------------------------------------------------------
  if (value_type == 0) {
    //CODE...
  } else if (value_type == 1) {//INPUT PIN
    if (isValueNumeric(8, 3)) {
      int pin_no = (int)(data[8] - '0') * 100 + (int)(data[9] - '0') * 10 + (int)(data[10] - '0')  ;
      pinMode(pin_no, INPUT);
      PIN_MODE_REGISTER[pin_no] = 0;
      PIN_FUNCTION_REGISTER[pin_no] = 0;
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.println(":INPUT_PIN$");

    }
  } else if (value_type == 2) {//INPUT_PULLUP PIN
    if (isValueNumeric(8, 3)) {
      int pin_no = (int)(data[8] - '0') * 100 + (int)(data[9] - '0') * 10 + (int)(data[10] - '0')  ;
      pinMode(pin_no, INPUT_PULLUP);
      PIN_MODE_REGISTER[pin_no] = 1;
      PIN_FUNCTION_REGISTER[pin_no] = 0;
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.println(":INPUT_PULLUP_PIN$");
    }
  } else if (value_type == 3) {//OUTPUT PIN
    if (isValueNumeric(8, 3)) {
      int pin_no = (int)(data[8] - '0') * 100 + (int)(data[9] - '0') * 10 + (int)(data[10] - '0')  ;
      pinMode(pin_no, OUTPUT);
      PIN_MODE_REGISTER[pin_no] = 2;
      PIN_FUNCTION_REGISTER[pin_no] = 0;
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.println(":OUTPUT_PIN$");
    }
  } else if (value_type == 4) {//DIGITAL_READ
    if (isValueNumeric(8, value_length)) {
      int pin_no = convert_to_int(8, 3);
      int period_ms = convert_to_int (11, 5);
      set_pin_registers(pin_no, 1, 0, 0, period_ms, 0);
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.print(":PERIOD:");
      Serial.print(period_ms);
      Serial.println(":DIGITAL_READ_PIN$");
    }
  } else if (value_type == 5) {//DIGITAL_WRITE
    if (isValueNumeric(8, value_length)) {
      int pin_no = convert_to_int(8, 3);
      int logic_value = convert_to_int (11, 1);
      set_pin_registers(pin_no, 2, 2, logic_value, -1, 0);
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.print(":STATE:");
      Serial.print(logic_value);
      Serial.println(":DIGITAL_WRITE_PIN$");
    }
  } else if (value_type == 6) {//ANALOG_READ
    if (isValueNumeric(8, value_length)) {
      int pin_no = convert_to_int(8, 3);
      int period_ms = convert_to_int (11, 5);
      set_pin_registers(pin_no, 3, 0, -1, period_ms, -1);
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.print(":PERIOD:");
      Serial.print(period_ms);
      Serial.println(":ANALOG_READ_PIN$");
    }
  } else if (value_type == 7) {//ANALOG_WRITE
    if (isValueNumeric(8, value_length)) {
      int pin_no = convert_to_int(8, 3);
      int byte_duty = convert_to_int (11, 3);
      set_pin_registers(pin_no, 4, 2, -1, -1, byte_duty);
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.print(":BYTE_DUTY:");
      Serial.print(byte_duty);
      Serial.println(":ANALOG_WRITE_PIN$");
    }
  } else if (value_type == 8) {//FAKE_ANALOG_WRITE
    if (isValueNumeric(8, value_length)) {
      int pin_no = convert_to_int(8, 3);
      int period = convert_to_int (11, 5);
      int duty = convert_to_int (16, 3);
      set_pin_registers(pin_no, 5, 2, -1, period, duty);
      Serial.print('#');
      Serial.print("SETUP:");
      Serial.print(pin_no);
      Serial.print(":PERIOD_MS:");
      Serial.print(period);
      Serial.print(":DUTY_100:");
      Serial.print(duty);
      Serial.println(":FAKE_ANALOG_WRITE_PIN$");
    }
  } else if (value_type == 9) {//UPDATE_BAUD_RATE
    //CODE...
  } else if (value_type == 10) {//SAVE_SARIAL_DATA_TO_VARIABLE_REGISTER
    if (isValueNumeric(8, value_length)) {
      int value = convert_to_int(9, 5);
      int variable_register_index = convert_to_int(14, 3);
      if (data[8] == '-') value = -value;
      VARIABLE_REGISTER[variable_register_index] = value;
      Serial.print('#');
      Serial.print("SAVE_VARIABLE:");
      Serial.print(value);
      Serial.print(":TO_REGISTER:");
      Serial.print(variable_register_index);
      Serial.println('$');
    }
  } else if (value_type == 11) {//SAVE_STRING_DATA_TO_STRING_REGISTER
    if (isValueNumeric(8, 2)) {
      int string_length = value_length - 2;
      int which_string = convert_to_int(8, 2);
      STRING_LENGTH_REGISTER[which_string] = string_length;
      Serial.print('#');
      Serial.print("TO_REGISTER:");
      Serial.print(which_string);
      Serial.print(":LENGTH:");
      Serial.print(string_length);
      Serial.print(":SAVE_STRING:");
      for (int i = 0 ; i < string_length; i++)
      {
        STRING_DATA_REGISTER[which_string][i] = data[10 + i];
        Serial.print((char)data[10 + i]);
      }
      Serial.println('$');
    }

  } else if (value_type == 12) {//PRINT_STATE_READ_REGISTER
    if (isValueNumeric(8, value_length)) {
      int logic_val = convert_to_int(8, 1);
      int print_read_register_index = convert_to_int(9, 3);
      PRINT_READ_REGISTER[print_read_register_index] = logic_val;
      Serial.print('#');
      Serial.print("SETTING:");
      Serial.print(":PRINT_READ:");
      Serial.print(print_read_register_index);
      Serial.print(":IF:");
      Serial.print(logic_val);
      Serial.println('$');
    }
  } else if (value_type == 13) {//PRINT_STATE_VARIABLE_REGISTER
    if (isValueNumeric(8, value_length)) {
      int logic_val = convert_to_int(8, 1);
      int print_variable_register_index = convert_to_int(9, 3);
      PRINT_VARIABLE_REGISTER[print_variable_register_index] = logic_val;
      Serial.print('#');
      Serial.print("SETTING:");
      Serial.print(":PRINT_VARIABLE:");
      Serial.print(print_variable_register_index);
      Serial.print(":IF:");
      Serial.print(logic_val);
      Serial.println('$');
    }
  } else if (value_type == 14) {//PRINT_MODE
    if (isValueNumeric(8, value_length)) {
      int print_mode = convert_to_int(8, 2);
      PRINT_MODE = print_mode;
      Serial.print('#');
      Serial.print("SETTING");
      Serial.print(":PRINT_MODE:");
      Serial.print(PRINT_MODE);
      Serial.println('$');
    }
  }
}

boolean isNumeric_or_Letter_or_Delimeter(char c) {
  boolean c1 = !(c >= '0' && c <= '9');
  boolean c2 = !(c >= 'A' && c <= 'Z');
  boolean c3 = !(c >= 'a' && c <= 'z');
  boolean c4 = !(c == start_del || c == end_del || c == seperator_del || c == minus_del || c == plus_del);

  if (c1 && c2 && c3 && c4) return false;

  return true;
}
boolean isNumeric(char c) {
  if ((c >= '0' && c <= '9') || c == '-' || c == '+') return true;
  return false;
}
boolean isValueNumeric(int start, int value_length) {
  if (value_length < 1)return false;

  for (int i = start; i < (start + value_length); i++) {
    if (!isNumeric(data[i]))return false;
  }
  return true;
}
int convert_to_int(int start, int integer_length) {
  int sum = 0;
  for (int i = start; i < start + integer_length; i++) {
    sum += (data[i] - '0') * pow(10, (integer_length - 1) - (i - start));
  }
  return sum;
}
void set_pin_registers(int pin_no, int function, int mode, int state, int period, int duty) {
  if (!(pin_no >= 0 && pin_no <= MAX_PIN)) return;
  if ( function == 1) {
    PIN_FUNCTION_REGISTER[pin_no] = 1; //DIGITAL_READ
    PIN_MODE_REGISTER[pin_no] = 0; //INPUT
    PIN_PERIOD_REGISTER[pin_no] = period; //UP TO 32767 ms

    PIN_STATE_REGISTER[pin_no] = state; //HIGH OR LOW
    PIN_DUTY_REGISTER[pin_no] = duty;
    PIN_ON_TIME_REGISTER[pin_no] = (float)period * (duty / 100.0);
  } else if (function == 2) {
    PIN_FUNCTION_REGISTER[pin_no] = 2; //DIGITAL_WRITE
    PIN_MODE_REGISTER[pin_no] = 2; //OUTPUT
    PIN_STATE_REGISTER[pin_no] = state; //HIGH OR LOW

    PIN_PERIOD_REGISTER[pin_no] = period; //UP TO 32767 ms
    PIN_DUTY_REGISTER[pin_no] = duty;
    PIN_ON_TIME_REGISTER[pin_no] = (float)period * (duty / 100.0);
  } else if (function == 3) {
    PIN_FUNCTION_REGISTER[pin_no] = 3; //ANALOG_READ
    PIN_MODE_REGISTER[pin_no] = 0; //INPUT
    PIN_PERIOD_REGISTER[pin_no] = period; //UP TO 32767 ms

    PIN_STATE_REGISTER[pin_no] = state; //HIGH OR LOW
    PIN_DUTY_REGISTER[pin_no] = duty;
    PIN_ON_TIME_REGISTER[pin_no] = (float)period * (duty / 100.0);
  } else if (function == 4) {
    PIN_FUNCTION_REGISTER[pin_no] = 4; //ANALOG_WRITE
    PIN_MODE_REGISTER[pin_no] = 2; //OUTPUT
    PIN_DUTY_REGISTER[pin_no] = duty;

    PIN_PERIOD_REGISTER[pin_no] = period; //UP TO 32767 ms
    PIN_STATE_REGISTER[pin_no] = state; //HIGH OR LOW
    PIN_ON_TIME_REGISTER[pin_no] = (float)period * (duty / 100.0);
  } else if (function == 5) {
    PIN_FUNCTION_REGISTER[pin_no] = 5; //FAKE_ANALOG_WRITE
    PIN_MODE_REGISTER[pin_no] = 2; //OUTPUT  ,
    PIN_PERIOD_REGISTER[pin_no] = period; //UP TO 32767 ms
    PIN_DUTY_REGISTER[pin_no] = duty;
    PIN_ON_TIME_REGISTER[pin_no] = (float)period * (duty / 100.0);

    PIN_STATE_REGISTER[pin_no] = state; //HIGH OR LOW
  }

}

void UPDATE_PINS() {
  for ( int i = 0 ; i < MAX_PIN; i++) {
    if (PIN_FUNCTION_REGISTER[i] == 0)continue;
    if (PIN_FUNCTION_REGISTER[i] == 1) { //digital read
      pinMode(i,INPUT);
      READ_REGISTER[i] = digitalRead(i);
    } else if (PIN_FUNCTION_REGISTER[i] == 2) { //digital write
      pinMode(i,OUTPUT);
      if (PIN_STATE_REGISTER[i])digitalWrite(i, HIGH);
      else digitalWrite(i, LOW);
    } else if (PIN_FUNCTION_REGISTER[i] == 3) { //analog read
      pinMode(i,INPUT);
      READ_REGISTER[i] = analogRead(i);
    } else if (PIN_FUNCTION_REGISTER[i] == 4) { //analog_write
      pinMode(i,OUTPUT);
      if (PIN_DUTY_REGISTER[i] == 255)digitalWrite(i, HIGH);
      else if (  PIN_DUTY_REGISTER[i] > 0 && PIN_DUTY_REGISTER[i] < 255 ) analogWrite(i, PIN_DUTY_REGISTER[i]);
      else if (PIN_DUTY_REGISTER[i] == 0)digitalWrite(i, LOW);
    } else if (PIN_FUNCTION_REGISTER[i] == 5) { //fake_analog_write
      pinMode(i,OUTPUT);
      int period = PIN_PERIOD_REGISTER[i];
      int duty = PIN_DUTY_REGISTER[i];
      int on_time = (int)(period * (duty / 100.0) );
      if (millis() % period <= on_time)digitalWrite(i, HIGH);
      else digitalWrite(i, LOW);
    }

  }
}
void PRINT() {
  if (PRINT_MODE == 0) {
    //DO NOTHING
  } else if (PRINT_MODE == 1) {
    //PRINT FOR ONCE
    print_read_and_variable_registers();
    PRINT_MODE == 0;
  }
  if (PRINT_MODE == 2 ) {
    print_read_and_variable_registers();
  } else {
    PRINT_MODE = 0;
  }
}

void print_read_and_variable_registers() {
  for (int i = 0; i < MAX_PIN; i++) {
    if (PRINT_READ_REGISTER[i] == 1) {
      Serial.print('#');
      Serial.print('R');
      Serial.print(':');
      Serial.print(i);
      Serial.print(':');
      Serial.print(READ_REGISTER[i]);
      Serial.println('$');
    }
  }
  for (int i = 0; i < MAX_PIN; i++) {
    if (PRINT_VARIABLE_REGISTER[i] == 1) {
      Serial.print('#');
      Serial.print('V');
      Serial.print(':');
      Serial.print(i);
      Serial.print(':');
      Serial.print(VARIABLE_REGISTER[i]);
      Serial.println('$');
    }
  }
}
