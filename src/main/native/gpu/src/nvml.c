#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include "nvml.h"
#include "NvidiaManagementLibrary.h"

static const unsigned int NVIDIA_MAJOR_DEVICE = 195;



JNIEXPORT jboolean JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_initialize
  (JNIEnv *env, jobject obj) {
        void* handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            exit(1);
        }
        if (handle == NULL) {
            printf("woops! no dynamic lib here no");
            return 1;
        }
        typedef void* (*arbitrary)();
        arbitrary nvmlInit;
        *(void**)(&nvmlInit) = dlsym(handle, "nvmlInit_v2");
        int ret = nvmlInit();
        if(ret == NVML_SUCCESS) {
        return 1;
        } else {
        fprintf(stderr, "nvmlInit call was not successful, return code: %d\n", ret);
        return 0;
        }
    }


JNIEXPORT jboolean JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_shutDown
  (JNIEnv *env, jobject obj) {
        void* handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
                if (!handle) {
                    fprintf(stderr, "dlopen failed: %s\n", dlerror());
                    exit(1);
                }
         typedef void* (*arbitrary)();
         arbitrary nvmlShutdown;
         *(void**)(&nvmlShutdown) = dlsym(handle, "nvmlShutdown");
         int ret = nvmlShutdown();
         if(ret == NVML_SUCCESS) {
                 return 1;
                 } else {
                 fprintf(stderr, "nvmlShutdown call was not successful, return code: %d\n", ret);
                 return 0;
                 }
    }

    JNIEXPORT jint JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_getNumGPUs
      (JNIEnv *env, jobject obj) {

        void* handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            exit(1);
        }
        if (handle == NULL) {
            printf("woops! no dynamic lib here no");
            return 1;
        }

        typedef void* (*arbitrary)();
                    arbitrary nvmlDeviceGetCount_v2;
                    *(void**)(&nvmlDeviceGetCount_v2) = dlsym(handle, "nvmlDeviceGetCount_v2");
                    int numAvailableDevices = 0;
                    int ret1 = nvmlDeviceGetCount_v2(&numAvailableDevices);
                    if(ret1 != NVML_SUCCESS) {
                    printf("could not query number of available GPU devices");
                                return 0;
                    }
                    return numAvailableDevices;
      }


JNIEXPORT jstring JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_queryMandatoryDevices
  (JNIEnv *env, jobject obj) {
        char deviceIDsBuf[6*10 + 6];
        char *pos = deviceIDsBuf;

        struct stat nvidiauvmStat;
        if (stat("/dev/nvidia-uvm", &nvidiauvmStat) < 0) {
              //device file needs to be created manually
              system("nvidia-modprobe -u -c=0");
         }

        int uvmDeviceMinor = minor(nvidiauvmStat.st_rdev);
        int uvmDeviceMajor = major(nvidiauvmStat.st_rdev);

        pos += sprintf(pos, "%d:%d ", uvmDeviceMajor, uvmDeviceMinor);

        struct stat nvidiactlStat;
        if (stat("/dev/nvidiactl", &nvidiactlStat) < 0) {
            return 1;
        }

        int actlDeviceMinor = minor(nvidiactlStat.st_rdev);
        int actlDeviceMajor = major(nvidiactlStat.st_rdev);

        pos += sprintf(pos, "%d:%d ", actlDeviceMajor, actlDeviceMinor);

        struct stat nvidiaUvmToolsStat;
        if (stat("/dev/nvidia-uvm-tools", &nvidiaUvmToolsStat) == 0) {
            int uvmToolsDeviceMinor = minor(nvidiaUvmToolsStat.st_rdev);
            int uvmToolsDeviceMajor = major(nvidiaUvmToolsStat.st_rdev);

            sprintf(pos, "%d:%d ", uvmToolsDeviceMajor, uvmToolsDeviceMinor);
        }
        (*env)->NewStringUTF(env, deviceIDsBuf);
        return (*env)->NewStringUTF(env, deviceIDsBuf);
}

JNIEXPORT jstring JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_queryAvailableDevices
  (JNIEnv *env, jobject obj, jint gpus) {
  char formattedBuf[0];

        void* handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            exit(1);
        }
        if (handle == NULL) {
            printf("woops! no dynamic lib here no");
            return 1;
        }
        else {
            typedef void* (*arbitrary)();

            int majorMinorDeviceIdPairs[gpus*2];

            char formattedBuf[10*(gpus+1) + (2*
            (gpus+1))];
            char *pos = formattedBuf;

            nvmlDevice_t device;
            arbitrary nvmlDeviceGetHandleByIndex;
            *(void**)(&nvmlDeviceGetHandleByIndex) = dlsym(handle, "nvmlDeviceGetHandleByIndex");
            int index;
            for (index = 0; index < gpus; index++) {

                //TODO handle the case when device throws error, they should not be used in scheduling
                int ret = nvmlDeviceGetHandleByIndex(index, &device);
                if(ret == NVML_SUCCESS) {
                    int minornumber = -1;
                                    arbitrary nvmlDeviceGetMinorNumber;
                                    *(void**)(&nvmlDeviceGetMinorNumber) = dlsym(handle, "nvmlDeviceGetMinorNumber");
                                    int ret3 = nvmlDeviceGetMinorNumber(device, &minornumber);

                                    majorMinorDeviceIdPairs[index*2] = 195;
                                    majorMinorDeviceIdPairs[index*2+1] = minornumber;
                } else {
                    fprintf(stderr, "nvmlDeviceGetMinorNumber call was not successful, return code: %d\n", ret);
                }

            }
            //create formatted string "major1:minor1 major2:minor2 major3:minor"
            int deviceIndex;
            for(deviceIndex = 0; deviceIndex < gpus;
             deviceIndex++) {
             pos += sprintf(pos, "%d:%d ", majorMinorDeviceIdPairs[deviceIndex*2], majorMinorDeviceIdPairs[deviceIndex*2+1]);
             }
             return (*env)->NewStringUTF(env, formattedBuf);
        }
        return (*env)->NewStringUTF(env, "");
    }