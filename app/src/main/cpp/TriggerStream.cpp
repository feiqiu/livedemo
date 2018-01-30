//
// Created by anjubao on 2016-10-19.
//

#include <jni.h>
#include <android/log.h>
#include <stddef.h>
#include "TriggerStream.h"
#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR,  x ,__VA_ARGS__)
#define TAG "triggerStream"


static jclass classMediaServer = NULL;
static JavaVM *javaVM = NULL;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    JNIEnv *env = NULL;
    javaVM = vm;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    LOGE(TAG, "JNI_OnLoad triggerStream %x ", javaVM);
    jclass cls = env->FindClass("org/laoguo/livescreen/MediaServer");
    classMediaServer = (jclass)env->NewGlobalRef(cls);
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}

void triggerStream(bool addStream) {
//    JNIEnv *env = NULL;
//    javaVM->AttachCurrentThread(&env, NULL);
//
//    jmethodID mid = env->GetStaticMethodID(classMediaServer, "triggerStream", "(Z)V");
//    if (mid == NULL) {
//        LOGE(TAG, "not find java method!");
//        return;
//    }
//    //LOGE(TAG, "run triggerStream");
//    env->CallStaticVoidMethod(classMediaServer, mid, addStream);
//    javaVM->DetachCurrentThread();
}


