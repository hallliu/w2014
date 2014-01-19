#ifndef FW_H
#define FW_H

double fw_parallel(int *, int, int);
double fw_serial(int *, int);

#ifdef TIME_WAIT
struct final_wait_data {
    double wo_ratio;
    double o_mean;
    double o_std;
};
#endif
#endif
