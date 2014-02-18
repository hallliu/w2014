#include "mex.h"
#include <math.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs != 4) {
        mexErrMsgIdAndTxt("MATLAB:matern_matrix:rhs", "This function takes four input arguments");
    }
    if (nlhs != 1) {
        mexErrMsgIdAndTxt("MATLAB:matern_matrix:lhs", "This function takes one output arguments");
    }


    const mxArray *_K = prhs[0];
    const mxArray *_X = prhs[1];
    const mxArray *_theta = prhs[2];
    const mxArray *__l = prhs[3];
    
    size_t d = mxGetM (_X);
    size_t n = mxGetN (_X);

    plhs[0] = mxCreateDoubleMatrix((mwSize) n, (mwSize) n, mxREAL);

    const double *theta = mxGetPr(_theta);
    const double *X = mxGetPr(_X);
    double *K = mxGetPr(_K);
    double *_l = mxGetPr(__l);
    int l = (int) round(*_l) - 1;

    double *dK = mxGetPr(plhs[0]);

    double rt5 = sqrt(5.);
    int i, j, k;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double mijl = X[i*d + l] - X[j*d + l];
            mijl = 5 * mijl * mijl / (3 * theta[l] * theta[l] * theta[l]);

            double r = 0;
            for (k = 0; k < d; k++) {
                double s = (X[i*d + k] - X[j*d + k]) / theta[k];
                r += s * s;
            }
            r = sqrt(r);

            /* r is reused here to hold the value of dK[i, j] */
            r = (K[i*n + j] - (5/3.)*r*r*exp(-rt5*r)) * mijl;

            dK[j*n + i] = r;
            dK[i*n + j] = r;
        }
    }
}
