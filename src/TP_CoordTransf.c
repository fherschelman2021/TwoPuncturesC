/* TP_CoordTransf.c */

#include "TwoPunctures.h"

void AB_To_XR (int nvar, double A, double B, double *X, double *R,
	       derivs *U)
{
  double At = 0.5 * (A + 1);
  double A_X, A_XX, B_R, B_RR;
  int ivar;
  
  *X = 2 * atanh (At);
  *R = Pih + 2 * atan (B);
  
  A_X = 1 - At * At;
  A_XX = -At * A_X;
  B_R = 0.5 * (1 + B * B);
  B_RR = B * B_R;
  
  for (ivar = 0; ivar < nvar; ivar++) {
    U->d11[ivar] = A_X * A_X * U->d11[ivar] + A_XX * U->d1[ivar];
    U->d12[ivar] = A_X * B_R * U->d12[ivar];
    U->d13[ivar] = A_X * U->d13[ivar];
    U->d22[ivar] = B_R * B_R * U->d22[ivar] + B_RR * U->d2[ivar];
    U->d23[ivar] = B_R * U->d23[ivar];
    U->d1[ivar] = A_X * U->d1[ivar];
    U->d2[ivar] = B_R * U->d2[ivar];
  }
  
}

void C_To_c (int nvar, double X, double R, double *x, double *r,
	     double par_b,
	     derivs *U)
{
  double C_c2, U_cb, U_CB;
  gsl_complex C, C_c, C_cc, c, c_C, c_CC, U_c, U_cc, U_C, U_CC;
  int ivar;

  C = gsl_complex_rect (X, R);

  c = gsl_complex_mul_real (gsl_complex_cosh (C), par_b);	/* c=b*cosh(C)*/
  c_C = gsl_complex_mul_real (gsl_complex_sinh (C), par_b);
  c_CC = c;

  C_c = gsl_complex_inverse (c_C);
  C_cc = gsl_complex_negative (gsl_complex_mul (gsl_complex_mul (C_c, C_c), gsl_complex_mul (C_c, c_CC)));
  C_c2 = gsl_complex_abs2 (C_c);
  
  for (ivar = 0; ivar < nvar; ivar++) {
    
    /* U_C = 0.5*(U_X3-i*U_R3)*/
    /* U_c = U_C*C_c = 0.5*(U_x3-i*U_r3)*/
    U_C = gsl_complex_rect (0.5 * U->d13[ivar], -0.5 * U->d23[ivar]);
    U_c = gsl_complex_mul (U_C, C_c);
    U->d13[ivar] = 2. * GSL_REAL(U_c);
    U->d23[ivar] = -2. * GSL_IMAG(U_c);

    /* U_C = 0.5*(U_X-i*U_R)*/
    /* U_c = U_C*C_c = 0.5*(U_x-i*U_r)*/
    U_C = gsl_complex_rect (0.5 * U->d1[ivar], -0.5 * U->d2[ivar]);
    U_c = gsl_complex_mul (U_C, C_c);
    U->d1[ivar] = 2. * GSL_REAL(U_c);
    U->d2[ivar] = -2. * GSL_IMAG(U_c);

    /* U_CC = 0.25*(U_XX-U_RR-2*i*U_XR)*/
    /* U_CB = d^2(U)/(dC*d\bar{C}) = 0.25*(U_XX+U_RR)*/
    U_CC = gsl_complex_rect (0.25 * (U->d11[ivar] - U->d22[ivar]), -0.5 * U->d12[ivar]);
    U_CB = 0.25 * (U->d11[ivar] + U->d22[ivar]);

    /* U_cc = C_cc*U_C+(C_c)^2*U_CC*/
    U_cb = U_CB * C_c2;
    U_cc = gsl_complex_add (gsl_complex_mul (C_cc, U_C), gsl_complex_mul (gsl_complex_mul (C_c, C_c), U_CC));

    /* U_xx = 2*(U_cb+Re[U_cc])*/
    /* U_rr = 2*(U_cb-Re[U_cc])*/
    /* U_rx = -2*Im[U_cc]*/
    U->d11[ivar] = 2 * (U_cb + GSL_REAL(U_cc));
    U->d22[ivar] = 2 * (U_cb - GSL_REAL(U_cc));
    U->d12[ivar] = -2 * GSL_IMAG(U_cc);
  }

  *x = GSL_REAL(c);
  *r = GSL_IMAG(c);
}

void rx3_To_xyz (int nvar, double x, double r, double phi,
		 double *y, double *z, derivs *U)
{
  int jvar;
  double
    sin_phi = sin (phi),
    cos_phi = cos (phi),
    sin2_phi = sin_phi * sin_phi,
    cos2_phi = cos_phi * cos_phi,
    sin_2phi = 2 * sin_phi * cos_phi,
    cos_2phi = cos2_phi - sin2_phi, r_inv = 1 / r, r_inv2 = r_inv * r_inv;
  
  *y = r * cos_phi;
  *z = r * sin_phi;
  
  for (jvar = 0; jvar < nvar; jvar++) {

    double U_x = U->d1[jvar], U_r = U->d2[jvar], U_3 = U->d3[jvar],
      U_xx = U->d11[jvar], U_xr = U->d12[jvar], U_x3 = U->d13[jvar],
      U_rr = U->d22[jvar], U_r3 = U->d23[jvar], U_33 = U->d33[jvar];

    U->d1[jvar] = U_x;		/* U_x*/
    U->d2[jvar] = U_r * cos_phi - U_3 * r_inv * sin_phi;	/* U_y*/
    U->d3[jvar] = U_r * sin_phi + U_3 * r_inv * cos_phi;	/* U_z*/
    U->d11[jvar] = U_xx;		/* U_xx*/
    U->d12[jvar] = U_xr * cos_phi - U_x3 * r_inv * sin_phi;	/* U_xy*/
    U->d13[jvar] = U_xr * sin_phi + U_x3 * r_inv * cos_phi;	/* U_xz*/
    U->d22[jvar] = U_rr * cos2_phi + r_inv2 * sin2_phi * (U_33 + r * U_r)	/* U_yy*/
      + sin_2phi * r_inv2 * (U_3 - r * U_r3);
    U->d23[jvar] = 0.5 * sin_2phi * (U_rr - r_inv * U_r - r_inv2 * U_33)	/* U_yz*/
      - cos_2phi * r_inv2 * (U_3 - r * U_r3);
    U->d33[jvar] = U_rr * sin2_phi + r_inv2 * cos2_phi * (U_33 + r * U_r)	/* U_zz*/
      - sin_2phi * r_inv2 * (U_3 - r * U_r3);
  }
  
}
