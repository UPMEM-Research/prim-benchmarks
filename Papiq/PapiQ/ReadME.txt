dpu-upmem-dpurte-clang mv.c -o mv

gcc --std=c99 -o mv_host mv_host.c `dpu-pkg-config --libs --cflags dpu`
