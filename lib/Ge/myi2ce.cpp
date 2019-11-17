#include "myi2ce.hpp"
#include "Wire.h"

bool Myi2cServer::exist(int n){
    Wire.beginTransmission(n);
    return Wire.endTransmission() == 0;
}
void Myi2cServer::init(){
    Wire.begin();
}

void Myi2cServer::handle(){
    if (interrupt != 0){
        Wire.beginTransmission(interrupt);
        if (Wire.endTransmission() == 0){
            interrupt = 0;
            #ifdef PCCOM
            Serial.println(F("[I2C-My] : Allocate ID Complete"));
            #endif
        }
    }else{
        Wire.beginTransmission(IDALLOCATOR_ADDRESS);
        if (Wire.endTransmission() == 0){

            #ifdef PCCOM
            Serial.println(F("[I2C-My] : ID Allocator detect new device"));
            #endif

            // New I2C Detect
            uint8_t alloc_id = AUTOALLOCATEE_BEGIN_ADDRESS;
            for (; alloc_id <= AUTOALLOCATEE_END_ADDRESS; alloc_id++)
            {
                Wire.beginTransmission(alloc_id);
                if(Wire.endTransmission() != 0){
                    break;
                }
            }

            #ifdef PCCOM
            Serial.print(F("[I2C-My] : Unoccupied ID("));
            Serial.print(alloc_id);
            Serial.println(F(")"));
            #endif
            
            Wire.beginTransmission(IDALLOCATOR_ADDRESS);
            Wire.write(alloc_id);
            interrupt = alloc_id;
            Wire.endTransmission();
        }
    }
}

size_t Myi2cServer::count(){
    size_t sz = 0;
    for (uint8_t id = AUTOALLOCATEE_BEGIN_ADDRESS; id <= AUTOALLOCATEE_END_ADDRESS; id++)
    {
        Wire.beginTransmission(id);
        if(Wire.endTransmission() == 0){
            sz++;
        }
    }
    return sz;
}

Vec4 Myi2cServer::get_vec(int nth){
    Wire.requestFrom(nth, Vec4::size);
    uint8_t buf[Vec4::size];
    for (size_t i = 0; i < Vec4::size; i++)
    {
        int temp = Wire.read();
        buf[i] = uint8_t(temp);
    }
    return Vec4::from(buf);
}

Header Myi2cServer::get_meta(int nth){
    Wire.requestFrom(nth, Vec4::size + Header::size);
    for (size_t i = 0; i < Vec4::size; i++){
        Wire.read();
    }
    uint8_t buf[Header::size];
    Wire.readBytes(buf, Header::size);
    return Header::from(buf);
}

void Myi2cServer::set_vec(int nth, Vec4 data){
    Wire.beginTransmission(nth);
    uint8_t buf[Vec4::size];
    data.to(buf);
    Wire.write(buf, Vec4::size);
    Wire.endTransmission();
}



void Myi2cClient::init(){
    Wire.begin(UNINITIALIZED_ADDRESS);
}

int cli_interrupt = 0;
uint8_t cli_temp_id = 0;
void Myi2cClient::begin(){
    #ifdef PCCOM
    Serial.println(F("[I2C-My] : Begin detect mode"));
    #endif

    Wire.begin(IDALLOCATOR_ADDRESS);
    Wire.onReceive([](int n){
        if(n <= 0){
            return;
        }
        cli_temp_id = Wire.read();
        cli_interrupt = 1;
    });
    while(cli_interrupt == 0){delay(10);};

    #ifdef PCCOM
    Serial.println(F("[I2C-My] : Allocated id"));
    #endif
    
    id = cli_temp_id;
    delay(10);

    #ifdef PCCOM
    Serial.print(F("[I2C-My] : Id = "));
    Serial.print(id);
    Serial.println();
    #endif

    Wire.begin(id);
}


Vec4 (*s_data_sender)(void);
void (*s_data_receiver)(Vec4);
Header (*s_meta_sender)(void);
void Myi2cClient::setup_handler(Vec4 (*data_sender)(void), void (*data_receiver)(Vec4), Header (*meta_sender)(void)){
    s_data_sender = data_sender;
    s_data_receiver = data_receiver;
    s_meta_sender = meta_sender;
    Wire.onReceive([](int n){
        if(n <= 0){
            return;
        }
        uint8_t buf[Vec4::size];
        Wire.readBytes(buf, Vec4::size);
        // for (int i = 0; i < n; i++)
        // {
        //     if(!Wire.available()){
        //         break;
        //     }
        //     *((uint8_t*)&temp + i) = Wire.read();
        // }
        s_data_receiver(Vec4::from(buf));
    });
    Wire.onRequest([](){
        Vec4 temp = s_data_sender();
        Header meta = s_meta_sender();
        uint8_t buf[Vec4::size + Header::size];
        temp.to(buf);
        meta.to(&buf[Vec4::size]);
        Wire.write(buf, Vec4::size + Header::size);
        
        // uint8_t buf[Vec4::size];
        // temp.to(buf);
        // Wire.write(buf, Vec4::size);
    });
}

Header Header::from(uint8_t* src){
    Header res;
    res.name = static_cast<NAME>((uint16_t(src[0]) << 8) | uint16_t(src[1]));
    res.x = static_cast<AXIS>((uint16_t(src[2]) << 8) | uint16_t(src[3]));
    res.y = static_cast<AXIS>((uint16_t(src[4]) << 8) | uint16_t(src[5]));
    res.z = static_cast<AXIS>((uint16_t(src[6]) << 8) | uint16_t(src[7]));
    res.w = static_cast<AXIS>((uint16_t(src[8]) << 8) | uint16_t(src[9]));
    res.data_min = Vec4::from(&src[10]);
    res.data_max = Vec4::from(&src[14]);
    return res;
}