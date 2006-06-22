/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __ML_H__
#define __ML_H__

// disable deprecation warning which appears in VisualStudio 8.0
#if _MSC_VER >= 1400
#pragma warning( disable : 4996 ) 
#endif

#include <cxcore.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*                               Main struct definitions                                  *
\****************************************************************************************/

/* log(2*PI) */
#define CV_LOG2PI (1.8378770664093454835606594728112)

/* columns of <trainData> matrix are training samples */
#define CV_COL_SAMPLE 0

/* rows of <trainData> matrix are training samples */
#define CV_ROW_SAMPLE 1

#define CV_IS_ROW_SAMPLE(flags) ((flags) & CV_ROW_SAMPLE)

struct CvVectors
{
    int type;
    int dims, count;
    CvVectors* next;
    union
    {
        uchar** ptr;
        float** fl;
        double** db;
    } data;
};


#define CV_STAT_MODEL_PARAM_FIELDS() \
    int flags

typedef struct CvStatModelParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
} CvStatModelParams;


struct CvStatModel;
typedef struct CvStatModelParams CvClassifierTrainParams;

typedef float (CV_CDECL *CvStatModelPredict)
    ( const struct CvStatModel* model, const CvMat* sample, CvMat* probs /* =0 */ );

typedef void (CV_CDECL *CvStatModelUpdate)
    ( struct CvStatModel* model, const CvMat* features, int sample_t_flag,
      const CvMat* responses, const CvStatModelParams* params,
      const CvMat* comp_idx /* =0 */,
      const CvMat* sample_idx /* =0 */,
      const CvMat* type_mask /* =0 */,
      const CvMat* missing_value_mask /* =0 */ );

typedef void (CV_CDECL *CvStatModelRelease)( struct CvStatModel** model );

typedef void* (CV_CDECL *CvStatModelGetObject)
    ( struct CvStatModel* model, const void* obj, int objType,
      int objIndex /* =-1 */);

typedef double (CV_CDECL *CvStatModelGetParamValue)
    ( struct CvStatModel* model, const void* obj, int paramType,
      int paramIndex /* =-1 */);

typedef void (CV_CDECL *CvStatModelSetParamValue)
    ( struct CvStatModel* model, const void* obj, int paramType,
      int paramIndex /* =-1 */,
      double paramValue /* =0 */);

typedef void (CV_CDECL *CvStatModelGetParamMat)
    ( struct CvStatModel* model, const void* obj, int paramType,
      int paramIndex, /* =-1 */
      CvMat* paramMat /* =NULL */);

typedef void (CV_CDECL *CvStatModelSetParamMat)
    ( struct CvStatModel* model, const void* obj, int paramType,
      int paramIndex, /* =-1 */
      CvMat* paramMat /* =NULL */);

#define CV_STAT_MODEL_FIELDS()                  \
    int   flags;                                \
    int   header_size;                          \
    CvStatModelRelease          release;        \
    CvStatModelPredict          predict;        \
    CvStatModelUpdate           update;         \
    CvStatModelGetObject        get_object;     \
    CvStatModelGetParamValue    get_value;      \
    CvStatModelSetParamValue    set_value;      \
    CvStatModelGetParamMat      get_mat;        \
    CvStatModelSetParamMat      set_mat


typedef struct CvStatModel
{
    CV_STAT_MODEL_FIELDS();
} CvStatModel;

typedef CvStatModel CvClassifier; /* for compatibility */

/* A structure, representing the lattice range of statmodel parameters.
   It is used for optimizing statmodel parameters by cross-validation method.
   The lattice is logarithmic, so <step> must be greater then 1. */
typedef struct CvParamLattice
{
    double min_val;
    double max_val;
    double step;
}
CvParamLattice;

CV_INLINE CvParamLattice cvParamLattice( double min_val, double max_val,
                                         double log_step )
{
    CvParamLattice pl;
    pl.min_val = MIN( min_val, max_val );
    pl.max_val = MAX( min_val, max_val );
    pl.step = MAX( log_step, 1. );
    return pl;
}

CV_INLINE CvParamLattice cvDefaultParamLattice( void )
{
    CvParamLattice pl = {0,0,0};
    return pl;
}

/* RTTI */
#define CV_STAT_MODEL_MAGIC_VAL 0x77440000
#define CV_CLASSIFIER_MAGIC_VAL CV_STAT_MODEL_MAGIC_VAL

/* Variable type */
#define CV_VAR_NUMERICAL    0
#define CV_VAR_ORDERED      0
#define CV_VAR_CATEGORICAL  1

#define CV_IS_STAT_MODEL( model ) ((model)!=0 && \
    (((CvStatModel*)(model))->flags & CV_MAGIC_MASK) == CV_STAT_MODEL_MAGIC_VAL)

#define CV_IS_CLASSIFIER  CV_IS_STAT_MODEL

#define CV_IS_CERTAIN_STAT_MODEL( model, model_id ) ((model) != 0 && \
    ((CvStatModel*)(model))->flags == ((model_id) | CV_STAT_MODEL_MAGIC_VAL))

/* flag values for classifier consturctor <flags> parameter */
#define CV_SVM_MAGIC_VAL            0x0000FF01
#define CV_TYPE_NAME_ML_SVM         "opencv-ml-svm"

#define CV_KNN_MAGIC_VAL            0x0000FF02
#define CV_TYPE_NAME_ML_KNN         "opencv-ml-knn"

#define CV_NBAYES_MAGIC_VAL         0x0000FF03
#define CV_TYPE_NAME_ML_NBAYES      "opencv-ml-bayesian"
#define CV_IS_NBAYES(model)         CV_IS_CERTAIN_STAT_MODEL(model,CV_NBAYES_MAGIC_VAL)

#define CV_EM_MAGIC_VAL             0x0000FF04
#define CV_TYPE_NAME_ML_EM          "opencv-ml-em"
#define CV_IS_EM(model)             CV_IS_CERTAIN_STAT_MODEL(model,CV_EM_MAGIC_VAL)

#define CV_BOOST_TREE_MAGIC_VAL     0x0000FF05
#define CV_TYPE_NAME_ML_BOOSTING    "opencv-ml-boost-tree"
#define CV_IS_BOOST_TREE(model)     CV_IS_CERTAIN_STAT_MODEL(model,CV_BOOST_TREE_MAGIC_VAL)

#define CV_TREE_MAGIC_VAL           0x0000FF06
#define CV_TYPE_NAME_ML_TREE        "opencv-ml-tree"

#define CV_ANN_MLP_MAGIC_VAL        0x0000FF07
#define CV_TYPE_NAME_ML_ANN_MLP     "opencv-ml-ann-mlp"

#define CV_CNN_MAGIC_VAL            0x0000FF08
#define CV_TYPE_NAME_ML_CNN         "opencv-ml-cnn"
#define CV_IS_CNN(model)            CV_IS_CERTAIN_STAT_MODEL(model,CV_CNN_MAGIC_VAL)

#define CV_RTREES_MAGIC_VAL         0x0000FF09
#define CV_TYPE_NAME_ML_RTREES      "opencv-ml-random-trees"
#define CV_IS_RTREES(model)         CV_IS_CERTAIN_STAT_MODEL(model,CV_RTREES_MAGIC_VAL)

#define CV_CROSSVAL_MAGIC_VAL       0x0000FF10
#define CV_TYPE_NAME_ML_CROSSVAL    "opencv-ml-cross-validation"
#define CV_IS_CROSSVAL(model)       CV_IS_CERTAIN_STAT_MODEL(model,CV_CROSSVAL_MAGIC_VAL)

