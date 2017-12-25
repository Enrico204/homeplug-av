#ifndef __HOMEPLUG_AV_H
#define __HOMEPLUG_AV_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>


#define GET_DEVICE_SW_VERSION_REQ_LEN   (3 + 47)
#define GET_DEVICE_SW_VERSION_REQ_TYPE  0xa000
#define GET_DEVICE_SW_VERSION_RES_TYPE  0xa001


struct HomePlugPacket {
	struct MACManagementHeader *header;
	char oui[3];
	void *payload;
};

struct MACManagementHeader {
	uint8_t version;
	uint16_t type;
};

struct GetDeviceSWVersionResponse {
	uint8_t status;
	uint8_t deviceId;
	uint8_t versionLength;
	char *version;
	uint8_t upgradable;
};

struct HomePlugPacket* parseResponse(char* buf, int len);
void freeResponse(struct HomePlugPacket *response);
size_t preparePacket(char *buf, size_t maxsize, struct HomePlugPacket *pkt);

#endif
