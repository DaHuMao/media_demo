set -o errexit
source script_tool/env.sh
source build_script/tool.sh
target_os=android
. build_script/env_config.sh

debug_dir="release"
if [[ "$is_debug" = true ]];then
  debug_dir="debug"
fi

jni_lib_dir=examples/android_demo/app/src/main/jniLibs/
HOST_OS=$(uname -s)
HOST_ARCH=$(uname -m)
if [ "$HOST_OS" = "Darwin" ]; then
  if [ "$HOST_ARCH" = "x86_64" ]; then
    libcpp_dir=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib
  elif [ "$HOST_ARCH" = "arm64" ]; then
    libcpp_dir=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/darwin-arm64/sysroot/usr/lib
  else
    log_abort "Unknown HOST_ARCH: $HOST_ARCH"
  fi
else
  log_abort "Unknown HOST_OS: $HOST_OS"
fi
for arch in $archs
do
  out_dir="out/$target_os/$debug_dir/$arch"
  SafeMakeDir $out_dir
  log_info "out_dir: $out_dir"
  gn gen $out_dir --root=. \
    --args="$common_args \
    target_cpu=\"${arch}\"" \
    $GN_GEN_PROJECT
  ninja -C $out_dir
  if [ "$arch" = "arm" ]; then
    lib_dir=$jni_lib_dir/armeabi-v7a
    libcpp_shared_dir=$libcpp_dir/arm-linux-androideabi
  elif [ "$arch" = "arm64" ]; then
    lib_dir=$jni_lib_dir/arm64-v8a
    libcpp_shared_dir=$libcpp_dir/aarch64-linux-android
  elif [ "$arch" = "x86" ]; then
    lib_dir=$jni_lib_dir/x86
    libcpp_shared_dir=$libcpp_dir/i686-linux-android
  elif [ "$arch" = "x64" ]; then
    lib_dir=$jni_lib_dir/x86_64
    libcpp_shared_dir=$libcpp_dir/x86_64-linux-android
  else
    log_abort "Unknown arch: $arch"
  fi
  if [ ! -d $lib_dir ]; then
    log_abort "lib_dir not exist: $lib_dir"
  fi
  log_info "cp $out_dir/lib.unstripped/lib*.so $lib_dir"
  cp $out_dir/lib.unstripped/lib*.so $lib_dir
  cp $libcpp_shared_dir/libc++_shared.so $lib_dir
done