/****************************************************************************************\
*                           Generic Functions for Statistical Models                     *
\****************************************************************************************/

/* releases stat model */
CV_INLINE void cvReleaseStatModel( CvStatModel** stat_model )
{
    if( stat_model && CV_IS_STAT_MODEL(*stat_model) && (*stat_model)->release )
        (*stat_model)->release( stat_model );
    else
        cvRelease( (void**)stat_model );
}

/* loads stat model from file */
CV_INLINE CvStatModel* cvLoadStatModel( const char* filename,
                                        const char* name CV_DEFAULT(NULL),
                                        const char** real_name CV_DEFAULT(NULL) )
{
    return  (CvStatModel*)cvLoad( filename, 0, name, real_name );
}

/* saves stat model to file */
CV_INLINE void cvSaveStatModel( const char* filename, const CvStatModel* stat_model,
                                const char* name CV_DEFAULT(NULL),
                                const char* comment CV_DEFAULT(NULL),
                                CvAttrList attributes CV_DEFAULT(cvAttrList()))
{
    cvSave( filename, stat_model, name, comment, attributes );
}


/* makes a prediction using a stat model */
CV_INLINE float cvStatModelPredict( const CvStatModel* stat_model,
                                    const CvMat* input, CvMat* probs CV_DEFAULT(NULL))
{
    if( CV_IS_STAT_MODEL(stat_model) && stat_model->predict )
        return stat_model->predict( stat_model, input, probs );
    else
        cvError( CV_StsNotImplemented, "cvStatModelPredict",
                 "The stat model does not have \"predict\" method", __FILE__, __LINE__ );
    return 0;
}

/* makes a prediction at multiple points using a stat model */
CVAPI(void) cvStatModelMultiPredict( const CvStatModel* stat_model,
                                    const CvArr* predict_input, int flags,
                                    CvMat* predict_output,
                                    CvMat* probs CV_DEFAULT(NULL),
                                    const CvMat* sample_idx CV_DEFAULT(NULL) );

/****************************************************************************************\
*                                  Naive Bayes classifier                                *
\****************************************************************************************/

#define CV_NB_CLASSIFIER_FIELDS()   \
    int     dims;                   \
    CvMat*  comp_idx;               \
    CvMat*  cls_labels;             \
    CvMat** count;                  \
    CvMat** sum;                    \
    CvMat** productsum;             \
    CvMat** avg;                    \
    CvMat** inv_eigen_values;       \
    CvMat** cov_rotate_mats;        \
    float*  c

typedef struct CvNBClassifier
{
    CV_STAT_MODEL_FIELDS();
    CV_NB_CLASSIFIER_FIELDS();
} CvNBClassifier;

CVAPI(CvStatModel*) cvCreateNBClassifier(
    const CvMat* train_data,
    int    tflag,
    const CvMat* train_classes,
    const CvStatModelParams* train_params CV_DEFAULT(0),
    const CvMat* comp_idx CV_DEFAULT(0),
    const CvMat* sample_idx CV_DEFAULT(0),
    const CvMat* type_mask CV_DEFAULT(0),
    const CvMat* missed_measurements_mask CV_DEFAULT(0));

/****************************************************************************************\
*                          K-Nearest Neighbour Classifier                                *
\****************************************************************************************/

// k Nearest Neighbors
class CV_EXPORTS CvKNearest
{
public:
    
    CvKNearest();
    virtual ~CvKNearest();

    CvKNearest( const CvMat* _train_data, const CvMat* _responses,
                const CvMat* _sample_idx=0, bool _is_regression=false, int max_k=32 );
        
    virtual bool train( const CvMat* _train_data, const CvMat* _responses,
                        const CvMat* _sample_idx=0, bool is_regression=false,
                        int _max_k=32, bool _update_base=false );

    virtual float find_nearest( const CvMat* _samples, int k, CvMat* results,
        const float** neighbors=0, CvMat* neighbor_responses=0, CvMat* dist=0 ) const;

    virtual void clear();
    int get_max_k() const;
    int get_var_count() const;
    int get_sample_count() const;
    bool is_regression() const;

protected:

    virtual float write_results( int k, int k1, int start, int end,
        const float* neighbor_responses, const float* dist, CvMat* _results,
        CvMat* _neighbor_responses, CvMat* _dist, Cv32suf* sort_buf ) const;

    virtual void find_neighbors_direct( const CvMat* _samples, int k, int start, int end,
        float* neighbor_responses, const float** neighbors, float* dist ) const;

    
    int max_k, var_count;
    int total;
    bool regression;
    CvVectors* samples;
};

/****************************************************************************************\
*                                   Support Vector Machines                              *
\****************************************************************************************/

// SVM training parameters
struct CV_EXPORTS CvSVMParams
{
    CvSVMParams();
    CvSVMParams( int _svm_type, int _kernel_type,
                 double _degree, double _gamma, double _coef0,
                 double _C, double _nu, double _p,
                 CvMat* _class_weights, CvTermCriteria _term_crit );
    
    int         svm_type;
    int         kernel_type;
    double      degree; // for poly
    double      gamma;  // for poly/rbf/sigmoid
    double      coef0;  // for poly/sigmoid

    double      C;  // for CV_SVM_C_SVC, CV_SVM_EPS_SVR and CV_SVM_NU_SVR
    double      nu; // for CV_SVM_NU_SVC, CV_SVM_ONE_CLASS, and CV_SVM_NU_SVR
    double      p; // for CV_SVM_EPS_SVR
    CvMat*      class_weights; // for CV_SVM_C_SVC
    CvTermCriteria term_crit; // termination criteria
};


struct CV_EXPORTS CvSVMKernel
{
    typedef void (CvSVMKernel::*Calc)( int vec_count, int vec_size, const float** vecs,
                                       const float* another, float* results );
    CvSVMKernel();
    CvSVMKernel( const CvSVMParams* _params, Calc _calc_func );
    virtual bool create( const CvSVMParams* _params, Calc _calc_func );
    virtual ~CvSVMKernel();
    
    virtual void clear();
    virtual void calc( int vcount, int n, const float** vecs, const float* another, float* results );

    const CvSVMParams* params;
    Calc calc_func;

    virtual void calc_non_rbf_base( int vec_count, int vec_size, const float** vecs,
                                    const float* another, float* results,
                                    double alpha, double beta );

    virtual void calc_linear( int vec_count, int vec_size, const float** vecs,
                              const float* another, float* results );
    virtual void calc_rbf( int vec_count, int vec_size, const float** vecs,
                           const float* another, float* results );
    virtual void calc_poly( int vec_count, int vec_size, const float** vecs,
                            const float* another, float* results );
    virtual void calc_sigmoid( int vec_count, int vec_size, const float** vecs,
                               const float* another, float* results );
};


struct CvSVMKernelRow
{
    CvSVMKernelRow* prev;
    CvSVMKernelRow* next;
    float* data;
};


struct CvSVMSolutionInfo
{
    double obj;
    double rho;
    double upper_bound_p;
    double upper_bound_n;
    double r;   // for Solver_NU
};

class CV_EXPORTS CvSVMSolver
{
public:
    typedef bool (CvSVMSolver::*SelectWorkingSet)( int& i, int& j );
    typedef float* (CvSVMSolver::*GetRow)( int i, float* row, float* dst, bool existed );
    typedef void (CvSVMSolver::*CalcRho)( double& rho, double& r );
    
