#include <WiFi.h>
#include <HTTPClient.h> 
#include <SPI.h>
#include "driver/spi_slave.h"

// --- Wi-Fi Credentials ---
const char* ssid = "yourssid";
const char* password = "your password";

// --- WEBHOOK URL ---
const char* webhook_url = "your web hook ID";

// --- SPI Pin Definitions (VSPI) ---
#define VSPI_MISO   19
#define VSPI_MOSI   23
#define VSPI_SCLK   18
#define VSPI_SS     5

// SPI data buffer
char spi_rx_buf[128];

// Create an HTTP client object
HTTPClient http;

// --- NEW: Variables for the 5-second timer ---
unsigned long lastTestMessageTime = 0;
const long testMessageInterval = 5000; // 5 seconds in milliseconds

void setup() {
    Serial.begin(115200); // For PC debugging

    // --- Configure SPI Slave (Same as before) ---
    spi_bus_config_t buscfg={
        .mosi_io_num = VSPI_MOSI,
        .miso_io_num = VSPI_MISO,
        .sclk_io_num = VSPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_slave_interface_config_t slvcfg={
        .spics_io_num = VSPI_SS,
        .flags = 0,
        .queue_size = 1,
        .mode = 0
    };
    esp_err_t ret = spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    // --- Connect to Wi-Fi ---
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

// --- NEW: Helper function to send data to the webhook ---
// This avoids duplicating the HTTP code
void sendHttpData(String payload) {
    if (WiFi.status() == WL_CONNECTED) {
        http.begin(webhook_url);
        http.addHeader("Content-Type", "application/json");

        // Send the POST request with the payload
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        } else {
            Serial.printf("Error on sending POST: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
    }
}


void loop() {
    // --- NEW: Check if it's time to send the 5-second test message ---
    unsigned long currentTime = millis();
    if (currentTime - lastTestMessageTime >= testMessageInterval) {
        lastTestMessageTime = currentTime; // Reset the timer

        String testPayload = "{test message: hello from esp32";
        Serial.println("Sending 5-second test message...");
        sendHttpData(testPayload);
    }

    // --- MODIFIED: Check for SPI data (non-blocking) ---
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = sizeof(spi_rx_buf) * 8;
    t.rx_buffer = spi_rx_buf;

    // --- CHANGE: Changed portMAX_DELAY to 0 to make this non-blocking ---
    // This now "polls" for data instead of "waiting"
    esp_err_t ret = spi_slave_transmit(VSPI_HOST, &t, 0); 
    
    // If data was received successfully
    if (ret == ESP_OK) {
        spi_rx_buf[t.trans_len / 8] = '\0';
        String sensor_data(spi_rx_buf);
        sensor_data.trim();

        if (sensor_data.length() > 0) {
            Serial.print("Received from STM32 via SPI: ");
            Serial.println(sensor_data);
            
            // Send the STM32 data to the webhook
            sendHttpData(sensor_data);
        }
    }
}