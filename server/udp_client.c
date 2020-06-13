
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(void)
{
  int sock;
  struct sockaddr_in sa;
  int bytes_sent;
  ssize_t recsize;
  socklen_t fromlen;

  char ip_client[INET_ADDRSTRLEN];
  char buffer[200];

  fromlen = sizeof sa;
  strcpy(buffer, "hello world!");
 
  /* create an Internet, datagram, socket using UDP */
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) {
      /* if socket failed to initialize, exit */
      printf("Error Creating Socket");
      exit(EXIT_FAILURE);
  }
 
  /* Zero out socket address */
  memset(&sa, 0, sizeof sa);
  
  /* The address is IPv4 */
  sa.sin_family = AF_INET;
 
   /* IPv4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value */
  //sa.sin_addr.s_addr = inet_addr("185.3.95.41");
  //sa.sin_addr.s_addr = inet_addr("192.168.88.2");
  //sa.sin_addr.s_addr = inet_addr("105.213.78.79");
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  /* sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654 */
  sa.sin_port = htons(42511);
 
  inet_ntop(AF_INET, &sa.sin_addr, ip_client, INET_ADDRSTRLEN);
  printf("Send packet to %s:%d\nData(%lu): %s\n\n",ip_client, ntohs(sa.sin_port), strlen(buffer), buffer);
  bytes_sent = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa);
  if (bytes_sent < 0) {
    printf("Error sending packet: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  recsize = recvfrom(sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
  if (recsize < 0) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  inet_ntop(AF_INET, &sa.sin_addr, ip_client, INET_ADDRSTRLEN);
  printf("Received packet from %s:%d\nData(%d): [%s]\n\n",ip_client, ntohs(sa.sin_port), (int)recsize, buffer);
 
  close(sock); /* close the socket */
  return 0;
}
