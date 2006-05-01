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
//                For Open Source Computer Vision Library
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

// 2006-02-17  Roman Stanchak <rstancha@cse.wustl.edu>


/*M//////////////////////////////////////////////////////////////////////////////////////////
// Macros for extending CvMat and IplImage -- primarily for operator overloading 
//////////////////////////////////////////////////////////////////////////////////////////M*/

// Macro to define python function of form B = A.f(c)
// where A is a CvArr type, c and B are arbitrary types
%define %wrap_cvGeneric_CvArr(cname, rettype, pyfunc, argtype, cvfunc, newobjcall)
%newobject cname::pyfunc(argtype arg);
%extend cname { 
	rettype pyfunc(argtype arg){
		rettype retarg = newobjcall;
		cvfunc;
		return retarg;
	}
}
%enddef

// Macro to define python function of the form B = A.f(c) 
// where A and B are both CvArr of same size and type
%define %wrap_cvArr_binaryop(pyfunc, argtype, cvfunc)
%wrap_cvGeneric_CvArr(CvMat, CvMat *, pyfunc, argtype, cvfunc, 
					  cvCreateMat(self->rows, self->cols, self->type));
%wrap_cvGeneric_CvArr(IplImage, IplImage *, pyfunc, argtype, cvfunc,
					  cvCreateImage(cvGetSize(self), self->depth, self->nChannels));
%enddef

// Macro to define python function of the form A = A.f(c) 
// where f modifies A inplace
// use for +=, etc
%define %wrap_cvGeneric_InPlace(cname, rettype, pyfunc, argtype, cvfunc)
%wrap_cvGeneric_CvArr(cname, rettype, pyfunc, argtype, cvfunc, self);
%enddef

/*M//////////////////////////////////////////////////////////////////////////////////////////
// Macros to map operators to specific OpenCV functions
//////////////////////////////////////////////////////////////////////////////////////////M*/

// map any OpenCV function of form cvFunc(src1, src2, dst)
%define %wrap_cvArith(pyfunc, cvfunc)
%wrap_cvArr_binaryop(pyfunc, CvArr *, cvfunc(self, arg, retarg));
%enddef

// map any OpenCV function of form cvFunc(src1, value, dst)
%define %wrap_cvArithS(pyfunc, cvfuncS)
%wrap_cvArr_binaryop(pyfunc, CvScalar, cvfuncS(self, arg, retarg));
%wrap_cvArr_binaryop(pyfunc, double, cvfuncS(self, cvScalar(arg), retarg));
%enddef

// same as wrap_cvArith
%define %wrap_cvLogic(pyfunc, cvfunc)
%wrap_cvArr_binaryop(pyfunc, CvArr *, cvfunc(self, arg, retarg))
%enddef

// same as wrap_cvArithS
%define %wrap_cvLogicS(pyfunc, cvfuncS)
%wrap_cvArr_binaryop(pyfunc, CvScalar, cvfuncS(self, arg, retarg));
%wrap_cvArr_binaryop(pyfunc, double, cvfuncS(self, cvScalar(arg), retarg));
%enddef

// Macro to map logical operations to cvCmp
%define %wrap_cvCmp(pyfunc, cmp_op)
%wrap_cvGeneric_CvArr(CvMat, CvArr *, pyfunc, CvMat *, 
                      cvCmp(self, arg, retarg, cmp_op), 
					  cvCreateMat(self->rows, self->cols, CV_8U));
%wrap_cvGeneric_CvArr(IplImage, CvArr *, pyfunc, IplImage *, 
                      cvCmp(self, arg, retarg, cmp_op), 
					  cvCreateImage(cvGetSize(self), 8, 1));
%enddef

%define %wrap_cvCmpS(pyfunc, cmp_op)
%wrap_cvGeneric_CvArr(CvMat, CvArr *, pyfunc, double, 
                      cvCmpS(self, arg, retarg, cmp_op), 
					  cvCreateMat(self->rows, self->cols, CV_8U));
%wrap_cvGeneric_CvArr(IplImage, CvArr *, pyfunc, double, 
                      cvCmpS(self, arg, retarg, cmp_op), 
					  cvCreateImage(cvGetSize(self), 8, 1));
