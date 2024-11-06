#include <RadioLib.h>

// Khởi tạo đối tượng SX1280
SX1280 lora = new Module(10, 9, 18, 19);

void setup() {
  // Khởi tạo cổng serial
  Serial.begin(115200);
  
  // Khởi tạo SX1280
  Serial.print(F("Initializing SX1280 ... "));
  int state = lora.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("Success!"));
  } else {
    Serial.print(F("Failed, code: "));
    Serial.println(state);
    while (true);
  }

  // Cấu hình tần số, băng thông, và công suất truyền
  lora.setFrequency(2400.0); // 2.4GHz
  lora.setBandwidth(500.0);  // 500 kHz
  lora.setOutputPower(10);   // 10 dBm

  // Chuyển sang chế độ nhận
  lora.startReceive();
}

void loop() {
  // Kiểm tra xem có dữ liệu đến không
  if (lora.available()) {
    String receivedMessage;
    
    // Nhận dữ liệu
    int state = lora.receive(receivedMessage);
    
    if (state == RADIOLIB_ERR_NONE) {
      Serial.print(F("Received message: "));
      Serial.println(receivedMessage);
    } else {
      Serial.print(F("Failed to receive message, code: "));
      Serial.println(state);
    }
  }
}
