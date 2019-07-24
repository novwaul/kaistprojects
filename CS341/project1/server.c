/* 20160788   */
/* InJe Hwang */
/* server.c   */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/select.h>
#include<sys/types.h>
#include<assert.h>
#include<sys/time.h>
#include<fcntl.h>

#define ARGNUM 3
#define FDMAX 3 // max fd num sotred
#define FAILURE 0
#define pPORTMAX 8 // max port digit
#define ERROR -1
#define TRUE 1
#define BUFMAX 2000 // Max buffer length > 1GB
#define CLMAX 12 // Max Content-Length
#define FNMAX 20 // Max File Name Length

/* Check request message format. Return ERROR(-1) if format is wrong. Return TRUE if format is correct. 
   Give request message, Content-Length, File Name start and end index if needed*/
int formchk(int fd, char *buf, int* carry) //carry : see "respond" function
{
  int i,len, prev, base, GET, POST,
      CL = 0 // for check Content-Length
      ;
  unsigned long size;
  char *temp;
  
  read(fd, buf, BUFMAX);
  /* check method */
  carry[5] = carry[6] = GET = POST = 0;
  if((buf[0] == 'G')&&(buf[1] == 'E')&&(buf[2] == 'T')) carry[5] = GET = 1;
  else if((buf[0] == 'P')&&(buf[1] == 'O')&&(buf[2] == 'S')&&(buf[3] == 'T')) carry[6] = POST = 1;
  else return ERROR;
  /* check request line format */
  if(GET == 1) base = 3;
  else base = 4;
  if(buf[base] != ' ') return ERROR; // check sp
  base++;
  if(buf[base] != '/') return ERROR; // check slash
  base++;
  carry[2] = base; //mark start of file name
  while(!isspace(buf[base])) base++;
  if(buf[base] != ' ') return ERROR; // check sp
  carry[3] = base - 1; //mark end of file name
  base++;
  if((buf[base++] != 'H')||(buf[base++] != 'T')||(buf[base++] != 'T')||(buf[base++] != 'P')|| //check HTTP/1.0\r\n
     (buf[base++] != '/')||(buf[base++] != '1')||(buf[base++] != '.')||(buf[base++] != '0')||
     (buf[base++] != '\r')||(buf[base++] != '\n'))
     return ERROR;
  if((GET == 1)&&(buf[base] == '\r')&&(buf[base + 1] == '\n')) return TRUE; // end without header & entity
  /* check header line */
  len = strlen("Connect-Length:");
  while(TRUE)
    {
      prev = base;
      while(!isspace(buf[base])) base++;
      if(buf[base] != ' ') return ERROR;
      if(buf[base - 1] != ':') return ERROR;
      if(base - prev == len) // check Content-Length
	{
	  temp = malloc(len+1);
	  for(i = 0; i < len ; i++)
	      temp[i] = buf[prev + i];
	  temp[len] = '\0';
	  if(strcmp(temp, "Content-Length:") == 0) //detect Content-Length
	      CL++;
	  free(temp);
	}
      base++;
      prev = base;
      while(!isspace(buf[base])) base++;
      if(buf[base] != '\r') return ERROR;
      if(CL != 0)
	{
	  carry[0] = prev; //start of content length 
	  carry[1] = base - 1; // end of content length
	}
      base++;
      if(buf[base] != '\n') return ERROR;
      if((buf[base + 1] == '\r')&&(buf[base + 2] == '\n'))
	{
	  carry[4] = base + 3; //start of entity body
	  break;
	}
      else base++;
    }
  if((POST == 1)&&(CL != 1)) return ERROR;//POST must have Content-Length

  return TRUE;
}
/* Respond to client. If request format is wrong, then send error message. If requested file is not found, then send error message.
   If everything is correct, then send confirm message, and do the things that are required by clients. */
