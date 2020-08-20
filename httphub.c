#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
//
#include <telink_usb.h>
#include <signal.h>
//2020/8/20

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#define DEST_PORT 80
//#define DEST_IP_ADDR "10.0.9.139"
#define DEST_IP_BY_NAME "101.132.165.232"

#define debug_mode 1
#define http_recv 0
#define use_libcurl 1

int sock_fd;

#define PORT 80
#define HOST_NAME "101.132.165.232"
#define URL "http://101.132.165.232/test/receiver.php"


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
        strcat(str1, "POST http://101.132.165.232/test/receiver.php HTTP/1.1\r\n");
        strcat(str1,"Host: 101.132.165.232\r\n");
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

#if http_recv
int 
#else
void
#endif
exchange(char *url,char *host_name,char *key,unsigned char *value,int length,char *recv_buf,int recv_buf_len)
{
	int send_num;
    char str1[4096];
    char temp[1024];
    char *ptr = temp;
    unsigned short payload_length;
    char curl_send_buf[1024];
    
    
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
    //toStr(value,length,temp);
    //begin
        //timestamp
        toStr(value,8,ptr);
        ptr += strlen(temp);
        //end
        
        //payload
        payload_length = value[12];
        payload_length <<= 8;
        payload_length |= value[13];
        if(payload_length > 0)
        {
        	*ptr = ',';
        	ptr++;
        	toStr(value+14,payload_length,ptr);
		}
        //end
    //end
    sprintf(str1+strlen(str1),"Content-Length: %d\r\n",strlen(temp)+strlen(key)+1);
    //
    strcat(str1,"Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(str1,"\r\n");
    //strcat(str1,"data=0123456789");
    //
    strcat(str1,key);
    strcat(str1,"=");
    strcat(str1,temp);
    memset(curl_send_buf,0,1024);
    strcat(curl_send_buf,key);
    strcat(curl_send_buf,"=");
    strcat(curl_send_buf,temp);
    //
    strcat(str1,"\r\n\r\n");
    
    
    printf("%s\n",str1);
    #if use_libcurl
    /* get a curl handle */ 
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */ 
    curl_easy_setopt(curl, CURLOPT_URL, "http://101.132.165.232/test/receiver.php");
    /* Now specify the POST data */ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curl_send_buf);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
    #else
    l0:
    send_num = send(sock_fd, str1,strlen(str1),0);
    if (send_num < 0)
    {
        perror("send error\n");
        //
        do
        {
        sock_fd = host_connect(HOST_NAME,PORT);
        }while(sock_fd < 0);
        //
        goto l0;
    }
    else
    {

    	printf("send successful\n");
    	#if http_recv
    	printf("begin recv:\n");
    	
    	int recv_num = recv(sock_fd,recv_buf,recv_buf_len,0);
    	if(recv_num < 0){
        	perror("recv error\n");
        	return -1;
    	} else {
            printf("recv successful\n");
            return recv_num;
        }
        #endif
    }
    #endif
}

//begin
#if http_recv
int 
#else
void
#endif
exchange_str(char *url,char *host_name,char *key,char *temp,char *recv_buf,int recv_buf_len)
{
	int send_num;
    char str1[4096];
    //char temp[1024];
    
    
    printf("begin send:\n");
    memset(str1,0,4096);
    //memset(temp,0,1024);
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
    //toStr(value,length,temp);
    sprintf(str1+strlen(str1),"Content-Length: %d\r\n",strlen(temp)+strlen(key)+1);
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
    l0:
    send_num = send(sock_fd, str1,strlen(str1),0);
    if (send_num < 0)
    {
        perror("send error\n");
        //
        do
        {
        sock_fd = host_connect(HOST_NAME,PORT);
        }while(sock_fd < 0);
        //
        goto l0;
    }
    else
    {

    	printf("send successful\n");
    	#if http_recv
    	printf("begin recv:\n");
    	
    	int recv_num = recv(sock_fd,recv_buf,recv_buf_len,0);
    	if(recv_num < 0){
        	perror("recv error\n");
        	return -1;
    	} else {
            printf("recv successful\n");
            return recv_num;
        }
        #endif
    }
    
}

