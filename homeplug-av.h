#ifndef __HOMEPLUG_AV_H
#define __HOMEPLUG_AV_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>


#define ATHEROS_HOMEPLUG_AV    { 0x00, 0xb0, 0x52, 0x00, 0x00, 0x01 }
#define BUF_SIZ                1400


#define GET_DEVICE_SW_VERSION_REQ_LEN   (3 + 47)
#define GET_DEVICE_SW_VERSION_REQ_TYPE  0xa000
#define GET_DEVICE_SW_VERSION_RES_TYPE  0xa001

#define NETWORK_INFO_REQ_LEN            (3 + 47)
#define NETWORK_INFO_REQ_TYPE           0xa038
#define NETWORK_INFO_RES_TYPE           0xa039


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

struct NetworkInfoResponse {
	uint8_t logicalNetworkNumber;
	struct NetworkInformation **networks;
	uint8_t stationNumber;
	struct StationInformation **stations;
};

struct NetworkInformation {
	uint8_t networkId[7];
	uint8_t shortNetworkId;
	uint8_t terminalEquipmentIdentifier;
	uint8_t stationRole;
	uint8_t ccoMAC[6];
	uint8_t ccoTEid;
};

struct StationInformation {
	uint8_t stationMAC[6];
	uint8_t stationTEid;
	uint8_t firstBridgedNode[6];
	uint8_t averagePHYTXDataRateMbps;
	uint8_t averagePHYRXDataRateMbps;
};

struct HomePlugPacket* parseResponse(char* buf, int len);
void freeResponse(struct HomePlugPacket *response);
size_t preparePacket(char *buf, size_t maxsize, struct HomePlugPacket *pkt);

#endif