    CvSVMSolver();

    CvSVMSolver( int count, int var_count, const float** samples, char* y,
                 int alpha_count, double* alpha, double Cp, double Cn,
                 CvMemStorage* storage, CvSVMKernel* kernel, GetRow get_row,
                 SelectWorkingSet select_working_set, CalcRho calc_rho );
    virtual bool create( int count, int var_count, const float** samples, char* y,
                 int alpha_count, double* alpha, double Cp, double Cn,
                 CvMemStorage* storage, CvSVMKernel* kernel, GetRow get_row,
                 SelectWorkingSet select_working_set, CalcRho calc_rho );
    virtual ~CvSVMSolver();

    virtual void clear();
    virtual bool solve_generic( CvSVMSolutionInfo& si );
    
    virtual bool solve_c_svc( int count, int var_count, const float** samples, char* y,
                              double Cp, double Cn, CvMemStorage* storage,
                              CvSVMKernel* kernel, double* alpha, CvSVMSolutionInfo& si );
    virtual bool solve_nu_svc( int count, int var_count, const float** samples, char* y,
                               CvMemStorage* storage, CvSVMKernel* kernel,
                               double* alpha, CvSVMSolutionInfo& si );
    virtual bool solve_one_class( int count, int var_count, const float** samples,
                                  CvMemStorage* storage, CvSVMKernel* kernel,
                                  double* alpha, CvSVMSolutionInfo& si );

    virtual bool solve_eps_svr( int count, int var_count, const float** samples, const float* y,
                                CvMemStorage* storage, CvSVMKernel* kernel,
                                double* alpha, CvSVMSolutionInfo& si );

    virtual bool solve_nu_svr( int count, int var_count, const float** samples, const float* y,
                               CvMemStorage* storage, CvSVMKernel* kernel,
                               double* alpha, CvSVMSolutionInfo& si );

    virtual float* get_row_base( int i, bool* _existed );
    virtual float* get_row( int i, float* dst );

    int sample_count;
    int var_count;
    int cache_size;
    int cache_line_size;
    const float** samples;
    const CvSVMParams* params;
    CvMemStorage* storage;
    CvSVMKernelRow lru_list;
    CvSVMKernelRow* rows;

    int alpha_count;

    double* G;
    double* alpha;

    // -1 - lower bound, 0 - free, 1 - upper bound
    char* alpha_status;

    char* y;
    double* b;
    float* buf[2];
    double eps;
    int max_iter;
    double C[2];  // C[0] == Cn, C[1] == Cp
    CvSVMKernel* kernel;
    
    SelectWorkingSet select_working_set_func;
    CalcRho calc_rho_func;
    GetRow get_row_func;

    virtual bool select_working_set( int& i, int& j );
    virtual bool select_working_set_nu_svm( int& i, int& j );
    virtual void calc_rho( double& rho, double& r );
    virtual void calc_rho_nu_svm( double& rho, double& r );

    virtual float* get_row_svc( int i, float* row, float* dst, bool existed );
    virtual float* get_row_one_class( int i, float* row, float* dst, bool existed );
    virtual float* get_row_svr( int i, float* row, float* dst, bool existed );
};


struct CvSVMDecisionFunc
{
    double rho;
    int sv_count;
    double* alpha;
    int* sv_index;
};


// SVM model
class CV_EXPORTS CvSVM
{
public:
    // SVM type
    enum { C_SVC=100, NU_SVC=101, ONE_CLASS=102, EPS_SVR=103, NU_SVR=104 };

    // SVM kernel type
    enum { LINEAR=0, POLY=1, RBF=2, SIGMOID=3 };

    CvSVM();
    virtual ~CvSVM();

    CvSVM( const CvMat* _train_data, const CvMat* _responses,
           const CvMat* _var_idx=0, const CvMat* _sample_idx=0,
           CvSVMParams _params=CvSVMParams() );
        
    virtual bool train( const CvMat* _train_data, const CvMat* _responses,
                        const CvMat* _var_idx=0, const CvMat* _sample_idx=0,
                        CvSVMParams _params=CvSVMParams() );

    virtual float predict( const CvMat* _sample ) const;
    virtual int get_support_vector_count() const;
    virtual const float* get_support_vector(int i) const;
    virtual void clear();

    virtual void save( const char* filename, const char* name=0 );
    virtual void load( const char* filename, const char* name=0 );
    
    virtual void write( CvFileStorage* storage, const char* name );
    virtual void read( CvFileStorage* storage, CvFileNode* node );
    int get_var_count() const { return var_idx ? var_idx->cols : var_all; }

protected:

    virtual bool set_params( const CvSVMParams& _params );
    virtual bool train1( int sample_count, int var_count, const float** samples,
                    const void* _responses, double Cp, double Cn,
                    CvMemStorage* _storage, double* alpha, double& rho );
    virtual void create_kernel();
    virtual void create_solver();

    virtual void write_params( CvFileStorage* fs );
    virtual void read_params( CvFileStorage* fs, CvFileNode* node );

    CvSVMParams params;
    CvMat* class_labels;
    int var_all;
    float** sv;
    int sv_total;
    CvMat* var_idx;
    CvMat* class_weights;
    CvSVMDecisionFunc* decision_func;
    CvMemStorage* storage;

    CvSVMSolver* solver;
    CvSVMKernel* kernel;
};


/* The function trains SVM model with optimal parameters, obtained by using cross-validation.
The parameters to be estimated should be indicated by setting theirs values to FLT_MAX.
The optimal parameters are saved in <model_params> */
/*CVAPI(CvStatModel*)
cvTrainSVM_CrossValidation( const CvMat* train_data, int tflag,
            const CvMat* responses,
            CvStatModelParams* model_params,
            const CvStatModelParams* cross_valid_params,
            const CvMat* comp_idx   CV_DEFAULT(0),
            const CvMat* sample_idx CV_DEFAULT(0),
            const CvParamLattice* degree_lattice CV_DEFAULT(0),
            const CvParamLattice* gamma_lattice  CV_DEFAULT(0),
            const CvParamLattice* coef0_lattice  CV_DEFAULT(0),
            const CvParamLattice* C_lattice      CV_DEFAULT(0),
            const CvParamLattice* nu_lattice     CV_DEFAULT(0),
            const CvParamLattice* p_lattice      CV_DEFAULT(0) );*/

/****************************************************************************************\
*                                   Boosted trees models                                 *
\****************************************************************************************/

/* Boosting type */
#define CV_BT_DISCRETE   0
#define CV_BT_REAL       1
#define CV_BT_LOGIT      2
#define CV_BT_GENTLE     3

/* Boosted trees training parameters */
typedef struct CvBtClassifierTrainParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    int boost_type;
    int num_iter;
    double infl_trim_rate;
    int num_splits;
}
CvBtClassifierTrainParams;

typedef struct CvTreeBoostClassifier
{
    CV_STAT_MODEL_FIELDS();
    /* number of internal tree nodes (splits) */
    int count;

    /* internal nodes (each is array of <count> elements) */
    int* comp_idx;
    float* threshold;
    int* left;
    int* right;

    /* leaves (array of <count>+1 elements) */
    float* val;
}
CvTreeBoostClassifier;

typedef struct CvBtClassifier
{
    CV_STAT_MODEL_FIELDS();
    CvBtClassifierTrainParams params;    
    CvMat* class_labels;
    int total_features;
    CvSeq* weak; /* weak classifiers (CvTreeBoostClassifier) pointers */
    CvMat* comp_idx;
    void* ts;
}
CvBtClassifier;