//end

#define    RESU_BUF_ADDR    0X800a
#define    RESU_BUF_READY   0X800c
#define    RESU_BUF_LENGTH  0X800e

unsigned char my_read_reg8(libusb_device_handle  *dev_handle,uint16_t addr)
{
	unsigned char data;
	telink_usb_r_mem(dev_handle, addr, &data, 1);
	return data;
}

unsigned short ResuBuf_GetAddr(libusb_device_handle  *dev_handle)
{
    unsigned short address;
    telink_usb_r_mem(dev_handle,RESU_BUF_ADDR , &address, 2);
    return address;
}

void ResuBuf_reset(libusb_device_handle  *dev_handle)
{
	unsigned char buf[2] = {0,0};
    telink_usb_w_mem(dev_handle, RESU_BUF_LENGTH, buf, 2);//length
    telink_usb_w_mem(dev_handle, RESU_BUF_READY, buf, 1);//ready
}

int ResuBuf_isReady(libusb_device_handle  *dev_handle)
{
    unsigned char ready = my_read_reg8(dev_handle,RESU_BUF_READY);
    return ready;
}

int Get_Device_Data(libusb_device_handle  *dev_handle, unsigned short addr,unsigned char *out,unsigned int out_buf_len)
{
	int counter = 0;
	while(1)
	{
		if(ResuBuf_isReady(dev_handle))
		{
			unsigned short length;
			telink_usb_r_mem(dev_handle,RESU_BUF_LENGTH , &length, 2);
			#if debug_mode
			printf("length = %d\n",length);
			#endif
			if(length > out_buf_len)
			{
				length = out_buf_len;
			}
			telink_usb_r_mem(dev_handle,addr , out, length);
			#if debug_mode
			{
				int i;
				for(i=0;i<length;i++)
				{
					printf("%x ",out[i]);
				}
				printf("\n");
			}
			#endif
			
			#if debug_mode
			usleep(100000);//100ms
			#endif
			ResuBuf_reset(dev_handle);
			return length;
		}else
		{
			if(counter == 10)
			{
				out[0] = 2;
				out[1] = 0x55;
				out[2] = 0xaa;
				return 3;
			}
			else
			{
				usleep(100000);//100ms
				counter++;
			}
		}
	}
}

//begin
int Get_Device_Data_new(libusb_device_handle  *dev_handle,unsigned char *out)
{
	int counter = 0;
	unsigned char cmdbuf[32]={0};
	unsigned char buff[256]={0};
	unsigned char wbuf[4]={0};
	unsigned char rbuf[4]={0};
	unsigned char  wptr,rptr,mask,size;
	unsigned long  adr;
	
	while(1)
	{
		telink_usb_r_mem(dev_handle,0x8000 , cmdbuf, 20);
		wptr = cmdbuf[5];
		rptr = cmdbuf[4];

		size = cmdbuf[6];
		mask = cmdbuf[7];
		adr = cmdbuf[0xd];
		adr = (adr << 8)|(cmdbuf[0xc]);
		if(wptr!=rptr)
		{
			adr = adr + (rptr*size);
			telink_usb_r_mem(dev_handle,adr , out, size);
			
			rptr = (rptr+1)&mask;
			wbuf[0] = rptr;
			
			telink_usb_w_mem(dev_handle,0x8004 , wbuf, 1);
			#if 0
			{
				int i;
				for(i=0;i<length;i++)
				{
					printf("%x ",out[i]);
				}
				printf("\n");
			}
			#endif
			
			return size;
		}else
		{
			if(counter == 255)
			{
				out[0] = 0;
				out[1] = 0;
				out[2] = 0;
				out[3] = 0;
				out[4] = 0;
				out[5] = 0;
				out[6] = 0;
				out[7] = 0;
				out[8] = 0;
				out[9] = 0;
				out[10] = 6;
				out[11] = 1;
				out[12] = 0;
				out[13] = 0;
				return 14;
			}
			else
			{
				//usleep(100000);//100ms
				counter++;
			}
		}
	}
}

