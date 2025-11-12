#ifndef CONSTANTS_H
#define CONSTANTS_H

// 버퍼 크기 상수
#define BUFFER_SIZE 1024
#define COMMAND_SIZE 512
#define PACKAGE_NAME_SIZE 256
#define MAX_PACKAGES 1024
#define MAX_LINES 1000
#define MAX_PACKAGE_NAME_LENGTH 255

// 패키지 관리 액션
#define ACTION_INSTALL 0
#define ACTION_REMOVE 1
#define ACTION_UPDATE 2

// UI 관련 상수
#define HEADER_LINES 2
#define FOOTER_LINES 1
#define MIN_ROWS 10
#define MIN_COLS 40

#endif // CONSTANTS_H
