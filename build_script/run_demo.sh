set -o errexit
source script_tool/env.sh
source build_script/tool.sh
target_os=mac
os_array=("--mac" "--win" "--ios" "--android" "--linux")
if [ $# -ge 1 ]; then
   # 检查第一个参数是否在数组中
   if [[ " ${os_array[@]} " =~ "$1 " ]]; then
      target_os=${1:2} # 去掉前面的 --
      shift # 剔除第一个参数
   fi
fi
log_info "target_os: $target_os"

. build_script/env_config.sh

debug_dir="release"
if [[ "$is_debug" = true ]];then
  debug_dir="debug"
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
  ninja  -C $out_dir
done
