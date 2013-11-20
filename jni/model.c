#define LOG_NDEBUG 0
#define LOG_TAG "SmartRemoteJNI"

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <jni.h>
#include <fcntl.h>
#include "ir_api.h"

extern int register_com_amlogic_smartremote_IRHandler(JNIEnv *env);

static IrCode_t gCode ;
static int gCode_ready = 0;
static int gDo_receive = 0;

static void log_code(IrCode_t *code)
{
    int i = 0 ,j = 0;
    char log[1000]={0} , buff[10]={0};
    LOGD("\n--------Log-IrCode-Start--------");
    LOGD("\nWndNum :%d" , code->WndNum);
    LOGD("\nTimeWnd:");
    for(i=0;i<code->WndNum;i++) {
        memset(buff,0,sizeof(buff));
        sprintf(buff,"%d ",code->TimeWnd[i]);
        strcat(log,buff);
        
        if(++j > 10 || i==(code->WndNum-1)) {
            LOGV("%s",log);
            memset(log,0,sizeof(log));
            j=0;
        }
    }

    LOGD("\n--------Log-IrCode-End--------");
}

static int save_key_file(const char *file , IrCode_t *code)
{
    int fd = -1 , ret = 0;
    LOGV("save file:%s",file);

    umask(0);
    fd = open(file , O_RDWR | O_TRUNC | O_CREAT , 0644);
    if(fd < 0) {
        LOGE("open save file failed:%s",strerror(errno));
        return -1;
    }

    ret = write(fd , &(code->WndNum) , sizeof(unsigned int));
    if(ret != sizeof(sizeof(unsigned int))) {
        LOGE("write num failed:%s",strerror(errno));
        close(fd);
        return -2;
    }

    ret = write(fd , code->TimeWnd , sizeof(unsigned int)*TIME_WND_MAX);
    if(ret != (sizeof(unsigned int)*TIME_WND_MAX )) {
        LOGE("write code failed:%s",strerror(errno));
        close(fd);
        return -3;
    }

    close(fd);
    return 0;
}

static int read_key_file(const char *file , IrCode_t *code)
{
    int fd = -1 , ret = 0;
    unsigned int num ;
    unsigned int data[TIME_WND_MAX] = {0};
    LOGV("read file:%s",file);

    fd = open(file , O_RDWR);
    if(fd < 0) {
        LOGW("open file failed:%s",strerror(errno));
        return -1;
    }

    ret = read(fd , &(code->WndNum) , sizeof(unsigned int));
    if(ret != sizeof(sizeof(unsigned int))) {
        LOGE("read num failed:%s",strerror(errno));
        close(fd);
        return -2;
    }

    ret = read(fd ,code->TimeWnd , sizeof(unsigned int)*TIME_WND_MAX);
    if(ret != (sizeof(unsigned int)*TIME_WND_MAX )) {
        LOGE("read data failed:%s",strerror(errno));
        close(fd);
        return -3;
    }

    //code->WndNum = num;
    //memcpy(code->TimeWnd,data,sizeof(unsigned int)*TIME_WND_MAX);

    close(fd);
    return 0;
}

JNIEXPORT void cancel_receive(JNIEnv *env, jclass clazz)
{
    gDo_receive = 0; 
}

JNIEXPORT jint check_env_and_init(JNIEnv *env, jclass clazz)
{
    int fd = -1;
    LOGV("check env and init ...");
    fd = OpenIrDevice();

    if(fd < 0) {
        LOGD("open device error:%s",strerror(errno));
        return -1;
    } else {
        CloseIrDevice(fd);
        return 0;
    }
}

JNIEXPORT jint send_action(JNIEnv *env, jclass clazz, jintArray code){
    int fd=-1 , len=0, i=0;
    long time = 0;
    jint *body = NULL;
	IrCode_t code_t;

    fd = OpenIrDevice();
    if(fd < 0) {
        LOGD("open device error:%s",strerror(errno));
        return -1;
    }

	len = (*env)->GetArrayLength(env,code);
	body = (*env)->GetIntArrayElements(env, code,0);
	code_t.WndNum = len;
	for(i = 0;i<len;i++){
		code_t.TimeWnd[i] = (unsigned int)(body[i]);
        time += code_t.TimeWnd[i];
	}
    LOGD("send start  time:%d", time);
    log_code(&code_t);
	SendIRCode(fd, code_t);
    usleep(time*10);
    LOGD("send end");
	(*env)->ReleaseIntArrayElements(env, code, body, 0);

    CloseIrDevice(fd);
	return 0;
}

