#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <string.h>
#include <strings.h>

#include "rawsocket.h"

void printMAC(const char *tag, uint8_t mac[6]) {
	printf("%s %X:%X:%X:%X:%X:%X\n", tag, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int sendtoeth(char* interface, char dstmac[6], char* pkt, size_t pktsize) {
	int txlen = 0;
	struct sockaddr_ll device;
	struct ifreq ifmac;

	bzero(&device, sizeof(device));

	int fd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if (fd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	char *sendbuf = malloc(pktsize + 14);
	struct ether_header *eh = (struct ether_header *) sendbuf;

	/* Get the MAC address of the interface to send on */
	memset(&ifmac, 0, sizeof(struct ifreq));
	strncpy(ifmac.ifr_name, interface, IFNAMSIZ-1);
	if (ioctl(fd, SIOCGIFHWADDR, &ifmac) < 0) {
		perror("SIOCGIFHWADDR");
	}

	/* Construct the Ethernet header */
	bzero(sendbuf, pktsize + 14);

	/* Ethernet header */
	memcpy(eh->ether_shost, ifmac.ifr_hwaddr.sa_data, 6);
	// printMAC("Src MAC: ", eh->ether_shost);

	memcpy(eh->ether_dhost, dstmac, 6);
	// printMAC("Dst MAC: ", eh->ether_dhost);

	/* Ethertype field */
	eh->ether_type = htons(ETHTYPE_HOMEPLUG_AV);
	txlen += sizeof(struct ether_header);
	memcpy(sendbuf + txlen, pkt, pktsize);
	txlen += pktsize;

	/* Index of the network device */
	if ((device.sll_ifindex = if_nametoindex(interface)) == 0) {
		perror("if_nametoindex() failed to obtain interface index ");
		return EXIT_FAILURE;
	}
	// printf("Interface %s index %d\n", interface, device.sll_ifindex);

	device.sll_family = AF_PACKET;
	/* Address length*/
	device.sll_halen = 6;

	/* Destination MAC */
	memcpy(device.sll_addr, dstmac, 6);

	/* Send packet */
	if (sendto(fd, sendbuf, txlen, 0, (struct sockaddr*)&device, sizeof(struct sockaddr_ll)) < 0) {
		perror("send");
		return EXIT_FAILURE;
	}
	close(fd);
	return EXIT_SUCCESS;
}

int createListeningSocket(char *interface) {
	int sockfd;
	int sockopt;
	struct ifreq ifopts;

	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHTYPE_HOMEPLUG_AV))) == -1) {
		perror("socket");
		return -1;
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, interface, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		return EXIT_FAILURE;
	}

	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, interface, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		return EXIT_FAILURE;
	}
	return sockfd;
}

ssize_t receive(int sockfd, char* buf, size_t bufsize, char mymac[6]) {
	size_t numbytes = recvfrom(sockfd, buf, bufsize, 0, NULL, NULL);
	if (numbytes == -1) {
		return -1;
	} else {
		if (memcmp(mymac, buf, 6) == 0) {
			return numbytes;
		} else {
			return -2;
		}
	}
}
