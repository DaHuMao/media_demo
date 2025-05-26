set -o errexit
source script_tool/env.sh
source build_script/tool.sh
target_os=mac
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
  ninja -C $out_dir
done