CVAPI(CvStatModel*)
cvCreateBtClassifier( CvMat* train_data,
                      int flags,
                      CvMat* responses,
                      CvStatModelParams* train_params CV_DEFAULT(0),
                      CvMat* comp_idx CV_DEFAULT(0),
                      CvMat* sample_idx CV_DEFAULT(0),
                      CvMat* type_mask CV_DEFAULT(0),
                      CvMat* missval_mask CV_DEFAULT(0));


/* updates the model by performing iterations */
CVAPI(void)
cvUpdateBtClassifier( CvBtClassifier* model,
                      CvMat* train_data,
                      int flags,
                      CvMat* responses,
                      CvStatModelParams* train_params CV_DEFAULT(0),
                      CvMat* comp_idx CV_DEFAULT(0),
                      CvMat* sample_idx CV_DEFAULT(0),
                      CvMat* type_mask CV_DEFAULT(0),
                      CvMat* missval_mask CV_DEFAULT(0));

/* evaluation type */
#define CV_BT_VALUE 0
#define CV_BT_INDEX 1

/* evaluates each weak classifier */
CVAPI(float)
cvEvalWeakClassifiers( const CvBtClassifier* bt, const CvMat* sample,
                       CvMat* weak_vals, CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ),
                       int eval_type CV_DEFAULT(CV_BT_VALUE));

/* removes weak classifiers from the model */
CVAPI(void)
cvPruneBtClassifier( CvBtClassifier* bt, CvSlice slice );

/* read/write. iteration step */
#define CV_BT_ITER_STEP         300
/* read only. number of iterations */
#define CV_BT_NUM_ITER          301
/* read/write. 1 by number_of_samples 32fC1 matrix of weights */
#define CV_BT_WEIGHTS           302

/* read only. number of classes. If value is 0 then it is regression tree. */
#define CV_MODEL_CLASS_NUM      2
/* read only. Split rule defined in train parameters structure and used for tree creation. */
#define CV_MODEL_FEATURE_NUM    5


/****************************************************************************************\
*                              Expectation - Maximization                                *
\****************************************************************************************/

/* Covariation matrices are supposed to be diagonal with equal diagonal elements */
#define CV_EM_COV_MAT_SPHERICAL     1
/* Covariation matrices  are supposed to be diagonal with different diagonal elements */
#define CV_EM_COV_MAT_DIAGONAL      2
/* Covariation matrices  are supposed to be general covariation matrices */
#define CV_EM_COV_MAT_GENERAL       3
/* The first steps of the EM algorithm */
#define CV_EM_START_E_STEP      1 // The first step is Expectation
#define CV_EM_START_M_STEP      2 // The first step is Maximization
#define CV_EM_START_AUTO_STEP   3 // Start with k-means

/*
   CvEMStatModelParams
 * nclusters        - number of clusters to cluster samples to.
 * cov_mat_type     - type of covariation matrice: general, diagonal, spherical.
                      Use constants CV_EM_COV_MAT_SPHERICAL, CV_EM_COV_MAT_DIAGONAL,
                      CV_EM_COV_MAT_GENERAL to initialize this field.
 * start_step       - the first step of the EM algorithm. Should be one of the following:
                      CV_EM_START_E_STEP, CV_EM_START_M_STEP, CV_EM_START_AUTO_STEP.
 * probs            - the initial matrice of conditional probabilities (use only in the case
                      CV_EM_START_M_STEP).
 * weights          - initial weights of gauss mixture (may be NULL).
 * means            - initial means of gauss mixture (may be NULL).
 * covs             - initial covs of gauss mixture (may be NULL).
                      <weights>, <means> and <covs> should be used only in the case
                      CV_EM_START_E_STEP.
 * term_crit        - termination criteria of the EM iterative algorithm.
 If <covs> == <weights> == NULL, then initial set of parameters is obtained by using
 k-means. If <means> != NULL, then k-means starts with the initial centers
 equal to <means>.
 */
#define CV_EM_MODEL_PARAM_FIELDS()  \
    int nclusters;                  \
    int cov_mat_type;               \
    int start_step;                 \
    CvMat* probs;                   \
    CvMat* weights;                 \
    CvMat* means;                   \
    CvMat** covs;                   \
    CvTermCriteria term_crit

typedef struct CvEMStatModelParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    CV_EM_MODEL_PARAM_FIELDS();
} CvEMStatModelParams;

/*
   CvEMStatModel
 * dims         - samples' dimension.
 * nclusters    - number of clusters to cluster samples to.
 * cov_mat_type - type of covariation matrice (look CvEMStatModelParams).
 * comp_idx     - vector that contains features' indices to process.
 * means        - calculated by the EM algorithm set of gaussians' means.
 * log_weight_div_det - auxilary vector that k-th component is equal to
                        (-2)*ln(weights_k/det(Sigma_k)^0.5),
                        where <weights_k> is the weight,
                        <Sigma_k> is the covariation matrice of k-th cluster.
 * inv_eigen_values   - set of 1*dims matrices, <inv_eigen_values>[k] contains
                        inversed eigen values of covariation matrice of the k-th cluster.
                        In the case of <cov_mat_type> == CV_EM_COV_MAT_DIAGONAL,
                        inv_eigen_values[k] = Sigma_k^(-1).
 * covs_rotate_mats   - used only if cov_mat_type == CV_EM_COV_MAT_GENERAL, in all the
                        other cases it is NULL. <covs_rotate_mats>[k] is the orthogonal
                        matrice, obtained by the SVD-decomposition of Sigma_k.
   Both <inv_eigen_values> and <covs_rotate_mats> fields are used for representation of
   covariation matrices and simplifying EM calculations.
   For fixed k denote
   u = covs_rotate_mats[k],
   v = inv_eigen_values[k],
   w = v^(-1);
   if <cov_mat_type> == CV_EM_COV_MAT_GENERAL, then Sigma_k = u w u',
   else                                             Sigma_k = w.
   Symbol ' means transposition.
 */
typedef struct CvEMStatModel
{
    CV_STAT_MODEL_FIELDS();
    int dims;
    int nclusters;
    int cov_mat_type;
    CvMat* comp_idx;
    CvMat* log_weight_div_det;
    CvMat* means;
    CvMat** inv_eigen_values;
    CvMat** cov_rotate_mats;
} CvEMStatModel;

CVAPI(void) cvEM( const CvMat* samples,
                  int tflag, CvMat* labels,
                  const CvEMStatModelParams* params,
                  const CvMat*  comp_idx CV_DEFAULT(0),
                  const CvMat*  sample_idx CV_DEFAULT(0),
                  CvMat*  probs CV_DEFAULT(0),
                  CvMat*  means CV_DEFAULT(0),
                  CvMat*  weights CV_DEFAULT(0),
                  CvMat** covs CV_DEFAULT(0),
                  CvEMStatModel** em_model CV_DEFAULT(0) );

/****************************************************************************************\
*                                      Decision Tree                                     *
\****************************************************************************************/

struct CvPair32s32f
{
    int i;
    float val;
};

struct CvDTreeSplit
{
    int var_idx;
    int inversed;
    float quality;
    CvDTreeSplit* next;
    union
    {
        int subset[2];
        struct
        {
            float c;
            int split_point;
        }
        ord;
    };
};


struct CvDTreeNode
{
    int sample_count;
    int depth;
    double value;
    int class_idx;

