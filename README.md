# 사용법
## platformio.ini
```
[env:<설정명>]
platform = <플랫폼명, uno, pro mini 모두 'atmelavr'>
board = <보드명, pro mini 의 경우 'pro16MHzatmega328', uno의 경우 'uno'>
framework = arduino
monitor_speed = 115200

lib_deps = 
    ArduinoJson


build_flags =
  -D SLAVE
  -D <전처리기 정의명>
```

env:builtin_led 가 예시로 있음


## lib/Ge/myi2ce.hpp
따로 손 댈 필요는 없음

# 전처리기
- `-D ROOT` : 루트모듈인 경우의 아두이노 엔트리 함수를 선택 표현해줌, 각 환경에는 ROOT혹은 SLAVE중 하나는 반드시 정의되어야 함
- `-D SLAVE` : 슬레이브모듈인 경우의 아두이노 엔트리 함수를 선택 표현해줌, 각 환경에는 ROOT혹은 SLAVE중 하나는 반드시 정의되어야 함
- `-D PCCOM` : PC 연결시 시리얼통신으로 로깅 데이터가 필요한 경우 선택, 만약 정의되지 않으면 로깅은 이뤄지지 않음

