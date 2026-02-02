#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

#include"coap.h"

#define SERVER_PORT 5683
coap_header server_packet,response_pack;

int server_fd;
int temp_req=0,hum_req=0,total_req=0;
struct sockaddr_in server_add,client_add;
socklen_t addr_len=sizeof(client_add);
int server_running=0;
pthread_t server_thread,sensor_thread;
pthread_mutex_t mutex;
int ch;

void display()
{
	system("clear");
	printf("-----SERVER MENU-----\n");
	printf("SERVER_STATUS:%s\n\n",server_running ? "ON":"OFF");
	printf("temp req:%d\n",temp_req);
	printf("hum req:%d\n",hum_req);
	printf("total req:%d\n",total_req);
	printf("1.START communication\n");
	printf("2.STOP communication\n");
	printf("3.EXIT\n");
	printf("Enter the your choice:\n");
	scanf("%d",&ch);
}

void printing_packet()
{
	printf("VERSION:%d\n",response_pack.version);
	printf("TYPE:%d\n",response_pack.type);
	printf("CODE:%x\n",response_pack.code);
	printf("MSG_ID:%d\n",response_pack.msg_id);
}


void *communication_thread(void *arg)
{
	printf("server communiuncation thread is started\n");
	while(1)
	{
		if(!server_running)
			break;

		int len=recvfrom(server_fd,&server_packet,sizeof(server_packet),0,(struct sockaddr*)&client_add,&addr_len);
		if(len<0)
			continue;

		if(server_packet.version !=COAP_VERSION)
		{
			printf("Invalid version\n");
			exit(0);
		}
		if(server_packet.type !=COAP_CON)
		{
			response_pack.type=COAP_RST;
		}
		else
		{
			response_pack.type=COAP_ACK;
		}
		if(server_packet.code !=COAP_GET) 
		{
			response_pack.code=COAP_FORBIDDEN;
			response_pack.payload_marker=0;
		}
		else
		{
			response_pack.code=COAP_CONTENT;
			response_pack.payload_marker=COAP_PAYLOAD_MARKER;
		}
		if(server_packet.token_len<0 || server_packet.token_len>8)
		{
			response_pack.code=COAP_BAD_REQUEST;
			response_pack.payload_marker=0;
		}
		int a=strcmp(server_packet.option[0].option_value,"temp");
		int b=strcmp(server_packet.option[0].option_value,"Humidity");
		int c=strcmp(server_packet.option[1].option_value,"Humidity");
		if(a == 0 && c!=0)
		{
			response_pack.token_len=4;
			printf("sending random Temperature...\n");
			fun();
			response_pack.payload.payload=(COAP_PAYLOAD_MARKER<<24)|(temp_int<<8);
			printf("Temp:%d\n",temp_int);
			temp_req++;
			total_req++;
			printf("temp req:%d\n",temp_req);
			printf("hum req:%d\n",hum_req);
			printf("total req:%d\n",total_req);
	
		}
		else if(b == 0 && c!=1)
		{
			response_pack.token_len=8;
			printf("sending random Humidity...\n");
			fun();
			printf("Hum:%d\n",hum_int);
			response_pack.payload.payload=(COAP_PAYLOAD_MARKER<<24)|(hum_int<<16);

			hum_req++;
			total_req++;
			printf("temp req:%d\n",temp_req);
			printf("hum req:%d\n",hum_req);
			printf("total req:%d\n",total_req);
		}
		else if(a==0 && c==0)
		{
			printf("sending the both Temperature and Humidity\n");
			
			fun();
			printf("Temp:%d\n",temp_int);
			printf("Hum:%d\n",hum_int);
			response_pack.payload.payload=(COAP_PAYLOAD_MARKER<<24)|(temp_int<<16)|(temp_dec<<8)|(hum_int);

			temp_req++;
			hum_req++;
			total_req++;
			printf("temp req:%d\n",temp_req);
			printf("hum req:%d\n",hum_req);
			printf("total req:%d\n",total_req);
		}
		else
		{
			response_pack.code=COAP_UNAVAILABLE;
			response_pack.payload_marker=0;
		}
		response_pack.option[0].option_delta=server_packet.option[0].option_delta;
		response_pack.option[1].option_delta=server_packet.option[1].option_delta;

		response_pack.option[0].opt_len=server_packet.option[0].opt_len;
		response_pack.option[1].opt_len=server_packet.option[1].opt_len;

		stpcpy(response_pack.option[0].option_value,server_packet.option[0].option_value);
		stpcpy(response_pack.option[1].option_value,server_packet.option[1].option_value);


		response_pack.token = server_packet.token;
		response_pack.msg_id = server_packet.msg_id;
		response_pack.version  = server_packet.version;
		printing_packet();
		sendto(server_fd,&response_pack,sizeof(response_pack),0,(struct sockaddr*)&client_add,addr_len);
	}
	close(server_fd);
	printf("server thread is stoped\n");
	pthread_exit(NULL);
}
void *sensor(void *arg)
{
	pthread_mutex_lock(&mutex);
	fun();
	pthread_mutex_unlock(&mutex);
}

		
int main()
{

	struct sockaddr_in server_add,client_add;
	socklen_t addr_len=sizeof(client_add);
	if((server_fd=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&server_add,0,sizeof(server_add));
	server_add.sin_family=AF_INET;
	server_add.sin_port=htons(SERVER_PORT);
	server_add.sin_addr.s_addr=inet_addr("0.0.0.0");

	if(bind(server_fd,(struct sockaddr *)&server_add,sizeof(server_add))<0)
	{
		perror("bind creation failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	pthread_create(&sensor_thread,NULL,sensor,NULL);
	while(1)
	{
		display();
		if(ch == 1 && !server_running)
		{
			server_running =1;
			pthread_create(&server_thread,NULL,communication_thread,NULL);
			printf("server started\n");
		}
		else if(ch == 2 && server_running)
		{
			server_running=0;
			pthread_cancel(server_thread);
			pthread_cancel(sensor_thread);
			printf("server stoped\n");
		}
		else if(ch == 3)
		{
			close(server_fd);
			exit(0);
			return 0;
		}
		else
		{
			printf("Invalied choice\n");
		}	
	}
}
