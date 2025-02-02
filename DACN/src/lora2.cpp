#include <Arduino.h>
#include "loramesher.h"
 
//Using LILYGO TTGO T-BEAM v1.1 
#define BOARD_LED   8
#define LED_ON      LOW
#define LED_OFF     HIGH
 
LoraMesher& radio = LoraMesher::getInstance();
 
uint32_t dataCounter = 0;
struct dataPacket {
    uint32_t counter = 0;
};
 
dataPacket* helloPacket = new dataPacket;
 
//Led flash
void led_Flash(uint16_t flashes, uint16_t delaymS) {
    uint16_t index;
    for (index = 1; index <= flashes; index++) {
        digitalWrite(BOARD_LED, LED_ON);
        delay(delaymS);
        digitalWrite(BOARD_LED, LED_OFF);
        delay(delaymS);
    }
}
 
void printPacket(dataPacket data) {
    Serial.printf("Hello Counter received nº %d\n", data.counter);
}
 
void printDataPacket(AppPacket<dataPacket>* packet) {
     Serial.printf("Packet arrived from %X with size %d\n", packet->src, packet->payloadSize);
 
    //Get the payload to iterate through it
    dataPacket* dPacket = packet->payload;
    size_t payloadLength = packet->getPayloadLength();
 
    for (size_t i = 0; i < payloadLength; i++) {
        //Print the packet
        printPacket(dPacket[i]);
    }
}
 
void processReceivedPackets(void*) {
    for (;;) {
        /* Wait for the notification of processReceivedPackets and enter blocking */
        ulTaskNotifyTake(pdPASS, portMAX_DELAY);
        led_Flash(1, 100); //one quick LED flashes to indicate a packet has arrived
 
        //Iterate through all the packets inside the Received User Packets FiFo
        while (radio.getReceivedQueueSize() > 0) {
            Serial.println("ReceivedUserData_TaskHandle notify received");
            Serial.printf("Queue receiveUserData size: %d\n", radio.getReceivedQueueSize());
 
            //Get the first element inside the Received User Packets FiFo
            AppPacket<dataPacket>* packet = radio.getNextAppPacket<dataPacket>();
 
            //Print the data packet
            printDataPacket(packet);
 
            //Delete the packet when used. It is very important to call this function to release the memory of the packet.
            radio.deletePacket(packet);
        }
    }
}
 
TaskHandle_t receiveLoRaMessage_Handle = NULL;
 
void createReceiveMessages() {
    int res = xTaskCreate(
        processReceivedPackets,
        "Receive App Task",
        4096,
        (void*) 1,
        2,
        &receiveLoRaMessage_Handle);
    if (res != pdPASS) {
        Serial.printf("Error: Receive App Task creation gave error: %d\n", res);
    }
 
    radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}
 
 
void setupLoraMesher() {
    //Get the configuration of the LoRaMesher
    LoraMesher::LoraMesherConfig config = LoraMesher::LoraMesherConfig();

    config.loraCs = 7;
    config.loraRst = 2;
    config.loraIrq = 3;
    config.loraIo1 = 1;

    config.module = LoraMesher::LoraModules::SX1280_MOD;
    //Init the loramesher with a processReceivedPackets function
    radio.begin();
 
    //Create the receive task and add it to the LoRaMesher
    createReceiveMessages();
 
    //Start LoRaMesher
    radio.start();
 
    Serial.println("Lora initialized");
}
 
 
void setup() {
    Serial.begin(115200);
    SPI.begin(4,5,6,7);
 
    Serial.println("initBoard");
    pinMode(BOARD_LED, OUTPUT); //setup pin as output for indicator LED
    led_Flash(2, 125);          //two quick LED flashes to indicate program start
    setupLoraMesher();
}
 
 
void loop() {
    for (;;) {
        Serial.printf("Send packet %d\n", dataCounter);
 
        helloPacket->counter = dataCounter++;
 
        //Create packet and send it.
        radio.createPacketAndSend(BROADCAST_ADDR, helloPacket, 1);
 
        //Wait 20 seconds to send the next packet
        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}