    CvDTreeNode* parent;
    CvDTreeNode* left;
    CvDTreeNode* right;

    int* num_valid;
    int offset;
    int buf_idx;
    double maxlr;

    // global pruning data
    int Tn, complexity;
    double alpha;
    double node_risk, tree_risk, tree_error;

    // cross-validation pruning data
    int* cv_Tn;
    double* cv_node_risk;
    double* cv_node_error;

    CvDTreeSplit* split;
};


struct CV_EXPORTS CvDTreeParams
{
    int max_categories;
    int max_depth;
    int min_sample_count;
    int cv_folds;
    bool use_surrogates;
    bool use_1se_rule;
    bool truncate_pruned_tree;
    float regression_accuracy;
    const float* priors;

    CvDTreeParams() : max_categories(10), max_depth(INT_MAX), min_sample_count(10),
        cv_folds(10), use_surrogates(true), use_1se_rule(true),
        truncate_pruned_tree(true), regression_accuracy(0.01f), priors(0)
    {}

    CvDTreeParams( int _max_depth, int _min_sample_count,
                   float _regression_accuracy, bool _use_surrogates,
                   int _max_categories, int _cv_folds,
                   bool _use_1se_rule, bool _truncate_pruned_tree,
                   const float* _priors ) :
        max_categories(_max_categories), max_depth(_max_depth),
        min_sample_count(_min_sample_count), use_surrogates(_use_surrogates),
        cv_folds(_cv_folds), use_1se_rule(_use_1se_rule), 
        truncate_pruned_tree(_truncate_pruned_tree),
        regression_accuracy(_regression_accuracy),
        priors(_priors)
    {}
};


struct CV_EXPORTS CvDTreeTrainData
{
    CvDTreeTrainData();
    CvDTreeTrainData( const CvMat* _train_data, int _tflag,
                      const CvMat* _responses, const CvMat* _var_idx=0,
                      const CvMat* _sample_idx=0, const CvMat* _var_type=0,
                      const CvMat* _missing_mask=0,
                      CvDTreeParams _params=CvDTreeParams(),
                      bool _shared=false );
    virtual ~CvDTreeTrainData();

    virtual void set_data( const CvMat* _train_data, int _tflag,
                          const CvMat* _responses, const CvMat* _var_idx=0,
                          const CvMat* _sample_idx=0, const CvMat* _var_type=0,
                          const CvMat* _missing_mask=0,
                          CvDTreeParams _params=CvDTreeParams(),
                          bool _shared=false );

    virtual void get_vectors( const CvMat* _subsample_idx,
         float* values, uchar* missing, float* responses, bool get_class_idx=false );

    virtual CvDTreeNode* subsample_data( const CvMat* _subsample_idx );

    // release all the data
    virtual void clear();

    int get_num_classes() const;
    int get_var_type(int vi) const;

    virtual int* get_class_labels( CvDTreeNode* n );
    virtual float* get_ord_responses( CvDTreeNode* n );
    virtual int* get_cv_labels( CvDTreeNode* n );
    virtual int* get_cat_var_data( CvDTreeNode* n, int vi );
    virtual CvPair32s32f* get_ord_var_data( CvDTreeNode* n, int vi );
    virtual int get_child_buf_idx( CvDTreeNode* n );

    ////////////////////////////////////

    virtual bool set_params( const CvDTreeParams& params );
    virtual CvDTreeNode* new_node( CvDTreeNode* parent, int count,
                                   int storage_idx, int offset );

    virtual CvDTreeSplit* new_split_ord( int vi, float cmp_val,
                int split_point, int inversed, float quality );
    virtual CvDTreeSplit* new_split_cat( int vi, float quality );
    virtual void free_node_data( CvDTreeNode* node );
    virtual void free_train_data();
    virtual void free_node( CvDTreeNode* node );

    int sample_count, var_all, var_count, max_c_count;
    int ord_var_count, cat_var_count;
    bool have_cv_labels, have_priors;
    bool is_classifier;

    int buf_count, buf_size;
    bool shared;

    CvMat* cat_count;
    CvMat* cat_ofs;
    CvMat* cat_map;

    CvMat* counts;
    CvMat* buf;
    CvMat* direction;
    CvMat* split_buf;

    CvMat* var_idx;
    CvMat* var_type; // i-th element =
                     //   k<0  - ordered
                     //   k>=0 - categorical, see k-th element of cat_* arrays
    CvMat* priors;

    CvDTreeParams params;

    CvMemStorage* tree_storage;
    CvMemStorage* temp_storage;

    CvDTreeNode* data_root;

    CvSet* node_heap;
    CvSet* split_heap;
    CvSet* cv_heap;
    CvSet* nv_heap;

    CvRNG rng;
};


class CV_EXPORTS CvDTree
{
public:
    CvDTree();
    virtual ~CvDTree();

    virtual bool train( const CvMat* _train_data, int _tflag,
                        const CvMat* _responses, const CvMat* _var_idx=0,
                        const CvMat* _sample_idx=0, const CvMat* _var_type=0,
                        const CvMat* _missing_mask=0,
                        CvDTreeParams params=CvDTreeParams() );

    virtual bool train( CvDTreeTrainData* _train_data, const CvMat* _subsample_idx );

    virtual CvDTreeNode* predict( const CvMat* _sample, const CvMat* _missing_data_mask=0,
                                  bool preprocessed_input=false ) const;
    virtual const CvMat* get_var_importance();
    virtual void clear();

    virtual void save( const char* filename, const char* name=0 );
    virtual void write( CvFileStorage* fs, const char* name );
    virtual void load( const char* filename, const char* name=0 );
    virtual void read( CvFileStorage* fs, CvFileNode* node );

protected:

    virtual bool do_train( const CvMat* _subsample_idx );

    virtual void try_split_node( CvDTreeNode* n );
    virtual void split_node_data( CvDTreeNode* n );
    virtual CvDTreeSplit* find_best_split( CvDTreeNode* n );
    virtual CvDTreeSplit* find_split_ord_gini( CvDTreeNode* n, int vi );
    virtual CvDTreeSplit* find_split_cat_gini( CvDTreeNode* n, int vi );
    virtual CvDTreeSplit* find_split_ord_reg( CvDTreeNode* n, int vi );
    virtual CvDTreeSplit* find_split_cat_reg( CvDTreeNode* n, int vi );
    virtual CvDTreeSplit* find_surrogate_split_ord( CvDTreeNode* n, int vi );
    virtual CvDTreeSplit* find_surrogate_split_cat( CvDTreeNode* n, int vi );
    virtual double calc_node_dir( CvDTreeNode* node );
    virtual void cluster_categories( const int* vectors, int vector_count,
        int var_count, int* sums, int k, int* cluster_labels );

    virtual void calc_node_value( CvDTreeNode* node );

    virtual void prune_cv();
    virtual double update_tree_rnc( int T, int fold );
    virtual int cut_tree( int T, int fold, double min_alpha );
    virtual void free_prune_data(bool cut_tree);
    virtual void free_tree();

    virtual void write_train_data_params( CvFileStorage* fs );
    virtual void read_train_data_params( CvFileStorage* fs, CvFileNode* node );
    virtual void write_tree_nodes( CvFileStorage* fs );
    virtual void read_tree_nodes( CvFileStorage* fs, CvFileNode* node );
    virtual void write_node( CvFileStorage* fs, CvDTreeNode* node );
    virtual void write_split( CvFileStorage* fs, CvDTreeSplit* split );
    virtual CvDTreeNode* read_node( CvFileStorage* fs, CvFileNode* node, CvDTreeNode* parent );
    virtual CvDTreeSplit* read_split( CvFileStorage* fs, CvFileNode* node );

