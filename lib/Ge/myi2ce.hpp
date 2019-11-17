#ifndef HEADER_MYI2C
#define HEADER_MYI2C

#include <Arduino.h>
#include <Wire.h>

const uint8_t UNINITIALIZED_ADDRESS = 0x70;
const uint8_t IDALLOCATOR_ADDRESS = 0x71;

const uint8_t AUTOALLOCATEE_BEGIN_ADDRESS = 0x03;
const uint8_t AUTOALLOCATEE_END_ADDRESS = 0x6F;

// void server_init();
// void server_id_allocator_loop();
// void client_init();
// void client_request_id();


enum NAME : uint16_t{
    UNDEFINED = 0x00000000,
    DEBUGGING,
};

enum AXIS : uint16_t{
    UNUSED = 0x00000000,
    BUILDIN_LED,
};

struct Vec4{
    int8_t x; 
    int8_t y; 
    int8_t z; 
    int8_t w;
public:
    Vec4() : x(0), y(0), z(0), w(0){}
    Vec4(
        int8_t c_x, 
        int8_t c_y, 
        int8_t c_z, 
        int8_t c_w
    ) : x(c_x), y(c_y), z(c_z), w(c_w){}
    const static size_t size = 4;
    static Vec4 from(uint8_t* src){
        Vec4 res;
        res.x = src[0];
        res.y = src[1];
        res.z = src[2];
        res.w = src[3];
        return res;
    }
    void to(uint8_t* dst){
        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
        dst[3] = w;
    }
};

struct Header{
    NAME name;

    AXIS x;
    AXIS y;
    AXIS z;
    AXIS w;

    Vec4 data_min;
    Vec4 data_max;
public:
    Header() : name(NAME::UNDEFINED), x(AXIS::UNUSED), y(AXIS::UNUSED), z(AXIS::UNUSED), w(AXIS::UNUSED){}
    const static size_t size = 18;
    static Header from(uint8_t* src);
    void to(uint8_t* dst){
        dst[0] = uint8_t(*(uint16_t*)&name >> 8 & 0xFF);
        dst[1] = uint8_t(*(uint16_t*)&name & 0xFF);

        dst[2] = uint8_t(*(uint16_t*)&x >> 8 & 0xFF);
        dst[3] = uint8_t(*(uint16_t*)&x & 0xFF);
        dst[4] = uint8_t(*(uint16_t*)&y >> 8 & 0xFF);
        dst[5] = uint8_t(*(uint16_t*)&y & 0xFF);
        dst[6] = uint8_t(*(uint16_t*)&z >> 8 & 0xFF);
        dst[7] = uint8_t(*(uint16_t*)&z & 0xFF);
        dst[8] = uint8_t(*(uint16_t*)&w >> 8 & 0xFF);
        dst[9] = uint8_t(*(uint16_t*)&w & 0xFF);

        data_min.to(&dst[10]);
        data_max.to(&dst[14]);
    }
};

class Myi2cServer{
    int interrupt;
public:
    Myi2cServer() : interrupt(0){}
    void init();
    void handle();

    size_t count();
    Vec4 get_vec(int n);
    void set_vec(int n, Vec4 data);
    bool exist(int n);
    Header get_meta(int n);
};

class Myi2cClient{
    uint8_t id;
public:
    Myi2cClient() : id(0){}
    void init();
    void begin();
    void setup_handler(Vec4 (*data_sender)(void), void (*data_receiver)(Vec4), Header (*meta_sender)(void));
    uint8_t get_id(){return id;};
};




#endif
