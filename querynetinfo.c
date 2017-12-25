#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include "rawsocket.h"
#include "homeplug-av.h"

int main(int argn, char **argv) {
	if (argn != 3) {
		printf("Usage: %s <interface> <MAC>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char sendbuf[BUF_SIZ];
	char srcmac[6];
	char dstmac[6];
	struct ifreq ifmac;
	char *interface = argv[1];

	char *macb;
	for (int i = 0; i < 6; i++) {
		macb = strtok(i == 0 ? argv[2] : NULL, ":");
		dstmac[i] = strtol(macb, NULL, 16);
	}

	printf("Send network info query to %02X:%02X:%02X:%02X:%02X:%02X\n",
		(uint8_t)dstmac[0], (uint8_t)dstmac[1],
		(uint8_t)dstmac[2], (uint8_t)dstmac[3],
		(uint8_t)dstmac[4], (uint8_t)dstmac[5]);

	int sockfd = createListeningSocket(interface);

	memset(&ifmac, 0, sizeof(struct ifreq));
	strncpy(ifmac.ifr_name, interface, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifmac) < 0) {
		perror("SIOCGIFHWADDR");
	}
	memcpy(srcmac, ifmac.ifr_hwaddr.sa_data, 6);

	// put content
	struct HomePlugPacket *pkt = malloc(sizeof(struct HomePlugPacket));
	pkt->header = malloc(sizeof(struct MACManagementHeader));
	pkt->header->version = 0;
	pkt->header->type = NETWORK_INFO_REQ_TYPE;
	pkt->oui[0] = 0x00;
	pkt->oui[1] = 0xb0;
	pkt->oui[2] = 0x52;
	pkt->payload = NULL;
	size_t r = preparePacket(sendbuf, BUF_SIZ, pkt);

	int ret = sendtoeth(interface, dstmac, sendbuf, r);
	if (ret != EXIT_SUCCESS) {
		return ret;
	}

	while(true) {
		ssize_t r = receive(sockfd, sendbuf, BUF_SIZ, srcmac);
		if (r == -1) {
			printf("Ending receiving\n");
			break;
		} else if (r > 0) {
			char *pkt2 = sendbuf + 14;

			struct HomePlugPacket* pkt = parseResponse(pkt2, r);

			struct NetworkInfoResponse *payload = (struct NetworkInfoResponse *)(pkt->payload);
			printf("Logical Networks: %d\n", payload->logicalNetworkNumber);
			for (int i = 0; i < payload->logicalNetworkNumber; i++) {
				struct NetworkInformation *info = payload->networks[i];
				printf("\tNetwork ID: %02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
					(uint8_t)info->networkId[0], (uint8_t)info->networkId[1],
					(uint8_t)info->networkId[2], (uint8_t)info->networkId[3],
					(uint8_t)info->networkId[4], (uint8_t)info->networkId[5],
					(uint8_t)info->networkId[6]);
				printf("\tShort Network ID: 0x%02X\n", (uint8_t)info->shortNetworkId);
				printf("\tTerminal Equipment Identifier: %d\n", info->terminalEquipmentIdentifier);
				printf("\tStation Role: ");
				switch(info->stationRole) {
					case 0:
						printf("Station\n");
						break;
					case 2:
						printf("Central coordinator\n");
						break;
					default:
						printf("Unknown: %02X\n", (uint8_t)info->stationRole);
						break;
				}
				printf("\tCCo MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
					(uint8_t)info->ccoMAC[0], (uint8_t)info->ccoMAC[1],
					(uint8_t)info->ccoMAC[2], (uint8_t)info->ccoMAC[3],
					(uint8_t)info->ccoMAC[4], (uint8_t)info->ccoMAC[5]);
				printf("\tCCo Terminal Equipment Identifier: %d\n", info->ccoTEid);
				printf("\n");
			}

			printf("Stations: %d\n", payload->stationNumber);
			for (int i = 0; i < payload->stationNumber; i++) {
				struct StationInformation *info = payload->stations[i];
				printf("\tStation MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
					(uint8_t)info->stationMAC[0], (uint8_t)info->stationMAC[1],
					(uint8_t)info->stationMAC[2], (uint8_t)info->stationMAC[3],
					(uint8_t)info->stationMAC[4], (uint8_t)info->stationMAC[5]);
				printf("\tStation Terminal Equipment Identifier: %d\n", info->stationTEid);
				printf("\tMAC Address of first Node Bridged by Station: %02X:%02X:%02X:%02X:%02X:%02X\n",
					(uint8_t)info->firstBridgedNode[0], (uint8_t)info->firstBridgedNode[1],
					(uint8_t)info->firstBridgedNode[2], (uint8_t)info->firstBridgedNode[3],
					(uint8_t)info->firstBridgedNode[4], (uint8_t)info->firstBridgedNode[5]);
				printf("\tAverage PHY Tx/Rx data Rate (Mbps): %d/%d\n",
					info->averagePHYTXDataRateMbps, info->averagePHYRXDataRateMbps);
				printf("\n");
			}
			freeResponse(pkt);
			break;
		}
	}

	return EXIT_SUCCESS;
}