JNIEXPORT jint redef_action(JNIEnv *env, jclass clazz)
{
    int fd = -1 , getCounts = 0 , ret = 0;

    fd = OpenIrDevice();
    if(fd < 0) {
        LOGD("open device error:%s",strerror(errno));
        return -1;
    }
    
    gDo_receive = 1;
    memset(&gCode , 0 , sizeof(IrCode_t));
    while(1) {

        gCode_ready = 0;
        if(!gDo_receive) {
            ret = -8;
            LOGD("redef cancel");
            break;
        }

        EnterIRreceiver(fd);
        usleep(2000*1000); 
        memset(&gCode ,0,sizeof(IrCode_t));
        gCode = GetIRCode(fd);
        ExitIRreceiver(fd);

        if(gCode.WndNum > 2 && gCode.WndNum <= TIME_WND_MAX ) {
            log_code(&gCode);
            gCode_ready = 1; 
            break;
        } else if(getCounts++ < 6){
            usleep(1000*200);
        } else {
            ret = -6;
            break;
        }
    }
    CloseIrDevice(fd);
	return ret;
}

JNIEXPORT jintArray receive_action(JNIEnv *env, jclass clazz){

	jintArray jcode;
	int i = 0;
	jint temp[TIME_WND_MAX];

    if(!gCode_ready) {
        LOGE("receive action mabe error");
    }
	
	jcode = (*env)->NewIntArray(env,gCode.WndNum);
	for(i = 0;i<gCode.WndNum;i++){
		temp[i] = gCode.TimeWnd[i];	
	}
	(*env)->SetIntArrayRegion(env,jcode,0,gCode.WndNum,temp);
	
	return jcode;		
}

JNIEXPORT jint check_action(JNIEnv *env, jclass clazz, jstring path)
{
    const char *key_path = (*env)->GetStringUTFChars(env,path,0);
    LOGV("check_action: %s",key_path);


    (*env)->ReleaseStringUTFChars(env , path, key_path);
	return 0;
}

JNIEXPORT void app_exit(JNIEnv *env, jclass clazz)
{
    LOGV("do exit");
}


static JNINativeMethod gMethods[] = {
	{ "cancelRecv",       "()V",                    (void*) cancel_receive},
	{ "checkAndInit",     "()I",		            (void*) check_env_and_init},
    { "sendAction", 	  "([I)I",	                (void*) send_action},
    { "receiveAction",    "()[I",	                (void*) receive_action},
    { "checkAction",      "(Ljava/lang/String;)I",	(void*) check_action},
    { "redefAction",      "()I",                    (void*) redef_action},
    { "appExit",          "()V",	                (void*) app_exit},

};

int jniRegisterNativeMethods(JNIEnv* env,
                             const char* className,
                             JNINativeMethod* gMethods,
                             int numMethods)
{
	jclass clazz;

	LOGI("Registering %s natives\n", className);
	clazz = (*env)->FindClass(env,className);
	if (clazz == NULL) {
	    LOGE("Native registration unable to find class '%s'\n", className);
	return -1;
	}
	if ((*env)->RegisterNatives(env,clazz, gMethods, numMethods) < 0) {
	    LOGE("RegisterNatives failed for '%s'\n", className);
	return -1;
	}
	return 0;
}

 static int register_com_amlogic_smartremote_RemoteModel(JNIEnv *env) {
	const char* const kClassPathName = "com/amlogic/smartremote/RemoteModel";
	return jniRegisterNativeMethods(env,kClassPathName , gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}



jint JNI_OnLoad(JavaVM* vm, void* reserved){
    jint result = -1;
    JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GetEnv failed!");
        return -1;
    }

    LOGI("GetEnv ok");    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    //register_com_amlogic_smartremote_IRHandler(env);
    register_com_amlogic_smartremote_RemoteModel(env);
    return result;
}

void JNI_OnUnload(JavaVM* vm, void* reserved){
	JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GetEnv failed!");
        return;
    }

	return;
}