    CvDTreeNode* root;
    
    int pruned_tree_idx;
    CvMat* var_importance;

    CvDTreeTrainData* data;
};


/****************************************************************************************\
*                                   Random Trees Classifier                              *
\****************************************************************************************/

class CvRTrees;

class CV_EXPORTS CvForestTree: public CvDTree
{
public:

    CvForestTree();
    virtual ~CvForestTree();
    virtual bool train( CvDTreeTrainData* _train_data, const CvMat* _subsample_idx, CvRTrees* forest );
    virtual CvDTreeSplit* find_best_split( CvDTreeNode* n );

    CvRTrees* forest;
};


struct CV_EXPORTS CvRTParams
{
    // Parameters for a tree
    int max_categories;
    int max_depth;
    int min_sample_count;
    int cv_folds;
    bool use_surrogates;
    bool use_1se_rule;
    float regression_accuracy;
    const float* priors;

    //Parameters for the forest
    bool calc_var_importance; // true <=> RF processes variable importance
    bool calc_proximities;    // true <=> RF processes proximities
    int nactive_vars;
    CvTermCriteria term_crit;

    CvRTParams() : max_categories(10), max_depth(INT_MAX), min_sample_count(10),
        cv_folds(10), use_surrogates(true), use_1se_rule(true),
        regression_accuracy(0.01f), priors(0),
        calc_var_importance(false), calc_proximities(false), nactive_vars(0)
    {
        term_crit.epsilon = 0.1;
        term_crit.max_iter = 50;
        term_crit.type = CV_TERMCRIT_ITER+CV_TERMCRIT_EPS;
    }

    CvRTParams( int _max_depth, int _min_sample_count,
                float _regression_accuracy, bool _use_surrogates,
                int _max_categories, int _cv_folds,
                bool _use_1se_rule,  const float* _priors,
                bool _calc_var_importance, bool _calc_proximities,
                int _nactive_vars, int max_num_of_trees_in_the_forest,
                float forest_accuracy, int termcrit_type ) :
        max_categories(_max_categories), max_depth(_max_depth),
        min_sample_count(_min_sample_count), use_surrogates(_use_surrogates),
        cv_folds(_cv_folds), use_1se_rule(_use_1se_rule),
        regression_accuracy(_regression_accuracy), priors(_priors),
        calc_var_importance(_calc_var_importance), calc_proximities(_calc_proximities),
        nactive_vars(_nactive_vars)
    {
        term_crit.epsilon  = forest_accuracy;
        term_crit.max_iter = max_num_of_trees_in_the_forest;
        term_crit.type     = termcrit_type;
    }
};

class CV_EXPORTS CvRTrees
{
public:
    CvRTrees();
    virtual ~CvRTrees();
    virtual bool train( const CvMat* _train_data, int _tflag,
                        const CvMat* _responses, const CvMat* _var_idx=0,
                        const CvMat* _sample_idx=0, const CvMat* _var_type=0,
                        const CvMat* _missing_mask=0,
                        CvRTParams params=CvRTParams() );
    virtual double predict( const CvMat* _sample, CvMat* probs = 0 ) const;

    virtual inline const CvMat* get_var_importance() const;
    virtual float get_proximity( int i, int j ) const;

    virtual void clear();

    virtual void save( const char* filename, const char* name=0 );
    virtual void write( CvFileStorage* fs, const char* name );
    virtual void load( const char* filename, const char* name=0 );
    virtual void read( CvFileStorage* fs, CvFileNode* node );

    CvRNG rng;
    CvMat* active_var_mask;
    int ntrees;

protected:
    bool grow_forest( CvDTreeTrainData* train_data, const CvTermCriteria term_crit );

    // array of the trees of the forest
    CvForestTree** trees;

    int nclasses;
    double oob_error;
    CvMat* var_importance;
    CvMat* proximities;
    int nsamples;
};

/****************************************************************************************\
*                              Artificial Neural Networks (ANN)                          *
\****************************************************************************************/

/////////////////////////////////// Multi-Layer Perceptrons //////////////////////////////

struct CV_EXPORTS CvANN_MLP_TrainParams
{
    CvANN_MLP_TrainParams();
    CvANN_MLP_TrainParams( CvTermCriteria term_crit, int train_method,
                           double param1, double param2=0 );
    ~CvANN_MLP_TrainParams();

    enum { BACKPROP=0, RPROP=1 };

    CvTermCriteria term_crit;
    int train_method;

    // backpropagation parameters
    double bp_dw_scale, bp_moment_scale;
    
    // rprop parameters
    double rp_dw0, rp_dw_plus, rp_dw_minus, rp_dw_min, rp_dw_max;
};


class CV_EXPORTS CvANN_MLP
{
public:
    CvANN_MLP();
    CvANN_MLP( const CvMat* _layer_sizes,
               int _activ_func=SIGMOID_SYM,
               double _f_param1=0, double _f_param2=0 );

    virtual ~CvANN_MLP();

    virtual void create( const CvMat* _layer_sizes,
                         int _activ_func=SIGMOID_SYM,
                         double _f_param1=0, double _f_param2=0 );

    virtual int train( const CvMat* _inputs, const CvMat* _outputs,
                       const CvMat* _sample_weights, const CvMat* _sample_idx=0,
                       CvANN_MLP_TrainParams _params = CvANN_MLP_TrainParams(),
                       int flags=0 );
    virtual void predict( const CvMat* _inputs,
                          CvMat* _outputs ) const;

    virtual void clear();

    // possible activation functions
    enum { IDENTITY = 0, SIGMOID_SYM = 1, GAUSSIAN = 2 };

    // available training flags
    enum { UPDATE_WEIGHTS = 1, NO_INPUT_SCALE = 2, NO_OUTPUT_SCALE = 4 };

    virtual void save( const char* filename, const char* name=0 );
    virtual void write( CvFileStorage* fs, const char* name );
    virtual void load( const char* filename, const char* name=0 );
    virtual void read( CvFileStorage* fs, CvFileNode* node );
    int get_layer_count() { return layer_sizes ? layer_sizes->cols : 0; }
    const CvMat* get_layer_sizes() { return layer_sizes; }

protected:

    virtual bool prepare_to_train( const CvMat* _inputs, const CvMat* _outputs,
            const CvMat* _sample_weights, const CvMat* _sample_idx,
            CvANN_MLP_TrainParams _params,
            CvVectors* _ivecs, CvVectors* _ovecs, double** _sw, int _flags );

    // sequential random backpropagation
    virtual int train_backprop( CvVectors _ivecs, CvVectors _ovecs, const double* _sw );
    
    // RPROP algorithm
    virtual int train_rprop( CvVectors _ivecs, CvVectors _ovecs, const double* _sw );

    virtual void calc_activ_func( CvMat* xf, const double* bias ) const;
    virtual void calc_activ_func_deriv( CvMat* xf, CvMat* deriv, const double* bias ) const;
    virtual void set_activ_func( int _activ_func=SIGMOID_SYM,
                                 double _f_param1=0, double _f_param2=0 );
    virtual void init_weights();
    virtual void scale_input( const CvMat* _src, CvMat* _dst ) const;
    virtual void scale_output( const CvMat* _src, CvMat* _dst ) const;
    virtual void calc_input_scale( const CvVectors* vecs, int flags );
    virtual void calc_output_scale( const CvVectors* vecs, int flags );

