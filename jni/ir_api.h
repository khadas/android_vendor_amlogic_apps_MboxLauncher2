#ifndef _IR_API_H
#define _IR_API_H

//by peter,2011,0728

#define TIME_WND_MAX 1024
#ifndef LOGD
    #define LOGV ALOGV
    #define LOGD ALOGD
    #define LOGI ALOGI
    #define LOGW ALOGW
    #define LOGE ALOGE
#endif
typedef struct IrCode {
    unsigned int WndNum;
    unsigned int TimeWnd[TIME_WND_MAX];
} IrCode_t;

#ifdef  __cplusplus
extern "C" {
#endif

int OpenIrDevice(void);
void CloseIrDevice(int fd);
void EnterIRreceiver(int fd);
void ExitIRreceiver(int fd);
struct IrCode GetIRCode(int fd); /*如果返回值的WndNum==0，说明没有得到红外键，可以delay100ms，再读*/
void SendIRCode(int fd, struct IrCode IrData);

#ifdef  __cplusplus
}
#endif
#endif
