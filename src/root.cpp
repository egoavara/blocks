#ifdef ROOT

#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "myi2ce.hpp"


// [ESP] :  루트모듈 자체 서버 이름 = 공유기 이름
const char* AP_NAME = "CUBES";
// [ESP] :  루트모듈 자체 서버 비밀번호 = 공유기 패스워드
const char* AP_PASSWORD = "cubes42$@";
// [ESP] : 공유기 기본 주소
IPAddress local_ip(192,168,1,1);
// [ESP] : 공유기 게이트웨이(dns 할당 등)
IPAddress gateway(192,168,1,1);
// [ESP] : 공유기 서브넷
IPAddress subnet(255,255,255,0);
// [ESP] : http 서버 포트
ESP8266WebServer server(80);


// [I2C-My] : i2c 자동 주소 할당 서버
Myi2cServer i2cserver;


// [ESP:HTTP] : /
void handle_root(void){
    String data;
    DynamicJsonDocument dom(100);
    JsonArray get_arr = dom.createNestedArray("get");
    get_arr.add("devices/count");
    get_arr.add("devices/{nth}");
    serializeJson(dom, data);
    server.send(200, "application/json", data);
}

// [ESP:HTTP] : /devices/
void handle_devices(void){
    String data;
    DynamicJsonDocument dom(512);
    auto arr = dom.as<JsonArray>();
    for (size_t i = AUTOALLOCATEE_BEGIN_ADDRESS; i < AUTOALLOCATEE_END_ADDRESS; i++)
    {
        if(!i2cserver.exist(i)){
            continue;
        }
        auto obj = arr.createNestedObject();
        obj["id"] = i;
        auto meta = i2cserver.get_meta(i);
        if(meta.name != NAME::UNDEFINED){
            obj["name"] = meta.name;
        }else{
            obj["error"] = "Undefined Name Device";
        }
        if(meta.x != AXIS::UNUSED){
            auto axis_x = obj.createNestedObject("x");
            axis_x["name"] = meta.x;
            axis_x["min"] = meta.data_min.x;
            axis_x["max"] = meta.data_max.x;
        }
        if(meta.y != AXIS::UNUSED){
            auto axis_y = obj.createNestedObject("y");
            axis_y["name"] = meta.y;
            axis_y["min"] = meta.data_min.y;
            axis_y["max"] = meta.data_max.y;
        }
        if(meta.z != AXIS::UNUSED){
            auto axis_z = obj.createNestedObject("z");
            axis_z["name"] = meta.z;
            axis_z["min"] = meta.data_min.z;
            axis_z["max"] = meta.data_max.z;
        }
        if(meta.w != AXIS::UNUSED){
            auto axis_w = obj.createNestedObject("w");
            axis_w["name"] = meta.w;
            axis_w["min"] = meta.data_min.w;
            axis_w["max"] = meta.data_max.w;
        }

    }
    
    serializeJson(dom, data);
    server.send(200, "application/json", data);
}

// [ESP:HTTP] : PageNotFound Error Handler
void handle_404(){
  server.send(404, "text/plain", "Not found");
}

void setup(){

    #ifdef PCCOM
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("[Logger] : Hello"));
    #endif
    // [I2C] : 서버 초기화
    i2cserver.init();
    #ifdef PCCOM
    Serial.println(F("[I2C-My] : Init"));
    #endif

    // [ESP] : AP 초기화
    WiFi.enableAP(true);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(AP_NAME, AP_PASSWORD);

    #ifdef PCCOM
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("[ESP] : ip=");
    Serial.println(myIP);
    #endif

    // [ESP:HTTP] : http 서버 핸들러 초기화
    server.on("/", handle_root);
    server.on("/devices/", handle_devices);
    // TODO : 서버 코드 업로더
    server.onNotFound(handle_404);
    // [ESP:HTTP] : http 서버 준비
    server.begin();
    
    #ifdef PCCOM
    Serial.println("[ESP] : Server Begin");
    #endif
    //
}

#ifdef PCCOM
void handle_pccom(void);
#endif
void loop(){
    // [I2C] : I2C 아이디 할당 서버 조작
    i2cserver.handle();
    // [ESP:HTTP] : http 서버 클라이언트 핸들
    server.handleClient();
    // TODO : 인터프리터 동작
    
    #ifdef PCCOM
    handle_pccom();
    #endif
}

#ifdef PCCOM
void handle_pccom(void){

    if(Serial.available()>0){
        auto input = Serial.read();
        if(input == 'r'){
            Serial.println(F("[CMD] : read"));
            for (size_t i = AUTOALLOCATEE_BEGIN_ADDRESS; i < AUTOALLOCATEE_END_ADDRESS; i++)
            {
                if(i2cserver.exist(i)){
                    delay(100);
                    Serial.print(F("> "));
                    Vec4 data = i2cserver.get_vec(i);
                    Serial.print(i);
                    Serial.print(F("("));
                    Serial.print(data.x);
                    Serial.print(F(", "));
                    Serial.print(data.y);
                    Serial.print(F(", "));
                    Serial.print(data.z);
                    Serial.print(F(", "));
                    Serial.print(data.w);
                    Serial.print(F(")"));
                    Serial.println();
                }
            }
        }
        else if(input == 'm'){
            Serial.println(F("[CMD] : meta"));
            for (size_t i = AUTOALLOCATEE_BEGIN_ADDRESS; i < AUTOALLOCATEE_END_ADDRESS; i++)
            {
                if(i2cserver.exist(i)){
                    Serial.print(F("> "));
                    auto data = i2cserver.get_meta(i);
                    StaticJsonDocument<512> dom;
                    
                    dom["id"] = i;
                    dom["name"] = uint16_t(data.name);
                    auto dom_axis = dom.createNestedObject("axis");
                    dom_axis["x"] = uint16_t(data.x);
                    dom_axis["y"] = uint16_t(data.y);
                    dom_axis["z"] = uint16_t(data.z);
                    dom_axis["w"] = uint16_t(data.w);
                    auto dom_min = dom.createNestedObject("min");
                    dom_min["x"] = data.data_min.x;
                    dom_min["y"] = data.data_min.y;
                    dom_min["z"] = data.data_min.z;
                    dom_min["w"] = data.data_min.w;
                    auto dom_max = dom.createNestedObject("max");
                    dom_max["x"] = data.data_max.x;
                    dom_max["y"] = data.data_max.y;
                    dom_max["z"] = data.data_max.z;
                    dom_max["w"] = data.data_max.w;
                    serializeJson(dom, Serial);
                    Serial.println();
                }
            }
        }
        else if(input == 'n'){
            Serial.println(F("[CMD] : on"));
            for (size_t i = AUTOALLOCATEE_BEGIN_ADDRESS; i < AUTOALLOCATEE_END_ADDRESS; i++)
            {
                if(i2cserver.exist(i)){
                    i2cserver.set_vec(i, Vec4(1,1,1,1));
                }
            }
        }
        else if(input == 'f'){
            Serial.println(F("[CMD] : off"));
            for (size_t i = AUTOALLOCATEE_BEGIN_ADDRESS; i < AUTOALLOCATEE_END_ADDRESS; i++)
            {
                if(i2cserver.exist(i)){
                    i2cserver.set_vec(i, Vec4(0,0,0,0));
                }
            }
        }

    }
}
#endif
#endif