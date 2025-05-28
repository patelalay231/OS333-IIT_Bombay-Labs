#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENTS 16

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  int epoll_fd, nfds;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;
  
  
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
  
  /* create socket */
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  
  /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  
  /* bind socket to this port number on this machine */
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0) 
    error("ERROR on binding");
  
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  /* listen for incoming connection requests */
  
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  
  epoll_fd = epoll_create(1);
  
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
  ev.data.fd = sockfd;
  
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) 
    error("ERROR epoll ctl");
  
  
  while(1) {
    
    nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    
    if(nfds == -1) error("ERROR epoll wait");
    
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == sockfd) {
	
	/* accept a new request, create a newsockfd */
	
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
          error("ERROR on accept");
	
	fcntl(newsockfd, F_SETFL, O_NONBLOCK);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsockfd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newsockfd, &ev) == -1) 
	  error("ERROR epoll ctl add");
	
      }
      else if(events[i].events & EPOLLIN) {
	
	/* read message from client */
	
	bzero(buffer,256);
	n = read(events[i].data.fd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);
	
	/* send reply to client */
	
	n = write(events[i].data.fd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");
	 
      }
    }

  }
}