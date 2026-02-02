#ifndef COAP_H
#define COAP_H

#include<stdint.h>

#define COAP_VERSION 1
#define COAP_PORT 5683

#define COAP_CON 0x00
#define COAP_NON 0x01
#define COAP_ACK 0x02
#define COAP_RST 0x03

#define COAP_PAYLOAD_MARKER 0xFF

#define COAP_GET 0x01
#define COAP_FORBIDDEN 0x83//4.03
#define COAP_BAD_REQUEST 0x80 // 4.00
#define COAP_UNAVAILABLE 0xA3 //5.03 service unavailable 
#define COAP_CONTENT 0x45 // 2.05 content
#define COAP_NOT_FOUND 132 // 4.04 not found
typedef struct
{
	uint8_t option_delta:4;
	uint8_t opt_len:4;
	char option_value[15];
}coap_option;

typedef struct
{
	uint32_t payload;
	uint16_t payload_len;
}coap_payload;

typedef struct
{
	uint8_t version:2;
	uint8_t type:2;
	uint8_t token_len:4;
	uint8_t code;
	uint16_t msg_id;
	uint8_t token;
	coap_option option[2];
	uint8_t payload_marker;
	coap_payload payload;
}coap_header;

int fun();
uint8_t hum_int,hum_dec,temp_int,temp_dec;
#endif
