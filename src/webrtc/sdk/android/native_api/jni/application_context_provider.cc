/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "sdk/android/native_api/jni/application_context_provider.h"
#include <jni.h>

#include "rtc_base/checks.h"
#include "sdk/android/native_api/jni/scoped_java_ref.h"
#include "sdk/android/src/jni/jni_tool/jni_help_tool.h"

namespace webrtc {
ScopedJavaLocalRef<jobject> GetAppContext(JNIEnv* jni) {
  return ScopedJavaLocalRef<jobject>(jni,
      jni_help::GetJavaInstance(jni, "org/webrtc/ApplicationContextProvider",
        "getApplicationContext", "android/content/Context"));
}

}  // namespace webrtc
