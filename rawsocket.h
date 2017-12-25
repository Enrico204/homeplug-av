#ifndef __RAWSOCKET_H
#define __RAWSOCKET_H


#define ETHTYPE_HOMEPLUG_AV    0x88e1


int createListeningSocket(char *interface);
int sendtoeth(char* interface, char dstmac[6], char* pkt, size_t pktsize);
ssize_t receive(int sockfd, char* buf, size_t bufsize, char mymac[6]);


#endif