%enddef


/*M//////////////////////////////////////////////////////////////////////////////////////////
// Actual Operator Declarations
//////////////////////////////////////////////////////////////////////////////////////////M*/

// Arithmetic operators 
%wrap_cvArith(__add__, cvAdd);
%wrap_cvArith(__sub__, cvSub);
%wrap_cvArith(__radd__, cvAdd);
%wrap_cvArith(__div__, cvDiv);

// matrix multiply for CvMat
%wrap_cvGeneric_CvArr(CvMat, CvArr*, __mul__, CvMat *, cvMatMul(self, arg, retarg), 
				      cvCreateMat(self->rows, self->cols, self->type));

// dot multiply for IplImage
%wrap_cvGeneric_CvArr(IplImage, CvArr*, __mul__, IplImage *, cvMul(self, arg, retarg), 
					  cvCreateImage(cvGetSize(self), self->depth, self->nChannels));



// special case for reverse operations
%wrap_cvArr_binaryop(__rsub__, CvArr *, cvSub(arg, self, retarg));
%wrap_cvArr_binaryop(__rdiv__, CvArr *, cvDiv(arg, self, retarg));
%wrap_cvArr_binaryop(__rmul__, CvArr *, cvMatMul(arg, self, retarg));

%wrap_cvArithS(__add__, cvAddS);
%wrap_cvArithS(__sub__, cvSubS);
%wrap_cvArithS(__radd__, cvAddS);
%wrap_cvArithS(__rsub__, cvSubRS);

// special case for MulS and DivS
%define %wrap_cvScale(pyfunc, scale)
%wrap_cvGeneric_CvArr(CvMat, CvArr *, pyfunc, double,
		cvScale(self, retarg, scale),
		cvCreateMat(self->rows, self->cols, self->type));
%wrap_cvGeneric_CvArr(IplImage, CvArr *, pyfunc, double,
		cvScale(self, retarg, scale),
		cvCreateImage(cvGetSize(self), self->depth, self->nChannels));
%enddef

%wrap_cvScale(__mul__, arg);
%wrap_cvScale(__rmul__, arg);
%wrap_cvScale(__div__, 1.0/arg);
%wrap_cvScale(__rdiv__, 1.0/arg);

//  Logical Operators
%wrap_cvLogic(__or__, cvOr)
%wrap_cvLogic(__and__, cvAnd)
%wrap_cvLogic(__xor__, cvXor)

%wrap_cvLogicS(__or__, cvOrS)
%wrap_cvLogicS(__and__, cvAndS)
%wrap_cvLogicS(__xor__, cvXorS)
%wrap_cvLogicS(__ror__, cvOrS)
%wrap_cvLogicS(__rand__, cvAndS)
%wrap_cvLogicS(__rxor__, cvXorS)

%wrap_cvCmp(__eq__, CV_CMP_EQ);
%wrap_cvCmp(__gt__, CV_CMP_GT);
%wrap_cvCmp(__ge__, CV_CMP_GE);
%wrap_cvCmp(__lt__, CV_CMP_LT);
%wrap_cvCmp(__le__, CV_CMP_LE);
%wrap_cvCmp(__ne__, CV_CMP_NE);

%wrap_cvCmpS(__eq__, CV_CMP_EQ);
%wrap_cvCmpS(__gt__, CV_CMP_GT);
%wrap_cvCmpS(__ge__, CV_CMP_GE);
%wrap_cvCmpS(__lt__, CV_CMP_LT);
%wrap_cvCmpS(__le__, CV_CMP_LE);
%wrap_cvCmpS(__ne__, CV_CMP_NE);

%wrap_cvCmpS(__req__, CV_CMP_EQ);
%wrap_cvCmpS(__rgt__, CV_CMP_GT);
%wrap_cvCmpS(__rge__, CV_CMP_GE);
%wrap_cvCmpS(__rlt__, CV_CMP_LT);
%wrap_cvCmpS(__rle__, CV_CMP_LE);
%wrap_cvCmpS(__rne__, CV_CMP_NE);


