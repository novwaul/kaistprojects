#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define METHOD_SIZE 4
#define VERSION_SIZE 10
#define PORT_SIZE 6

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *user_error = "Proxy does not support the requested command\r\n";
static const char *default_port = "80";  

int doit(int fd);
void * proxy_thread(void * argp);

int main(int argc, char ** argv)
{
	int listenfd, * connfd;
	socklen_t clientlen;
    	struct sockaddr_storage clientaddr;
	pthread_t tid;

    	/* Check command line args */
    	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
    
	}
    	listenfd = Open_listenfd(argv[1]);
    	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = malloc(sizeof(int));
		*connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		Pthread_create(&tid, NULL, proxy_thread, connfd);                                            
    	}
    	return 0; // cannot reach
}

void * proxy_thread(void * argp)
{
	int connfd = * (int *)argp;
	Pthread_detach(Pthread_self());
	free(argp);
	doit(connfd);
	Close(connfd);
	return NULL; 
}

int doit(int fd)
{
    	char temp[MAXLINE], buf[MAXLINE], send_buf[MAXLINE], method[METHOD_SIZE], hostname[MAXLINE], path[MAXLINE], version[VERSION_SIZE], port[PORT_SIZE], * buf_probe, * temp_probe; 
	int serverfd, error = 0, host = 0, user_agent = 0, size, temp_size;
    	rio_t rio, rio_serv;

	/* Read request line and headers */
    	Rio_readinitb(&rio, fd);
    	if (!Rio_readlineb(&rio, buf, MAXLINE)) 
		return 0; // EOF 
	/* find GET */
	buf_probe = buf;
	strncpy(method, buf_probe, METHOD_SIZE);
	if(method[METHOD_SIZE - 1] == ' ' && !strncmp(method, "GET", strlen("GET")))
	{
		method[METHOD_SIZE - 1] = '\0';
		buf_probe += METHOD_SIZE;
	}	
    	else error = 1;  // error occur
	/* find HOST & PORT */
	if( !error && !strncmp(buf_probe, "http://", strlen("http://")))
	{
		buf_probe += strlen("http://");
		temp_probe = buf_probe;
		while(*temp_probe != '/' && *temp_probe != ':')
		{
			temp_probe++;
		}
		strncpy(hostname, buf_probe, temp_probe - buf_probe);
		hostname[temp_probe - buf_probe] = '\0';
		buf_probe = temp_probe;
		if(*temp_probe == '/') // use default port
		{
			strncpy(port, default_port, strlen(default_port) + 1);
		}			
		else // use user-defined port
		{ 
			while(*temp_probe != '/')
				temp_probe++;
			buf_probe++;
			strncpy(port, buf_probe, temp_probe - buf_probe);
			port[temp_probe - buf_probe] = '\0';
			buf_probe = temp_probe;
		}
	}
	else error = 1;
	/* find PATH and VERSION */
	if(!error)
	{
		/* find PATH */
		temp_probe = buf_probe;
		while(*temp_probe != ' ') 
			temp_probe++;
		strncpy(path, buf_probe, temp_probe - buf_probe);
		path[temp_probe - buf_probe] = '\0';
		buf_probe = temp_probe + 1;
		/* find VERSION */
		strncpy(version, buf_probe, VERSION_SIZE);
		if((!strncmp(version,"HTTP/1.0",strlen("HTTP/1.0")) || !strncmp(version,"HTTP/1.1",strlen("HTTP/1.1")))
			&& version[VERSION_SIZE - 2] == '\r' && version[VERSION_SIZE - 1] == '\n')
		{
			version[VERSION_SIZE - 3] = '0'; // convert to HTTP/1.0
			version[VERSION_SIZE - 2] = '\0';
		}
		else error = 1;
	}
	/* error handling */
	if(error)
	{
		Rio_writen(fd, (void * )user_error, strlen(user_error));
		return 0;
	}
	/* open server socket */
	serverfd = Open_clientfd(hostname, port);
	Rio_readinitb(&rio_serv, serverfd);	
	/* send request message to server*/
	sprintf(send_buf, "%s %s %s\r\n", method, path, version);
	Rio_writen(serverfd, send_buf, strlen(send_buf));
	if((size = Rio_readlineb(&rio, buf, MAXLINE)) == 0)
	{
		Close(serverfd);
		return 0; // error
	}
	else if(strncmp(buf, "User-Agent:", strlen("User-Agent:")))
	{
		sprintf(send_buf, "%s", user_agent_hdr);
		Rio_writen(serverfd, send_buf, strlen(send_buf));
	}
	else 
	{
		user_agent = 1;
		Rio_writen(serverfd, buf, size);
	}
	if((temp_size = Rio_readlineb(&rio, temp, MAXLINE)) == 0)
	{
		Close(serverfd);
		return 0; // error
	}
	else if(strncmp(buf, "Host:", strlen("Host:")))
	{
		Rio_writen(serverfd, send_buf, strlen(send_buf));
	}
	else 
	{
		host = 1;
		Rio_writen(serverfd, temp, temp_size);
	}
	if(!user_agent) 
	{
		Rio_writen(serverfd, buf, size);
	}
	if(!host) 
	{
		Rio_writen(serverfd, temp, temp_size);
	}
	while((size = Rio_readlineb(&rio, buf, MAXLINE)))
	{
		if(!strncmp(buf, "Connection:", strlen("Connectoin")) || !strncmp(buf, "Proxy-Connection:", strlen("Proxy-Connection"))) 
			continue; 
		if(!strncmp(buf, "\r\n", strlen("\r\n")))
		Rio_writen(serverfd, buf, size);
		if(!strncmp(buf, "\r\n", strlen("\r\n")))
			break;
	}
	sprintf(send_buf, "Connection: close\r\nProxy-Connection: close\r\n\r\n");
	Rio_writen(serverfd, send_buf, strlen(send_buf));
	/* send request result to client */
	int count = 0, ref = -1;
	while((size = Rio_readlineb(&rio_serv, (void*)buf, MAXLINE)) != 0)
	{
		Rio_writen(fd, buf, size);
		if(!strncmp(buf,"Content-length: ",strlen("Content-length ")))
			ref = atoi(buf + strlen("Content-length"));
		if(!strncmp(buf, "\r\n", strlen("\r\n"))) break;	
	}
	while((size = Rio_readnb(&rio_serv, buf, MAXLINE)) != 0)
	{
		count += size;
		Rio_writen(fd, buf, size);
		if(count == ref) break;
	}
	Close(serverfd);
	return 1;       
}

