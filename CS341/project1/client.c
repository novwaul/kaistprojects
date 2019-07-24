/* 20160788   */
/* InJe Hwang */
/* client.c   */
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define ARGNUM 3
#define FAILURE 0
#define TRUE 1
#define ERROR -1
#define CLMAX 12 // max content-length
#define RPMAX 2// respond receive max

int main(int argc, char** argv)
{
  int GET = 0, POST = 0, sockfd, contck, len, i, c,
      dotcount, coloncount,        // check number of dot & colon
      index[5],                    // 0~2 : dot(.) index, 3 : colon(:) index, 4 : slash(/) index
      sub01, sub12, sub23, sub34   // sub01 = index[1] - index[0], likewise..
     ;
  struct sockaddr_in servaddr;
  char *COMMAND, *URL, *pPORT, *pIP,
    *wnt,                   //store strings after '/' 
    CL[CLMAX],              //store Content-Length
    respond[RPMAX],         //store respond from server
    *data;                  //store user input data 
  unsigned long size;
  
  /* check argument number */
  if(argc != ARGNUM)
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument number");
      return FAILURE; // exit program
    }

  /* check argumemt value */
  COMMAND = argv[1];
  if(strcmp(COMMAND, "-g") == 0) GET = TRUE;
  else if(strcmp(COMMAND, "-p") == 0) POST = TRUE;
  else // error
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument value");
      return FAILURE;
    }
  
  /* check format of IP and PORT */
  URL = argv[2];
  len = strlen(URL);
  dotcount = coloncount = 0;
  for( i = 0 ; i < len ; i++) 
    {
      /* check whether digit */
      if (isdigit(c = URL[i])) continue;
      /* check dot part */
      else if( c == '.')
	{
	  if((coloncount != 0) || (dotcount == 3))//error
	    {
	      fprintf(stderr, "%s\n", "Error: Invalid argument value");
	      return FAILURE;
	    }
	  else // correct
	    {
	      index[dotcount] = i;
	      dotcount++;
	      continue;
	    }
	}
      /* check colon part */
      else if( c == ':' )
	{
	  if((coloncount == 1) || (dotcount != 3))//error
	    {
	      fprintf(stderr, "%s\n", "Error: Invalid argument value");
	      return FAILURE;
	    }
	  else// correct
	    {
	      index[3] = i;
	      coloncount++;
	      continue;
	    }
	}
      /* check slash part */
      else if( c == '/')
	{ // error
	  if((coloncount != 1) || ( dotcount != 3) ||
	     (URL[i+1] == '\0'/*check if URL is finishsed at slash*/  ))
	    {
	      fprintf(stderr, "%s\n", "Error: Invalid argument value");
	      return FAILURE;
	    }
	  else //correct
	    {
	      index[4] = i;
	      break; // exit forloop
	    }
	}
      /* Invalid URL*/
      else
	{
	  fprintf(stderr, "%s\n", "Error: Invalid argument value");
	  return FAILURE;
	}
    }
  /* check length of IP & PORT */
  sub01 = index[1] - index[0];
  sub12 = index[2] - index[1];
  sub23 = index[3] - index[2];
  sub34 = index[4] - index[3];
  if((index[0] < 1)||(sub01 < 2) || (sub12 < 2) || (sub23 < 2) || (sub34 < 2)
     || (index[0] > 3) || (sub01 > 4) || (sub12 > 4 ) || (sub23 > 4)) //error
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument value");
      return FAILURE;
    }
  /* store IP and PORT number */
  servaddr.sin_family = AF_INET;
  /* store PORT */
  pPORT = malloc(index[4] - index[3] -1 );
  for( i = 0 ; i < index[4] - index[3] - 1; i++)
    {
      pPORT[i] = URL[index[3] + 1 + i];
    }
  servaddr.sin_port = htons(atoi(pPORT)); // for Big-endian
  free(pPORT);
  /* store IP */
  pIP = malloc(index[3]+1);// for string
  strncpy(pIP, URL, index[3]);
  inet_pton(AF_INET, pIP, &servaddr.sin_addr);
  free(pIP);
  

  /* open a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == ERROR) //error
    {
      fprintf(stderr, "%s\n", "Error: Cannot open socket");
      return FAILURE;
    }

  /* connect with server */
  contck = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if(contck == ERROR) //error
    {
      fprintf(stderr, "%s\n", "Error: Cannot connect");
      return FAILURE;
    }

  /* request to server */
  /* get file name */
  wnt = malloc(len - index[4]);
  for(i = 0; i < len - index[4]; i++)
    {
      wnt[i] = URL[index[4] + i];
    }
  /* GET */
  if(GET == TRUE)
    {
      /* request */
      write(sockfd, "GET ", strlen("GET "));
      write(sockfd, wnt, len - index[4]);
      write(sockfd, " HTTP/1.0\r\n\r\n", strlen(" HTTP/1.0\r\n\r\n"));
    }
  /* POST */
  else if(POST == TRUE)
    {
      /* get size of user input */
      fseek(stdin, 0, SEEK_END);
      size = ftell(stdin);
      rewind(stdin);
      /* convert data size to string */
      sprintf(CL,"%lu", size);
      /* request */
      write(sockfd, "POST ", strlen("POST "));
      write(sockfd, wnt, len - index[4]);
      write(sockfd, " HTTP/1.0\r\nContent-Length: ", strlen(" HTTP/1.0\r\nContent-Length: "));
      write(sockfd, CL, strlen(CL));
      write(sockfd, "\r\n\r\n", strlen("\r\n\r\n"));
      data = malloc(size);
      fread(data,1,size, stdin);
      write(sockfd, data, size);
    }
  else
    assert(FAILURE); //must not approach
  
  /* respond from server */
  while(TRUE)
    {
      if( read(sockfd, respond , RPMAX - 1) > 0)
	{
	  respond[RPMAX - 1] = '\0';
	  fprintf(stdout, "%s", respond);
	}
      else break;
    }
  
  /*exit*/
  close(sockfd);
  free(wnt);
  free(data);
  return 1;
}