    virtual void write_params( CvFileStorage* fs );
    virtual void read_params( CvFileStorage* fs, CvFileNode* node );

    CvMat* layer_sizes;
    CvMat* wbuf;
    CvMat* sample_weights;
    double** weights;
    double f_param1, f_param2;
    double min_val, max_val, min_val1, max_val1;
    int activ_func;
    int max_count, max_buf_sz;
    CvANN_MLP_TrainParams params;
    CvRNG rng;
};


/****************************************************************************************\
*                            Convolutional Neural Network                                *
\****************************************************************************************/
typedef struct CvCNNLayer CvCNNLayer;
typedef struct CvCNNetwork CvCNNetwork;

#define CV_CNN_LEARN_RATE_DECREASE_HYPERBOLICALLY  1
#define CV_CNN_LEARN_RATE_DECREASE_SQRT_INV        2
#define CV_CNN_LEARN_RATE_DECREASE_LOG_INV         3

#define CV_CNN_GRAD_ESTIM_RANDOM        0
#define CV_CNN_GRAD_ESTIM_BY_WORST_IMG  1

#define ICV_CNN_LAYER                0x55550000
#define ICV_CNN_CONVOLUTION_LAYER    0x00001111
#define ICV_CNN_SUBSAMPLING_LAYER    0x00002222
#define ICV_CNN_FULLCONNECT_LAYER    0x00003333

#define ICV_IS_CNN_LAYER( layer )                                          \
    ( ((layer) != NULL) && ((((CvCNNLayer*)(layer))->flags & CV_MAGIC_MASK)\
        == ICV_CNN_LAYER ))

#define ICV_IS_CNN_CONVOLUTION_LAYER( layer )                              \
    ( (ICV_IS_CNN_LAYER( layer )) && (((CvCNNLayer*) (layer))->flags       \
        & ~CV_MAGIC_MASK) == ICV_CNN_CONVOLUTION_LAYER )

#define ICV_IS_CNN_SUBSAMPLING_LAYER( layer )                              \
    ( (ICV_IS_CNN_LAYER( layer )) && (((CvCNNLayer*) (layer))->flags       \
        & ~CV_MAGIC_MASK) == ICV_CNN_SUBSAMPLING_LAYER )

#define ICV_IS_CNN_FULLCONNECT_LAYER( layer )                              \
    ( (ICV_IS_CNN_LAYER( layer )) && (((CvCNNLayer*) (layer))->flags       \
        & ~CV_MAGIC_MASK) == ICV_CNN_FULLCONNECT_LAYER )

typedef void (CV_CDECL *CvCNNLayerForward)
    ( CvCNNLayer* layer, const CvMat* input, CvMat* output );

typedef void (CV_CDECL *CvCNNLayerBackward)
    ( CvCNNLayer* layer, int t, const CvMat* X, const CvMat* dE_dY, CvMat* dE_dX );

typedef void (CV_CDECL *CvCNNLayerRelease)
    (CvCNNLayer** layer);

typedef void (CV_CDECL *CvCNNetworkAddLayer)
    (CvCNNetwork* network, CvCNNLayer* layer);

typedef void (CV_CDECL *CvCNNetworkRelease)
    (CvCNNetwork** network);

#define CV_CNN_LAYER_FIELDS()           \
    /* Indicator of the layer's type */ \
    int flags;                          \
                                        \
    /* Number of input images */        \
    int n_input_planes;                 \
    /* Height of each input image */    \
    int input_height;                   \
    /* Width of each input image */     \
    int input_width;                    \
                                        \
    /* Number of output images */       \
    int n_output_planes;                \
    /* Height of each output image */   \
    int output_height;                  \
    /* Width of each output image */    \
    int output_width;                   \
                                        \
    /* Learning rate at the first iteration */                      \
    float init_learn_rate;                                          \
    /* Dynamics of learning rate decreasing */                      \
    int learn_rate_decrease_type;                                   \
    /* Trainable weights of the layer (including bias) */           \
    /* i-th row is a set of weights of the i-th output plane */     \
    CvMat* weights;                                                 \
                                                                    \
    CvCNNLayerForward  forward;                                     \
    CvCNNLayerBackward backward;                                    \
    CvCNNLayerRelease  release;                                     \
    /* Pointers to the previous and next layers in the network */   \
    CvCNNLayer* prev_layer;                                         \
    CvCNNLayer* next_layer

typedef struct CvCNNLayer
{
    CV_CNN_LAYER_FIELDS();
}CvCNNLayer;

typedef struct CvCNNConvolutionLayer
{
    CV_CNN_LAYER_FIELDS();
    // Kernel size (height and width) for convolution.
    int K;
    // connections matrix, (i,j)-th element is 1 iff there is a connection between
    // i-th plane of the current layer and j-th plane of the previous layer;
    // (i,j)-th element is equal to 0 otherwise
    CvMat *connect_mask;
    // value of the learning rate for updating weights at the first iteration
}CvCNNConvolutionLayer;

typedef struct CvCNNSubSamplingLayer
{
    CV_CNN_LAYER_FIELDS();
    // ratio between the heights (or widths - ratios are supposed to be equal)
    // of the input and output planes
    int sub_samp_scale;
    // amplitude of sigmoid activation function
    float a;
    // scale parameter of sigmoid activation function
    float s;
    // exp2ssumWX = exp(2<s>*(bias+w*(x1+...+x4))), where x1,...x4 are some elements of X
    // - is the vector used in computing of the activation function in backward
    CvMat* exp2ssumWX;
    // (x1+x2+x3+x4), where x1,...x4 are some elements of X
    // - is the vector used in computing of the activation function in backward
    CvMat* sumX;
}CvCNNSubSamplingLayer;

// Structure of the last layer.
typedef struct CvCNNFullConnectLayer
{
    CV_CNN_LAYER_FIELDS();
    // amplitude of sigmoid activation function
    float a;
    // scale parameter of sigmoid activation function
    float s;
    // exp2ssumWX = exp(2*<s>*(W*X)) - is the vector used in computing of the 
    // activation function and it's derivative by the formulae
    // activ.func. = <a>(exp(2<s>WX)-1)/(exp(2<s>WX)+1) == <a> - 2<a>/(<exp2ssumWX> + 1)
    // (activ.func.)' = 4<a><s>exp(2<s>WX)/(exp(2<s>WX)+1)^2
    CvMat* exp2ssumWX;
}CvCNNFullConnectLayer;

typedef struct CvCNNetwork
{
    int n_layers;
    CvCNNLayer* layers;
    CvCNNetworkAddLayer add_layer;
    CvCNNetworkRelease release;
}CvCNNetwork;

typedef struct CvCNNStatModel
{
    CV_STAT_MODEL_FIELDS();
    CvCNNetwork* network;
    // etalons are allocated as rows, the i-th etalon has label cls_labeles[i]
    CvMat* etalons;
    // classes labels
    CvMat* cls_labels;
}CvCNNStatModel;

typedef struct CvCNNStatModelParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    // network must be created by the functions cvCreateCNNetwork and <add_layer>
    CvCNNetwork* network;
    CvMat* etalons;
    // termination criteria
    int max_iter;
    int start_iter;
    int grad_estim_type;
}CvCNNStatModelParams;

CVAPI(CvCNNLayer*) cvCreateCNNConvolutionLayer(
    int n_input_planes, int input_height, int input_width,
    int n_output_planes, int K,
    float init_learn_rate, int learn_rate_decrease_type,
    CvMat* connect_mask CV_DEFAULT(0), CvMat* weights CV_DEFAULT(0) );

