#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#define DEST_PORT 80
//#define DEST_IP_ADDR "10.0.9.139"
#define DEST_IP_BY_NAME "zongxiaodong.cn"


void process_info(int fd)
{
    int send_num;
    char send_buf [] = "helloworld";
    char recv_buf [4096];
    char str1[4096];
    while (1)
    {
        printf("begin send\n");
        memset(str1,0,4096);
        strcat(str1, "POST http://www.zongxiaodong.cn/receiver.php HTTP/1.1\r\n");
        strcat(str1,"Host: zongxiaodong.cn\r\n");
        strcat(str1,"Content-Length: 15\r\n");
        strcat(str1,"Content-Type: application/x-www-form-urlencoded\r\n");
        strcat(str1,"\r\n");
        strcat(str1,"data=0123456789");
        strcat(str1,"\r\n\r\n");
        printf("str1 = %s\n",str1);
        send_num = send(fd, str1,strlen(str1),0);
        if (send_num < 0)
        {
            perror("send error");
            exit(1);
        }
        else
        {

        printf("send successful\n");
        printf("begin recv:\n");
        int recv_num = recv(fd,recv_buf,sizeof(recv_buf),0);
        if(recv_num < 0){
            perror("recv");
            exit(1);
        } else {
                printf("recv sucess:%s\n",recv_buf);
                }
         }
        break;
        sleep(5);
    }
}


int host_connect(char *host_name,unsigned short port)
{
	int sock_fd;
    struct sockaddr_in addr_serv;
    
    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock_fd < 0)
    {
        perror("sock error\n");
        return -1;
    }
    else
    {
        printf("sock successful\n");      
    }

    struct hostent* hostInfo = gethostbyname(host_name);
    
    if(NULL == hostInfo){
        perror("hostInfo is null\n");
        return -6;
    }else
    {
    	printf("get host info successful\n");
	}

    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(port);
    //addr_serv.sin_addr.s_addr = inet_addr(DEST_IP_ADDR);

    printf("host ip address = %s \n",inet_ntoa(*((struct in_addr*)hostInfo->h_addr)));
    memcpy(&addr_serv.sin_addr, &(*hostInfo->h_addr_list[0]), hostInfo->h_length);

    if (connect(sock_fd, (struct sockaddr*)(&addr_serv), sizeof(addr_serv)) < 0)
    {
        perror("connect error\n");
        return -1;
    }
    else
    {
        printf("connect successful\n");
    }
    
    return sock_fd;
}

void toStr(unsigned char *src,unsigned int len,char *dest)
{
    int i;
    char *p = dest;
    unsigned char temp;
    
    if((len == 0)||(src == NULL)||(dest == NULL))
    {
        return;
    }
    
    for(i=0;i<len;i++)
    {
        temp = src[i]>>4;
        if(temp<10)
        {
            *p = temp + 0x30;
            p++;
        }else{
            *p = temp + 0x37;
            p++;
        }

        temp = src[i]&0x0f;
        if(temp<10)
        {
            *p = temp + 0x30;
            p++;
        }else{
            *p = temp + 0x37;
            p++;
        }

        *p = ' ';
        p++;
    }
    p--;
    *p = 0;
}

int exchange(int fd,char *url,char *host_name,char *key,unsigned char *value,int length,char *recv_buf,int recv_buf_len)
{
	int send_num;
    char str1[4096];
    char temp[1024];
    
    
    printf("begin send:\n");
    memset(str1,0,4096);
    memset(temp,0,1024);
    //strcat(str1, "POST http://www.zongxiaodong.cn/receiver.php HTTP/1.1\r\n");
    //
    strcat(str1, "POST ");
    strcat(str1, url);
    strcat(str1, " HTTP/1.1\r\n");
    //
    //strcat(str1,"Host: zongxiaodong.cn\r\n");
    //
    strcat(str1,"Host: ");
    strcat(str1,host_name);
    strcat(str1,"\r\n");
    //
    //strcat(str1,"Content-Length: 15\r\n");
    //
    toStr(value,length,temp);
    sprintf(str1+strlen(str1),"Content-Length: %d\r\n",strlen(temp));
    //
    strcat(str1,"Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(str1,"\r\n");
    //strcat(str1,"data=0123456789");
    //
    strcat(str1,key);
    strcat(str1,"=");
    strcat(str1,temp);
    //
    strcat(str1,"\r\n\r\n");
    
    
    printf("%s\n",str1);
    
    send_num = send(fd, str1,strlen(str1),0);
    if (send_num < 0)
    {
        perror("send error\n");
        return -1;
    }
    else
    {

    	printf("send successful\n");
    	printf("begin recv:\n");
    	
    	int recv_num = recv(fd,recv_buf,recv_buf_len,0);
    	if(recv_num < 0){
        	perror("recv error\n");
        	return -1;
    	} else {
            printf("recv successful\n");
            return recv_num;
        }
    }
    
}

#define PORT 80
#define HOST_NAME "zongxiaodong.cn"
#define URL "http://www.zongxiaodong.cn/receiver.php"

int main()
{
	#if 0
    int sock_fd;
    struct sockaddr_in addr_serv;
    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("sock error\n");
        exit(1);
    }
    else
    {
        printf("sock successful\n");      
    }

    struct hostent* hostInfo = gethostbyname(DEST_IP_BY_NAME);
    if(NULL == hostInfo){
        printf("hostInfo is null\n");
        return -6;
    }

    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(DEST_PORT);
    //addr_serv.sin_addr.s_addr = inet_addr(DEST_IP_ADDR);

     printf("Ip address = %s \n",inet_ntoa(*((struct in_addr*)hostInfo->h_addr)));
    memcpy(&addr_serv.sin_addr, &(*hostInfo->h_addr_list[0]), hostInfo->h_length);

    if (connect(sock_fd, (struct sockaddr*)(&addr_serv), sizeof(addr_serv)) < 0)
    {
        perror("connect error\n");
        exit(1);
    }
    else
    {
        printf("connect successful\n");
    }
    process_info(sock_fd);
    #else
    int sock_fd;
    char recv_buf[4096];
    int recv_num;
    unsigned char value[6] = {0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    
    sock_fd = host_connect(HOST_NAME,PORT);
    if(sock_fd < 0)
    {
    	exit(1);
	}
    recv_num = exchange(sock_fd,URL,HOST_NAME,"data",value,6,recv_buf,4096);
    if(recv_num < 0)
    {
    	exit(1);
	}else
	{
		printf("data recv:%s\n",recv_buf);
		return 0;
	}
    #endif
}

