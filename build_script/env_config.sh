source script_tool/env.sh
source script_tool/tool_function.sh
if [ "$target_os" = "" ]; then
  log_abort "target_os is not set"
fi

is_debug=false
archs=""
build_all_arch=0
common_args=""
while [[ $# > 0 ]]; do
  case "$1" in
    --release)
      is_debug=false
      shift
      ;;
    --debug)
      is_debug=true
      shift
      ;;
    --arm)
      archs="$archs arm"
      shift
      ;;
    --arm64)
      archs="$archs arm64"
      shift
      ;;
    --x64)
      archs="$archs x64"
      shift
      ;;
    --x86)
      archs="$archs x86"
      shift
      ;;
    --all_arch)
      build_all_arch=1
      shift
      ;;
    --v)
      common_args="$common_args verbose_build=true"
      shift
      ;;
    *)
      log_abort "[ERROR] Unknown option: $1"
      shift
      ;;
  esac
done

default_archs="arm64"
case "$target_os" in
  ios)
    if [[ $build_all_arch -eq 1 ]]; then
      archs="arm64 arm x64"
    fi
    GN_GEN_PROJECT="--ide=xcode --export-compile-commands"
    ;;
  mac)
    if [[ $build_all_arch -eq 1 ]]; then
      archs="arm64 x64"
    fi
    default_archs="x64"
    #clang_base_path="$HOME/GitDownload/llvm-build"
    #custom_llvm_bin_dir="${clang_base_path}/bin/"
    #clang_base_path=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/
    #external_args="clang_base_path=\"${clang_base_path}\" custom_llvm_bin_dir=\"${custom_llvm_bin_dir}\""
    GN_GEN_PROJECT="--ide=xcode --export-compile-commands"
    ;;
  win)
    if [[ $build_all_arch -eq 1 ]]; then
      archs="x64 x86"
    fi
    default_archs="x64"
    GN_GEN_PROJECT="--ide=vs2019"
    export DEPOT_TOOLS_WIN_TOOLCHAIN=0
    export GYP_MSVS_VERSION=2019
    ;;
  linux)
    if [[ $build_all_arch -eq 1 ]]; then
      archs="x64"
    fi
    default_archs="x64"
    GN_GEN_PROJECT="--ide=json"
    ;;
  android)
    if [[ $build_all_arch -eq 1 ]]; then
      archs="arm64 arm x64"
    fi
    if [ $ANDROID_SDK_ROOT ]; then
      log_info "[INFO] ANDROID_SDK_ROOT Dir : $ANDROID_SDK_ROOT"
    else
      log_abort "[ERROR] Not set ANDROID_SDK_ROOT. Please export ANDROID_SDK_ROOT to Android SDK path."
    fi

    if [ ! -d "$ANDROID_SDK_ROOT" ]; then
      log_abort "[ERROR] Android SDK path does not exist - $ANDROID_SDK_ROOT"
    fi

    if [ $ANDROID_NDK_ROOT ]; then
      log_info "[INFO] ANDROID_NDK_ROOT Dir : $ANDROID_NDK_ROOT"
    else
      log_abort "[ERROR] Not set ANDROID_NDK_ROOT. Please export ANDROID_NDK_ROOT to NDK21 path."
    fi

    if [ ! -d "$ANDROID_NDK_ROOT" ]; then
      log_abort "[ERROR] Android NDK path does not exist - $ANDROID_NDK_ROOT"
    fi

    export ANDROID_NDK_HOME=${ANDROID_NDK_ROOT}

    ## Android NDK gn 参数
    clang_base_path="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/darwin-x86_64"
    ndk_properties_file="${ANDROID_NDK_ROOT}/source.properties"
    # 从文件中读取 Pkg.Revision 的值
    pkg_revision=$(grep "Pkg.Revision" $ndk_properties_file | cut -d "=" -f 2 | tr -d " ")
    # 提取主版本号，即第一个点号之前的数字
    android_ndk_major_version=$(echo $pkg_revision | cut -d "." -f 1)
    target_sysroot="${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
    external_args="enable_java_templates=false \
                   android_sdk_root=\"${ANDROID_SDK_ROOT}\" \
                   android_ndk_root=\"${ANDROID_NDK_ROOT}\" \
                   target_sysroot=\"${target_sysroot}\" \
                   clang_base_path=\"${clang_base_path}\""
    ## 默认只编译 armv7a, 如果需要同时编译armv8a使用. archs="arm64 arm"
    GN_GEN_PROJECT="--ide=json --export-compile-commands"
    ;;
  *)
    log_abort "[ERROR] Unsupported target_os : $target_os"
    ;;
esac

if [[ "$archs" == "" ]]; then
  archs=$default_archs
fi

common_args="is_debug=${is_debug} \
enable_dsyms=true \
use_custom_libcxx=false \
strip_debug_info=false \
clang_use_chrome_plugins=false \
rtc_include_tests=false \
strip_debug_info=false \
target_os=\"${target_os}\" ${external_args}"

log_info "common_args: $common_args"
