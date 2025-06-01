#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/select.h>
#include<sys/time.h>
#include<errno.h>

int main(int argc, char **argv){
	int f ;
	int wt, rd;
	char writeData[100] = "Hello from user space!";
	char readData[100];
	f = open("/dev/device_lab4_1", O_RDWR);
	if(f < 0) {
		printf("[Lab4_1] Can not open the device file\n");
		exit(1);
	}
  
  printf("[Lab4_1] Open the device file success\n");

	wt = write(f, &writeData, sizeof(writeData));
	printf("[Lab4_1] Result of write = %d\n", wt);
	rd = read(f, &readData, sizeof(readData));
	printf("[Lab4_1] Result of read = %d\n", rd);
	printf("[Lab4_1] Read data from device: %s\n", readData);
	close(f);
}