CVAPI(CvCNNLayer*) cvCreateCNNSubSamplingLayer(
    int n_input_planes, int input_height, int input_width,
    int sub_samp_scale, float a, float s,
    float init_learn_rate, int learn_rate_decrease_type, CvMat* weights CV_DEFAULT(0) );

CVAPI(CvCNNLayer*) cvCreateCNNFullConnectLayer( 
    int n_inputs, int n_outputs, float a, float s,
    float init_learn_rate, int learning_type, CvMat* weights CV_DEFAULT(0) );

CVAPI(CvCNNetwork*) cvCreateCNNetwork( CvCNNLayer* first_layer );

CVAPI(CvStatModel*) cvTrainCNNClassifier(
            const CvMat* train_data, int tflag,
            const CvMat* responses,
            const CvStatModelParams* params,
            const CvMat* CV_DEFAULT(0),
            const CvMat* sample_idx CV_DEFAULT(0),
            const CvMat* CV_DEFAULT(0), const CvMat* CV_DEFAULT(0) );

/****************************************************************************************\
*                               Estimate classifiers algorithms                          *
\****************************************************************************************/
typedef const CvMat* (CV_CDECL *CvStatModelEstimateGetMat)
                    ( const CvStatModel* estimateModel );

typedef int (CV_CDECL *CvStatModelEstimateNextStep)
                    ( CvStatModel* estimateModel );

typedef void (CV_CDECL *CvStatModelEstimateCheckClassifier)
                    ( CvStatModel* estimateModel,
                const CvStatModel* model, 
                const CvMat*       features, 
                      int          sample_t_flag,
                const CvMat*       responses );

typedef void (CV_CDECL *CvStatModelEstimateCheckClassifierEasy)
                    ( CvStatModel* estimateModel,
                const CvStatModel* model );

typedef float (CV_CDECL *CvStatModelEstimateGetCurrentResult)
                    ( const CvStatModel* estimateModel,
                            float*       correlation );

typedef void (CV_CDECL *CvStatModelEstimateReset)
                    ( CvStatModel* estimateModel );

//-------------------------------- Cross-validation --------------------------------------
#define CV_CROSS_VALIDATION_ESTIMATE_CLASSIFIER_PARAM_FIELDS()    \
    CV_STAT_MODEL_PARAM_FIELDS();                                 \
    int     k_fold;                                               \
    int     is_regression;                                        \
    CvRNG*  rng

typedef struct CvCrossValidationParams
{
    CV_CROSS_VALIDATION_ESTIMATE_CLASSIFIER_PARAM_FIELDS();
} CvCrossValidationParams;

#define CV_CROSS_VALIDATION_ESTIMATE_CLASSIFIER_FIELDS()    \
    CvStatModelEstimateGetMat               getTrainIdxMat; \
    CvStatModelEstimateGetMat               getCheckIdxMat; \
    CvStatModelEstimateNextStep             nextStep;       \
    CvStatModelEstimateCheckClassifier      check;          \
    CvStatModelEstimateGetCurrentResult     getResult;      \
    CvStatModelEstimateReset                reset;          \
    int     is_regression;                                  \
    int     folds_all;                                      \
    int     samples_all;                                    \
    int*    sampleIdxAll;                                   \
    int*    folds;                                          \
    int     max_fold_size;                                  \
    int         current_fold;                               \
    int         is_checked;                                 \
    CvMat*      sampleIdxTrain;                             \
    CvMat*      sampleIdxEval;                              \
    CvMat*      predict_results;                            \
    int     correct_results;                                \
    int     all_results;                                    \
    double  sq_error;                                       \
    double  sum_correct;                                    \
    double  sum_predict;                                    \
    double  sum_cc;                                         \
    double  sum_pp;                                         \
    double  sum_cp

typedef struct CvCrossValidationModel
{
    CV_STAT_MODEL_FIELDS();
    CV_CROSS_VALIDATION_ESTIMATE_CLASSIFIER_FIELDS();
} CvCrossValidationModel;

CVAPI(CvStatModel*) 
cvCreateCrossValidationEstimateModel
           ( int                samples_all,
       const CvStatModelParams* estimateParams CV_DEFAULT(0),
       const CvMat*             sampleIdx CV_DEFAULT(0) );

CVAPI(float) 
cvCrossValidation( const CvMat*             trueData,
                         int                tflag,
                   const CvMat*             trueClasses,
                         CvStatModel*     (*createClassifier)( const CvMat*,
                                                                     int,
                                                               const CvMat*,
                                                               const CvStatModelParams*,
                                                               const CvMat*,
                                                               const CvMat*,
                                                               const CvMat*,
                                                               const CvMat* ),
                   const CvStatModelParams* estimateParams CV_DEFAULT(0),
                   const CvStatModelParams* trainParams CV_DEFAULT(0),
                   const CvMat*             compIdx CV_DEFAULT(0),
                   const CvMat*             sampleIdx CV_DEFAULT(0),
                         CvStatModel**      pCrValModel CV_DEFAULT(0),
                   const CvMat*             typeMask CV_DEFAULT(0),
                   const CvMat*             missedMeasurementMask CV_DEFAULT(0) );


/****************************************************************************************\
*                           Auxilary functions declarations                              *
\****************************************************************************************/

/* Generates <sample> from multivariate normal distribution, where <mean> - is an
   average row vector, <cov> - symmetric covariation matrix */
CVAPI(void) cvRandMVNormal( CvMat* mean, CvMat* cov, CvMat* sample,
                           CvRNG* rng CV_DEFAULT(0) );

/* Generates sample from gaussian mixture distribution */
CVAPI(void) cvRandGaussMixture( CvMat* means[],
                               CvMat* covs[],
                               float weights[],
                               int clsnum,
                               CvMat* sample,
                               CvMat* sampClasses CV_DEFAULT(0) );


/* functions to access to internal structure and parameters of stat model */
CVAPI(void*) cvGetObject( CvStatModel* model, 
                         void* object, 
                         int objectType, 
                         int index CV_DEFAULT(0));
CVAPI(double) cvGetParamValue( CvStatModel* model, 
                        void* object, 
                        int paramType, 
                        int index CV_DEFAULT(0));


CVAPI(void) cvSetParamValue( CvStatModel* model, 
                      void* object, 
                      int paramType, 
                      int index CV_DEFAULT(0),
                      double value CV_DEFAULT(0));

CVAPI(void) cvGetParamMat( CvStatModel* model, 
                    void* object, 
                    int paramType, 
                    int index CV_DEFAULT(0),
                    CvMat* mat CV_DEFAULT(0));

CVAPI(void) cvSetParamMat( CvStatModel* model, 
                    void* object, 
                    int paramType, 
                    int index CV_DEFAULT(0),
                    CvMat* mat CV_DEFAULT(0));

#define CV_TS_CONCENTRIC_SPHERES 0

/* creates test set */
CVAPI(void) cvCreateTestSet( int type, CvMat** samples,
                 int num_samples,
                 int num_features,
                 CvMat** responses,
                 int num_classes, ... );

/* Aij <- Aji for i > j if lower_to_upper != 0
              for i < j if lower_to_upper = 0 */
CVAPI(void) cvCompleteSymm( CvMat* matrix, int lower_to_upper );

#ifdef __cplusplus
}
#endif

#endif /*__ML_H__*/
/* End of file. */
