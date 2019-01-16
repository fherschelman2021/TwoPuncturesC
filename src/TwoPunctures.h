/* TwoPunctures.h 
   
   Files of "TwoPunctures":

   TwoPunctures.c
   TP_FuncAndJacobian.c
   TP_CoordTransf.c
   TP_Equations.c
   TP_Newton.c
   TP_utilities.c 
*/

#ifndef TP_HEADER_H
#define TP_HEADER_H
// -------------------------------------------------------------------------

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_linalg.h>

#define GFINDEX3D (i, j, k, n, nn) ( (i) + (n)*(j) + (nn)*(k) )
#define ERROR(s) {printf(s); exit(0);}
#define DBGSTOP(s) {printf("DEBUGSTOP: %s",s); exit(0);}

/* params routines are in TP_Utilities.c */
#define NPARAMS (100)
#define STRLEN (256)
#define STREQL(s1,s2) ((strcmp((s1),(s2))==0))
enum{
  INTEGER,
  REAL,
  PARAMSTYPES,
};
typedef struct {
  int type[NPARAMS];
  char key[NPARAMS][STRLEN];
  double val[NPARAMS];
  int n;
  //parameters *next;
} parameters;

// some globals arrays needed in TwoPunctures and Equations
double par_P_plus[3]; 
double par_P_minus[3];
double par_S_plus[3]; 
double par_S_minus[3]; 

// interp options
enum{
  taylor_expansion,// use a Taylor expansion about the nearest collocation point (fast, but might be inaccurate)
  evaluation, // evaluate using all spectral coefficients (slow)
};
// lapse options 
enum{
  antisymmetric, // antisymmetric lapse for two puncture black holes, -1 <= alpha <= +1
  averaged, // averaged lapse for two puncture black holes, 0 <= alpha <= +1"
  psin, // Based on the initial conformal factor
  brownsville, // See Phys. Rev. D 74, 041501 (2006)
};
// conformalfactor options
enum{
  NONSTATIC, // conformal_state = 0 (?)
  FACTOR0, // CCTK_EQUALS(metric_type, "static conformal") && CCTK_EQUALS(conformal_storage, "factor")
  FACTOR1, // CCTK_EQUALS(metric_type, "static conformal") && CCTK_EQUALS(conformal_storage, "factor+derivs")
  FACTOR2, // CCTK_EQUALS(metric_type, "static conformal") && CCTK_EQUALS(conformal_storage, "factor+derivs+2nd derivs")
};
#define use_sources (0) // do not add the source terms
#define rescale_sources (1) // If sources are used - rescale them after solving?

// Turn on the interpolation ?
#define CARTESIAN_INTERP (0)

#define Pi  3.14159265358979323846264338328
#define Pih 1.57079632679489661923132169164	/* Pi/2*/
#define Piq 0.78539816339744830961566084582	/* Pi/4*/

#define TINY 1.0e-20
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

#define StencilSize 19
#define N_PlaneRelax 1
#define NRELAX 200
#define Step_Relax 1

typedef struct DERIVS
{
  double *d0, *d1, *d2, *d3, *d11, *d12, *d13, *d22, *d23, *d33;
} derivs;

/* TP_Utilies.c */
#define nrerror TP_nrerror
#define ivector TP_ivector
#define dvector TP_dvector
#define imatrix TP_imatrix
#define dmatrix TP_dmatrix
#define d3tensor TP_d3tensor
#define free_ivector TP_free_ivector
#define free_dvector TP_free_dvector
#define free_imatrix TP_free_imatrix
#define free_dmatrix TP_free_dmatrix
#define free_d3tensor TP_free_d3tensor

void nrerror (char error_text[]);
int *ivector (long nl, long nh);
double *dvector (long nl, long nh);
int **imatrix (long nrl, long nrh, long ncl, long nch);
double **dmatrix (long nrl, long nrh, long ncl, long nch);
double ***d3tensor (long nrl, long nrh, long ncl, long nch, long ndl,
		    long ndh);
void free_ivector (int *v, long nl, long nh);
void free_dvector (double *v, long nl, long nh);
void free_imatrix (int **m, long nrl, long nrh, long ncl, long nch);
void free_dmatrix (double **m, long nrl, long nrh, long ncl, long nch);
void free_d3tensor (double ***t, long nrl, long nrh, long ncl, long nch,
		    long ndl, long ndh);

int minimum2 (int i, int j);
int minimum3 (int i, int j, int k);
int maximum2 (int i, int j);
int maximum3 (int i, int j, int k);
int pow_int (int mantisse, int exponent);

