#include "mex.h"
#include <math.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs != 2) {
        mexErrMsgIdAndTxt("MATLAB:matern_matrix:rhs", "This function takes two input arguments");
    }
    if (nlhs != 1) {
        mexErrMsgIdAndTxt("MATLAB:matern_matrix:lhs", "This function takes one output arguments");
    }


    const mxArray *_X = prhs[0];
    const mxArray *_theta = prhs[1];
    
    size_t d = mxGetM (_X);
    size_t n = mxGetN (_X);

    plhs[0] = mxCreateDoubleMatrix((mwSize) n, (mwSize) n, mxREAL);

    const double *theta = mxGetPr(_theta);
    const double *X = mxGetPr(_X);
    double *K = mxGetPr(plhs[0]);

    double rt5 = sqrt(5.);
    int i, j, k;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double r = 0;
            for (k = 0; k < d; k++) {
                double s = (X[i*d + k] - X[j*d + k]) / theta[k];
                r += s * s;
            }
            r = sqrt(r);
            r = (1 + rt5*r + (5/3.)*r*r) * exp(-rt5 * r);
            K[j*n + i] = r;
            K[i*n + j] = r;
        }
    }
}
