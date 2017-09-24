#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include "nvml.h"
#include "NvidiaManagementLibrary.h"

static const unsigned int NVIDIA_MAJOR_DEVICE = 195;

JNIEXPORT jboolean JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_initialize
  (JNIEnv * env, jobject obj) {
    void * handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);

    if (handle == NULL) {
      fprintf(stderr, "Could not load libnvidia-ml.so.1 library, is LD_LIBRARY_PATH set correctly?");
      return 0;
    }
    typedef void * ( * arbitrary)();
    arbitrary nvmlInit;
    * (void * * )( & nvmlInit) = dlsym(handle, "nvmlInit_v2");
    int ret = nvmlInit();
    if (ret == NVML_SUCCESS) {
      return 1;
    } else {
      fprintf(stderr, "nvmlInit call was not successful, return code: %d\n", ret);
      return 0;
    }
  }

JNIEXPORT jboolean JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_shutDown
  (JNIEnv * env, jobject obj) {
    void * handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    if (handle == NULL) {
      fprintf(stderr, "Could not load libnvidia-ml.so.1 library, is LD_LIBRARY_PATH set correctly?");
      return 0;
    }
    typedef void * ( * arbitrary)();
    arbitrary nvmlShutdown;
    * (void * * )( & nvmlShutdown) = dlsym(handle, "nvmlShutdown");
    int ret = nvmlShutdown();
    if (ret == NVML_SUCCESS) {
      return 1;
    } else {
      fprintf(stderr, "nvmlShutdown call was not successful, return code: %d\n", ret);
      return 0;
    }
  }

JNIEXPORT jint JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_getNumGPUs
  (JNIEnv * env, jobject obj) {

    void * handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    if (handle == NULL) {
      fprintf(stderr, "Could not load libnvidia-ml.so.1 library, is LD_LIBRARY_PATH set correctly?");
      return 0;
    }

    typedef void * ( * arbitrary)();
    arbitrary nvmlDeviceGetCount_v2;
    * (void * * )( & nvmlDeviceGetCount_v2) = dlsym(handle, "nvmlDeviceGetCount_v2");
    int numAvailableDevices = 0;
    int ret1 = nvmlDeviceGetCount_v2( & numAvailableDevices);
    if (ret1 != NVML_SUCCESS) {
      printf("could not query number of available GPU devices");
      return 0;
    }

    int numNonErroneousDevices = 0;
    int index;
    nvmlDevice_t device;
    arbitrary nvmlDeviceGetHandleByIndex_v2;
    * (void * * )( & nvmlDeviceGetHandleByIndex_v2) = dlsym(handle, "nvmlDeviceGetHandleByIndex_v2");
    for (index = 0; index < numAvailableDevices; index++) {
      int ret = nvmlDeviceGetHandleByIndex_v2(index, & device);
      if (ret == NVML_SUCCESS) {
          numNonErroneousDevices++;
      } else {
          fprintf(stderr, "nvmlDeviceGetMinorNumber call was not successful, return code: %d\n", ret);
      }
    }
    return numNonErroneousDevices;
  }

