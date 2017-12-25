
#include "homeplug-av.h"
#include <string.h>


struct HomePlugPacket* parseResponse(char* buf, int len) {
	assert(buf != NULL);
	assert(len > 3);

	size_t offset = 0;

	struct HomePlugPacket *response = malloc(sizeof(struct HomePlugPacket));
	response->header = malloc(sizeof(struct MACManagementHeader));
	response->header->version = buf[0];
	response->header->type = (buf[2] << 8) | (buf[1]);

	memcpy(response->oui, buf + 3, 3);
	offset += 6;

	if (response->header->type == GET_DEVICE_SW_VERSION_RES_TYPE) {
		// device sw version response
		response->payload = malloc(sizeof(struct GetDeviceSWVersionResponse));
		struct GetDeviceSWVersionResponse *payload = (struct GetDeviceSWVersionResponse*)response->payload;
		payload->status = buf[offset];
		offset++;

		payload->deviceId = buf[offset];
		offset++;

		payload->versionLength = buf[offset];
		offset++;

		payload->version = malloc(payload->versionLength + 1);
		bzero(payload->version, payload->versionLength + 1);
		memcpy(payload->version, buf + offset, payload->versionLength);
		buf += payload->versionLength;

		payload->upgradable = buf[offset];
		offset++;
	} else if (response->header->type == NETWORK_INFO_RES_TYPE) {
		struct NetworkInfoResponse *payload = malloc(sizeof(struct NetworkInfoResponse));
		response->payload = payload;

		payload->logicalNetworkNumber = buf[offset];
		offset++;

		payload->networks = malloc(sizeof(struct NetworkInformation*) * payload->logicalNetworkNumber);

		for (int i = 0; i < payload->logicalNetworkNumber; i++) {
			struct NetworkInformation *net = malloc(sizeof(struct NetworkInformation));

			memcpy(net->networkId, buf + offset, 7);
			offset += 7;

			net->shortNetworkId = buf[offset];
			offset++;

			net->stationRole = buf[offset];
			offset++;

			memcpy(net->ccoMAC, buf + offset, 6);
			offset += 6;

			net->ccoTEid = buf[offset];

			payload->networks[i] = net;
		}

		payload->stationNumber = buf[offset];
		offset++;

		payload->stations = malloc(sizeof(struct StationInformation*) * payload->stationNumber);

		for (int i = 0; i < payload->stationNumber; i++) {
			struct StationInformation *info = malloc(sizeof(struct StationInformation));

			memcpy(info->stationMAC, buf + offset, 6),
			offset += 6;

			info->stationTEid = buf[offset];
			offset++;

			memcpy(info->firstBridgedNode, buf + offset, 6),
			offset += 6;

			info->averagePHYTXDataRateMbps = buf[offset];
			offset++;

			info->averagePHYRXDataRateMbps = buf[offset];
			offset++;

			payload->stations[i] = info;
		}
	}
	return response;
}

void freeResponse(struct HomePlugPacket *response) {
	assert(response != NULL);
	if (response->header->type == GET_DEVICE_SW_VERSION_RES_TYPE) {
		free(((struct GetDeviceSWVersionResponse*)(response->payload))->version);
		free(response->payload);
	} else if (response->header->type == NETWORK_INFO_RES_TYPE) {
		struct NetworkInfoResponse *payload = (struct NetworkInfoResponse *)response->payload;
		for (int i = 0; i < payload->logicalNetworkNumber; i++) {
			free(payload->networks[i]);
		}
		free(payload->networks);

		for (int i = 0; i < payload->stationNumber; i++) {
			free(payload->stations[i]);
		}
		free(payload->stations);

		free(payload);
	}
	free(response->header);
}

size_t preparePacket(char *buf, size_t maxsize, struct HomePlugPacket *pkt) {
	assert(buf != NULL);
	assert(pkt != NULL);
	assert(maxsize > 6);

	bzero(buf, maxsize);
	size_t r = 0;

	// Copy version
	buf[0] = pkt->header->version;
	r++;

	// Copy packet type
	memcpy(buf + 1, &(pkt->header->type), 2);
	r =+ 2;

	// Copy Vendor MME
	memcpy(buf + 3, pkt->oui, 3);
	r =+ 3;

	if (pkt->header->type == GET_DEVICE_SW_VERSION_REQ_TYPE
		|| pkt->header->type == NETWORK_INFO_REQ_TYPE) {
		r += 44;
	}

	return r;
}
