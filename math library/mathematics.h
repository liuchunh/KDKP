#include <stdio.h>
#include <stdlib.h>

extern const double eps;
extern const long double PI;
extern const long double E;
extern const double my_nan;

typedef long long LL;

LL my_abs(LL x);

double my_fabs(double x);

LL ipow(LL a, LL b);

double fpow(double a, LL b);

double fmax(double a, double b);

double fmin(double a, double b);

double sqrt_bs(double a);

double sqrt_nt(double a);

double Deg2Rad(double deg);

double Rad2Deg(double rad);

double taylor_sin(double x);

double taylor_cos(double x);

double ln(double x);

double my_log(double a, double b);

double taylor_exp(double x);

LL factorial(LL n);

double arctan(double x);

double my_atan2(double y, double x);