//end

void handle_pipe(int sig)
{
	printf("receive sigpipe signal\n");
}

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
    
    char recv_buf[4096];
    int recv_num;
    unsigned char value[6] = {0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    libusb_device_handle  *dev_handle;
    unsigned short data_addr;
    unsigned short data_length;
    unsigned char data_buf[4096];
    struct sigaction action;
    CURL *curl;
  CURLcode res;
    
    action.sa_handler = handle_pipe;
sigemptyset(&action.sa_mask);
action.sa_flags = 0;
sigaction(SIGPIPE, &action, NULL);

curl_global_init(CURL_GLOBAL_ALL);
    
    dev_handle = telink_usb_open(0x248a, 0x5320);
	if(dev_handle == NULL)
	{
		printf("open the usb dev failed!\n");
		exit(1);
	}
	
	data_addr = ResuBuf_GetAddr(dev_handle);
	
	#if debug_mode
	printf("data_addr = %x\n",data_addr);
	#endif
    
    sock_fd = host_connect(HOST_NAME,PORT);
    if(sock_fd < 0)
    {
    	exit(1);
	}
	
	while(1)
	{
		//add code
		#if 0
		data_length = Get_Device_Data(dev_handle, data_addr,data_buf,4096);
		if(data_length > 1)
		{
			unsigned char data_type;
			data_type = data_buf[0];
			if(data_type == 0)//string
			{
				//add code
				data_buf[data_length] = 0;
				#if debug_mode
				{
					int i;
					for(i=0;i<data_length+1;i++)
					{
						printf("%x ",data_buf[i]);
					}
					printf("\n");
				}
				#endif
				recv_num = exchange_str(URL,HOST_NAME,"thread_log",data_buf+1,recv_buf,4096);
			}
			else if(data_type == 1)//bytes
			{
				recv_num = exchange(URL,HOST_NAME,"data",data_buf+1,data_length-1,recv_buf,4096);
			}
			else if(data_type == 2)//keep alive
			{
				recv_num = exchange(URL,HOST_NAME,"keep_alive",data_buf+1,data_length-1,recv_buf,4096);
			}
    		
    		if(recv_num < 0)
    		{
    			//exit(1);
			}else
			{
				printf("data recv:%s\n",recv_buf);
				//return 0;
			}
		}
		#else
		data_length = Get_Device_Data_new(dev_handle,data_buf);
		if(data_length >= 14)
		{
			unsigned char proto_type;
			unsigned char data_type;
			proto_type = data_buf[10];
			data_type = data_buf[11];
			
			if(proto_type == 1)//zigbee
			{
				if(data_type == 1)//stream
				{
					#if http_recv
					recv_num = 
					#endif
					exchange(URL,HOST_NAME,"zigbee_data",data_buf,data_length,recv_buf,4096);
				}else//string
				{
					
				}
			}else if(proto_type == 6)//test
			{
				#if http_recv
				recv_num = 
				#endif
				exchange(URL,HOST_NAME,"test",data_buf,data_length,recv_buf,4096);
			}else
			{
				
			}
			
			#if http_recv
			if(recv_num < 0)
    		{
    			//exit(1);
			}else
			{
				printf("data recv:%s\n",recv_buf);
				//return 0;
			}
			#endif
		}
		
		
		#endif
	}
    #endif
    
    curl_global_cleanup();
}

//add this line to test the fork function of the github.
//add this line to test the fork function of the github. -- tianyiyang@aliyun.com
