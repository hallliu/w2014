#ifndef FW_H
#define FW_H

double fw_serial(int *, int);

#ifdef TIME_WAIT
struct final_wait_data {
    double wo_ratio;
    double o_mean;
    double o_std;
};

struct final_wait_data *fw_parallel(int *, int, int);
#else
double fw_parallel(int *, int, int);
#endif
#endif