void chebft_Zeros (double u[], int n, int inv);
void chebft_Extremes (double u[], int n, int inv);
void chder (double *c, double *cder, int n);
double chebev (double a, double b, double c[], int m, double x);
void fourft (double *u, int N, int inv);
void fourder (double u[], double du[], int N);
void fourder2 (double u[], double d2u[], int N);
double fourev (double *u, int N, double x);

double norm1 (double *v, int n);
double norm2 (double *v, int n);
double scalarproduct (double *v, double *w, int n);

void params_read(char *fname);
void params_write(char * fname);
double params_getd(char * key);
int params_geti(char * key);
void params_set(char * key, double val);
void params_add(char * key, int type, double val);

/* TwoPunctures.c */
static void set_initial_guess(derivs v);
void TwoPunctures (
		   char * inputfile, // file to set input parameters
		   int * imin, int * imax, // min/max indexes of Cartesian grid in the 3 directions
		   int * nxyz, 
		   double * x, double * y, double * z, // Catersian coords
		   double * alp, // lapse
		   double * psi, // conf factor, and drvts:
		   double * psix, double * psiy, double * psiz,
		   double * psixx, double * psixy, double * psixz,
		   double * psiyy, double * psiyz, double * psizz,
		   // metric
		   double * gxx, double * gxy, double * gxz,
		   double * gyy, double * gyz, double * gzz,
		   // curv
		   double * kxx, double * kxy, double * kxz,
		   double * kyy, double * kyz, double * kzz
		   );

/* TP_FuncAndJacobian.c */
int Index (int ivar, int i, int j, int k, int nvar, int n1, int n2, int n3);
void allocate_derivs (derivs * v, int n);
void free_derivs (derivs * v, int n);
void Derivatives_AB3 (int nvar, int n1, int n2, int n3, derivs v);
void F_of_v (int nvar, int n1, int n2, int n3, derivs v,
	     double *F, derivs u);
void J_times_dv (int nvar, int n1, int n2, int n3, derivs dv,
		 double *Jdv, derivs u);
void JFD_times_dv (int i, int j, int k, int nvar, int n1,
		   int n2, int n3, derivs dv, derivs u, double *values);
void SetMatrix_JFD (int nvar, int n1, int n2, int n3,
		    derivs u, int *ncols, int **cols, double **Matrix);
double PunctEvalAtArbitPosition (double *v, int ivar, double A, double B, double phi,
				 int nvar, int n1, int n2, int n3);
void calculate_derivs (int i, int j, int k, int ivar, int nvar, int n1,
		       int n2, int n3, derivs v, derivs vv);
double interpol (double a, double b, double c, derivs v);
double PunctTaylorExpandAtArbitPosition (int ivar, int nvar, int n1,
                                         int n2, int n3, derivs v, double x,
                                         double y, double z);
double PunctIntPolAtArbitPosition (int ivar, int nvar, int n1,
				   int n2, int n3, derivs v, double x,
				   double y, double z);
void SpecCoef(int n1, int n2, int n3, int ivar, double *v, double *cf);
double PunctEvalAtArbitPositionFast (double *v, int ivar, double A, double B, double phi,
				     int nvar, int n1, int n2, int n3);
double PunctIntPolAtArbitPositionFast (int ivar, int nvar, int n1,
				       int n2, int n3, derivs v, double x,
				       double y, double z);

/* TP_CoordTransf.c */
void AB_To_XR (int nvar, double A, double B, double *X,
	       double *R, derivs U);
void C_To_c (int nvar, double X, double R, double *x,
	     double *r, derivs U);
void rx3_To_xyz (int nvar, double x, double r, double phi, double *y,
		 double *z, derivs U);

/* TP_Equations.c */
double BY_KKofxyz (double x, double y, double z);
void BY_Aijofxyz (double x, double y, double z, double Aij[3][3]);
void NonLinEquations (double rho_adm,
		      double A, double B, double X, double R,
		      double x, double r, double phi,
		      double y, double z, derivs U, double *values);
void LinEquations (double A, double B, double X, double R,
		   double x, double r, double phi,
		   double y, double z, derivs dU, derivs U, double *values);

/* TP_Newton.c */
void TestRelax (int nvar, int n1, int n2, int n3, derivs v, double *dv);
void Newton (int nvar, int n1, int n2, int n3, derivs v,
	     double tol, int itmax);

// -------------------------------------------------------------------------
#endif /* TP_HEADER_H */
