#include <iostream>
#include <sys/time.h>
#include <cstring>
#include "packer.h"

void Usage() {
    std::cout << "Usage: resource-packer [OPTIONAL] [version] inputpath outputpath" << std::endl;
    std::cout << "    -c compress, version src_dir dst_file." << std::endl;
    std::cout << "    -x extract, src_file dst_dir." << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 4 && argc!=5) {
        Usage();
        return 1;
    }

    if (0!=strcmp("-c", argv[1]) && 0!=strcmp("-x", argv[1])) {
        Usage();
        return 1;
    }

    bool success = false;
    bool compress = (0==strcmp("-c", argv[1]));

    struct timeval start, stop;
    memset(&start,0,sizeof(struct timeval));
    memset(&stop,0,sizeof(struct timeval));
    gettimeofday(&start,0);
    packer::Packer res_packer;
    if (compress) {
        if (argc != 5) {
            Usage();
            return 1;
        }
        const char *version = argv[2];
        const char *src_path = argv[3];
        const char *out_path = argv[4];
        if (res_packer.Open(out_path, packer::MODE_WRITE)) {
            res_packer.SetVersion(version);
            res_packer.AddDir(src_path);
            res_packer.Close();
            success = true;
        }
    } else {
        if (argc != 4) {
            Usage();
            return 1;
        }
        const char *src_path = argv[2];
        const char *out_path = argv[3];
        if (res_packer.Open(src_path, packer::MODE_READ)) {
            const std::string version = res_packer.GetVersion();
            std::cout << "Resource Version: " << version << std::endl;
            res_packer.Extract(out_path);
            res_packer.Close();
            success = true;
        }
    }
    gettimeofday(&stop,0);
    if (!success) std::cout << "fail." << std::endl;
    int64_t time_diff = (stop.tv_sec - start.tv_sec) * 1e6;
    time_diff += (stop.tv_usec - start.tv_usec);
    printf("Elapse Time: %.3fms\n", time_diff/1000.0);
    return 0;
}