// misc operators for python
%wrap_cvArr_binaryop(__pow__, double, cvPow(self, retarg, arg))

// TODO -- other Python operators listed below and at:
// http://docs.python.org/ref/numeric-types.html

// __abs__ -- cvAbs
// __nonzero__
// __hash__ ??
// __repr__  -- full string representation
// __str__  -- compact representation
// __call__ -- ??
// __len__ -- number of rows? or elements?
// __iter__ -- ??
// __contains__ -- cvCmpS, cvMax ?
// __floordiv__ ??
// __mul__ -- cvGEMM
// __lshift__ -- ??
// __rshift__ -- ??
// __pow__ -- cvPow

// Called to implement the unary arithmetic operations (-, +, abs() and ~). 
//__neg__(  self)
//__pos__(  self)
//__abs__(  self)
//__invert__(  self)

// Called to implement the built-in functions complex(), int(), long(), and float(). Should return a value of the appropriate type.  Can I abuse this to return an array of the correct type??? scipy only allows return of length 1 arrays.
// __complex__( self )
// __int__( self )
// __long__( self )
// __float__( self )

// Incremental
/*
__iadd__(  self, other)
__isub__(  self, other)
__imul__(  self, other)
__idiv__(  self, other)
__itruediv__(  self, other)
__ifloordiv__(  self, other)
__imod__(  self, other)
__ipow__(  self, other[, modulo])
__ilshift__(  self, other)
__irshift__(  self, other)
__iand__(  self, other)
__ixor__(  self, other)
__ior__(  self, other)

// Reverse operations -- scalar only
__radd__(  self, other)
__rsub__(  self, other)
__rmul__(  self, other)
__rdiv__(  self, other)
__rtruediv__(  self, other)
__rfloordiv__(  self, other)
__rmod__(  self, other)
__rdivmod__(  self, other)
__rpow__(  self, other)
__rlshift__(  self, other)
__rrshift__(  self, other)
__rand__(  self, other)
__rxor__(  self, other)
__ror__(  self, other)
*/

/*M//////////////////////////////////////////////////////////////////////////////////////////
// Slice access and assignment for CvArr types
//////////////////////////////////////////////////////////////////////////////////////////M*/

// TODO: CvMatND

%newobject CvMat::__getitem__(PyObject * object);
%newobject _IplImage::__getitem__(PyObject * object);

// slice access and assignment for CvMat
%extend CvMat
{
	char * __str__(){
		static char str[8];
		cvArrPrint( self );
		str[0]=0;
		return str;
	}

	// scalar assignment
	void __setitem__(PyObject * object, double val){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvSet(&tmp, cvScalarAll(val));
	}
	void __setitem__(PyObject * object, CvScalar val){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvSet(&tmp, val);
	}

	// array slice assignement
	void __setitem__(PyObject * object, CvArr * arr){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvCopy(arr, &tmp);
	}
	
	// slice access
	CvMat * __getitem__(PyObject * object){
		CvRect subrect = PySlice_to_CvRect( self, object );
		CvMat * mat = (CvMat *) cvAlloc(sizeof(CvMat));
		return cvGetSubRect(self, mat, subrect);
	}

}

// slice access and assignment for IplImage 
%extend _IplImage
{
	char * __str__(){
		static char str[8];
		cvArrPrint( self );
		str[0]=0;
		return str;
	}

	// scalar assignment
	void __setitem__(PyObject * object, double val){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvSet(&tmp, cvScalarAll(val));
	}
	void __setitem__(PyObject * object, CvScalar val){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvSet(&tmp, val);
	}

	// array slice assignment
	void __setitem__(PyObject * object, CvArr * arr){
		CvMat tmp;
		CvRect subrect = PySlice_to_CvRect( self, object );
		cvGetSubRect(self, &tmp, subrect);
		cvCopy(arr, &tmp);
	}

	// slice access
	IplImage * __getitem__(PyObject * object){
		CvRect subrect = PySlice_to_CvRect( self, object );
		CvMat mat;
		IplImage * im = (IplImage *) cvAlloc(sizeof(IplImage));
		cvGetSubRect(self, &mat, subrect);
		return cvGetImage(&mat, im);
	}
}
