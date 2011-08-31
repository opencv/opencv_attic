#include "perf_precomp.hpp"
#include <iostream>

using namespace std;
using namespace cv;
using namespace perf;


/*
// Scalar sum(InputArray arr)
*/
PERF_TEST_P( Size_MatType_param, sum, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int type = std::tr1::get<1>(GetParam());
    Mat arr(sz, type);
    randu(arr, 0, 256);
	Scalar s;
    
    declare.in(arr);

    TEST_CYCLE(100) { s = sum(arr); }

    SANITY_CHECK(s);
}



/*
// Scalar mean(InputArray src)
*/
PERF_TEST_P( Size_MatType_param, mean, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int type = std::tr1::get<1>(GetParam());
    Mat src(sz, type);
	Scalar s;
    
    declare.in(src, WARMUP_RNG);
    
    TEST_CYCLE(100) { s = mean(src); }
    
    SANITY_CHECK(s);
}



/*
// Scalar mean(InputArray src, InputArray mask=noArray())
*/
PERF_TEST_P( Size_MatType_param, mean_mask, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int type = std::tr1::get<1>(GetParam());
    Mat src(sz, type);
    Mat mask = Mat::ones(src.size(), CV_8U);
	cv::Scalar s;
    
    declare.in(src, WARMUP_RNG).in(mask);
    
    TEST_CYCLE(100) { s = mean(src, mask); }
    
    SANITY_CHECK(s);
}


class NormType
{
public:
    NormType(int val=0) : _val(val) {}
    operator int() const {return _val;}

private:
    int _val;
};

CV_EXPORTS void PrintTo(const NormType& t, std::ostream* os)
{
        int val = t;
        if(val & NORM_RELATIVE)
        {
            *os << "NORM_RELATIVE+";
            val &= ~NORM_RELATIVE;
        }
        switch( val )
        {
            case NORM_INF: *os << "NORM_INF"; break;
            case NORM_L1:  *os << "NORM_L1";  break;
            case NORM_L2:  *os << "NORM_L2";  break;
            case NORM_MINMAX: *os << "NORM_MINMAX"; break;
            default: *os << "INVALID_NORM";
        }
}

typedef std::tr1::tuple<Size, MatType, NormType> Size_MatType_NormType;
typedef perf::TestBaseWithParam<Size_MatType_NormType> Size_MatType_NormType_param;

/*
// double norm(InputArray src1, int normType=NORM_L2)
*/
PERF_TEST_P( Size_MatType_NormType_param, norm, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2 )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src1(sz, matType);
    randu(src1, 0, 256);
	double n;
    
    declare.in(src1);

    TEST_CYCLE(100) { n = norm(src1, normType); }
    
    SANITY_CHECK(n);
}


/*
// double norm(InputArray src1, int normType=NORM_L2, InputArray mask=noArray())
*/
PERF_TEST_P( Size_MatType_NormType_param, norm_mask, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2 )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src1(sz, matType);
    randu(src1, 0, 256);
    Mat mask = Mat::ones(sz, CV_8U);
	double n;
    
    declare.in(src1, mask);
    
    TEST_CYCLE(100) { n = norm(src1, normType, mask); }
    
    SANITY_CHECK(n);
}


/*
// double norm(InputArray src1, InputArray src2, int normType)
*/
PERF_TEST_P( Size_MatType_NormType_param, norm2, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2, (int)(NORM_RELATIVE+NORM_INF), (int)(NORM_RELATIVE+NORM_L1), (int)(NORM_RELATIVE+NORM_L2) )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src1(sz, matType);
    randu(src1, 0, 256);
    Mat src2(sz, matType);
    randu(src2, 0, 256);
	double n;
    
    declare.in(src1, src2);
    
    TEST_CYCLE(100) { n = norm(src1, src2, normType); }
    
    SANITY_CHECK(n);
}


/*
// double norm(InputArray src1, InputArray src2, int normType, InputArray mask=noArray())
*/
PERF_TEST_P( Size_MatType_NormType_param, norm2_mask,
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2, (int)(NORM_RELATIVE+NORM_INF), (int)(NORM_RELATIVE+NORM_L1), (int)(NORM_RELATIVE+NORM_L2) )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src1(sz, matType);
    randu(src1, 0, 256);
    Mat src2(sz, matType);
    randu(src2, 0, 256);
    Mat mask = Mat::ones(sz, CV_8U);
	double n;
    
    declare.in(src1, src2, mask);
    
    TEST_CYCLE(100) { n = norm(src1, src2, normType, mask); }
    
    SANITY_CHECK(n);
}



