
#include <Wire.h>
#include <WirePacker.h>
#include <WireSlaveRequest.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define I2C_SLAVE_ADDR_L 0x01
#define I2C_SLAVE_ADDR_R 0x02
#define MAX_SLAVE_RESPONSE_LENGTH 16

int32_t Ang_ctn_l = 0;
int32_t Ang_ctn_r = 0;

#define AUTO_STOP_INTERVAL 2000
long lastMotorCommand = 0;


void setup() {
  Serial2.begin(115200);
  Serial.begin(57600);
  Wire.begin(SDA_PIN, SCL_PIN);   
  while (!Serial){
    delay(10);
  }

  if(xTaskCreatePinnedToCore( task_i2c_l, "task_i2c_l", 2048, NULL, 2, NULL,0) != pdPASS){
            Serial.println("Error en creacion tarea task_l");
            exit(-1);
  }
  if(xTaskCreatePinnedToCore( task_i2c_r, "task_i2c_r", 2048, NULL, 2, NULL,0) != pdPASS){
            Serial.println("Error en creacion tarea task_l");
            exit(-1);
  }

}

// request encoder right wheel
void task_i2c_r(void *pvParameter) {
  while(1){
    String enc_r = "";
    Wire.requestFrom(I2C_SLAVE_ADDR_R, MAX_SLAVE_RESPONSE_LENGTH);
    vTaskDelay(10/ portTICK_PERIOD_MS);
    int i = 0;
    while (Wire.available()) { // peripheral may send less than requested
      char c = (char)Wire.read(); // receive a byte as character
      if (i==0) { 
        enc_r += c;
      } else {
        if (isDigit(c)) enc_r += c;
      }
      i++;
    }
  
    Ang_ctn_r = enc_r.toInt();
  }
}
// request encoder left wheel
void task_i2c_l(void *pvParameter) {
  while(1){
    String enc_l = "";
    Wire.requestFrom(I2C_SLAVE_ADDR_L, MAX_SLAVE_RESPONSE_LENGTH);
    vTaskDelay(10/ portTICK_PERIOD_MS);
    int i = 0;
    while (Wire.available()) { // peripheral may send less than requested
      char c = (char)Wire.read(); // receive a byte as character
      if (i==0) { 
        enc_l += c;
      } else {
        if (isDigit(c)) enc_l += c;
      }
      i++;
    }
    
    Ang_ctn_l = enc_l.toInt();
  }
}

void send_vel_l(String l){
  int str_len = l.length() + 1; 
  char char_array[str_len];
  l.toCharArray(char_array, str_len);
  Wire.beginTransmission(I2C_SLAVE_ADDR_L);
  Wire.write(char_array);
  Wire.endTransmission();    
}

void send_vel_r(String r){
  int str_len = r.length() + 1; 
  char char_array[str_len];
  r.toCharArray(char_array, str_len);
  Wire.beginTransmission(I2C_SLAVE_ADDR_R);
  Wire.write(char_array);
  Wire.endTransmission();    
}

void loop() {

  if(Serial.available() > 0){

      lastMotorCommand = millis();
      String str = Serial.readStringUntil('\r');
      if (str.indexOf("e") == 0 ) {
        Serial.print(Ang_ctn_l); 
        Serial.print(" "); 
        Serial.println(Ang_ctn_r);
        Serial.flush();
      }

      if (str.indexOf("u") == 0 ) {
        Serial.println("OK"); 
        Serial.flush();
      }
      
      if (str.indexOf("r") == 0 ) {
        send_vel_l("r");
        send_vel_r("r");
        //reset contador encoder
        Serial.println("OK"); 
        Serial.flush();
      }
            
      if (str.indexOf("m") == 0 ) {
        Serial2.println(str);
        str.replace("m", "");
        int i1 = str.indexOf(",");

        String firstValue = str.substring(0, i1);
        String second = str.substring(i1 + 1);
        send_vel_l(firstValue);
        send_vel_r(second);
        Serial.println("OK"); 
        Serial.flush();
      }
      
      if (millis()>(AUTO_STOP_INTERVAL + lastMotorCommand) and lastMotorCommand > 0){
        // send_vel_l("r");
      }

 }

}

