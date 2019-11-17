#ifdef SLAVE

#include <Arduino.h>
#include <ArduinoJson.h>

#include "myi2ce.hpp"

Myi2cClient i2cclient;


// TODO : 모듈별로 다르게 정의해야 하는 함수들
// module_setup, module_loop, get_meta, get_data, set_data
// WARN : get_meta, get_data, set_data  이 함수들은 반드시 빠르게 동작해야 함
//        만약 이 함수들 내부에 시간이 오래걸리는 동작(시리얼 통신, SPI 통신)등이 들어가면 마스터에서 쓰레기값을 받을 가능성이 높아짐(특히 Serial은 100%)
//        이를 피하기 위해 module_loop에서 오래걸리는 동작을 처리하고 전역 변수에 넣은 뒤 위 함수들에선 전역변수 참조를 통해 해결할 것

// 마스터로부터 값 변경이 요청될 경우
void set_data(Vec4 data);
// 마스터로부터 값 참조가 요청된 경우
Vec4 get_data();
// 마스터로부터 디바이스 메타데이터 요청을 받은 경우
Header get_meta();
// 각 모듈별로 필요한 setup 함수 ex)핀 모드 셋업, SPI통신 셋업 등
void module_setup(void);
// 각 모듈별로 필요한 loop 함수 ex) SPI 값 참조 등
void module_loop(void);

void setup(){
    #ifdef PCCOM
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("[PCCOM] : Hello"));
    #endif

    // [I2C] : 서버와의 연결 대기, 반드시 시작과 함께 이를 호출해야 함
    i2cclient.init();

    #ifdef PCCOM
    Serial.println(F("[I2C-My] : Initalized"));
    #endif

    // [PIN] : 7번 핀 입력 대기
    pinMode(7, INPUT_PULLUP);
    
    #ifdef PCCOM
    Serial.println(F("[PIN] : Waiting"));
    #endif

    while(digitalRead(7) == HIGH);

    #ifdef PCCOM
    Serial.println(F("[PIN] : Complete"));
    #endif

    // [I2C] : ID 할당 요청
    i2cclient.begin();

    #ifdef PCCOM
    Serial.print(F("[I2C-My] : ID Detected "));
    Serial.println(i2cclient.get_id());
    #endif
    // [I2C] : 핸들러 셋업
    i2cclient.setup_handler(
        get_data,
        set_data,
        get_meta
    );
    
    #ifdef PCCOM
    Serial.println(F("[I2C-My] : Setup handlers "));
    #endif

    // [PIN] : 다음 디바이스 동작을 위한 코드
    // TODO, 적당한 코드 수정이 필요, 현재 구조로는 트랜지스터가 필요함
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    #ifdef PCCOM
    Serial.println(F("[PIN] : Turn on next slave"));
    #endif

    // 각 모듈별로 필요한 setup 함수
    module_setup();
    #ifdef PCCOM
    Serial.println(F("[MODULE] : Custom Module Initializer"));
    #endif
}
void loop(){
    // 각 모듈별로 필요한 loop 함수
    module_loop();
}

// TODO : 각 모듈별로 함수들 정의
#ifdef MODULE_BUILTIN_LED
    void module_setup(void){
        pinMode(LED_BUILTIN, OUTPUT);
    }
    void module_loop(void){
        
    }
    void set_data(Vec4 data){
        if (data.x > 0.5){
            digitalWrite(LED_BUILTIN, HIGH);
        }else{
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
    Vec4 get_data(){
        Vec4 data;
        if(digitalRead(LED_BUILTIN) == HIGH){
            data.x = 1.f;
        }else{
            data.x = 0.f;
        }
        return data;
    }
    Header get_meta(){
        Header header;
        header.name = NAME::DEBUGGING;
        header.x = AXIS::BUILDIN_LED;
        header.data_min = Vec4(0,0,0,0);
        header.data_max = Vec4(1,0,0,0);
        return header;
    }
#endif

#endif