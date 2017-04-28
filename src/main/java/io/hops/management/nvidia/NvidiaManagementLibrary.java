/*
 * Copyright (C) 2015 hops.io.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package io.hops.management.nvidia;

import io.hops.*;

/**
 * A helper to load the native libnvidia-ml.so.1
 */
public class NvidiaManagementLibrary implements GPUManagementLibrary {
  
  static {
    System.loadLibrary("hopsnvml-1.0");
  }
  
  @Override
  public native boolean initialize();
  
  @Override
  public native boolean shutDown();
  
  @Override
  public native int getNumGPUs();
  
  @Override
  public native String queryMandatoryDevices();
  
  @Override
  public native String queryAvailableDevices(int configuredGPUs);
}