/*
// void normalize(const InputArray src, OutputArray dst, double alpha=1, double beta=0, int normType=NORM_L2)
*/
PERF_TEST_P( Size_MatType_NormType_param, normalize, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2 )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src(sz, matType);
    randu(src, 0, 256);
    Mat dst(sz, matType);
    double alpha = 100.;
    if(normType==NORM_L1) alpha = src.total() * src.channels();
    if(normType==NORM_L2) alpha = src.total()/10;
    
    declare.in(src).out(dst);
    
    TEST_CYCLE(100) { normalize(src, dst, alpha, 0., normType);  }
    
    SANITY_CHECK(dst);
}


/*
// void normalize(const InputArray src, OutputArray dst, double alpha=1, double beta=0, int normType=NORM_L2, int rtype=-1, InputArray mask=noArray())
*/
PERF_TEST_P( Size_MatType_NormType_param, normalize_mask, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2 )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src(sz, matType);
    randu(src, 0, 256);
    Mat dst(sz, matType);
    Mat mask = Mat::ones(sz, CV_8U);
    double alpha = 100.;
    if(normType==NORM_L1) alpha = src.total() * src.channels();
    if(normType==NORM_L2) alpha = src.total()/10;
    
    declare.in(src, mask).out(dst);
    
    TEST_CYCLE(100) { normalize(src, dst, alpha, 0., normType, -1, mask);  }
    
    SANITY_CHECK(dst);
}


/*
// void normalize(const InputArray src, OutputArray dst, double alpha=1, double beta=0, int normType=NORM_L2, int rtype=-1)
*/
PERF_TEST_P( Size_MatType_NormType_param, normalize_32f, 
    testing::Combine(
        testing::Values( TYPICAL_MAT_SIZES ), 
        testing::Values( TYPICAL_MAT_TYPES ),
        testing::Values( (int)NORM_INF, (int)NORM_L1, (int)NORM_L2 )
    )
)
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    int normType = std::tr1::get<2>(GetParam());
    Mat src(sz, matType);
    randu(src, 0, 256);
    Mat dst(sz, matType);
    double alpha = 100.;
    if(normType==NORM_L1) alpha = src.total() * src.channels();
    if(normType==NORM_L2) alpha = src.total()/10;
    
    declare.in(src).out(dst);
    
    TEST_CYCLE(100) { normalize(src, dst, alpha, 0., normType, CV_32F);  }
    
    SANITY_CHECK(dst);
}


/*
// void normalize(const InputArray src, OutputArray dst, double alpha=1, double beta=0, int normType=NORM_L2)
*/
PERF_TEST_P( Size_MatType_param, normalize_minmax, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    Mat src(sz, matType);
    randu(src, 0, 256);
    Mat dst(sz, matType);
    
    declare.in(src).out(dst);
    
    TEST_CYCLE(100) { normalize(src, dst, 20., 100., NORM_MINMAX);  }
    
    SANITY_CHECK(dst);
}




/*
// void meanStdDev(InputArray src, OutputArray mean, OutputArray stddev)
*/
PERF_TEST_P( Size_MatType_param, meanStdDev, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    Mat src(sz, matType);
    Mat mean, dev;

    declare.in(src, WARMUP_RNG);

    TEST_CYCLE(100) { meanStdDev(src, mean, dev);  }
    
    SANITY_CHECK(mean);
    SANITY_CHECK(dev);
}




/*
// void meanStdDev(InputArray src, OutputArray mean, OutputArray stddev, InputArray mask=noArray())
*/
PERF_TEST_P( Size_MatType_param, meanStdDev_mask, TYPICAL_MATS )
{
    Size sz = std::tr1::get<0>(GetParam());
    int matType = std::tr1::get<1>(GetParam());
    Mat src(sz, matType);
    Mat mask = Mat::ones(sz, CV_8U);
    Mat mean, dev;

    declare.in(src, WARMUP_RNG).in(mask);
    
    TEST_CYCLE(100) { meanStdDev(src, mean, dev, mask);  }
    
    SANITY_CHECK(mean);
    SANITY_CHECK(dev);
}

