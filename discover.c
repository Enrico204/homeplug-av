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

char dstmac[] = ATHEROS_HOMEPLUG_AV;

int main(int argn, char **argv) {
	if (argn != 2) {
		printf("Usage: %s <interface>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char sendbuf[BUF_SIZ];
	char srcmac[6];
	struct ifreq ifmac;
	char *interface = argv[1];

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
	pkt->header->type = GET_DEVICE_SW_VERSION_REQ_TYPE;
	pkt->oui[0] = 0x00;
	pkt->oui[1] = 0xb0;
	pkt->oui[2] = 0x52;
	pkt->payload = NULL;
	size_t r = preparePacket(sendbuf, BUF_SIZ, pkt);

	int ret = sendtoeth(interface, dstmac, sendbuf, r);
	if (ret != EXIT_SUCCESS) {
		return ret;
	}

	printf("MAC Address\t\tOUI\tStatus\tDeviceId\tUpgradable\tVersion\n");

	while(true) {
		ssize_t r = receive(sockfd, sendbuf, BUF_SIZ, srcmac);
		if (r == -1) {
			printf("Ending receiving\n");
			break;
		} else if (r > 0) {
			char *pkt2 = sendbuf + 14;

			struct HomePlugPacket* pkt = parseResponse(pkt2, r);
			// printf("Version: %d - Packet Type: 0x%04X\n", pkt->header->version, pkt->header->type);
			// printf("OUI: %02X%02X%02X\n", 0x000000FF & pkt->oui[0], 0x000000FF & pkt->oui[1], 0x000000FF & pkt->oui[2]);
			struct GetDeviceSWVersionResponse *payload = (struct GetDeviceSWVersionResponse *)(pkt->payload);
			printf("%02X:%02X:%02X:%02X:%02X:%02X\t%02X%02X%02X\t%d\t%d\t\t%c\t\t%s\n",
				(uint8_t)sendbuf[6], (uint8_t)sendbuf[7], (uint8_t)sendbuf[8],
				(uint8_t)sendbuf[9], (uint8_t)sendbuf[10], (uint8_t)sendbuf[11],
				(uint8_t)pkt->oui[0], (uint8_t)pkt->oui[1], (uint8_t)pkt->oui[2],
				payload->status, payload->deviceId, payload->upgradable == 0 ? 'N':'Y', payload->version);
			freeResponse(pkt);
		}
	}

	return EXIT_SUCCESS;
}