int respond(int clfd, int sockfd ,fd_set *fdset_read,fd_set *fdset_temp,  struct timeval* time)
{
  int j;
  char buf[BUFMAX], *fn, *data, *cl;
  int carry[7]; //0: start position of content length num 1: end position of the num
                //2,3: start/end position of file name
                //4: start position of entity body 5: GET status 6: POST status
  int cls, cle, fns, fne, ebs, GET, POST;
  int fd;
  unsigned long size, nsize;
  FILE *fin, *fout;

	  if(formchk(clfd, buf, carry) == TRUE)
	    {
	      /* rename parameters */
	      cls = carry[0]; cle = carry[1]; fns = carry[2]; fne = carry[3]; ebs = carry[4]; GET = carry[5]; POST = carry[6];
	      /* get file name */
	      fn = malloc(fne - fns + 2);
	      for( j = 0 ; j < fne -fns + 1; j++)
		fn[j] = buf[fns + j];
	      fn[fne - fns + 1] = '\0';
	      /* GET method */
	      if(GET == 1)
		{
		  if((fin = fopen(fn, "r")) == NULL)
		    {//Not found	
	              write(clfd, "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
		            strlen("HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"));
	              close(clfd);
		      free(fn);
		      return ERROR;
		    }
		  else//send requested file
		    {
		      /* get data size */
		      fseek(fin, 0, SEEK_END);
		      size = ftell(fin);
		      rewind(fin);
		      /* convert data size to string */
		      cl =  malloc(CLMAX);
		      sprintf(cl,"%lu", size);
		      /* respond */
		      write(clfd, "HTTP/1.0 200 OK\r\nContent-Length: ", strlen("HTTP/1.0 200 OK\r\nContent-Length: "));
		      write(clfd, cl, strlen(cl));
		      write(clfd, "\r\nConnection: close\r\n\r\n", strlen("\r\nConnection: close\r\n\r\n"));
		      data = malloc(1);
		      for( j = 0; 1*j < size ; j++)
			{
			  
			  fread(data,1,1,fin);
			  write(clfd, data, 1);
			  /* for non blocking */
			  FD_ZERO(fdset_temp);
			  *fdset_temp = *fdset_read;
			  select(FD_SETSIZE, fdset_temp, NULL, NULL, time);
			  if(FD_ISSET(sockfd, fdset_temp))
			    {
			      fd = accept(sockfd, NULL, NULL);
			      fcntl(fd, F_SETFL, O_NONBLOCK);
			      FD_SET(fd, fdset_read);
			    }
			  else continue;
			}
		      fclose(fin);
		      /* terminate socket */
		      free(cl);
		      free(fn);
		      free(data);
		      close(clfd);
		      return TRUE;
		    }
		}
	      /* POST method */
	      else if(POST == 1)
		{
		  /* get data size */
		  cl = malloc(cle - cls + 2);
		  for( j = 0 ; j < cle - cls + 1; j++)
		      cl[j] = buf[cls + j];
		  cl[cle - cls + 1] = '\0';
		  size = atoi(cl);
		  free(cl);
		  
		  /* get data & create file */
		  fout = fopen(fn ,"w");		  
		  if( BUFMAX - ebs < size ) //for big size file
		    {
		      /* send first arrived data */
		      fwrite(buf + ebs, 1 , BUFMAX - ebs ,fout);
		      /* send last data */
		      nsize = size - BUFMAX + ebs;
		      data = malloc(1);
		      for(j = 0; 1*j < nsize   ; j++)
			{
			  read(clfd, data, 1);
			  fwrite(data,1, 1, fout);
			  /* for non blocking */
			  FD_ZERO(fdset_temp);
			  *fdset_temp = *fdset_read;
			  select(FD_SETSIZE, fdset_temp, NULL, NULL, time);
			  if(FD_ISSET(sockfd, fdset_temp))
			    {
			      fd = accept(sockfd, NULL, NULL);
			      fcntl(fd, F_SETFL, O_NONBLOCK);
			      FD_SET(fd, fdset_read);
			    }
			  else continue;			  
			}
		    }
		  else // for small size file
		    {
		      fwrite(buf + ebs,1,size,fout);
		    }
		  fclose(fout);
		  /* respond */
		  write(clfd, "HTTP/1.0 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
			strlen("HTTP/1.0 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"));
		  /*terminate socket */
		  if( BUFMAX - ebs < size )
		    free(data);
		  free(fn);
		  close(clfd);
		  return TRUE;
		}
	      /* error */
	      else assert(FAILURE); //cannot reach
	    }
	  /*error*/
	  else
	    {
	      write(clfd, "HTTP/1.0 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
		    strlen("HTTP/1.0 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"));
	      close(clfd);
	      return ERROR;
	    }
}

