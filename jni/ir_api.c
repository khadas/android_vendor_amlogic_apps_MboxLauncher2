#define LOG_NDEBUG 0
#define LOG_TAG "AmlIR"

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/Log.h>


#include "ir_api.h"

#define IRRECEIVER_IOC_SEND 0x5500
#define IRRECEIVER_IOC_RECV 0x5501
#define IRRECEIVER_IOC_STUDY_S 0x5502
#define IRRECEIVER_IOC_STUDY_E 0x5503

#define IR_DEV "/dev/irblaster0"

int OpenIrDevice(void){
    int fd = open(IR_DEV, O_RDONLY);
    if(fd < 0){
        LOGE("open ir device error\n");
    }
    return fd;
}

void CloseIrDevice(int fd)
{
    close(fd);
}

void EnterIRreceiver(int fd){
	//int fd = open(IR_DEV, O_RDONLY);
	//if(fd < 0)
	//{
	//	LOGE("open ir device error\n");
	//	return;	
	//}	
	
	ioctl(fd, IRRECEIVER_IOC_STUDY_S,NULL);
	//close(fd);
}

void ExitIRreceiver(int fd){
	//int fd = open(IR_DEV, O_RDONLY);
	//if(fd < 0)
	//{
	//	LOGE("open ir device error\n");
	//	return;	
	//}	
	
	ioctl(fd, IRRECEIVER_IOC_STUDY_E,NULL);
	//close(fd);
}

struct IrCode GetIRCode(int fd){
	int i;
	struct IrCode recv;
	memset((void*)&recv,0,sizeof(IrCode_t));
	//int fd = open(IR_DEV, O_RDONLY);
	//if(fd < 0)
	//{
	//	LOGE("open ir device error\n");
	//	return recv;	
	//}	
	
	int ret = -100;
	ret = ioctl(fd, IRRECEIVER_IOC_RECV,&recv);
	//for(i=0; i<recv.WndNum; i++)
	//{
	//	LOGV(" Get IR code,count=%d,time wnd:[%d]",recv.WndNum,recv.TimeWnd[i]);
	//}
	//close(fd);
	return recv;
}
void SendIRCode(int fd, struct IrCode IrData){
	int i;
	//for(i=0; i<IrData.WndNum; i++)
	//{
	//	LOGV(" Send IR code,order=%d,time wnd:[%d]",i,IrData.TimeWnd[i]);
	//}
	//int fd = open(IR_DEV, O_RDONLY);
	//if(fd < 0)
	//{
	//	LOGE("open ir device error\n");
	//	return;	
	//}		
	int ret = -100;
	ret = ioctl(fd, IRRECEIVER_IOC_SEND,&IrData);
	//close(fd);
	return;	
}
