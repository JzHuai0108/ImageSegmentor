
#define WINDOW_TITLE "MRF Color Image Segmentation Demo $Revision: 1.1 $"
#define VERSION      "MRF Color Image Segmentation Demo $Revision: 1.1 $" \
  " (Last built " __DATE__" "__TIME__") "
#define COPYRIGHT    "(c) 2006 by Mihaly Gara, Csaba Gradwohl & Zoltan Kato" \
  " (SZTE - Hungary)"


#include "cv.h"
#include "highgui.h"
#include "CKProcessTimeCounter.h"
static CKProcessTimeCounter timer("core"); // CPU timer
static bool timer_valid = false;
/* ColorMRF class: it handles all image operations such as
 * loading, saving, etc... 
 */
class ColorMRF
{
public:
	void SetImage(int, int, unsigned char*data=NULL);
  ColorMRF();    // constructor
  ~ColorMRF();
  unsigned char *GetOrigImage();
  unsigned char *GetLImage();
  unsigned char *GetUImage();
  unsigned char *GetVImage();
  bool IsOutput();			// TRUE if  out_image <> NULL
  void SetNoRegions(int n);	      	// sets the number of regions,
					// allocates/frees memory for
					// mean vectors and covariance matrices
  int GetNoRegions() { return no_regions; }
  void SetBeta(double b) { beta = b; }
  void SetT(double x) { t = x; }
  void SetT0(double t) { T0 = t; }
  void SetC(double x) { c = x; }
  void SetAlpha(double x) { alpha = x; }
  void SetLuv();		    // Luv settings
  int GetK() { return K; }
  double GetT() { return T; }
  double GetE() { return E; }
  double GetTimer() { return (timer_valid? timer.GetElapsedTimeMs() : 0.0); }
	void SetMeanVar(int region, double**);

  void CalculateMeanAndCovariance(int *region);// computes mean and
					      // covariance of the given region.
  double CalculateEnergy();                   // computes global energy
					      // based on the current
					      // lableing in data
  double LocalEnergy(int i, int j, int label);// computes the local
					      // energy at site (i,j)
					      // assuming "label" has
					      // been assigned to it.

  void Metropolis(bool mmd=false);  // executes Metropolis or MMD (if mmd=true)
  void ICM();			    // executes ICM
  void Gibbs();			    // executes Gibbs sampler
unsigned char *in_data, *out_data;    // input & output images. in_image contains
private:
 
 
  unsigned char  *l_data;		    // L* component
  unsigned char  *u_data;		    // u* component
  unsigned char  *v_data;		    // v* component
  int width, height;		    // width and height of the
				    // displayed image
  int no_regions;	            // number of regions for Gaussian
				    // parameter computation
  int *out_regions;                 // display color of each label (=mean color)
  double beta;                      // strength of second order clique potential
  double t;			    // Stop criteraia threshold: stop
				    // if (deltaE < t)
  double T0;		            // Initial temperature (not used by ICM)

  double c;			    // Temperature scheduler's factor:
				    // T(n+1)=c*T(n).
  double alpha;		            // alpha value for MMD
  double **mean;			    // computed mean values and
  double **variance;		    // variances and 
  double **covariance;		    // covariances for each region
  double **invcov;		    // inverse covariance matrix
  double *denom;                   // denominator for inverse covariance
  double E;			    // current global energy
  double E_old;			    // global energy in the prvious iteration
  double T;			    // current temperature
  int K;			    // current iteration #
  int **classes;		    // this is the labeled image
  double ***in_image_data;	    // Input image (in RGB color space)

  void InitOutImage();

  unsigned char *scale(double *luv_vector); // scaling into [0,255]
  double *LuvToRGB(double *luv_pixel);// convert a pixel from CIE-L*u*v* to RGB

  void CreateOutput();	           // creates and draws the output
				   // image based on the current labeling
  double Singleton(int i, int j, int label); // computes singleton
					     // potential at site
					     // (i,j) having a label "label"
  double Doubleton(int i, int j, int label); // computes doubleton
					     // potential at site
					     // (i,j) having a label "label"
};