int main(int argc, char** argv)
{
  char *COMMAND, pPORT[pPORTMAX];
  int PORT, len, i, sockfd, c, clfd;
  struct sockaddr_in myaddr, caddr[FDMAX];
  struct timeval time;
  fd_set fdset_read, fdset_temp;
  
  /* check argument number */
  if(argc != ARGNUM)
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument number");
      return FAILURE;
    }
  /* check argument value */
  COMMAND = argv[1];
  if(strcmp(COMMAND, "-p") != 0)//error
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument value");
      return FAILURE;
    }

  /* check PORT number format */
  strcpy(pPORT, argv[2]);
  len = strlen(pPORT);
  if(len <= 2) //error
    {
      fprintf(stderr, "%s\n", "Error: Invalid argument value");
      return FAILURE;
    }
  for(i = 0; i  < len; i++)
    {
      c = pPORT[i];
      if((i == 0)&&(c != '[' ))//error
	{
	  fprintf(stderr, "%s\n", "Error: Invalid argument value");
	  return FAILURE;
	}
      else if((i == len - 1)&&(c != ']'))//error
	{
	  fprintf(stderr, "%s\n", "Error: Invalid argument value");
	  return FAILURE;
	}
      else if((i != len -1)&&(i != 0)&&(!isdigit(c)))//error
	{
	  fprintf(stderr, "%s\n", "Error: Invalid argument value");
	  return FAILURE;
	}
      else continue;
    }
  
  /* get PORT number */
  for(i = 0; i < len -1 ; i++)
    {
      pPORT[i] = pPORT[i+1];//shift to left
    }
  PORT = atoi(pPORT);//get PORT

  /* open socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == ERROR)
    {
      fprintf(stderr, "%s\n", "Error: Cannot open socket");
      return FAILURE;
    }
  /* non blocking */
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  /* bind */
  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons(PORT);
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr)) == -1)
    { //error
      fprintf(stderr, "%s\n", "Error: Cannot bind socket");
      return FAILURE;
    }
  else;

  /* listen */
  FD_ZERO(&fdset_read);//clear
  FD_SET(sockfd, &fdset_read);
  if(listen(sockfd, 3) == ERROR)
    {
      fprintf(stderr, "%s\n", "Error: Cannot listen connection");
      return FAILURE;
    }

  /* do connection */
  time.tv_sec = time.tv_usec = 0;
  while(TRUE)
    {
      FD_ZERO(&fdset_temp); //clear
      fdset_temp = fdset_read;
      if(select(FD_SETSIZE, &fdset_temp, NULL, NULL, &time) == ERROR)//error
	{
	  fprintf(stderr, "%s\n", "Error: Cannot select connection");
	  return FAILURE;
	}
      for( i = 0; i < FD_SETSIZE; i++)
	{
	if(FD_ISSET(i, &fdset_temp)) // detect change
	  {
	    /* new connection */
	    if(sockfd == i)
	      {
		clfd = accept(sockfd, NULL, NULL);
		fcntl(clfd, F_SETFL, O_NONBLOCK);
		FD_SET(clfd, &fdset_read);
	      }
	    /* do respond */
	    else
	      {
		respond(i ,sockfd, &fdset_read, &fdset_temp, &time);
		FD_CLR(i,&fdset_read);
	      }
	  }
	}
    }
}
