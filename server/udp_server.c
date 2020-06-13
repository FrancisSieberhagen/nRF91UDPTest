
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> /* for close() for socket */ 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>

int main(void)
{
  int sock, childpid;
  struct sockaddr_in sa; 
  char buffer[1024];
  ssize_t recsize;

  socklen_t fromlen;
  char ip_server[INET_ADDRSTRLEN];
  char ip_client[INET_ADDRSTRLEN];

  int bytes_sent;

  FILE *fptr;
  time_t now;

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(42511);
  fromlen = sizeof sa;

  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (bind(sock, (struct sockaddr *)&sa, sizeof sa) == -1) {
    perror("error bind failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  inet_ntop(AF_INET, &sa.sin_addr.s_addr, ip_server, INET_ADDRSTRLEN);
  printf("Server %s:%d\n",ip_server, ntohs(sa.sin_port));

  for (;;) {
    recsize = recvfrom(sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
    if (recsize < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    buffer[recsize] = '\0';

    if ( (childpid = fork()) == 0) {

      if ((fptr = fopen("udp_server.txt","a")) == NULL){
        printf("Error! opening file");
        exit(1);
      }

      inet_ntop(AF_INET, &sa.sin_addr, ip_client, INET_ADDRSTRLEN);
      printf("Received packet from %s:%d\nData(%d): [%s]\n\n",ip_client, ntohs(sa.sin_port), (int)recsize, buffer);
      time(&now);
      fprintf(fptr, "%sReceived packet from %s:%d\nData(%d): [%s]\n\n",ctime(&now), ip_client, ntohs(sa.sin_port), (int)recsize, buffer);

      printf("Send packet to %s:%d\nData(%lu): %s\n\n",ip_client, ntohs(sa.sin_port), strlen(buffer), buffer);

      bytes_sent = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa);
      if (bytes_sent < 0) {
        printf("Error sending packet: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      fclose(fptr);
    }
    
  }
}
