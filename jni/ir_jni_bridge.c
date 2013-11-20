#define LOG_NDEBUG 0
#define LOG_TAG "SmartRemoteJNI"

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include "ir_api.h"



JNIEXPORT jint enter_receiver(JNIEnv *env, jclass clazz, jint fd)
{
	EnterIRreceiver(fd);	
	return 0;
}

JNIEXPORT jint exit_receiver(JNIEnv *env, jclass clazz, jint fd)
{	
	ExitIRreceiver(fd);
	return 0;
}

JNIEXPORT jintArray get_code(JNIEnv *env, jclass clazz, jint fd){
	
	jintArray jcode;
	int i = 0;
	jint temp[TIME_WND_MAX];
	
	IrCode_t code_t;
	code_t = GetIRCode(fd);
	jcode = (*env)->NewIntArray(env,code_t.WndNum);
	for(i = 0;i<code_t.WndNum;i++){
		temp[i] = code_t.TimeWnd[i];	
	}
	(*env)->SetIntArrayRegion(env,jcode,0,code_t.WndNum,temp);
	
	return jcode;		
}

JNIEXPORT void send_code(JNIEnv *env, jclass clazz,jint fd, jintArray code){
	IrCode_t code_t;
	int len = (*env)->GetArrayLength(env,code);
	jint *body = (*env)->GetIntArrayElements(env, code,0);
	int i = 0;
	code_t.WndNum = len;
	for(i = 0;i<len;i++){
		//code_t.TimeWnd[i] = body[i]&0x00ff;
		code_t.TimeWnd[i] = (unsigned int)(body[i]);
	}
	SendIRCode(fd, code_t);
	(*env)->ReleaseIntArrayElements(env, code, body, 0);

	return;
}

JNIEXPORT jint open_device(JNIEnv *env, jclass clazz){
 	int ret = OpenIrDevice();
        return ret;      
}

JNIEXPORT void close_device(JNIEnv *env, jclass clazz, jint fd){
 	CloseIrDevice(fd);
        return;      
}

//
static JNINativeMethod gMethods[] = {     
	{ "native_open_device",             "()I",		(void*) open_device},
    { "native_enter_receiver", 	        "(I)I",		(void*) enter_receiver},
    { "native_exit_receiver",           "(I)I",		(void*) exit_receiver},
    { "native_send_code",               "(I[I)V",	(void*) send_code},
    { "native_close_device",            "(I)V",		(void*) close_device},
    { "native_get_code",                "(I)[I",	(void*) get_code},

};
 
static int jniRegisterNativeMethods(JNIEnv* env,
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

int register_com_amlogic_smartremote_IRHandler(JNIEnv *env) {
	const char* const kClassPathName = "com/amlogic/smartremote/IRHandler";
	return jniRegisterNativeMethods(env,kClassPathName , gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}