JNIEXPORT jstring JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_queryMandatoryDevices
  (JNIEnv * env, jobject obj) {
    char deviceIDsBuf[6 * 10 + 6];
    char * pos = deviceIDsBuf;

    int foundDrivers = -1;

    struct stat nvidiauvmStat;
    int ret = stat("/dev/nvidia-uvm", & nvidiauvmStat);
    if (ret == 0) {
      int uvmDeviceMinor = minor(nvidiauvmStat.st_rdev);
      int uvmDeviceMajor = major(nvidiauvmStat.st_rdev);
      pos += sprintf(pos, "%d:%d ", uvmDeviceMajor, uvmDeviceMinor);
      foundDrivers = 1;
    } else {
    //device file needs to be created manually
      system("nvidia-modprobe -u -c=0");
      ret = stat("/dev/nvidia-uvm", & nvidiauvmStat);
      if (ret == 0) {
        int uvmDeviceMinor = minor(nvidiauvmStat.st_rdev);
        int uvmDeviceMajor = major(nvidiauvmStat.st_rdev);
        pos += sprintf(pos, "%d:%d ", uvmDeviceMajor, uvmDeviceMinor);
        foundDrivers = 1;
      }
    }

    struct stat nvidiactlStat;
    if (stat("/dev/nvidiactl", & nvidiactlStat) == 0) {
      int actlDeviceMinor = minor(nvidiactlStat.st_rdev);
      int actlDeviceMajor = major(nvidiactlStat.st_rdev);
      pos += sprintf(pos, "%d:%d ", actlDeviceMajor, actlDeviceMinor);
      foundDrivers = 1;
    };

    struct stat nvidiaUvmToolsStat;
    if (stat("/dev/nvidia-uvm-tools", & nvidiaUvmToolsStat) == 0) {
      int uvmToolsDeviceMinor = minor(nvidiaUvmToolsStat.st_rdev);
      int uvmToolsDeviceMajor = major(nvidiaUvmToolsStat.st_rdev);
      foundDrivers = 1;

      sprintf(pos, "%d:%d ", uvmToolsDeviceMajor, uvmToolsDeviceMinor);
    }
    if(foundDrivers == 1) {
    return ( * env) -> NewStringUTF(env, deviceIDsBuf);
    } else {
    return ( * env) -> NewStringUTF(env, ""); }
  }

JNIEXPORT jstring JNICALL Java_io_hops_management_nvidia_NvidiaManagementLibrary_queryAvailableDevices
  (JNIEnv * env, jobject obj, jint reportedGPUs) {

    if(reportedGPUs == 0) {
        return ( * env) -> NewStringUTF(env, "");
    }
    char formattedBuf[0];

    void * handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);

    if (handle == NULL) {
      fprintf(stderr, "Could not load libnvidia-ml.so.1 library, is LD_LIBRARY_PATH set correctly?");
      return ( * env) -> NewStringUTF(env, "");
    } else {
      typedef void * ( * arbitrary)();

      int majorMinorDeviceIdPairs[reportedGPUs * 2];

      char formattedBuf[10 * (reportedGPUs + 1) + (2 *
        (reportedGPUs + 1))];
      char * pos = formattedBuf;

      nvmlDevice_t device;
      arbitrary nvmlDeviceGetHandleByIndex;
      * (void * * )( & nvmlDeviceGetHandleByIndex) = dlsym(handle, "nvmlDeviceGetHandleByIndex");
      int index = 0;
      int numSchedulableGPUs = 0;
      while (numSchedulableGPUs != reportedGPUs) {

        int ret = nvmlDeviceGetHandleByIndex(index, & device);
        if (ret == NVML_SUCCESS) {
          int minornumber = -1;
          arbitrary nvmlDeviceGetMinorNumber;
          * (void * * )( & nvmlDeviceGetMinorNumber) = dlsym(handle, "nvmlDeviceGetMinorNumber");
          int ret3 = nvmlDeviceGetMinorNumber(device, & minornumber);

          majorMinorDeviceIdPairs[numSchedulableGPUs * 2] = 195;
          majorMinorDeviceIdPairs[numSchedulableGPUs * 2 + 1] = minornumber;
          numSchedulableGPUs++;
        } else {
          fprintf(stderr, "nvmlDeviceGetMinorNumber call was not successful, return code: %d\n", ret);
        }
        index++;
      }
      //create formatted string "major1:minor1 major2:minor2 major3:minor"
      int deviceIndex;
      for (deviceIndex = 0; deviceIndex < reportedGPUs; deviceIndex++) {
        pos += sprintf(pos, "%d:%d ", majorMinorDeviceIdPairs[deviceIndex * 2], majorMinorDeviceIdPairs[deviceIndex * 2 + 1]);
      }
      return ( * env) -> NewStringUTF(env, formattedBuf);
    }
    return ( * env) -> NewStringUTF(env, "");
  }