/**
 * @file main.cpp
 * @brief License Plate Reader - ESP32 Main Entry Point
 * @author Andrea Venegas, Sergio Lara
 * @date 2026-03-18
 * @version 2.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "LEDController.h"
#include "WiFiManager.h"
#include "ImageBuffer.h"
#include "OCREngine.h"
#include "ResultTransmitter.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define LED_PIN          2
#define WIFI_SSID        "LicensePlateReader"
#define WIFI_PASSWORD    "lpr12345"
#define SERVER_PORT      5000
#define OCR_SERVER_PORT  5001

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

WebServer server(SERVER_PORT);
LEDController ledController(LED_PIN);
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, SERVER_PORT);
ImageBuffer imageBuffer;
OCREngine ocrEngine;
ResultTransmitter resultTransmitter;

// ============================================================================
// HTTP HANDLERS
// ============================================================================

// ============================================================================
// HTML HELPERS
// ============================================================================

String buildPage(const String& body) {
    String html = "<html><head><title>LPR</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>*{box-sizing:border-box;margin:0;padding:0}body{font-family:monospace;background:#fff;color:#000;max-width:400px;margin:60px auto;padding:20px}";
    html += "h1{font-size:1.1rem;font-weight:normal;border-bottom:1px solid #000;padding-bottom:8px;margin-bottom:24px}";
    html += "p{font-size:.8rem;color:#666;margin-bottom:12px}";
    html += "input[type=file]{display:block;width:100%;font-size:.85rem;margin-bottom:12px}";
    html += "button,a.btn{display:block;width:100%;padding:10px;background:#000;color:#fff;border:none;font-family:monospace;font-size:.9rem;cursor:pointer;text-align:center;text-decoration:none;margin-top:8px}";
    html += "button:hover,a.btn:hover{background:#333}";
    html += ".result{border:1px solid #000;padding:16px;margin:20px 0;text-align:center}";
    html += ".plate{font-size:2rem;font-weight:bold;letter-spacing:4px;margin:8px 0}";
    html += ".err{color:#900}</style></head><body>";
    html += "<h1>License Plate Reader</h1>";
    html += body;
    html += "</body></html>";
    return html;
}

// ============================================================================
// HTTP HANDLERS
// ============================================================================

void handleRoot() {
    String body = "<p>ESP32 v2.0</p>";
    body += "<form id='f' method='POST' action='/upload' enctype='multipart/form-data'>";
    body += "<input type='file' id='fi' name='image' accept='image/*' capture='environment'>";
    body += "<p id='st'></p>";
    body += "<button type='submit'>Analyze</button></form>";
    body += "<script>";
    // compress(file, maxBytes, callback) - reduces quality in a loop until it fits
    body += "function compress(file,maxB,cb){";
    body += "var r=new FileReader();r.onload=function(e){";
    body += "var img=new Image();img.onload=function(){";
    body += "var c=document.createElement('canvas');";
    body += "var s=Math.min(1,400/img.width);";
    body += "c.width=Math.round(img.width*s);c.height=Math.round(img.height*s);";
    body += "c.getContext('2d').drawImage(img,0,0,c.width,c.height);";
    body += "var q=0.6;var tries=0;";
    body += "function attempt(){";
    body += "var d=c.toDataURL('image/jpeg',q);";
    body += "var b=atob(d.split(',')[1]);";
    body += "var arr=new Uint8Array(b.length);";
    body += "for(var i=0;i<b.length;i++)arr[i]=b.charCodeAt(i);";
    body += "var blob=new Blob([arr],{type:'image/jpeg'});";
    body += "document.getElementById('st').textContent='Size: '+Math.round(blob.size/1024)+'KB (q='+q.toFixed(2)+')';";
    body += "if(blob.size<=maxB||tries>=8){cb(blob);}";
    body += "else{q=Math.round((q-0.08)*100)/100;tries++;attempt();}}";
    body += "attempt();};img.src=e.target.result;};r.readAsDataURL(file);}";
    // form submit handler
    body += "document.getElementById('f').addEventListener('submit',function(e){";
    body += "e.preventDefault();var fi=document.getElementById('fi').files[0];";
    body += "if(!fi)return;";
    body += "var st=document.getElementById('st');st.textContent='Compressing...';";
    body += "compress(fi,45000,function(blob){";
    body += "st.textContent='Uploading '+Math.round(blob.size/1024)+'KB...';";
    body += "var fd=new FormData();fd.append('image',blob,'plate.jpg');";
    body += "var x=new XMLHttpRequest();x.open('POST','/upload');";
    body += "x.onload=function(){document.open();document.write(x.responseText);document.close()};";
    body += "x.onerror=function(){st.textContent='Upload failed'};";
    body += "x.send(fd);});});";
    body += "</script>";
    server.send(200, "text/html", buildPage(body));
}

// Called for each chunk of the multipart upload
void handleUploadData() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.print("[Upload] File: ");
        Serial.println(upload.filename);
        imageBuffer.clear();
        ledController.setState(LED_PROCESSING);
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        // Append chunk to buffer
        uint8_t* buf = imageBuffer.getData();
        uint32_t currentSize = imageBuffer.getSize();
        uint32_t space = MAX_IMAGE_SIZE - currentSize;
        uint32_t toWrite = upload.currentSize < space ? upload.currentSize : space;
        if (toWrite > 0) {
            memcpy(buf + currentSize, upload.buf, toWrite);
            imageBuffer.setSize(currentSize + toWrite);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        Serial.print("[Upload] Done: ");
        Serial.print(imageBuffer.getSize());
        Serial.println(" bytes");
    }
}

// Called after multipart upload completes
void handleUploadComplete() {
    uint32_t imgSize = imageBuffer.getSize();
    Serial.print("[Server] Processing image, size=");
    Serial.println(imgSize);

    if (imgSize == 0) {
        String body = "<div class='result'><p class='err'>No image received</p></div>";
        body += "<a class='btn' href='/'>Back</a>";
        server.send(400, "text/html", buildPage(body));
        ledController.setState(LED_IDLE);
        return;
    }

    // Send to OCR engine (Python companion)
    OCRResult ocrResult = ocrEngine.processImage(imageBuffer.getData(), imgSize);
    imageBuffer.clear();

    String body;
    if (ocrResult.plate.length() > 0) {
        body = "<div class='result'><p>Plate detected:</p>";
        body += "<p class='plate'>" + ocrResult.plate + "</p>";
        body += "<p>Confidence: " + String(ocrResult.confidence * 100, 1) + "%</p></div>";
        ledController.setState(LED_DONE);
    } else {
        body = "<div class='result'><p class='err'>Could not read plate</p>";
        body += "<p>Make sure the OCR server is running and the image is clear.</p></div>";
        ledController.setState(LED_ERROR);
    }
    body += "<a class='btn' href='/'>Analyze another</a>";
    server.send(200, "text/html", buildPage(body));

    delay(2000);
    ledController.setState(LED_IDLE);
}

void handleStatus() {
    StaticJsonDocument<256> doc;
    doc["status"] = "running";
    doc["version"] = "2.0";
    doc["clients"] = wifiManager.getConnectedClients();
    doc["uptime_ms"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleRegister() {
    String ip = server.client().remoteIP().toString();
    ocrEngine.setServer(ip, OCR_SERVER_PORT);
    Serial.println("[OCR] Companion registered from: " + ip);
    server.send(200, "application/json", "{\"status\":\"registered\",\"ip\":\"" + ip + "\"}");
}

void handleNotFound() {
    server.send(404, "application/json", "{\"error\": \"Not found\"}");
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  License Plate Reader v2.0");
    Serial.println("  ESP32 Firmware");
    Serial.println("========================================");
    Serial.print("  Build: ");
    Serial.println(__TIMESTAMP__);
    Serial.print("  Free heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println();

    // Initialize LED
    ledController.init();
    ledController.setState(LED_IDLE);

    // Initialize WiFi AP
    wifiManager.initSoftAP();

    // Configure OCR engine (Python companion on connected laptop)
    // The companion runs on the machine that connects to this AP
    // Default gateway for AP clients is 192.168.4.1 (the ESP32)
    // The companion typically runs on the laptop at a known IP
    ocrEngine.setServer("192.168.4.2", OCR_SERVER_PORT);  // default, updated by /register

    // Register HTTP routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, handleUploadComplete, handleUploadData);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/register", HTTP_POST, handleRegister);
    server.onNotFound(handleNotFound);

    // Collect Content-Length header for upload handler
    const char* headerKeys[] = {"Content-Length"};
    server.collectHeaders(headerKeys, 1);

    // Start server
    server.begin();
    Serial.println("[Server] HTTP server started on port " + String(SERVER_PORT));
    Serial.println("[Server] Routes:");
    Serial.println("  GET  /        - Web interface");
    Serial.println("  POST /upload  - Upload image for OCR");
    Serial.println("  GET  /status  - System status");
    Serial.println();
    Serial.println("[Boot] Initialization complete. Waiting for client...");
    Serial.println("[Boot] Connect to WiFi: " + String(WIFI_SSID));
    Serial.println("[Boot] Then open: http://" + wifiManager.getLocalIP().toString() + ":" + String(SERVER_PORT));
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    server.handleClient();
    ledController.update();
    delay(10);
}
