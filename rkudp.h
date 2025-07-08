#ifndef rkudp_h
#define rkudp_h

#include <stdint.h>


struct hdr_rkudp {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t checksum;
};





#endif
