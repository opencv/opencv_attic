# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _cv

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


CV_AUTOSTEP = _cv.CV_AUTOSTEP
CV_MAX_ARR = _cv.CV_MAX_ARR
CV_NO_DEPTH_CHECK = _cv.CV_NO_DEPTH_CHECK
CV_NO_CN_CHECK = _cv.CV_NO_CN_CHECK
CV_NO_SIZE_CHECK = _cv.CV_NO_SIZE_CHECK
CV_CMP_EQ = _cv.CV_CMP_EQ
CV_CMP_GT = _cv.CV_CMP_GT
CV_CMP_GE = _cv.CV_CMP_GE
CV_CMP_LT = _cv.CV_CMP_LT
CV_CMP_LE = _cv.CV_CMP_LE
CV_CMP_NE = _cv.CV_CMP_NE
CV_CHECK_RANGE = _cv.CV_CHECK_RANGE
CV_CHECK_QUIET = _cv.CV_CHECK_QUIET
CV_RAND_UNI = _cv.CV_RAND_UNI
CV_RAND_NORMAL = _cv.CV_RAND_NORMAL
CV_GEMM_A_T = _cv.CV_GEMM_A_T
CV_GEMM_B_T = _cv.CV_GEMM_B_T
CV_GEMM_C_T = _cv.CV_GEMM_C_T
CV_SVD_MODIFY_A = _cv.CV_SVD_MODIFY_A
CV_SVD_U_T = _cv.CV_SVD_U_T
CV_SVD_V_T = _cv.CV_SVD_V_T
CV_LU = _cv.CV_LU
CV_SVD = _cv.CV_SVD
CV_SVD_SYM = _cv.CV_SVD_SYM
CV_COVAR_SCRAMBLED = _cv.CV_COVAR_SCRAMBLED
CV_COVAR_NORMAL = _cv.CV_COVAR_NORMAL
CV_COVAR_USE_AVG = _cv.CV_COVAR_USE_AVG
CV_COVAR_SCALE = _cv.CV_COVAR_SCALE
CV_C = _cv.CV_C
CV_L1 = _cv.CV_L1
CV_L2 = _cv.CV_L2
CV_NORM_MASK = _cv.CV_NORM_MASK
CV_RELATIVE = _cv.CV_RELATIVE
CV_DIFF = _cv.CV_DIFF
CV_DIFF_C = _cv.CV_DIFF_C
CV_DIFF_L1 = _cv.CV_DIFF_L1
CV_DIFF_L2 = _cv.CV_DIFF_L2
CV_RELATIVE_C = _cv.CV_RELATIVE_C
CV_RELATIVE_L1 = _cv.CV_RELATIVE_L1
CV_RELATIVE_L2 = _cv.CV_RELATIVE_L2
CV_DXT_FORWARD = _cv.CV_DXT_FORWARD
CV_DXT_INVERSE = _cv.CV_DXT_INVERSE
CV_DXT_SCALE = _cv.CV_DXT_SCALE
CV_DXT_INV_SCALE = _cv.CV_DXT_INV_SCALE
CV_DXT_INVERSE_SCALE = _cv.CV_DXT_INVERSE_SCALE
CV_DXT_ROWS = _cv.CV_DXT_ROWS
CV_DXT_MUL_CONJ = _cv.CV_DXT_MUL_CONJ
CV_FRONT = _cv.CV_FRONT
CV_BACK = _cv.CV_BACK
CV_GRAPH_VERTEX = _cv.CV_GRAPH_VERTEX
CV_GRAPH_TREE_EDGE = _cv.CV_GRAPH_TREE_EDGE
CV_GRAPH_BACK_EDGE = _cv.CV_GRAPH_BACK_EDGE
CV_GRAPH_FORWARD_EDGE = _cv.CV_GRAPH_FORWARD_EDGE
CV_GRAPH_CROSS_EDGE = _cv.CV_GRAPH_CROSS_EDGE
CV_GRAPH_ANY_EDGE = _cv.CV_GRAPH_ANY_EDGE
CV_GRAPH_NEW_TREE = _cv.CV_GRAPH_NEW_TREE
CV_GRAPH_BACKTRACKING = _cv.CV_GRAPH_BACKTRACKING
CV_GRAPH_OVER = _cv.CV_GRAPH_OVER
CV_GRAPH_ALL_ITEMS = _cv.CV_GRAPH_ALL_ITEMS
CV_GRAPH_ITEM_VISITED_FLAG = _cv.CV_GRAPH_ITEM_VISITED_FLAG
CV_GRAPH_SEARCH_TREE_NODE_FLAG = _cv.CV_GRAPH_SEARCH_TREE_NODE_FLAG
CV_GRAPH_FORWARD_EDGE_FLAG = _cv.CV_GRAPH_FORWARD_EDGE_FLAG
CV_FILLED = _cv.CV_FILLED
CV_AA = _cv.CV_AA
CV_FONT_HERSHEY_SIMPLEX = _cv.CV_FONT_HERSHEY_SIMPLEX
CV_FONT_HERSHEY_PLAIN = _cv.CV_FONT_HERSHEY_PLAIN
CV_FONT_HERSHEY_DUPLEX = _cv.CV_FONT_HERSHEY_DUPLEX
CV_FONT_HERSHEY_COMPLEX = _cv.CV_FONT_HERSHEY_COMPLEX
CV_FONT_HERSHEY_TRIPLEX = _cv.CV_FONT_HERSHEY_TRIPLEX
CV_FONT_HERSHEY_COMPLEX_SMALL = _cv.CV_FONT_HERSHEY_COMPLEX_SMALL
CV_FONT_HERSHEY_SCRIPT_SIMPLEX = _cv.CV_FONT_HERSHEY_SCRIPT_SIMPLEX
CV_FONT_HERSHEY_SCRIPT_COMPLEX = _cv.CV_FONT_HERSHEY_SCRIPT_COMPLEX
CV_FONT_ITALIC = _cv.CV_FONT_ITALIC
CV_FONT_VECTOR0 = _cv.CV_FONT_VECTOR0
CV_ErrModeLeaf = _cv.CV_ErrModeLeaf
CV_ErrModeParent = _cv.CV_ErrModeParent
CV_ErrModeSilent = _cv.CV_ErrModeSilent
CV_PI = _cv.CV_PI
CV_LOG2 = _cv.CV_LOG2
IPL_DEPTH_SIGN = _cv.IPL_DEPTH_SIGN
IPL_DEPTH_1U = _cv.IPL_DEPTH_1U
IPL_DEPTH_8U = _cv.IPL_DEPTH_8U
IPL_DEPTH_16U = _cv.IPL_DEPTH_16U
IPL_DEPTH_32F = _cv.IPL_DEPTH_32F
IPL_DEPTH_8S = _cv.IPL_DEPTH_8S
IPL_DEPTH_16S = _cv.IPL_DEPTH_16S
IPL_DEPTH_32S = _cv.IPL_DEPTH_32S
IPL_DATA_ORDER_PIXEL = _cv.IPL_DATA_ORDER_PIXEL
IPL_DATA_ORDER_PLANE = _cv.IPL_DATA_ORDER_PLANE
IPL_ORIGIN_TL = _cv.IPL_ORIGIN_TL
IPL_ORIGIN_BL = _cv.IPL_ORIGIN_BL
IPL_ALIGN_4BYTES = _cv.IPL_ALIGN_4BYTES
IPL_ALIGN_8BYTES = _cv.IPL_ALIGN_8BYTES
IPL_ALIGN_16BYTES = _cv.IPL_ALIGN_16BYTES
IPL_ALIGN_32BYTES = _cv.IPL_ALIGN_32BYTES
IPL_ALIGN_DWORD = _cv.IPL_ALIGN_DWORD
IPL_ALIGN_QWORD = _cv.IPL_ALIGN_QWORD
IPL_BORDER_CONSTANT = _cv.IPL_BORDER_CONSTANT
IPL_BORDER_REPLICATE = _cv.IPL_BORDER_REPLICATE
IPL_BORDER_REFLECT = _cv.IPL_BORDER_REFLECT
IPL_BORDER_WRAP = _cv.IPL_BORDER_WRAP
IPL_IMAGE_HEADER = _cv.IPL_IMAGE_HEADER
IPL_IMAGE_DATA = _cv.IPL_IMAGE_DATA
IPL_IMAGE_ROI = _cv.IPL_IMAGE_ROI
CV_TYPE_NAME_IMAGE = _cv.CV_TYPE_NAME_IMAGE
IPL_DEPTH_64F = _cv.IPL_DEPTH_64F
CV_CN_MAX = _cv.CV_CN_MAX
CV_CN_SHIFT = _cv.CV_CN_SHIFT
CV_DEPTH_MAX = _cv.CV_DEPTH_MAX
CV_8U = _cv.CV_8U
CV_8S = _cv.CV_8S
CV_16U = _cv.CV_16U
CV_16S = _cv.CV_16S
CV_32S = _cv.CV_32S
CV_32F = _cv.CV_32F
CV_64F = _cv.CV_64F
CV_USRTYPE1 = _cv.CV_USRTYPE1
CV_AUTO_STEP = _cv.CV_AUTO_STEP
CV_MAT_CN_MASK = _cv.CV_MAT_CN_MASK
CV_MAT_DEPTH_MASK = _cv.CV_MAT_DEPTH_MASK
CV_MAT_TYPE_MASK = _cv.CV_MAT_TYPE_MASK
CV_MAT_CONT_FLAG_SHIFT = _cv.CV_MAT_CONT_FLAG_SHIFT
CV_MAT_CONT_FLAG = _cv.CV_MAT_CONT_FLAG
CV_MAT_TEMP_FLAG_SHIFT = _cv.CV_MAT_TEMP_FLAG_SHIFT
CV_MAT_TEMP_FLAG = _cv.CV_MAT_TEMP_FLAG
CV_MAGIC_MASK = _cv.CV_MAGIC_MASK
CV_MAT_MAGIC_VAL = _cv.CV_MAT_MAGIC_VAL
CV_TYPE_NAME_MAT = _cv.CV_TYPE_NAME_MAT
CV_MATND_MAGIC_VAL = _cv.CV_MATND_MAGIC_VAL
CV_TYPE_NAME_MATND = _cv.CV_TYPE_NAME_MATND
CV_MAX_DIM = _cv.CV_MAX_DIM
CV_MAX_DIM_HEAP = _cv.CV_MAX_DIM_HEAP
CV_SPARSE_MAT_MAGIC_VAL = _cv.CV_SPARSE_MAT_MAGIC_VAL
CV_TYPE_NAME_SPARSE_MAT = _cv.CV_TYPE_NAME_SPARSE_MAT
CV_HIST_MAGIC_VAL = _cv.CV_HIST_MAGIC_VAL
CV_HIST_UNIFORM_FLAG = _cv.CV_HIST_UNIFORM_FLAG
CV_HIST_RANGES_FLAG = _cv.CV_HIST_RANGES_FLAG
CV_HIST_ARRAY = _cv.CV_HIST_ARRAY
CV_HIST_SPARSE = _cv.CV_HIST_SPARSE
CV_HIST_TREE = _cv.CV_HIST_TREE
CV_HIST_UNIFORM = _cv.CV_HIST_UNIFORM
CV_TERMCRIT_ITER = _cv.CV_TERMCRIT_ITER
CV_TERMCRIT_NUMBER = _cv.CV_TERMCRIT_NUMBER
CV_TERMCRIT_EPS = _cv.CV_TERMCRIT_EPS
CV_WHOLE_SEQ_END_INDEX = _cv.CV_WHOLE_SEQ_END_INDEX
CV_STORAGE_MAGIC_VAL = _cv.CV_STORAGE_MAGIC_VAL
CV_TYPE_NAME_SEQ = _cv.CV_TYPE_NAME_SEQ
CV_TYPE_NAME_SEQ_TREE = _cv.CV_TYPE_NAME_SEQ_TREE
CV_SET_ELEM_IDX_MASK = _cv.CV_SET_ELEM_IDX_MASK
CV_TYPE_NAME_GRAPH = _cv.CV_TYPE_NAME_GRAPH
CV_SEQ_MAGIC_VAL = _cv.CV_SEQ_MAGIC_VAL
CV_SET_MAGIC_VAL = _cv.CV_SET_MAGIC_VAL
CV_SEQ_ELTYPE_BITS = _cv.CV_SEQ_ELTYPE_BITS
CV_SEQ_ELTYPE_MASK = _cv.CV_SEQ_ELTYPE_MASK
CV_SEQ_ELTYPE_GENERIC = _cv.CV_SEQ_ELTYPE_GENERIC
CV_SEQ_ELTYPE_PTR = _cv.CV_SEQ_ELTYPE_PTR
CV_SEQ_ELTYPE_PPOINT = _cv.CV_SEQ_ELTYPE_PPOINT
CV_SEQ_ELTYPE_GRAPH_EDGE = _cv.CV_SEQ_ELTYPE_GRAPH_EDGE
CV_SEQ_ELTYPE_GRAPH_VERTEX = _cv.CV_SEQ_ELTYPE_GRAPH_VERTEX
CV_SEQ_ELTYPE_TRIAN_ATR = _cv.CV_SEQ_ELTYPE_TRIAN_ATR
CV_SEQ_ELTYPE_CONNECTED_COMP = _cv.CV_SEQ_ELTYPE_CONNECTED_COMP
CV_SEQ_KIND_BITS = _cv.CV_SEQ_KIND_BITS
CV_SEQ_KIND_MASK = _cv.CV_SEQ_KIND_MASK
CV_SEQ_KIND_GENERIC = _cv.CV_SEQ_KIND_GENERIC
CV_SEQ_KIND_CURVE = _cv.CV_SEQ_KIND_CURVE
CV_SEQ_KIND_BIN_TREE = _cv.CV_SEQ_KIND_BIN_TREE
CV_SEQ_KIND_GRAPH = _cv.CV_SEQ_KIND_GRAPH
CV_SEQ_KIND_SUBDIV2D = _cv.CV_SEQ_KIND_SUBDIV2D
CV_SEQ_FLAG_SHIFT = _cv.CV_SEQ_FLAG_SHIFT
CV_SEQ_FLAG_CLOSED = _cv.CV_SEQ_FLAG_CLOSED
CV_SEQ_FLAG_SIMPLE = _cv.CV_SEQ_FLAG_SIMPLE
CV_SEQ_FLAG_CONVEX = _cv.CV_SEQ_FLAG_CONVEX
CV_SEQ_FLAG_HOLE = _cv.CV_SEQ_FLAG_HOLE
CV_GRAPH_FLAG_ORIENTED = _cv.CV_GRAPH_FLAG_ORIENTED
CV_GRAPH = _cv.CV_GRAPH
CV_ORIENTED_GRAPH = _cv.CV_ORIENTED_GRAPH
CV_SEQ_POLYGON_TREE = _cv.CV_SEQ_POLYGON_TREE
CV_SEQ_CONNECTED_COMP = _cv.CV_SEQ_CONNECTED_COMP
CV_STORAGE_READ = _cv.CV_STORAGE_READ
CV_STORAGE_WRITE = _cv.CV_STORAGE_WRITE
CV_STORAGE_WRITE_TEXT = _cv.CV_STORAGE_WRITE_TEXT
CV_STORAGE_WRITE_BINARY = _cv.CV_STORAGE_WRITE_BINARY
CV_NODE_NONE = _cv.CV_NODE_NONE
CV_NODE_INT = _cv.CV_NODE_INT
CV_NODE_INTEGER = _cv.CV_NODE_INTEGER
CV_NODE_REAL = _cv.CV_NODE_REAL
CV_NODE_FLOAT = _cv.CV_NODE_FLOAT
CV_NODE_STR = _cv.CV_NODE_STR
CV_NODE_STRING = _cv.CV_NODE_STRING
CV_NODE_REF = _cv.CV_NODE_REF
CV_NODE_SEQ = _cv.CV_NODE_SEQ
CV_NODE_MAP = _cv.CV_NODE_MAP
CV_NODE_TYPE_MASK = _cv.CV_NODE_TYPE_MASK
CV_NODE_FLOW = _cv.CV_NODE_FLOW
CV_NODE_USER = _cv.CV_NODE_USER
CV_NODE_EMPTY = _cv.CV_NODE_EMPTY
CV_NODE_NAMED = _cv.CV_NODE_NAMED
CV_NODE_SEQ_SIMPLE = _cv.CV_NODE_SEQ_SIMPLE
CV_StsOk = _cv.CV_StsOk
CV_StsBackTrace = _cv.CV_StsBackTrace
CV_StsError = _cv.CV_StsError
CV_StsInternal = _cv.CV_StsInternal
CV_StsNoMem = _cv.CV_StsNoMem
CV_StsBadArg = _cv.CV_StsBadArg
CV_StsBadFunc = _cv.CV_StsBadFunc
CV_StsNoConv = _cv.CV_StsNoConv
CV_StsAutoTrace = _cv.CV_StsAutoTrace
CV_HeaderIsNull = _cv.CV_HeaderIsNull
CV_BadImageSize = _cv.CV_BadImageSize
CV_BadOffset = _cv.CV_BadOffset
CV_BadDataPtr = _cv.CV_BadDataPtr
CV_BadStep = _cv.CV_BadStep
CV_BadModelOrChSeq = _cv.CV_BadModelOrChSeq
CV_BadNumChannels = _cv.CV_BadNumChannels
CV_BadNumChannel1U = _cv.CV_BadNumChannel1U
CV_BadDepth = _cv.CV_BadDepth
CV_BadAlphaChannel = _cv.CV_BadAlphaChannel
CV_BadOrder = _cv.CV_BadOrder
CV_BadOrigin = _cv.CV_BadOrigin
CV_BadAlign = _cv.CV_BadAlign
CV_BadCallBack = _cv.CV_BadCallBack
CV_BadTileSize = _cv.CV_BadTileSize
CV_BadCOI = _cv.CV_BadCOI
CV_BadROISize = _cv.CV_BadROISize
CV_MaskIsTiled = _cv.CV_MaskIsTiled
CV_StsNullPtr = _cv.CV_StsNullPtr
CV_StsVecLengthErr = _cv.CV_StsVecLengthErr
CV_StsFilterStructContentErr = _cv.CV_StsFilterStructContentErr
CV_StsKernelStructContentErr = _cv.CV_StsKernelStructContentErr
CV_StsFilterOffsetErr = _cv.CV_StsFilterOffsetErr
CV_StsBadSize = _cv.CV_StsBadSize
CV_StsDivByZero = _cv.CV_StsDivByZero
CV_StsInplaceNotSupported = _cv.CV_StsInplaceNotSupported
CV_StsObjectNotFound = _cv.CV_StsObjectNotFound
CV_StsUnmatchedFormats = _cv.CV_StsUnmatchedFormats
CV_StsBadFlag = _cv.CV_StsBadFlag
CV_StsBadPoint = _cv.CV_StsBadPoint
CV_StsBadMask = _cv.CV_StsBadMask
CV_StsUnmatchedSizes = _cv.CV_StsUnmatchedSizes
CV_StsUnsupportedFormat = _cv.CV_StsUnsupportedFormat
CV_StsOutOfRange = _cv.CV_StsOutOfRange
CV_StsParseError = _cv.CV_StsParseError
CV_StsNotImplemented = _cv.CV_StsNotImplemented
CV_StsBadMemBlock = _cv.CV_StsBadMemBlock
CV_BLUR_NO_SCALE = _cv.CV_BLUR_NO_SCALE
CV_BLUR = _cv.CV_BLUR
CV_GAUSSIAN = _cv.CV_GAUSSIAN
CV_MEDIAN = _cv.CV_MEDIAN
CV_BILATERAL = _cv.CV_BILATERAL
CV_SCHARR = _cv.CV_SCHARR
CV_BGR2BGRA = _cv.CV_BGR2BGRA
CV_RGB2RGBA = _cv.CV_RGB2RGBA
CV_BGRA2BGR = _cv.CV_BGRA2BGR
CV_RGBA2RGB = _cv.CV_RGBA2RGB
CV_BGR2RGBA = _cv.CV_BGR2RGBA
CV_RGB2BGRA = _cv.CV_RGB2BGRA
CV_RGBA2BGR = _cv.CV_RGBA2BGR
CV_BGRA2RGB = _cv.CV_BGRA2RGB
CV_BGR2RGB = _cv.CV_BGR2RGB
CV_RGB2BGR = _cv.CV_RGB2BGR
CV_BGRA2RGBA = _cv.CV_BGRA2RGBA
CV_RGBA2BGRA = _cv.CV_RGBA2BGRA
CV_BGR2GRAY = _cv.CV_BGR2GRAY
CV_RGB2GRAY = _cv.CV_RGB2GRAY
CV_GRAY2BGR = _cv.CV_GRAY2BGR
CV_GRAY2RGB = _cv.CV_GRAY2RGB
CV_GRAY2BGRA = _cv.CV_GRAY2BGRA
CV_GRAY2RGBA = _cv.CV_GRAY2RGBA
CV_BGRA2GRAY = _cv.CV_BGRA2GRAY
CV_RGBA2GRAY = _cv.CV_RGBA2GRAY
CV_BGR2BGR565 = _cv.CV_BGR2BGR565
CV_RGB2BGR565 = _cv.CV_RGB2BGR565
CV_BGR5652BGR = _cv.CV_BGR5652BGR
CV_BGR5652RGB = _cv.CV_BGR5652RGB
CV_BGRA2BGR565 = _cv.CV_BGRA2BGR565
CV_RGBA2BGR565 = _cv.CV_RGBA2BGR565
CV_BGR5652BGRA = _cv.CV_BGR5652BGRA
CV_BGR5652RGBA = _cv.CV_BGR5652RGBA
CV_GRAY2BGR565 = _cv.CV_GRAY2BGR565
CV_BGR5652GRAY = _cv.CV_BGR5652GRAY
CV_BGR2BGR555 = _cv.CV_BGR2BGR555
CV_RGB2BGR555 = _cv.CV_RGB2BGR555
CV_BGR5552BGR = _cv.CV_BGR5552BGR
CV_BGR5552RGB = _cv.CV_BGR5552RGB
CV_BGRA2BGR555 = _cv.CV_BGRA2BGR555
CV_RGBA2BGR555 = _cv.CV_RGBA2BGR555
CV_BGR5552BGRA = _cv.CV_BGR5552BGRA
CV_BGR5552RGBA = _cv.CV_BGR5552RGBA
CV_GRAY2BGR555 = _cv.CV_GRAY2BGR555
CV_BGR5552GRAY = _cv.CV_BGR5552GRAY
CV_BGR2XYZ = _cv.CV_BGR2XYZ
CV_RGB2XYZ = _cv.CV_RGB2XYZ
CV_XYZ2BGR = _cv.CV_XYZ2BGR
CV_XYZ2RGB = _cv.CV_XYZ2RGB
CV_BGR2YCrCb = _cv.CV_BGR2YCrCb
CV_RGB2YCrCb = _cv.CV_RGB2YCrCb
CV_YCrCb2BGR = _cv.CV_YCrCb2BGR
CV_YCrCb2RGB = _cv.CV_YCrCb2RGB
CV_BGR2HSV = _cv.CV_BGR2HSV
CV_RGB2HSV = _cv.CV_RGB2HSV
CV_BGR2Lab = _cv.CV_BGR2Lab
CV_RGB2Lab = _cv.CV_RGB2Lab
CV_BayerBG2BGR = _cv.CV_BayerBG2BGR
CV_BayerGB2BGR = _cv.CV_BayerGB2BGR
CV_BayerRG2BGR = _cv.CV_BayerRG2BGR
CV_BayerGR2BGR = _cv.CV_BayerGR2BGR
CV_BayerBG2RGB = _cv.CV_BayerBG2RGB
CV_BayerGB2RGB = _cv.CV_BayerGB2RGB
CV_BayerRG2RGB = _cv.CV_BayerRG2RGB
CV_BayerGR2RGB = _cv.CV_BayerGR2RGB
CV_BGR2Luv = _cv.CV_BGR2Luv
CV_RGB2Luv = _cv.CV_RGB2Luv
CV_BGR2HLS = _cv.CV_BGR2HLS
CV_RGB2HLS = _cv.CV_RGB2HLS
CV_HSV2BGR = _cv.CV_HSV2BGR
CV_HSV2RGB = _cv.CV_HSV2RGB
CV_Lab2BGR = _cv.CV_Lab2BGR
CV_Lab2RGB = _cv.CV_Lab2RGB
CV_Luv2BGR = _cv.CV_Luv2BGR
CV_Luv2RGB = _cv.CV_Luv2RGB
CV_HLS2BGR = _cv.CV_HLS2BGR
CV_HLS2RGB = _cv.CV_HLS2RGB
CV_COLORCVT_MAX = _cv.CV_COLORCVT_MAX
CV_INTER_NN = _cv.CV_INTER_NN
CV_INTER_LINEAR = _cv.CV_INTER_LINEAR
CV_INTER_CUBIC = _cv.CV_INTER_CUBIC
CV_INTER_AREA = _cv.CV_INTER_AREA
CV_WARP_FILL_OUTLIERS = _cv.CV_WARP_FILL_OUTLIERS
CV_WARP_INVERSE_MAP = _cv.CV_WARP_INVERSE_MAP
CV_SHAPE_RECT = _cv.CV_SHAPE_RECT
CV_SHAPE_CROSS = _cv.CV_SHAPE_CROSS
CV_SHAPE_ELLIPSE = _cv.CV_SHAPE_ELLIPSE
CV_SHAPE_CUSTOM = _cv.CV_SHAPE_CUSTOM
CV_MOP_OPEN = _cv.CV_MOP_OPEN
CV_MOP_CLOSE = _cv.CV_MOP_CLOSE
CV_MOP_GRADIENT = _cv.CV_MOP_GRADIENT
CV_MOP_TOPHAT = _cv.CV_MOP_TOPHAT
CV_MOP_BLACKHAT = _cv.CV_MOP_BLACKHAT
CV_TM_SQDIFF = _cv.CV_TM_SQDIFF
CV_TM_SQDIFF_NORMED = _cv.CV_TM_SQDIFF_NORMED
CV_TM_CCORR = _cv.CV_TM_CCORR
CV_TM_CCORR_NORMED = _cv.CV_TM_CCORR_NORMED
CV_TM_CCOEFF = _cv.CV_TM_CCOEFF
CV_TM_CCOEFF_NORMED = _cv.CV_TM_CCOEFF_NORMED
CV_LKFLOW_PYR_A_READY = _cv.CV_LKFLOW_PYR_A_READY
CV_LKFLOW_PYR_B_READY = _cv.CV_LKFLOW_PYR_B_READY
CV_LKFLOW_INITIAL_GUESSES = _cv.CV_LKFLOW_INITIAL_GUESSES
CV_POLY_APPROX_DP = _cv.CV_POLY_APPROX_DP
CV_DOMINANT_IPAN = _cv.CV_DOMINANT_IPAN
CV_CONTOURS_MATCH_I1 = _cv.CV_CONTOURS_MATCH_I1
CV_CONTOURS_MATCH_I2 = _cv.CV_CONTOURS_MATCH_I2
CV_CONTOURS_MATCH_I3 = _cv.CV_CONTOURS_MATCH_I3
CV_CONTOUR_TREES_MATCH_I1 = _cv.CV_CONTOUR_TREES_MATCH_I1
CV_CLOCKWISE = _cv.CV_CLOCKWISE
CV_COUNTER_CLOCKWISE = _cv.CV_COUNTER_CLOCKWISE
CV_COMP_CORREL = _cv.CV_COMP_CORREL
CV_COMP_CHISQR = _cv.CV_COMP_CHISQR
CV_COMP_INTERSECT = _cv.CV_COMP_INTERSECT
CV_VALUE = _cv.CV_VALUE
CV_ARRAY = _cv.CV_ARRAY
CV_DIST_MASK_3 = _cv.CV_DIST_MASK_3
CV_DIST_MASK_5 = _cv.CV_DIST_MASK_5
CV_THRESH_BINARY = _cv.CV_THRESH_BINARY
CV_THRESH_BINARY_INV = _cv.CV_THRESH_BINARY_INV
CV_THRESH_TRUNC = _cv.CV_THRESH_TRUNC
CV_THRESH_TOZERO = _cv.CV_THRESH_TOZERO
CV_THRESH_TOZERO_INV = _cv.CV_THRESH_TOZERO_INV
CV_ADAPTIVE_THRESH_MEAN_C = _cv.CV_ADAPTIVE_THRESH_MEAN_C
CV_ADAPTIVE_THRESH_GAUSSIAN_C = _cv.CV_ADAPTIVE_THRESH_GAUSSIAN_C
CV_FLOODFILL_FIXED_RANGE = _cv.CV_FLOODFILL_FIXED_RANGE
CV_FLOODFILL_MASK_ONLY = _cv.CV_FLOODFILL_MASK_ONLY
CV_HOUGH_STANDARD = _cv.CV_HOUGH_STANDARD
CV_HOUGH_PROBABILISTIC = _cv.CV_HOUGH_PROBABILISTIC
CV_HOUGH_MULTI_SCALE = _cv.CV_HOUGH_MULTI_SCALE
CV_HAAR_DO_CANNY_PRUNING = _cv.CV_HAAR_DO_CANNY_PRUNING
CV_RODRIGUES_M2V = _cv.CV_RODRIGUES_M2V
CV_RODRIGUES_V2M = _cv.CV_RODRIGUES_V2M
CV_FM_7POINT = _cv.CV_FM_7POINT
CV_FM_8POINT = _cv.CV_FM_8POINT
CV_FM_RANSAC = _cv.CV_FM_RANSAC
CV_FM_LMEDS = _cv.CV_FM_LMEDS
CV_RETR_EXTERNAL = _cv.CV_RETR_EXTERNAL
CV_RETR_LIST = _cv.CV_RETR_LIST
CV_RETR_CCOMP = _cv.CV_RETR_CCOMP
CV_RETR_TREE = _cv.CV_RETR_TREE
CV_CHAIN_CODE = _cv.CV_CHAIN_CODE
CV_CHAIN_APPROX_NONE = _cv.CV_CHAIN_APPROX_NONE
CV_CHAIN_APPROX_SIMPLE = _cv.CV_CHAIN_APPROX_SIMPLE
CV_CHAIN_APPROX_TC89_L1 = _cv.CV_CHAIN_APPROX_TC89_L1
CV_CHAIN_APPROX_TC89_KCOS = _cv.CV_CHAIN_APPROX_TC89_KCOS
CV_LINK_RUNS = _cv.CV_LINK_RUNS
CV_SUBDIV2D_VIRTUAL_POINT_FLAG = _cv.CV_SUBDIV2D_VIRTUAL_POINT_FLAG
CV_DIST_USER = _cv.CV_DIST_USER
CV_DIST_L1 = _cv.CV_DIST_L1
CV_DIST_L2 = _cv.CV_DIST_L2
CV_DIST_C = _cv.CV_DIST_C
CV_DIST_L12 = _cv.CV_DIST_L12
CV_DIST_FAIR = _cv.CV_DIST_FAIR
CV_DIST_WELSCH = _cv.CV_DIST_WELSCH
CV_DIST_HUBER = _cv.CV_DIST_HUBER
CV_HAAR_MAGIC_VAL = _cv.CV_HAAR_MAGIC_VAL
CV_TYPE_NAME_HAAR = _cv.CV_TYPE_NAME_HAAR
CV_HAAR_FEATURE_MAX = _cv.CV_HAAR_FEATURE_MAX

cvRound = _cv.cvRound

cvFloor = _cv.cvFloor

cvCeil = _cv.cvCeil

cvIsNaN = _cv.cvIsNaN

cvIsInf = _cv.cvIsInf

cvRNG = _cv.cvRNG

cvRandInt = _cv.cvRandInt

cvRandReal = _cv.cvRandReal
class IplImage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, IplImage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, IplImage, name)
    def __repr__(self):
        return "<C IplImage instance at %s>" % (self.this,)
    __swig_setmethods__["ID"] = _cv.IplImage_ID_set
    __swig_getmethods__["ID"] = _cv.IplImage_ID_get
    if _newclass:ID = property(_cv.IplImage_ID_get, _cv.IplImage_ID_set)
    __swig_setmethods__["nChannels"] = _cv.IplImage_nChannels_set
    __swig_getmethods__["nChannels"] = _cv.IplImage_nChannels_get
    if _newclass:nChannels = property(_cv.IplImage_nChannels_get, _cv.IplImage_nChannels_set)
    __swig_setmethods__["depth"] = _cv.IplImage_depth_set
    __swig_getmethods__["depth"] = _cv.IplImage_depth_get
    if _newclass:depth = property(_cv.IplImage_depth_get, _cv.IplImage_depth_set)
    __swig_setmethods__["dataOrder"] = _cv.IplImage_dataOrder_set
    __swig_getmethods__["dataOrder"] = _cv.IplImage_dataOrder_get
    if _newclass:dataOrder = property(_cv.IplImage_dataOrder_get, _cv.IplImage_dataOrder_set)
    __swig_setmethods__["origin"] = _cv.IplImage_origin_set
    __swig_getmethods__["origin"] = _cv.IplImage_origin_get
    if _newclass:origin = property(_cv.IplImage_origin_get, _cv.IplImage_origin_set)
    __swig_setmethods__["align"] = _cv.IplImage_align_set
    __swig_getmethods__["align"] = _cv.IplImage_align_get
    if _newclass:align = property(_cv.IplImage_align_get, _cv.IplImage_align_set)
    __swig_setmethods__["width"] = _cv.IplImage_width_set
    __swig_getmethods__["width"] = _cv.IplImage_width_get
    if _newclass:width = property(_cv.IplImage_width_get, _cv.IplImage_width_set)
    __swig_setmethods__["height"] = _cv.IplImage_height_set
    __swig_getmethods__["height"] = _cv.IplImage_height_get
    if _newclass:height = property(_cv.IplImage_height_get, _cv.IplImage_height_set)
    __swig_setmethods__["roi"] = _cv.IplImage_roi_set
    __swig_getmethods__["roi"] = _cv.IplImage_roi_get
    if _newclass:roi = property(_cv.IplImage_roi_get, _cv.IplImage_roi_set)
    __swig_setmethods__["imageSize"] = _cv.IplImage_imageSize_set
    __swig_getmethods__["imageSize"] = _cv.IplImage_imageSize_get
    if _newclass:imageSize = property(_cv.IplImage_imageSize_get, _cv.IplImage_imageSize_set)
    __swig_setmethods__["widthStep"] = _cv.IplImage_widthStep_set
    __swig_getmethods__["widthStep"] = _cv.IplImage_widthStep_get
    if _newclass:widthStep = property(_cv.IplImage_widthStep_get, _cv.IplImage_widthStep_set)
    def __del__(self, destroy=_cv.delete_IplImage):
        try:
            if self.thisown: destroy(self)
        except: pass
    def imageData_set(*args): return _cv.IplImage_imageData_set(*args)
    def imageData_get(*args): return _cv.IplImage_imageData_get(*args)
    def __init__(self, *args):
        _swig_setattr(self, IplImage, 'this', _cv.new_IplImage(*args))
        _swig_setattr(self, IplImage, 'thisown', 1)

class IplImagePtr(IplImage):
    def __init__(self, this):
        _swig_setattr(self, IplImage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, IplImage, 'thisown', 0)
        _swig_setattr(self, IplImage,self.__class__,IplImage)
_cv.IplImage_swigregister(IplImagePtr)

class IplROI(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, IplROI, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, IplROI, name)
    def __repr__(self):
        return "<C IplROI instance at %s>" % (self.this,)
    __swig_setmethods__["coi"] = _cv.IplROI_coi_set
    __swig_getmethods__["coi"] = _cv.IplROI_coi_get
    if _newclass:coi = property(_cv.IplROI_coi_get, _cv.IplROI_coi_set)
    __swig_setmethods__["xOffset"] = _cv.IplROI_xOffset_set
    __swig_getmethods__["xOffset"] = _cv.IplROI_xOffset_get
    if _newclass:xOffset = property(_cv.IplROI_xOffset_get, _cv.IplROI_xOffset_set)
    __swig_setmethods__["yOffset"] = _cv.IplROI_yOffset_set
    __swig_getmethods__["yOffset"] = _cv.IplROI_yOffset_get
    if _newclass:yOffset = property(_cv.IplROI_yOffset_get, _cv.IplROI_yOffset_set)
    __swig_setmethods__["width"] = _cv.IplROI_width_set
    __swig_getmethods__["width"] = _cv.IplROI_width_get
    if _newclass:width = property(_cv.IplROI_width_get, _cv.IplROI_width_set)
    __swig_setmethods__["height"] = _cv.IplROI_height_set
    __swig_getmethods__["height"] = _cv.IplROI_height_get
    if _newclass:height = property(_cv.IplROI_height_get, _cv.IplROI_height_set)
    def __init__(self, *args):
        _swig_setattr(self, IplROI, 'this', _cv.new_IplROI(*args))
        _swig_setattr(self, IplROI, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_IplROI):
        try:
            if self.thisown: destroy(self)
        except: pass

class IplROIPtr(IplROI):
    def __init__(self, this):
        _swig_setattr(self, IplROI, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, IplROI, 'thisown', 0)
        _swig_setattr(self, IplROI,self.__class__,IplROI)
_cv.IplROI_swigregister(IplROIPtr)

class IplConvKernel(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, IplConvKernel, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, IplConvKernel, name)
    def __repr__(self):
        return "<C IplConvKernel instance at %s>" % (self.this,)
    __swig_setmethods__["nCols"] = _cv.IplConvKernel_nCols_set
    __swig_getmethods__["nCols"] = _cv.IplConvKernel_nCols_get
    if _newclass:nCols = property(_cv.IplConvKernel_nCols_get, _cv.IplConvKernel_nCols_set)
    __swig_setmethods__["nRows"] = _cv.IplConvKernel_nRows_set
    __swig_getmethods__["nRows"] = _cv.IplConvKernel_nRows_get
    if _newclass:nRows = property(_cv.IplConvKernel_nRows_get, _cv.IplConvKernel_nRows_set)
    __swig_setmethods__["anchorX"] = _cv.IplConvKernel_anchorX_set
    __swig_getmethods__["anchorX"] = _cv.IplConvKernel_anchorX_get
    if _newclass:anchorX = property(_cv.IplConvKernel_anchorX_get, _cv.IplConvKernel_anchorX_set)
    __swig_setmethods__["anchorY"] = _cv.IplConvKernel_anchorY_set
    __swig_getmethods__["anchorY"] = _cv.IplConvKernel_anchorY_get
    if _newclass:anchorY = property(_cv.IplConvKernel_anchorY_get, _cv.IplConvKernel_anchorY_set)
    __swig_setmethods__["values"] = _cv.IplConvKernel_values_set
    __swig_getmethods__["values"] = _cv.IplConvKernel_values_get
    if _newclass:values = property(_cv.IplConvKernel_values_get, _cv.IplConvKernel_values_set)
    __swig_setmethods__["nShiftR"] = _cv.IplConvKernel_nShiftR_set
    __swig_getmethods__["nShiftR"] = _cv.IplConvKernel_nShiftR_get
    if _newclass:nShiftR = property(_cv.IplConvKernel_nShiftR_get, _cv.IplConvKernel_nShiftR_set)
    def __del__(self, destroy=_cv.delete_IplConvKernel):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self, *args):
        _swig_setattr(self, IplConvKernel, 'this', _cv.new_IplConvKernel(*args))
        _swig_setattr(self, IplConvKernel, 'thisown', 1)

class IplConvKernelPtr(IplConvKernel):
    def __init__(self, this):
        _swig_setattr(self, IplConvKernel, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, IplConvKernel, 'thisown', 0)
        _swig_setattr(self, IplConvKernel,self.__class__,IplConvKernel)
_cv.IplConvKernel_swigregister(IplConvKernelPtr)

class IplConvKernelFP(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, IplConvKernelFP, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, IplConvKernelFP, name)
    def __repr__(self):
        return "<C IplConvKernelFP instance at %s>" % (self.this,)
    __swig_setmethods__["nCols"] = _cv.IplConvKernelFP_nCols_set
    __swig_getmethods__["nCols"] = _cv.IplConvKernelFP_nCols_get
    if _newclass:nCols = property(_cv.IplConvKernelFP_nCols_get, _cv.IplConvKernelFP_nCols_set)
    __swig_setmethods__["nRows"] = _cv.IplConvKernelFP_nRows_set
    __swig_getmethods__["nRows"] = _cv.IplConvKernelFP_nRows_get
    if _newclass:nRows = property(_cv.IplConvKernelFP_nRows_get, _cv.IplConvKernelFP_nRows_set)
    __swig_setmethods__["anchorX"] = _cv.IplConvKernelFP_anchorX_set
    __swig_getmethods__["anchorX"] = _cv.IplConvKernelFP_anchorX_get
    if _newclass:anchorX = property(_cv.IplConvKernelFP_anchorX_get, _cv.IplConvKernelFP_anchorX_set)
    __swig_setmethods__["anchorY"] = _cv.IplConvKernelFP_anchorY_set
    __swig_getmethods__["anchorY"] = _cv.IplConvKernelFP_anchorY_get
    if _newclass:anchorY = property(_cv.IplConvKernelFP_anchorY_get, _cv.IplConvKernelFP_anchorY_set)
    __swig_setmethods__["values"] = _cv.IplConvKernelFP_values_set
    __swig_getmethods__["values"] = _cv.IplConvKernelFP_values_get
    if _newclass:values = property(_cv.IplConvKernelFP_values_get, _cv.IplConvKernelFP_values_set)
    def __init__(self, *args):
        _swig_setattr(self, IplConvKernelFP, 'this', _cv.new_IplConvKernelFP(*args))
        _swig_setattr(self, IplConvKernelFP, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_IplConvKernelFP):
        try:
            if self.thisown: destroy(self)
        except: pass

class IplConvKernelFPPtr(IplConvKernelFP):
    def __init__(self, this):
        _swig_setattr(self, IplConvKernelFP, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, IplConvKernelFP, 'thisown', 0)
        _swig_setattr(self, IplConvKernelFP,self.__class__,IplConvKernelFP)
_cv.IplConvKernelFP_swigregister(IplConvKernelFPPtr)

class CvMat(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMat, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMat, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvMat instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cv.CvMat_type_set
    __swig_getmethods__["type"] = _cv.CvMat_type_get
    if _newclass:type = property(_cv.CvMat_type_get, _cv.CvMat_type_set)
    __swig_setmethods__["step"] = _cv.CvMat_step_set
    __swig_getmethods__["step"] = _cv.CvMat_step_get
    if _newclass:step = property(_cv.CvMat_step_get, _cv.CvMat_step_set)
    __swig_setmethods__["refcount"] = _cv.CvMat_refcount_set
    __swig_getmethods__["refcount"] = _cv.CvMat_refcount_get
    if _newclass:refcount = property(_cv.CvMat_refcount_get, _cv.CvMat_refcount_set)
    __swig_setmethods__["rows"] = _cv.CvMat_rows_set
    __swig_getmethods__["rows"] = _cv.CvMat_rows_get
    if _newclass:rows = property(_cv.CvMat_rows_get, _cv.CvMat_rows_set)
    __swig_setmethods__["cols"] = _cv.CvMat_cols_set
    __swig_getmethods__["cols"] = _cv.CvMat_cols_get
    if _newclass:cols = property(_cv.CvMat_cols_get, _cv.CvMat_cols_set)
    __swig_getmethods__["data"] = _cv.CvMat_data_get
    if _newclass:data = property(_cv.CvMat_data_get)
    def __del__(self, destroy=_cv.delete_CvMat):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMatPtr(CvMat):
    def __init__(self, this):
        _swig_setattr(self, CvMat, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMat, 'thisown', 0)
        _swig_setattr(self, CvMat,self.__class__,CvMat)
_cv.CvMat_swigregister(CvMatPtr)

class CvMat_data(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMat_data, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMat_data, name)
    def __repr__(self):
        return "<C CvMat_data instance at %s>" % (self.this,)
    __swig_setmethods__["ptr"] = _cv.CvMat_data_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvMat_data_ptr_get
    if _newclass:ptr = property(_cv.CvMat_data_ptr_get, _cv.CvMat_data_ptr_set)
    __swig_setmethods__["s"] = _cv.CvMat_data_s_set
    __swig_getmethods__["s"] = _cv.CvMat_data_s_get
    if _newclass:s = property(_cv.CvMat_data_s_get, _cv.CvMat_data_s_set)
    __swig_setmethods__["i"] = _cv.CvMat_data_i_set
    __swig_getmethods__["i"] = _cv.CvMat_data_i_get
    if _newclass:i = property(_cv.CvMat_data_i_get, _cv.CvMat_data_i_set)
    __swig_setmethods__["fl"] = _cv.CvMat_data_fl_set
    __swig_getmethods__["fl"] = _cv.CvMat_data_fl_get
    if _newclass:fl = property(_cv.CvMat_data_fl_get, _cv.CvMat_data_fl_set)
    __swig_setmethods__["db"] = _cv.CvMat_data_db_set
    __swig_getmethods__["db"] = _cv.CvMat_data_db_get
    if _newclass:db = property(_cv.CvMat_data_db_get, _cv.CvMat_data_db_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMat_data, 'this', _cv.new_CvMat_data(*args))
        _swig_setattr(self, CvMat_data, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMat_data):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMat_dataPtr(CvMat_data):
    def __init__(self, this):
        _swig_setattr(self, CvMat_data, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMat_data, 'thisown', 0)
        _swig_setattr(self, CvMat_data,self.__class__,CvMat_data)
_cv.CvMat_data_swigregister(CvMat_dataPtr)


cvMat = _cv.cvMat

cvmGet = _cv.cvmGet

cvmSet = _cv.cvmSet

cvCvToIplDepth = _cv.cvCvToIplDepth
class CvMatND(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMatND, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMatND, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvMatND instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cv.CvMatND_type_set
    __swig_getmethods__["type"] = _cv.CvMatND_type_get
    if _newclass:type = property(_cv.CvMatND_type_get, _cv.CvMatND_type_set)
    __swig_setmethods__["dims"] = _cv.CvMatND_dims_set
    __swig_getmethods__["dims"] = _cv.CvMatND_dims_get
    if _newclass:dims = property(_cv.CvMatND_dims_get, _cv.CvMatND_dims_set)
    __swig_setmethods__["refcount"] = _cv.CvMatND_refcount_set
    __swig_getmethods__["refcount"] = _cv.CvMatND_refcount_get
    if _newclass:refcount = property(_cv.CvMatND_refcount_get, _cv.CvMatND_refcount_set)
    __swig_getmethods__["dim"] = _cv.CvMatND_dim_get
    if _newclass:dim = property(_cv.CvMatND_dim_get)
    __swig_getmethods__["data"] = _cv.CvMatND_data_get
    if _newclass:data = property(_cv.CvMatND_data_get)
    def __del__(self, destroy=_cv.delete_CvMatND):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMatNDPtr(CvMatND):
    def __init__(self, this):
        _swig_setattr(self, CvMatND, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMatND, 'thisown', 0)
        _swig_setattr(self, CvMatND,self.__class__,CvMatND)
_cv.CvMatND_swigregister(CvMatNDPtr)

class CvMatND_dim(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMatND_dim, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMatND_dim, name)
    def __repr__(self):
        return "<C CvMatND_dim instance at %s>" % (self.this,)
    __swig_setmethods__["size"] = _cv.CvMatND_dim_size_set
    __swig_getmethods__["size"] = _cv.CvMatND_dim_size_get
    if _newclass:size = property(_cv.CvMatND_dim_size_get, _cv.CvMatND_dim_size_set)
    __swig_setmethods__["step"] = _cv.CvMatND_dim_step_set
    __swig_getmethods__["step"] = _cv.CvMatND_dim_step_get
    if _newclass:step = property(_cv.CvMatND_dim_step_get, _cv.CvMatND_dim_step_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMatND_dim, 'this', _cv.new_CvMatND_dim(*args))
        _swig_setattr(self, CvMatND_dim, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMatND_dim):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMatND_dimPtr(CvMatND_dim):
    def __init__(self, this):
        _swig_setattr(self, CvMatND_dim, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMatND_dim, 'thisown', 0)
        _swig_setattr(self, CvMatND_dim,self.__class__,CvMatND_dim)
_cv.CvMatND_dim_swigregister(CvMatND_dimPtr)

class CvMatND_data(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMatND_data, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMatND_data, name)
    def __repr__(self):
        return "<C CvMatND_data instance at %s>" % (self.this,)
    __swig_setmethods__["ptr"] = _cv.CvMatND_data_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvMatND_data_ptr_get
    if _newclass:ptr = property(_cv.CvMatND_data_ptr_get, _cv.CvMatND_data_ptr_set)
    __swig_setmethods__["fl"] = _cv.CvMatND_data_fl_set
    __swig_getmethods__["fl"] = _cv.CvMatND_data_fl_get
    if _newclass:fl = property(_cv.CvMatND_data_fl_get, _cv.CvMatND_data_fl_set)
    __swig_setmethods__["db"] = _cv.CvMatND_data_db_set
    __swig_getmethods__["db"] = _cv.CvMatND_data_db_get
    if _newclass:db = property(_cv.CvMatND_data_db_get, _cv.CvMatND_data_db_set)
    __swig_setmethods__["i"] = _cv.CvMatND_data_i_set
    __swig_getmethods__["i"] = _cv.CvMatND_data_i_get
    if _newclass:i = property(_cv.CvMatND_data_i_get, _cv.CvMatND_data_i_set)
    __swig_setmethods__["s"] = _cv.CvMatND_data_s_set
    __swig_getmethods__["s"] = _cv.CvMatND_data_s_get
    if _newclass:s = property(_cv.CvMatND_data_s_get, _cv.CvMatND_data_s_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMatND_data, 'this', _cv.new_CvMatND_data(*args))
        _swig_setattr(self, CvMatND_data, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMatND_data):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMatND_dataPtr(CvMatND_data):
    def __init__(self, this):
        _swig_setattr(self, CvMatND_data, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMatND_data, 'thisown', 0)
        _swig_setattr(self, CvMatND_data,self.__class__,CvMatND_data)
_cv.CvMatND_data_swigregister(CvMatND_dataPtr)

class CvSparseMat(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSparseMat, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSparseMat, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvSparseMat instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cv.CvSparseMat_type_set
    __swig_getmethods__["type"] = _cv.CvSparseMat_type_get
    if _newclass:type = property(_cv.CvSparseMat_type_get, _cv.CvSparseMat_type_set)
    __swig_setmethods__["dims"] = _cv.CvSparseMat_dims_set
    __swig_getmethods__["dims"] = _cv.CvSparseMat_dims_get
    if _newclass:dims = property(_cv.CvSparseMat_dims_get, _cv.CvSparseMat_dims_set)
    __swig_setmethods__["refcount"] = _cv.CvSparseMat_refcount_set
    __swig_getmethods__["refcount"] = _cv.CvSparseMat_refcount_get
    if _newclass:refcount = property(_cv.CvSparseMat_refcount_get, _cv.CvSparseMat_refcount_set)
    __swig_setmethods__["heap"] = _cv.CvSparseMat_heap_set
    __swig_getmethods__["heap"] = _cv.CvSparseMat_heap_get
    if _newclass:heap = property(_cv.CvSparseMat_heap_get, _cv.CvSparseMat_heap_set)
    __swig_setmethods__["hashtable"] = _cv.CvSparseMat_hashtable_set
    __swig_getmethods__["hashtable"] = _cv.CvSparseMat_hashtable_get
    if _newclass:hashtable = property(_cv.CvSparseMat_hashtable_get, _cv.CvSparseMat_hashtable_set)
    __swig_setmethods__["hashsize"] = _cv.CvSparseMat_hashsize_set
    __swig_getmethods__["hashsize"] = _cv.CvSparseMat_hashsize_get
    if _newclass:hashsize = property(_cv.CvSparseMat_hashsize_get, _cv.CvSparseMat_hashsize_set)
    __swig_setmethods__["valoffset"] = _cv.CvSparseMat_valoffset_set
    __swig_getmethods__["valoffset"] = _cv.CvSparseMat_valoffset_get
    if _newclass:valoffset = property(_cv.CvSparseMat_valoffset_get, _cv.CvSparseMat_valoffset_set)
    __swig_setmethods__["idxoffset"] = _cv.CvSparseMat_idxoffset_set
    __swig_getmethods__["idxoffset"] = _cv.CvSparseMat_idxoffset_get
    if _newclass:idxoffset = property(_cv.CvSparseMat_idxoffset_get, _cv.CvSparseMat_idxoffset_set)
    __swig_setmethods__["size"] = _cv.CvSparseMat_size_set
    __swig_getmethods__["size"] = _cv.CvSparseMat_size_get
    if _newclass:size = property(_cv.CvSparseMat_size_get, _cv.CvSparseMat_size_set)
    def __del__(self, destroy=_cv.delete_CvSparseMat):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSparseMatPtr(CvSparseMat):
    def __init__(self, this):
        _swig_setattr(self, CvSparseMat, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSparseMat, 'thisown', 0)
        _swig_setattr(self, CvSparseMat,self.__class__,CvSparseMat)
_cv.CvSparseMat_swigregister(CvSparseMatPtr)

class CvSparseNode(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSparseNode, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSparseNode, name)
    def __repr__(self):
        return "<C CvSparseNode instance at %s>" % (self.this,)
    __swig_setmethods__["hashval"] = _cv.CvSparseNode_hashval_set
    __swig_getmethods__["hashval"] = _cv.CvSparseNode_hashval_get
    if _newclass:hashval = property(_cv.CvSparseNode_hashval_get, _cv.CvSparseNode_hashval_set)
    __swig_setmethods__["next"] = _cv.CvSparseNode_next_set
    __swig_getmethods__["next"] = _cv.CvSparseNode_next_get
    if _newclass:next = property(_cv.CvSparseNode_next_get, _cv.CvSparseNode_next_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSparseNode, 'this', _cv.new_CvSparseNode(*args))
        _swig_setattr(self, CvSparseNode, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSparseNode):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSparseNodePtr(CvSparseNode):
    def __init__(self, this):
        _swig_setattr(self, CvSparseNode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSparseNode, 'thisown', 0)
        _swig_setattr(self, CvSparseNode,self.__class__,CvSparseNode)
_cv.CvSparseNode_swigregister(CvSparseNodePtr)

class CvSparseMatIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSparseMatIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSparseMatIterator, name)
    def __repr__(self):
        return "<C CvSparseMatIterator instance at %s>" % (self.this,)
    __swig_setmethods__["mat"] = _cv.CvSparseMatIterator_mat_set
    __swig_getmethods__["mat"] = _cv.CvSparseMatIterator_mat_get
    if _newclass:mat = property(_cv.CvSparseMatIterator_mat_get, _cv.CvSparseMatIterator_mat_set)
    __swig_setmethods__["node"] = _cv.CvSparseMatIterator_node_set
    __swig_getmethods__["node"] = _cv.CvSparseMatIterator_node_get
    if _newclass:node = property(_cv.CvSparseMatIterator_node_get, _cv.CvSparseMatIterator_node_set)
    __swig_setmethods__["curidx"] = _cv.CvSparseMatIterator_curidx_set
    __swig_getmethods__["curidx"] = _cv.CvSparseMatIterator_curidx_get
    if _newclass:curidx = property(_cv.CvSparseMatIterator_curidx_get, _cv.CvSparseMatIterator_curidx_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSparseMatIterator, 'this', _cv.new_CvSparseMatIterator(*args))
        _swig_setattr(self, CvSparseMatIterator, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSparseMatIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSparseMatIteratorPtr(CvSparseMatIterator):
    def __init__(self, this):
        _swig_setattr(self, CvSparseMatIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSparseMatIterator, 'thisown', 0)
        _swig_setattr(self, CvSparseMatIterator,self.__class__,CvSparseMatIterator)
_cv.CvSparseMatIterator_swigregister(CvSparseMatIteratorPtr)

class CvHistogram(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHistogram, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHistogram, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvHistogram instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cv.CvHistogram_type_set
    __swig_getmethods__["type"] = _cv.CvHistogram_type_get
    if _newclass:type = property(_cv.CvHistogram_type_get, _cv.CvHistogram_type_set)
    __swig_setmethods__["bins"] = _cv.CvHistogram_bins_set
    __swig_getmethods__["bins"] = _cv.CvHistogram_bins_get
    if _newclass:bins = property(_cv.CvHistogram_bins_get, _cv.CvHistogram_bins_set)
    __swig_setmethods__["thresh"] = _cv.CvHistogram_thresh_set
    __swig_getmethods__["thresh"] = _cv.CvHistogram_thresh_get
    if _newclass:thresh = property(_cv.CvHistogram_thresh_get, _cv.CvHistogram_thresh_set)
    __swig_setmethods__["thresh2"] = _cv.CvHistogram_thresh2_set
    __swig_getmethods__["thresh2"] = _cv.CvHistogram_thresh2_get
    if _newclass:thresh2 = property(_cv.CvHistogram_thresh2_get, _cv.CvHistogram_thresh2_set)
    __swig_setmethods__["mat"] = _cv.CvHistogram_mat_set
    __swig_getmethods__["mat"] = _cv.CvHistogram_mat_get
    if _newclass:mat = property(_cv.CvHistogram_mat_get, _cv.CvHistogram_mat_set)
    def __del__(self, destroy=_cv.delete_CvHistogram):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHistogramPtr(CvHistogram):
    def __init__(self, this):
        _swig_setattr(self, CvHistogram, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHistogram, 'thisown', 0)
        _swig_setattr(self, CvHistogram,self.__class__,CvHistogram)
_cv.CvHistogram_swigregister(CvHistogramPtr)

class CvRect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvRect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvRect, name)
    def __repr__(self):
        return "<C CvRect instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvRect_x_set
    __swig_getmethods__["x"] = _cv.CvRect_x_get
    if _newclass:x = property(_cv.CvRect_x_get, _cv.CvRect_x_set)
    __swig_setmethods__["y"] = _cv.CvRect_y_set
    __swig_getmethods__["y"] = _cv.CvRect_y_get
    if _newclass:y = property(_cv.CvRect_y_get, _cv.CvRect_y_set)
    __swig_setmethods__["width"] = _cv.CvRect_width_set
    __swig_getmethods__["width"] = _cv.CvRect_width_get
    if _newclass:width = property(_cv.CvRect_width_get, _cv.CvRect_width_set)
    __swig_setmethods__["height"] = _cv.CvRect_height_set
    __swig_getmethods__["height"] = _cv.CvRect_height_get
    if _newclass:height = property(_cv.CvRect_height_get, _cv.CvRect_height_set)
    def __init__(self, *args):
        _swig_setattr(self, CvRect, 'this', _cv.new_CvRect(*args))
        _swig_setattr(self, CvRect, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvRect):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvRectPtr(CvRect):
    def __init__(self, this):
        _swig_setattr(self, CvRect, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvRect, 'thisown', 0)
        _swig_setattr(self, CvRect,self.__class__,CvRect)
_cv.CvRect_swigregister(CvRectPtr)


cvRect = _cv.cvRect

cvRectToROI = _cv.cvRectToROI

cvROIToRect = _cv.cvROIToRect
class CvTermCriteria(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvTermCriteria, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvTermCriteria, name)
    def __repr__(self):
        return "<C CvTermCriteria instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cv.CvTermCriteria_type_set
    __swig_getmethods__["type"] = _cv.CvTermCriteria_type_get
    if _newclass:type = property(_cv.CvTermCriteria_type_get, _cv.CvTermCriteria_type_set)
    __swig_setmethods__["max_iter"] = _cv.CvTermCriteria_max_iter_set
    __swig_getmethods__["max_iter"] = _cv.CvTermCriteria_max_iter_get
    if _newclass:max_iter = property(_cv.CvTermCriteria_max_iter_get, _cv.CvTermCriteria_max_iter_set)
    __swig_setmethods__["epsilon"] = _cv.CvTermCriteria_epsilon_set
    __swig_getmethods__["epsilon"] = _cv.CvTermCriteria_epsilon_get
    if _newclass:epsilon = property(_cv.CvTermCriteria_epsilon_get, _cv.CvTermCriteria_epsilon_set)
    def __init__(self, *args):
        _swig_setattr(self, CvTermCriteria, 'this', _cv.new_CvTermCriteria(*args))
        _swig_setattr(self, CvTermCriteria, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvTermCriteria):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvTermCriteriaPtr(CvTermCriteria):
    def __init__(self, this):
        _swig_setattr(self, CvTermCriteria, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvTermCriteria, 'thisown', 0)
        _swig_setattr(self, CvTermCriteria,self.__class__,CvTermCriteria)
_cv.CvTermCriteria_swigregister(CvTermCriteriaPtr)


cvTermCriteria = _cv.cvTermCriteria
class CvPoint(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPoint, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPoint, name)
    def __repr__(self):
        return "<C CvPoint instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvPoint_x_set
    __swig_getmethods__["x"] = _cv.CvPoint_x_get
    if _newclass:x = property(_cv.CvPoint_x_get, _cv.CvPoint_x_set)
    __swig_setmethods__["y"] = _cv.CvPoint_y_set
    __swig_getmethods__["y"] = _cv.CvPoint_y_get
    if _newclass:y = property(_cv.CvPoint_y_get, _cv.CvPoint_y_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPoint, 'this', _cv.new_CvPoint(*args))
        _swig_setattr(self, CvPoint, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPoint):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPointPtr(CvPoint):
    def __init__(self, this):
        _swig_setattr(self, CvPoint, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPoint, 'thisown', 0)
        _swig_setattr(self, CvPoint,self.__class__,CvPoint)
_cv.CvPoint_swigregister(CvPointPtr)


cvPoint = _cv.cvPoint
class CvPoint2D32f(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPoint2D32f, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPoint2D32f, name)
    def __repr__(self):
        return "<C CvPoint2D32f instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvPoint2D32f_x_set
    __swig_getmethods__["x"] = _cv.CvPoint2D32f_x_get
    if _newclass:x = property(_cv.CvPoint2D32f_x_get, _cv.CvPoint2D32f_x_set)
    __swig_setmethods__["y"] = _cv.CvPoint2D32f_y_set
    __swig_getmethods__["y"] = _cv.CvPoint2D32f_y_get
    if _newclass:y = property(_cv.CvPoint2D32f_y_get, _cv.CvPoint2D32f_y_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPoint2D32f, 'this', _cv.new_CvPoint2D32f(*args))
        _swig_setattr(self, CvPoint2D32f, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPoint2D32f):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPoint2D32fPtr(CvPoint2D32f):
    def __init__(self, this):
        _swig_setattr(self, CvPoint2D32f, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPoint2D32f, 'thisown', 0)
        _swig_setattr(self, CvPoint2D32f,self.__class__,CvPoint2D32f)
_cv.CvPoint2D32f_swigregister(CvPoint2D32fPtr)


cvPoint2D32f = _cv.cvPoint2D32f

cvPointTo32f = _cv.cvPointTo32f

cvPointFrom32f = _cv.cvPointFrom32f
class CvPoint3D32f(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPoint3D32f, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPoint3D32f, name)
    def __repr__(self):
        return "<C CvPoint3D32f instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvPoint3D32f_x_set
    __swig_getmethods__["x"] = _cv.CvPoint3D32f_x_get
    if _newclass:x = property(_cv.CvPoint3D32f_x_get, _cv.CvPoint3D32f_x_set)
    __swig_setmethods__["y"] = _cv.CvPoint3D32f_y_set
    __swig_getmethods__["y"] = _cv.CvPoint3D32f_y_get
    if _newclass:y = property(_cv.CvPoint3D32f_y_get, _cv.CvPoint3D32f_y_set)
    __swig_setmethods__["z"] = _cv.CvPoint3D32f_z_set
    __swig_getmethods__["z"] = _cv.CvPoint3D32f_z_get
    if _newclass:z = property(_cv.CvPoint3D32f_z_get, _cv.CvPoint3D32f_z_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPoint3D32f, 'this', _cv.new_CvPoint3D32f(*args))
        _swig_setattr(self, CvPoint3D32f, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPoint3D32f):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPoint3D32fPtr(CvPoint3D32f):
    def __init__(self, this):
        _swig_setattr(self, CvPoint3D32f, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPoint3D32f, 'thisown', 0)
        _swig_setattr(self, CvPoint3D32f,self.__class__,CvPoint3D32f)
_cv.CvPoint3D32f_swigregister(CvPoint3D32fPtr)


cvPoint3D32f = _cv.cvPoint3D32f
class CvPoint2D64f(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPoint2D64f, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPoint2D64f, name)
    def __repr__(self):
        return "<C CvPoint2D64f instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvPoint2D64f_x_set
    __swig_getmethods__["x"] = _cv.CvPoint2D64f_x_get
    if _newclass:x = property(_cv.CvPoint2D64f_x_get, _cv.CvPoint2D64f_x_set)
    __swig_setmethods__["y"] = _cv.CvPoint2D64f_y_set
    __swig_getmethods__["y"] = _cv.CvPoint2D64f_y_get
    if _newclass:y = property(_cv.CvPoint2D64f_y_get, _cv.CvPoint2D64f_y_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPoint2D64f, 'this', _cv.new_CvPoint2D64f(*args))
        _swig_setattr(self, CvPoint2D64f, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPoint2D64f):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPoint2D64fPtr(CvPoint2D64f):
    def __init__(self, this):
        _swig_setattr(self, CvPoint2D64f, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPoint2D64f, 'thisown', 0)
        _swig_setattr(self, CvPoint2D64f,self.__class__,CvPoint2D64f)
_cv.CvPoint2D64f_swigregister(CvPoint2D64fPtr)


cvPoint2D64f = _cv.cvPoint2D64f
class CvPoint3D64f(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPoint3D64f, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPoint3D64f, name)
    def __repr__(self):
        return "<C CvPoint3D64f instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cv.CvPoint3D64f_x_set
    __swig_getmethods__["x"] = _cv.CvPoint3D64f_x_get
    if _newclass:x = property(_cv.CvPoint3D64f_x_get, _cv.CvPoint3D64f_x_set)
    __swig_setmethods__["y"] = _cv.CvPoint3D64f_y_set
    __swig_getmethods__["y"] = _cv.CvPoint3D64f_y_get
    if _newclass:y = property(_cv.CvPoint3D64f_y_get, _cv.CvPoint3D64f_y_set)
    __swig_setmethods__["z"] = _cv.CvPoint3D64f_z_set
    __swig_getmethods__["z"] = _cv.CvPoint3D64f_z_get
    if _newclass:z = property(_cv.CvPoint3D64f_z_get, _cv.CvPoint3D64f_z_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPoint3D64f, 'this', _cv.new_CvPoint3D64f(*args))
        _swig_setattr(self, CvPoint3D64f, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPoint3D64f):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPoint3D64fPtr(CvPoint3D64f):
    def __init__(self, this):
        _swig_setattr(self, CvPoint3D64f, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPoint3D64f, 'thisown', 0)
        _swig_setattr(self, CvPoint3D64f,self.__class__,CvPoint3D64f)
_cv.CvPoint3D64f_swigregister(CvPoint3D64fPtr)


cvPoint3D64f = _cv.cvPoint3D64f
class CvSize(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSize, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSize, name)
    def __repr__(self):
        return "<C CvSize instance at %s>" % (self.this,)
    __swig_setmethods__["width"] = _cv.CvSize_width_set
    __swig_getmethods__["width"] = _cv.CvSize_width_get
    if _newclass:width = property(_cv.CvSize_width_get, _cv.CvSize_width_set)
    __swig_setmethods__["height"] = _cv.CvSize_height_set
    __swig_getmethods__["height"] = _cv.CvSize_height_get
    if _newclass:height = property(_cv.CvSize_height_get, _cv.CvSize_height_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSize, 'this', _cv.new_CvSize(*args))
        _swig_setattr(self, CvSize, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSize):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSizePtr(CvSize):
    def __init__(self, this):
        _swig_setattr(self, CvSize, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSize, 'thisown', 0)
        _swig_setattr(self, CvSize,self.__class__,CvSize)
_cv.CvSize_swigregister(CvSizePtr)


cvSize = _cv.cvSize
class CvSize2D32f(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSize2D32f, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSize2D32f, name)
    def __repr__(self):
        return "<C CvSize2D32f instance at %s>" % (self.this,)
    __swig_setmethods__["width"] = _cv.CvSize2D32f_width_set
    __swig_getmethods__["width"] = _cv.CvSize2D32f_width_get
    if _newclass:width = property(_cv.CvSize2D32f_width_get, _cv.CvSize2D32f_width_set)
    __swig_setmethods__["height"] = _cv.CvSize2D32f_height_set
    __swig_getmethods__["height"] = _cv.CvSize2D32f_height_get
    if _newclass:height = property(_cv.CvSize2D32f_height_get, _cv.CvSize2D32f_height_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSize2D32f, 'this', _cv.new_CvSize2D32f(*args))
        _swig_setattr(self, CvSize2D32f, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSize2D32f):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSize2D32fPtr(CvSize2D32f):
    def __init__(self, this):
        _swig_setattr(self, CvSize2D32f, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSize2D32f, 'thisown', 0)
        _swig_setattr(self, CvSize2D32f,self.__class__,CvSize2D32f)
_cv.CvSize2D32f_swigregister(CvSize2D32fPtr)


cvSize2D32f = _cv.cvSize2D32f
class CvBox2D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvBox2D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvBox2D, name)
    def __repr__(self):
        return "<C CvBox2D instance at %s>" % (self.this,)
    __swig_setmethods__["center"] = _cv.CvBox2D_center_set
    __swig_getmethods__["center"] = _cv.CvBox2D_center_get
    if _newclass:center = property(_cv.CvBox2D_center_get, _cv.CvBox2D_center_set)
    __swig_setmethods__["size"] = _cv.CvBox2D_size_set
    __swig_getmethods__["size"] = _cv.CvBox2D_size_get
    if _newclass:size = property(_cv.CvBox2D_size_get, _cv.CvBox2D_size_set)
    __swig_setmethods__["angle"] = _cv.CvBox2D_angle_set
    __swig_getmethods__["angle"] = _cv.CvBox2D_angle_get
    if _newclass:angle = property(_cv.CvBox2D_angle_get, _cv.CvBox2D_angle_set)
    def __init__(self, *args):
        _swig_setattr(self, CvBox2D, 'this', _cv.new_CvBox2D(*args))
        _swig_setattr(self, CvBox2D, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvBox2D):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvBox2DPtr(CvBox2D):
    def __init__(self, this):
        _swig_setattr(self, CvBox2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvBox2D, 'thisown', 0)
        _swig_setattr(self, CvBox2D,self.__class__,CvBox2D)
_cv.CvBox2D_swigregister(CvBox2DPtr)

class CvSlice(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSlice, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSlice, name)
    def __repr__(self):
        return "<C CvSlice instance at %s>" % (self.this,)
    __swig_setmethods__["start_index"] = _cv.CvSlice_start_index_set
    __swig_getmethods__["start_index"] = _cv.CvSlice_start_index_get
    if _newclass:start_index = property(_cv.CvSlice_start_index_get, _cv.CvSlice_start_index_set)
    __swig_setmethods__["end_index"] = _cv.CvSlice_end_index_set
    __swig_getmethods__["end_index"] = _cv.CvSlice_end_index_get
    if _newclass:end_index = property(_cv.CvSlice_end_index_get, _cv.CvSlice_end_index_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSlice, 'this', _cv.new_CvSlice(*args))
        _swig_setattr(self, CvSlice, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSlice):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSlicePtr(CvSlice):
    def __init__(self, this):
        _swig_setattr(self, CvSlice, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSlice, 'thisown', 0)
        _swig_setattr(self, CvSlice,self.__class__,CvSlice)
_cv.CvSlice_swigregister(CvSlicePtr)


cvSlice = _cv.cvSlice
class CvScalar(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvScalar, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvScalar, name)
    def __repr__(self):
        return "<C CvScalar instance at %s>" % (self.this,)
    __swig_setmethods__["val"] = _cv.CvScalar_val_set
    __swig_getmethods__["val"] = _cv.CvScalar_val_get
    if _newclass:val = property(_cv.CvScalar_val_get, _cv.CvScalar_val_set)
    def __init__(self, *args):
        _swig_setattr(self, CvScalar, 'this', _cv.new_CvScalar(*args))
        _swig_setattr(self, CvScalar, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvScalar):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvScalarPtr(CvScalar):
    def __init__(self, this):
        _swig_setattr(self, CvScalar, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvScalar, 'thisown', 0)
        _swig_setattr(self, CvScalar,self.__class__,CvScalar)
_cv.CvScalar_swigregister(CvScalarPtr)


cvScalar = _cv.cvScalar

cvRealScalar = _cv.cvRealScalar

cvScalarAll = _cv.cvScalarAll
class CvMemBlock(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMemBlock, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMemBlock, name)
    def __repr__(self):
        return "<C CvMemBlock instance at %s>" % (self.this,)
    __swig_setmethods__["prev"] = _cv.CvMemBlock_prev_set
    __swig_getmethods__["prev"] = _cv.CvMemBlock_prev_get
    if _newclass:prev = property(_cv.CvMemBlock_prev_get, _cv.CvMemBlock_prev_set)
    __swig_setmethods__["next"] = _cv.CvMemBlock_next_set
    __swig_getmethods__["next"] = _cv.CvMemBlock_next_get
    if _newclass:next = property(_cv.CvMemBlock_next_get, _cv.CvMemBlock_next_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMemBlock, 'this', _cv.new_CvMemBlock(*args))
        _swig_setattr(self, CvMemBlock, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMemBlock):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMemBlockPtr(CvMemBlock):
    def __init__(self, this):
        _swig_setattr(self, CvMemBlock, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMemBlock, 'thisown', 0)
        _swig_setattr(self, CvMemBlock,self.__class__,CvMemBlock)
_cv.CvMemBlock_swigregister(CvMemBlockPtr)

class CvMemStorage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMemStorage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMemStorage, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvMemStorage instance at %s>" % (self.this,)
    __swig_setmethods__["signature"] = _cv.CvMemStorage_signature_set
    __swig_getmethods__["signature"] = _cv.CvMemStorage_signature_get
    if _newclass:signature = property(_cv.CvMemStorage_signature_get, _cv.CvMemStorage_signature_set)
    __swig_setmethods__["bottom"] = _cv.CvMemStorage_bottom_set
    __swig_getmethods__["bottom"] = _cv.CvMemStorage_bottom_get
    if _newclass:bottom = property(_cv.CvMemStorage_bottom_get, _cv.CvMemStorage_bottom_set)
    __swig_setmethods__["top"] = _cv.CvMemStorage_top_set
    __swig_getmethods__["top"] = _cv.CvMemStorage_top_get
    if _newclass:top = property(_cv.CvMemStorage_top_get, _cv.CvMemStorage_top_set)
    __swig_setmethods__["parent"] = _cv.CvMemStorage_parent_set
    __swig_getmethods__["parent"] = _cv.CvMemStorage_parent_get
    if _newclass:parent = property(_cv.CvMemStorage_parent_get, _cv.CvMemStorage_parent_set)
    __swig_setmethods__["block_size"] = _cv.CvMemStorage_block_size_set
    __swig_getmethods__["block_size"] = _cv.CvMemStorage_block_size_get
    if _newclass:block_size = property(_cv.CvMemStorage_block_size_get, _cv.CvMemStorage_block_size_set)
    __swig_setmethods__["free_space"] = _cv.CvMemStorage_free_space_set
    __swig_getmethods__["free_space"] = _cv.CvMemStorage_free_space_get
    if _newclass:free_space = property(_cv.CvMemStorage_free_space_get, _cv.CvMemStorage_free_space_set)
    def __del__(self, destroy=_cv.delete_CvMemStorage):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMemStoragePtr(CvMemStorage):
    def __init__(self, this):
        _swig_setattr(self, CvMemStorage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMemStorage, 'thisown', 0)
        _swig_setattr(self, CvMemStorage,self.__class__,CvMemStorage)
_cv.CvMemStorage_swigregister(CvMemStoragePtr)

class CvMemStoragePos(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMemStoragePos, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMemStoragePos, name)
    def __repr__(self):
        return "<C CvMemStoragePos instance at %s>" % (self.this,)
    __swig_setmethods__["top"] = _cv.CvMemStoragePos_top_set
    __swig_getmethods__["top"] = _cv.CvMemStoragePos_top_get
    if _newclass:top = property(_cv.CvMemStoragePos_top_get, _cv.CvMemStoragePos_top_set)
    __swig_setmethods__["free_space"] = _cv.CvMemStoragePos_free_space_set
    __swig_getmethods__["free_space"] = _cv.CvMemStoragePos_free_space_get
    if _newclass:free_space = property(_cv.CvMemStoragePos_free_space_get, _cv.CvMemStoragePos_free_space_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMemStoragePos, 'this', _cv.new_CvMemStoragePos(*args))
        _swig_setattr(self, CvMemStoragePos, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMemStoragePos):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMemStoragePosPtr(CvMemStoragePos):
    def __init__(self, this):
        _swig_setattr(self, CvMemStoragePos, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMemStoragePos, 'thisown', 0)
        _swig_setattr(self, CvMemStoragePos,self.__class__,CvMemStoragePos)
_cv.CvMemStoragePos_swigregister(CvMemStoragePosPtr)

class CvSeqBlock(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSeqBlock, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSeqBlock, name)
    def __repr__(self):
        return "<C CvSeqBlock instance at %s>" % (self.this,)
    __swig_setmethods__["prev"] = _cv.CvSeqBlock_prev_set
    __swig_getmethods__["prev"] = _cv.CvSeqBlock_prev_get
    if _newclass:prev = property(_cv.CvSeqBlock_prev_get, _cv.CvSeqBlock_prev_set)
    __swig_setmethods__["next"] = _cv.CvSeqBlock_next_set
    __swig_getmethods__["next"] = _cv.CvSeqBlock_next_get
    if _newclass:next = property(_cv.CvSeqBlock_next_get, _cv.CvSeqBlock_next_set)
    __swig_setmethods__["start_index"] = _cv.CvSeqBlock_start_index_set
    __swig_getmethods__["start_index"] = _cv.CvSeqBlock_start_index_get
    if _newclass:start_index = property(_cv.CvSeqBlock_start_index_get, _cv.CvSeqBlock_start_index_set)
    __swig_setmethods__["count"] = _cv.CvSeqBlock_count_set
    __swig_getmethods__["count"] = _cv.CvSeqBlock_count_get
    if _newclass:count = property(_cv.CvSeqBlock_count_get, _cv.CvSeqBlock_count_set)
    __swig_setmethods__["data"] = _cv.CvSeqBlock_data_set
    __swig_getmethods__["data"] = _cv.CvSeqBlock_data_get
    if _newclass:data = property(_cv.CvSeqBlock_data_get, _cv.CvSeqBlock_data_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSeqBlock, 'this', _cv.new_CvSeqBlock(*args))
        _swig_setattr(self, CvSeqBlock, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSeqBlock):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSeqBlockPtr(CvSeqBlock):
    def __init__(self, this):
        _swig_setattr(self, CvSeqBlock, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSeqBlock, 'thisown', 0)
        _swig_setattr(self, CvSeqBlock,self.__class__,CvSeqBlock)
_cv.CvSeqBlock_swigregister(CvSeqBlockPtr)

class CvSeq(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSeq, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSeq, name)
    def __repr__(self):
        return "<C CvSeq instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvSeq_flags_set
    __swig_getmethods__["flags"] = _cv.CvSeq_flags_get
    if _newclass:flags = property(_cv.CvSeq_flags_get, _cv.CvSeq_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvSeq_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvSeq_header_size_get
    if _newclass:header_size = property(_cv.CvSeq_header_size_get, _cv.CvSeq_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvSeq_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvSeq_h_prev_get
    if _newclass:h_prev = property(_cv.CvSeq_h_prev_get, _cv.CvSeq_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvSeq_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvSeq_h_next_get
    if _newclass:h_next = property(_cv.CvSeq_h_next_get, _cv.CvSeq_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvSeq_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvSeq_v_prev_get
    if _newclass:v_prev = property(_cv.CvSeq_v_prev_get, _cv.CvSeq_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvSeq_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvSeq_v_next_get
    if _newclass:v_next = property(_cv.CvSeq_v_next_get, _cv.CvSeq_v_next_set)
    __swig_setmethods__["total"] = _cv.CvSeq_total_set
    __swig_getmethods__["total"] = _cv.CvSeq_total_get
    if _newclass:total = property(_cv.CvSeq_total_get, _cv.CvSeq_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvSeq_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvSeq_elem_size_get
    if _newclass:elem_size = property(_cv.CvSeq_elem_size_get, _cv.CvSeq_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvSeq_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvSeq_block_max_get
    if _newclass:block_max = property(_cv.CvSeq_block_max_get, _cv.CvSeq_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvSeq_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvSeq_ptr_get
    if _newclass:ptr = property(_cv.CvSeq_ptr_get, _cv.CvSeq_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvSeq_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvSeq_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvSeq_delta_elems_get, _cv.CvSeq_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvSeq_storage_set
    __swig_getmethods__["storage"] = _cv.CvSeq_storage_get
    if _newclass:storage = property(_cv.CvSeq_storage_get, _cv.CvSeq_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvSeq_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvSeq_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvSeq_free_blocks_get, _cv.CvSeq_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvSeq_first_set
    __swig_getmethods__["first"] = _cv.CvSeq_first_get
    if _newclass:first = property(_cv.CvSeq_first_get, _cv.CvSeq_first_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSeq, 'this', _cv.new_CvSeq(*args))
        _swig_setattr(self, CvSeq, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSeq):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSeqPtr(CvSeq):
    def __init__(self, this):
        _swig_setattr(self, CvSeq, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSeq, 'thisown', 0)
        _swig_setattr(self, CvSeq,self.__class__,CvSeq)
_cv.CvSeq_swigregister(CvSeqPtr)

class CvSetElem(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSetElem, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSetElem, name)
    def __repr__(self):
        return "<C CvSetElem instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvSetElem_flags_set
    __swig_getmethods__["flags"] = _cv.CvSetElem_flags_get
    if _newclass:flags = property(_cv.CvSetElem_flags_get, _cv.CvSetElem_flags_set)
    __swig_setmethods__["next_free"] = _cv.CvSetElem_next_free_set
    __swig_getmethods__["next_free"] = _cv.CvSetElem_next_free_get
    if _newclass:next_free = property(_cv.CvSetElem_next_free_get, _cv.CvSetElem_next_free_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSetElem, 'this', _cv.new_CvSetElem(*args))
        _swig_setattr(self, CvSetElem, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSetElem):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSetElemPtr(CvSetElem):
    def __init__(self, this):
        _swig_setattr(self, CvSetElem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSetElem, 'thisown', 0)
        _swig_setattr(self, CvSetElem,self.__class__,CvSetElem)
_cv.CvSetElem_swigregister(CvSetElemPtr)

class CvSet(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSet, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSet, name)
    def __repr__(self):
        return "<C CvSet instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvSet_flags_set
    __swig_getmethods__["flags"] = _cv.CvSet_flags_get
    if _newclass:flags = property(_cv.CvSet_flags_get, _cv.CvSet_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvSet_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvSet_header_size_get
    if _newclass:header_size = property(_cv.CvSet_header_size_get, _cv.CvSet_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvSet_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvSet_h_prev_get
    if _newclass:h_prev = property(_cv.CvSet_h_prev_get, _cv.CvSet_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvSet_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvSet_h_next_get
    if _newclass:h_next = property(_cv.CvSet_h_next_get, _cv.CvSet_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvSet_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvSet_v_prev_get
    if _newclass:v_prev = property(_cv.CvSet_v_prev_get, _cv.CvSet_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvSet_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvSet_v_next_get
    if _newclass:v_next = property(_cv.CvSet_v_next_get, _cv.CvSet_v_next_set)
    __swig_setmethods__["total"] = _cv.CvSet_total_set
    __swig_getmethods__["total"] = _cv.CvSet_total_get
    if _newclass:total = property(_cv.CvSet_total_get, _cv.CvSet_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvSet_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvSet_elem_size_get
    if _newclass:elem_size = property(_cv.CvSet_elem_size_get, _cv.CvSet_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvSet_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvSet_block_max_get
    if _newclass:block_max = property(_cv.CvSet_block_max_get, _cv.CvSet_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvSet_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvSet_ptr_get
    if _newclass:ptr = property(_cv.CvSet_ptr_get, _cv.CvSet_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvSet_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvSet_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvSet_delta_elems_get, _cv.CvSet_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvSet_storage_set
    __swig_getmethods__["storage"] = _cv.CvSet_storage_get
    if _newclass:storage = property(_cv.CvSet_storage_get, _cv.CvSet_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvSet_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvSet_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvSet_free_blocks_get, _cv.CvSet_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvSet_first_set
    __swig_getmethods__["first"] = _cv.CvSet_first_get
    if _newclass:first = property(_cv.CvSet_first_get, _cv.CvSet_first_set)
    __swig_setmethods__["free_elems"] = _cv.CvSet_free_elems_set
    __swig_getmethods__["free_elems"] = _cv.CvSet_free_elems_get
    if _newclass:free_elems = property(_cv.CvSet_free_elems_get, _cv.CvSet_free_elems_set)
    __swig_setmethods__["active_count"] = _cv.CvSet_active_count_set
    __swig_getmethods__["active_count"] = _cv.CvSet_active_count_get
    if _newclass:active_count = property(_cv.CvSet_active_count_get, _cv.CvSet_active_count_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSet, 'this', _cv.new_CvSet(*args))
        _swig_setattr(self, CvSet, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSet):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSetPtr(CvSet):
    def __init__(self, this):
        _swig_setattr(self, CvSet, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSet, 'thisown', 0)
        _swig_setattr(self, CvSet,self.__class__,CvSet)
_cv.CvSet_swigregister(CvSetPtr)

class CvGraphEdge(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvGraphEdge, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvGraphEdge, name)
    def __repr__(self):
        return "<C CvGraphEdge instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvGraphEdge_flags_set
    __swig_getmethods__["flags"] = _cv.CvGraphEdge_flags_get
    if _newclass:flags = property(_cv.CvGraphEdge_flags_get, _cv.CvGraphEdge_flags_set)
    __swig_setmethods__["weight"] = _cv.CvGraphEdge_weight_set
    __swig_getmethods__["weight"] = _cv.CvGraphEdge_weight_get
    if _newclass:weight = property(_cv.CvGraphEdge_weight_get, _cv.CvGraphEdge_weight_set)
    __swig_setmethods__["next"] = _cv.CvGraphEdge_next_set
    __swig_getmethods__["next"] = _cv.CvGraphEdge_next_get
    if _newclass:next = property(_cv.CvGraphEdge_next_get, _cv.CvGraphEdge_next_set)
    __swig_setmethods__["vtx"] = _cv.CvGraphEdge_vtx_set
    __swig_getmethods__["vtx"] = _cv.CvGraphEdge_vtx_get
    if _newclass:vtx = property(_cv.CvGraphEdge_vtx_get, _cv.CvGraphEdge_vtx_set)
    def __init__(self, *args):
        _swig_setattr(self, CvGraphEdge, 'this', _cv.new_CvGraphEdge(*args))
        _swig_setattr(self, CvGraphEdge, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvGraphEdge):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvGraphEdgePtr(CvGraphEdge):
    def __init__(self, this):
        _swig_setattr(self, CvGraphEdge, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvGraphEdge, 'thisown', 0)
        _swig_setattr(self, CvGraphEdge,self.__class__,CvGraphEdge)
_cv.CvGraphEdge_swigregister(CvGraphEdgePtr)

class CvGraphVtx(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvGraphVtx, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvGraphVtx, name)
    def __repr__(self):
        return "<C CvGraphVtx instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvGraphVtx_flags_set
    __swig_getmethods__["flags"] = _cv.CvGraphVtx_flags_get
    if _newclass:flags = property(_cv.CvGraphVtx_flags_get, _cv.CvGraphVtx_flags_set)
    __swig_setmethods__["first"] = _cv.CvGraphVtx_first_set
    __swig_getmethods__["first"] = _cv.CvGraphVtx_first_get
    if _newclass:first = property(_cv.CvGraphVtx_first_get, _cv.CvGraphVtx_first_set)
    def __init__(self, *args):
        _swig_setattr(self, CvGraphVtx, 'this', _cv.new_CvGraphVtx(*args))
        _swig_setattr(self, CvGraphVtx, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvGraphVtx):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvGraphVtxPtr(CvGraphVtx):
    def __init__(self, this):
        _swig_setattr(self, CvGraphVtx, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvGraphVtx, 'thisown', 0)
        _swig_setattr(self, CvGraphVtx,self.__class__,CvGraphVtx)
_cv.CvGraphVtx_swigregister(CvGraphVtxPtr)

class CvGraphVtx2D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvGraphVtx2D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvGraphVtx2D, name)
    def __repr__(self):
        return "<C CvGraphVtx2D instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvGraphVtx2D_flags_set
    __swig_getmethods__["flags"] = _cv.CvGraphVtx2D_flags_get
    if _newclass:flags = property(_cv.CvGraphVtx2D_flags_get, _cv.CvGraphVtx2D_flags_set)
    __swig_setmethods__["first"] = _cv.CvGraphVtx2D_first_set
    __swig_getmethods__["first"] = _cv.CvGraphVtx2D_first_get
    if _newclass:first = property(_cv.CvGraphVtx2D_first_get, _cv.CvGraphVtx2D_first_set)
    __swig_setmethods__["ptr"] = _cv.CvGraphVtx2D_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvGraphVtx2D_ptr_get
    if _newclass:ptr = property(_cv.CvGraphVtx2D_ptr_get, _cv.CvGraphVtx2D_ptr_set)
    def __init__(self, *args):
        _swig_setattr(self, CvGraphVtx2D, 'this', _cv.new_CvGraphVtx2D(*args))
        _swig_setattr(self, CvGraphVtx2D, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvGraphVtx2D):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvGraphVtx2DPtr(CvGraphVtx2D):
    def __init__(self, this):
        _swig_setattr(self, CvGraphVtx2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvGraphVtx2D, 'thisown', 0)
        _swig_setattr(self, CvGraphVtx2D,self.__class__,CvGraphVtx2D)
_cv.CvGraphVtx2D_swigregister(CvGraphVtx2DPtr)

class CvGraph(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvGraph, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvGraph, name)
    def __repr__(self):
        return "<C CvGraph instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvGraph_flags_set
    __swig_getmethods__["flags"] = _cv.CvGraph_flags_get
    if _newclass:flags = property(_cv.CvGraph_flags_get, _cv.CvGraph_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvGraph_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvGraph_header_size_get
    if _newclass:header_size = property(_cv.CvGraph_header_size_get, _cv.CvGraph_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvGraph_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvGraph_h_prev_get
    if _newclass:h_prev = property(_cv.CvGraph_h_prev_get, _cv.CvGraph_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvGraph_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvGraph_h_next_get
    if _newclass:h_next = property(_cv.CvGraph_h_next_get, _cv.CvGraph_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvGraph_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvGraph_v_prev_get
    if _newclass:v_prev = property(_cv.CvGraph_v_prev_get, _cv.CvGraph_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvGraph_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvGraph_v_next_get
    if _newclass:v_next = property(_cv.CvGraph_v_next_get, _cv.CvGraph_v_next_set)
    __swig_setmethods__["total"] = _cv.CvGraph_total_set
    __swig_getmethods__["total"] = _cv.CvGraph_total_get
    if _newclass:total = property(_cv.CvGraph_total_get, _cv.CvGraph_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvGraph_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvGraph_elem_size_get
    if _newclass:elem_size = property(_cv.CvGraph_elem_size_get, _cv.CvGraph_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvGraph_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvGraph_block_max_get
    if _newclass:block_max = property(_cv.CvGraph_block_max_get, _cv.CvGraph_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvGraph_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvGraph_ptr_get
    if _newclass:ptr = property(_cv.CvGraph_ptr_get, _cv.CvGraph_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvGraph_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvGraph_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvGraph_delta_elems_get, _cv.CvGraph_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvGraph_storage_set
    __swig_getmethods__["storage"] = _cv.CvGraph_storage_get
    if _newclass:storage = property(_cv.CvGraph_storage_get, _cv.CvGraph_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvGraph_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvGraph_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvGraph_free_blocks_get, _cv.CvGraph_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvGraph_first_set
    __swig_getmethods__["first"] = _cv.CvGraph_first_get
    if _newclass:first = property(_cv.CvGraph_first_get, _cv.CvGraph_first_set)
    __swig_setmethods__["free_elems"] = _cv.CvGraph_free_elems_set
    __swig_getmethods__["free_elems"] = _cv.CvGraph_free_elems_get
    if _newclass:free_elems = property(_cv.CvGraph_free_elems_get, _cv.CvGraph_free_elems_set)
    __swig_setmethods__["active_count"] = _cv.CvGraph_active_count_set
    __swig_getmethods__["active_count"] = _cv.CvGraph_active_count_get
    if _newclass:active_count = property(_cv.CvGraph_active_count_get, _cv.CvGraph_active_count_set)
    __swig_setmethods__["edges"] = _cv.CvGraph_edges_set
    __swig_getmethods__["edges"] = _cv.CvGraph_edges_get
    if _newclass:edges = property(_cv.CvGraph_edges_get, _cv.CvGraph_edges_set)
    def __init__(self, *args):
        _swig_setattr(self, CvGraph, 'this', _cv.new_CvGraph(*args))
        _swig_setattr(self, CvGraph, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvGraph):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvGraphPtr(CvGraph):
    def __init__(self, this):
        _swig_setattr(self, CvGraph, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvGraph, 'thisown', 0)
        _swig_setattr(self, CvGraph,self.__class__,CvGraph)
_cv.CvGraph_swigregister(CvGraphPtr)

class CvChain(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvChain, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvChain, name)
    def __repr__(self):
        return "<C CvChain instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvChain_flags_set
    __swig_getmethods__["flags"] = _cv.CvChain_flags_get
    if _newclass:flags = property(_cv.CvChain_flags_get, _cv.CvChain_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvChain_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvChain_header_size_get
    if _newclass:header_size = property(_cv.CvChain_header_size_get, _cv.CvChain_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvChain_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvChain_h_prev_get
    if _newclass:h_prev = property(_cv.CvChain_h_prev_get, _cv.CvChain_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvChain_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvChain_h_next_get
    if _newclass:h_next = property(_cv.CvChain_h_next_get, _cv.CvChain_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvChain_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvChain_v_prev_get
    if _newclass:v_prev = property(_cv.CvChain_v_prev_get, _cv.CvChain_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvChain_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvChain_v_next_get
    if _newclass:v_next = property(_cv.CvChain_v_next_get, _cv.CvChain_v_next_set)
    __swig_setmethods__["total"] = _cv.CvChain_total_set
    __swig_getmethods__["total"] = _cv.CvChain_total_get
    if _newclass:total = property(_cv.CvChain_total_get, _cv.CvChain_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvChain_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvChain_elem_size_get
    if _newclass:elem_size = property(_cv.CvChain_elem_size_get, _cv.CvChain_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvChain_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvChain_block_max_get
    if _newclass:block_max = property(_cv.CvChain_block_max_get, _cv.CvChain_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvChain_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvChain_ptr_get
    if _newclass:ptr = property(_cv.CvChain_ptr_get, _cv.CvChain_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvChain_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvChain_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvChain_delta_elems_get, _cv.CvChain_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvChain_storage_set
    __swig_getmethods__["storage"] = _cv.CvChain_storage_get
    if _newclass:storage = property(_cv.CvChain_storage_get, _cv.CvChain_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvChain_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvChain_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvChain_free_blocks_get, _cv.CvChain_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvChain_first_set
    __swig_getmethods__["first"] = _cv.CvChain_first_get
    if _newclass:first = property(_cv.CvChain_first_get, _cv.CvChain_first_set)
    __swig_setmethods__["origin"] = _cv.CvChain_origin_set
    __swig_getmethods__["origin"] = _cv.CvChain_origin_get
    if _newclass:origin = property(_cv.CvChain_origin_get, _cv.CvChain_origin_set)
    def __init__(self, *args):
        _swig_setattr(self, CvChain, 'this', _cv.new_CvChain(*args))
        _swig_setattr(self, CvChain, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvChain):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvChainPtr(CvChain):
    def __init__(self, this):
        _swig_setattr(self, CvChain, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvChain, 'thisown', 0)
        _swig_setattr(self, CvChain,self.__class__,CvChain)
_cv.CvChain_swigregister(CvChainPtr)

class CvContour(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvContour, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvContour, name)
    def __repr__(self):
        return "<C CvContour instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvContour_flags_set
    __swig_getmethods__["flags"] = _cv.CvContour_flags_get
    if _newclass:flags = property(_cv.CvContour_flags_get, _cv.CvContour_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvContour_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvContour_header_size_get
    if _newclass:header_size = property(_cv.CvContour_header_size_get, _cv.CvContour_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvContour_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvContour_h_prev_get
    if _newclass:h_prev = property(_cv.CvContour_h_prev_get, _cv.CvContour_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvContour_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvContour_h_next_get
    if _newclass:h_next = property(_cv.CvContour_h_next_get, _cv.CvContour_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvContour_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvContour_v_prev_get
    if _newclass:v_prev = property(_cv.CvContour_v_prev_get, _cv.CvContour_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvContour_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvContour_v_next_get
    if _newclass:v_next = property(_cv.CvContour_v_next_get, _cv.CvContour_v_next_set)
    __swig_setmethods__["total"] = _cv.CvContour_total_set
    __swig_getmethods__["total"] = _cv.CvContour_total_get
    if _newclass:total = property(_cv.CvContour_total_get, _cv.CvContour_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvContour_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvContour_elem_size_get
    if _newclass:elem_size = property(_cv.CvContour_elem_size_get, _cv.CvContour_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvContour_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvContour_block_max_get
    if _newclass:block_max = property(_cv.CvContour_block_max_get, _cv.CvContour_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvContour_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvContour_ptr_get
    if _newclass:ptr = property(_cv.CvContour_ptr_get, _cv.CvContour_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvContour_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvContour_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvContour_delta_elems_get, _cv.CvContour_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvContour_storage_set
    __swig_getmethods__["storage"] = _cv.CvContour_storage_get
    if _newclass:storage = property(_cv.CvContour_storage_get, _cv.CvContour_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvContour_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvContour_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvContour_free_blocks_get, _cv.CvContour_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvContour_first_set
    __swig_getmethods__["first"] = _cv.CvContour_first_get
    if _newclass:first = property(_cv.CvContour_first_get, _cv.CvContour_first_set)
    __swig_setmethods__["rect"] = _cv.CvContour_rect_set
    __swig_getmethods__["rect"] = _cv.CvContour_rect_get
    if _newclass:rect = property(_cv.CvContour_rect_get, _cv.CvContour_rect_set)
    __swig_setmethods__["color"] = _cv.CvContour_color_set
    __swig_getmethods__["color"] = _cv.CvContour_color_get
    if _newclass:color = property(_cv.CvContour_color_get, _cv.CvContour_color_set)
    __swig_setmethods__["reserved"] = _cv.CvContour_reserved_set
    __swig_getmethods__["reserved"] = _cv.CvContour_reserved_get
    if _newclass:reserved = property(_cv.CvContour_reserved_get, _cv.CvContour_reserved_set)
    def __init__(self, *args):
        _swig_setattr(self, CvContour, 'this', _cv.new_CvContour(*args))
        _swig_setattr(self, CvContour, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvContour):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvContourPtr(CvContour):
    def __init__(self, this):
        _swig_setattr(self, CvContour, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvContour, 'thisown', 0)
        _swig_setattr(self, CvContour,self.__class__,CvContour)
_cv.CvContour_swigregister(CvContourPtr)

class CvSeqWriter(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSeqWriter, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSeqWriter, name)
    def __repr__(self):
        return "<C CvSeqWriter instance at %s>" % (self.this,)
    __swig_setmethods__["header_size"] = _cv.CvSeqWriter_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvSeqWriter_header_size_get
    if _newclass:header_size = property(_cv.CvSeqWriter_header_size_get, _cv.CvSeqWriter_header_size_set)
    __swig_setmethods__["seq"] = _cv.CvSeqWriter_seq_set
    __swig_getmethods__["seq"] = _cv.CvSeqWriter_seq_get
    if _newclass:seq = property(_cv.CvSeqWriter_seq_get, _cv.CvSeqWriter_seq_set)
    __swig_setmethods__["block"] = _cv.CvSeqWriter_block_set
    __swig_getmethods__["block"] = _cv.CvSeqWriter_block_get
    if _newclass:block = property(_cv.CvSeqWriter_block_get, _cv.CvSeqWriter_block_set)
    __swig_setmethods__["ptr"] = _cv.CvSeqWriter_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvSeqWriter_ptr_get
    if _newclass:ptr = property(_cv.CvSeqWriter_ptr_get, _cv.CvSeqWriter_ptr_set)
    __swig_setmethods__["block_min"] = _cv.CvSeqWriter_block_min_set
    __swig_getmethods__["block_min"] = _cv.CvSeqWriter_block_min_get
    if _newclass:block_min = property(_cv.CvSeqWriter_block_min_get, _cv.CvSeqWriter_block_min_set)
    __swig_setmethods__["block_max"] = _cv.CvSeqWriter_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvSeqWriter_block_max_get
    if _newclass:block_max = property(_cv.CvSeqWriter_block_max_get, _cv.CvSeqWriter_block_max_set)
    __swig_setmethods__["reserved"] = _cv.CvSeqWriter_reserved_set
    __swig_getmethods__["reserved"] = _cv.CvSeqWriter_reserved_get
    if _newclass:reserved = property(_cv.CvSeqWriter_reserved_get, _cv.CvSeqWriter_reserved_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSeqWriter, 'this', _cv.new_CvSeqWriter(*args))
        _swig_setattr(self, CvSeqWriter, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSeqWriter):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSeqWriterPtr(CvSeqWriter):
    def __init__(self, this):
        _swig_setattr(self, CvSeqWriter, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSeqWriter, 'thisown', 0)
        _swig_setattr(self, CvSeqWriter,self.__class__,CvSeqWriter)
_cv.CvSeqWriter_swigregister(CvSeqWriterPtr)

class CvSeqReader(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSeqReader, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSeqReader, name)
    def __repr__(self):
        return "<C CvSeqReader instance at %s>" % (self.this,)
    __swig_setmethods__["header_size"] = _cv.CvSeqReader_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvSeqReader_header_size_get
    if _newclass:header_size = property(_cv.CvSeqReader_header_size_get, _cv.CvSeqReader_header_size_set)
    __swig_setmethods__["seq"] = _cv.CvSeqReader_seq_set
    __swig_getmethods__["seq"] = _cv.CvSeqReader_seq_get
    if _newclass:seq = property(_cv.CvSeqReader_seq_get, _cv.CvSeqReader_seq_set)
    __swig_setmethods__["block"] = _cv.CvSeqReader_block_set
    __swig_getmethods__["block"] = _cv.CvSeqReader_block_get
    if _newclass:block = property(_cv.CvSeqReader_block_get, _cv.CvSeqReader_block_set)
    __swig_setmethods__["ptr"] = _cv.CvSeqReader_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvSeqReader_ptr_get
    if _newclass:ptr = property(_cv.CvSeqReader_ptr_get, _cv.CvSeqReader_ptr_set)
    __swig_setmethods__["block_min"] = _cv.CvSeqReader_block_min_set
    __swig_getmethods__["block_min"] = _cv.CvSeqReader_block_min_get
    if _newclass:block_min = property(_cv.CvSeqReader_block_min_get, _cv.CvSeqReader_block_min_set)
    __swig_setmethods__["block_max"] = _cv.CvSeqReader_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvSeqReader_block_max_get
    if _newclass:block_max = property(_cv.CvSeqReader_block_max_get, _cv.CvSeqReader_block_max_set)
    __swig_setmethods__["delta_index"] = _cv.CvSeqReader_delta_index_set
    __swig_getmethods__["delta_index"] = _cv.CvSeqReader_delta_index_get
    if _newclass:delta_index = property(_cv.CvSeqReader_delta_index_get, _cv.CvSeqReader_delta_index_set)
    __swig_setmethods__["prev_elem"] = _cv.CvSeqReader_prev_elem_set
    __swig_getmethods__["prev_elem"] = _cv.CvSeqReader_prev_elem_get
    if _newclass:prev_elem = property(_cv.CvSeqReader_prev_elem_get, _cv.CvSeqReader_prev_elem_set)
    __swig_setmethods__["reserved"] = _cv.CvSeqReader_reserved_set
    __swig_getmethods__["reserved"] = _cv.CvSeqReader_reserved_get
    if _newclass:reserved = property(_cv.CvSeqReader_reserved_get, _cv.CvSeqReader_reserved_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSeqReader, 'this', _cv.new_CvSeqReader(*args))
        _swig_setattr(self, CvSeqReader, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSeqReader):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSeqReaderPtr(CvSeqReader):
    def __init__(self, this):
        _swig_setattr(self, CvSeqReader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSeqReader, 'thisown', 0)
        _swig_setattr(self, CvSeqReader,self.__class__,CvSeqReader)
_cv.CvSeqReader_swigregister(CvSeqReaderPtr)

class CvAttrList(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvAttrList, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvAttrList, name)
    def __repr__(self):
        return "<C CvAttrList instance at %s>" % (self.this,)
    __swig_setmethods__["attr"] = _cv.CvAttrList_attr_set
    __swig_getmethods__["attr"] = _cv.CvAttrList_attr_get
    if _newclass:attr = property(_cv.CvAttrList_attr_get, _cv.CvAttrList_attr_set)
    __swig_setmethods__["next"] = _cv.CvAttrList_next_set
    __swig_getmethods__["next"] = _cv.CvAttrList_next_get
    if _newclass:next = property(_cv.CvAttrList_next_get, _cv.CvAttrList_next_set)
    def __init__(self, *args):
        _swig_setattr(self, CvAttrList, 'this', _cv.new_CvAttrList(*args))
        _swig_setattr(self, CvAttrList, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvAttrList):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvAttrListPtr(CvAttrList):
    def __init__(self, this):
        _swig_setattr(self, CvAttrList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvAttrList, 'thisown', 0)
        _swig_setattr(self, CvAttrList,self.__class__,CvAttrList)
_cv.CvAttrList_swigregister(CvAttrListPtr)


cvAttrList = _cv.cvAttrList
class CvString(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvString, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvString, name)
    def __repr__(self):
        return "<C CvString instance at %s>" % (self.this,)
    __swig_setmethods__["len"] = _cv.CvString_len_set
    __swig_getmethods__["len"] = _cv.CvString_len_get
    if _newclass:len = property(_cv.CvString_len_get, _cv.CvString_len_set)
    __swig_setmethods__["ptr"] = _cv.CvString_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvString_ptr_get
    if _newclass:ptr = property(_cv.CvString_ptr_get, _cv.CvString_ptr_set)
    def __init__(self, *args):
        _swig_setattr(self, CvString, 'this', _cv.new_CvString(*args))
        _swig_setattr(self, CvString, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvString):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvStringPtr(CvString):
    def __init__(self, this):
        _swig_setattr(self, CvString, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvString, 'thisown', 0)
        _swig_setattr(self, CvString,self.__class__,CvString)
_cv.CvString_swigregister(CvStringPtr)

class CvStringHashNode(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvStringHashNode, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvStringHashNode, name)
    def __repr__(self):
        return "<C CvStringHashNode instance at %s>" % (self.this,)
    __swig_setmethods__["hashval"] = _cv.CvStringHashNode_hashval_set
    __swig_getmethods__["hashval"] = _cv.CvStringHashNode_hashval_get
    if _newclass:hashval = property(_cv.CvStringHashNode_hashval_get, _cv.CvStringHashNode_hashval_set)
    __swig_setmethods__["str"] = _cv.CvStringHashNode_str_set
    __swig_getmethods__["str"] = _cv.CvStringHashNode_str_get
    if _newclass:str = property(_cv.CvStringHashNode_str_get, _cv.CvStringHashNode_str_set)
    __swig_setmethods__["next"] = _cv.CvStringHashNode_next_set
    __swig_getmethods__["next"] = _cv.CvStringHashNode_next_get
    if _newclass:next = property(_cv.CvStringHashNode_next_get, _cv.CvStringHashNode_next_set)
    def __init__(self, *args):
        _swig_setattr(self, CvStringHashNode, 'this', _cv.new_CvStringHashNode(*args))
        _swig_setattr(self, CvStringHashNode, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvStringHashNode):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvStringHashNodePtr(CvStringHashNode):
    def __init__(self, this):
        _swig_setattr(self, CvStringHashNode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvStringHashNode, 'thisown', 0)
        _swig_setattr(self, CvStringHashNode,self.__class__,CvStringHashNode)
_cv.CvStringHashNode_swigregister(CvStringHashNodePtr)

class CvFileNode(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvFileNode, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvFileNode, name)
    def __repr__(self):
        return "<C CvFileNode instance at %s>" % (self.this,)
    __swig_setmethods__["tag"] = _cv.CvFileNode_tag_set
    __swig_getmethods__["tag"] = _cv.CvFileNode_tag_get
    if _newclass:tag = property(_cv.CvFileNode_tag_get, _cv.CvFileNode_tag_set)
    __swig_setmethods__["info"] = _cv.CvFileNode_info_set
    __swig_getmethods__["info"] = _cv.CvFileNode_info_get
    if _newclass:info = property(_cv.CvFileNode_info_get, _cv.CvFileNode_info_set)
    __swig_getmethods__["data"] = _cv.CvFileNode_data_get
    if _newclass:data = property(_cv.CvFileNode_data_get)
    def __init__(self, *args):
        _swig_setattr(self, CvFileNode, 'this', _cv.new_CvFileNode(*args))
        _swig_setattr(self, CvFileNode, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvFileNode):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvFileNodePtr(CvFileNode):
    def __init__(self, this):
        _swig_setattr(self, CvFileNode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvFileNode, 'thisown', 0)
        _swig_setattr(self, CvFileNode,self.__class__,CvFileNode)
_cv.CvFileNode_swigregister(CvFileNodePtr)

class CvFileNode_data(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvFileNode_data, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvFileNode_data, name)
    def __repr__(self):
        return "<C CvFileNode_data instance at %s>" % (self.this,)
    __swig_setmethods__["f"] = _cv.CvFileNode_data_f_set
    __swig_getmethods__["f"] = _cv.CvFileNode_data_f_get
    if _newclass:f = property(_cv.CvFileNode_data_f_get, _cv.CvFileNode_data_f_set)
    __swig_setmethods__["i"] = _cv.CvFileNode_data_i_set
    __swig_getmethods__["i"] = _cv.CvFileNode_data_i_get
    if _newclass:i = property(_cv.CvFileNode_data_i_get, _cv.CvFileNode_data_i_set)
    __swig_setmethods__["str"] = _cv.CvFileNode_data_str_set
    __swig_getmethods__["str"] = _cv.CvFileNode_data_str_get
    if _newclass:str = property(_cv.CvFileNode_data_str_get, _cv.CvFileNode_data_str_set)
    __swig_setmethods__["seq"] = _cv.CvFileNode_data_seq_set
    __swig_getmethods__["seq"] = _cv.CvFileNode_data_seq_get
    if _newclass:seq = property(_cv.CvFileNode_data_seq_get, _cv.CvFileNode_data_seq_set)
    __swig_setmethods__["map"] = _cv.CvFileNode_data_map_set
    __swig_getmethods__["map"] = _cv.CvFileNode_data_map_get
    if _newclass:map = property(_cv.CvFileNode_data_map_get, _cv.CvFileNode_data_map_set)
    def __init__(self, *args):
        _swig_setattr(self, CvFileNode_data, 'this', _cv.new_CvFileNode_data(*args))
        _swig_setattr(self, CvFileNode_data, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvFileNode_data):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvFileNode_dataPtr(CvFileNode_data):
    def __init__(self, this):
        _swig_setattr(self, CvFileNode_data, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvFileNode_data, 'thisown', 0)
        _swig_setattr(self, CvFileNode_data,self.__class__,CvFileNode_data)
_cv.CvFileNode_data_swigregister(CvFileNode_dataPtr)

class CvTypeInfo(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvTypeInfo, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvTypeInfo, name)
    def __repr__(self):
        return "<C CvTypeInfo instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvTypeInfo_flags_set
    __swig_getmethods__["flags"] = _cv.CvTypeInfo_flags_get
    if _newclass:flags = property(_cv.CvTypeInfo_flags_get, _cv.CvTypeInfo_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvTypeInfo_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvTypeInfo_header_size_get
    if _newclass:header_size = property(_cv.CvTypeInfo_header_size_get, _cv.CvTypeInfo_header_size_set)
    __swig_setmethods__["prev"] = _cv.CvTypeInfo_prev_set
    __swig_getmethods__["prev"] = _cv.CvTypeInfo_prev_get
    if _newclass:prev = property(_cv.CvTypeInfo_prev_get, _cv.CvTypeInfo_prev_set)
    __swig_setmethods__["next"] = _cv.CvTypeInfo_next_set
    __swig_getmethods__["next"] = _cv.CvTypeInfo_next_get
    if _newclass:next = property(_cv.CvTypeInfo_next_get, _cv.CvTypeInfo_next_set)
    __swig_setmethods__["type_name"] = _cv.CvTypeInfo_type_name_set
    __swig_getmethods__["type_name"] = _cv.CvTypeInfo_type_name_get
    if _newclass:type_name = property(_cv.CvTypeInfo_type_name_get, _cv.CvTypeInfo_type_name_set)
    __swig_setmethods__["is_instance"] = _cv.CvTypeInfo_is_instance_set
    __swig_getmethods__["is_instance"] = _cv.CvTypeInfo_is_instance_get
    if _newclass:is_instance = property(_cv.CvTypeInfo_is_instance_get, _cv.CvTypeInfo_is_instance_set)
    __swig_setmethods__["release"] = _cv.CvTypeInfo_release_set
    __swig_getmethods__["release"] = _cv.CvTypeInfo_release_get
    if _newclass:release = property(_cv.CvTypeInfo_release_get, _cv.CvTypeInfo_release_set)
    __swig_setmethods__["read"] = _cv.CvTypeInfo_read_set
    __swig_getmethods__["read"] = _cv.CvTypeInfo_read_get
    if _newclass:read = property(_cv.CvTypeInfo_read_get, _cv.CvTypeInfo_read_set)
    __swig_setmethods__["write"] = _cv.CvTypeInfo_write_set
    __swig_getmethods__["write"] = _cv.CvTypeInfo_write_get
    if _newclass:write = property(_cv.CvTypeInfo_write_get, _cv.CvTypeInfo_write_set)
    __swig_setmethods__["clone"] = _cv.CvTypeInfo_clone_set
    __swig_getmethods__["clone"] = _cv.CvTypeInfo_clone_get
    if _newclass:clone = property(_cv.CvTypeInfo_clone_get, _cv.CvTypeInfo_clone_set)
    def __init__(self, *args):
        _swig_setattr(self, CvTypeInfo, 'this', _cv.new_CvTypeInfo(*args))
        _swig_setattr(self, CvTypeInfo, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvTypeInfo):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvTypeInfoPtr(CvTypeInfo):
    def __init__(self, this):
        _swig_setattr(self, CvTypeInfo, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvTypeInfo, 'thisown', 0)
        _swig_setattr(self, CvTypeInfo,self.__class__,CvTypeInfo)
_cv.CvTypeInfo_swigregister(CvTypeInfoPtr)

class CvPluginFuncInfo(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvPluginFuncInfo, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvPluginFuncInfo, name)
    def __repr__(self):
        return "<C CvPluginFuncInfo instance at %s>" % (self.this,)
    __swig_setmethods__["func_addr"] = _cv.CvPluginFuncInfo_func_addr_set
    __swig_getmethods__["func_addr"] = _cv.CvPluginFuncInfo_func_addr_get
    if _newclass:func_addr = property(_cv.CvPluginFuncInfo_func_addr_get, _cv.CvPluginFuncInfo_func_addr_set)
    __swig_setmethods__["default_func_addr"] = _cv.CvPluginFuncInfo_default_func_addr_set
    __swig_getmethods__["default_func_addr"] = _cv.CvPluginFuncInfo_default_func_addr_get
    if _newclass:default_func_addr = property(_cv.CvPluginFuncInfo_default_func_addr_get, _cv.CvPluginFuncInfo_default_func_addr_set)
    __swig_setmethods__["func_names"] = _cv.CvPluginFuncInfo_func_names_set
    __swig_getmethods__["func_names"] = _cv.CvPluginFuncInfo_func_names_get
    if _newclass:func_names = property(_cv.CvPluginFuncInfo_func_names_get, _cv.CvPluginFuncInfo_func_names_set)
    __swig_setmethods__["search_modules"] = _cv.CvPluginFuncInfo_search_modules_set
    __swig_getmethods__["search_modules"] = _cv.CvPluginFuncInfo_search_modules_get
    if _newclass:search_modules = property(_cv.CvPluginFuncInfo_search_modules_get, _cv.CvPluginFuncInfo_search_modules_set)
    __swig_setmethods__["loaded_from"] = _cv.CvPluginFuncInfo_loaded_from_set
    __swig_getmethods__["loaded_from"] = _cv.CvPluginFuncInfo_loaded_from_get
    if _newclass:loaded_from = property(_cv.CvPluginFuncInfo_loaded_from_get, _cv.CvPluginFuncInfo_loaded_from_set)
    def __init__(self, *args):
        _swig_setattr(self, CvPluginFuncInfo, 'this', _cv.new_CvPluginFuncInfo(*args))
        _swig_setattr(self, CvPluginFuncInfo, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvPluginFuncInfo):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvPluginFuncInfoPtr(CvPluginFuncInfo):
    def __init__(self, this):
        _swig_setattr(self, CvPluginFuncInfo, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvPluginFuncInfo, 'thisown', 0)
        _swig_setattr(self, CvPluginFuncInfo,self.__class__,CvPluginFuncInfo)
_cv.CvPluginFuncInfo_swigregister(CvPluginFuncInfoPtr)

class CvModuleInfo(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvModuleInfo, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvModuleInfo, name)
    def __repr__(self):
        return "<C CvModuleInfo instance at %s>" % (self.this,)
    __swig_setmethods__["next"] = _cv.CvModuleInfo_next_set
    __swig_getmethods__["next"] = _cv.CvModuleInfo_next_get
    if _newclass:next = property(_cv.CvModuleInfo_next_get, _cv.CvModuleInfo_next_set)
    __swig_setmethods__["name"] = _cv.CvModuleInfo_name_set
    __swig_getmethods__["name"] = _cv.CvModuleInfo_name_get
    if _newclass:name = property(_cv.CvModuleInfo_name_get, _cv.CvModuleInfo_name_set)
    __swig_setmethods__["version"] = _cv.CvModuleInfo_version_set
    __swig_getmethods__["version"] = _cv.CvModuleInfo_version_get
    if _newclass:version = property(_cv.CvModuleInfo_version_get, _cv.CvModuleInfo_version_set)
    __swig_setmethods__["func_tab"] = _cv.CvModuleInfo_func_tab_set
    __swig_getmethods__["func_tab"] = _cv.CvModuleInfo_func_tab_get
    if _newclass:func_tab = property(_cv.CvModuleInfo_func_tab_get, _cv.CvModuleInfo_func_tab_set)
    def __init__(self, *args):
        _swig_setattr(self, CvModuleInfo, 'this', _cv.new_CvModuleInfo(*args))
        _swig_setattr(self, CvModuleInfo, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvModuleInfo):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvModuleInfoPtr(CvModuleInfo):
    def __init__(self, this):
        _swig_setattr(self, CvModuleInfo, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvModuleInfo, 'thisown', 0)
        _swig_setattr(self, CvModuleInfo,self.__class__,CvModuleInfo)
_cv.CvModuleInfo_swigregister(CvModuleInfoPtr)


cvAlloc = _cv.cvAlloc

cvFree = _cv.cvFree

cvCreateImageHeader = _cv.cvCreateImageHeader

cvInitImageHeader = _cv.cvInitImageHeader

cvCreateImage = _cv.cvCreateImage

cvReleaseImageHeader = _cv.cvReleaseImageHeader

cvReleaseImage = _cv.cvReleaseImage

cvCloneImage = _cv.cvCloneImage

cvSetImageCOI = _cv.cvSetImageCOI

cvGetImageCOI = _cv.cvGetImageCOI

cvSetImageROI = _cv.cvSetImageROI

cvResetImageROI = _cv.cvResetImageROI

cvGetImageROI = _cv.cvGetImageROI

cvCreateMatHeader = _cv.cvCreateMatHeader

cvInitMatHeader = _cv.cvInitMatHeader

cvCreateMat = _cv.cvCreateMat

cvReleaseMat = _cv.cvReleaseMat

cvDecRefData = _cv.cvDecRefData

cvIncRefData = _cv.cvIncRefData

cvCloneMat = _cv.cvCloneMat

cvGetSubRect = _cv.cvGetSubRect

cvGetRows = _cv.cvGetRows

cvGetRow = _cv.cvGetRow

cvGetCols = _cv.cvGetCols

cvGetCol = _cv.cvGetCol

cvGetDiag = _cv.cvGetDiag

cvScalarToRawData = _cv.cvScalarToRawData

cvRawDataToScalar = _cv.cvRawDataToScalar

cvCreateMatNDHeader = _cv.cvCreateMatNDHeader

cvCreateMatND = _cv.cvCreateMatND

cvInitMatNDHeader = _cv.cvInitMatNDHeader

cvReleaseMatND = _cv.cvReleaseMatND

cvCloneMatND = _cv.cvCloneMatND

cvCreateSparseMat = _cv.cvCreateSparseMat

cvReleaseSparseMat = _cv.cvReleaseSparseMat

cvCloneSparseMat = _cv.cvCloneSparseMat

cvInitSparseMatIterator = _cv.cvInitSparseMatIterator

cvGetNextSparseNode = _cv.cvGetNextSparseNode
class CvNArrayIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvNArrayIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvNArrayIterator, name)
    def __repr__(self):
        return "<C CvNArrayIterator instance at %s>" % (self.this,)
    __swig_setmethods__["count"] = _cv.CvNArrayIterator_count_set
    __swig_getmethods__["count"] = _cv.CvNArrayIterator_count_get
    if _newclass:count = property(_cv.CvNArrayIterator_count_get, _cv.CvNArrayIterator_count_set)
    __swig_setmethods__["dims"] = _cv.CvNArrayIterator_dims_set
    __swig_getmethods__["dims"] = _cv.CvNArrayIterator_dims_get
    if _newclass:dims = property(_cv.CvNArrayIterator_dims_get, _cv.CvNArrayIterator_dims_set)
    __swig_setmethods__["size"] = _cv.CvNArrayIterator_size_set
    __swig_getmethods__["size"] = _cv.CvNArrayIterator_size_get
    if _newclass:size = property(_cv.CvNArrayIterator_size_get, _cv.CvNArrayIterator_size_set)
    __swig_setmethods__["ptr"] = _cv.CvNArrayIterator_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvNArrayIterator_ptr_get
    if _newclass:ptr = property(_cv.CvNArrayIterator_ptr_get, _cv.CvNArrayIterator_ptr_set)
    __swig_setmethods__["stack"] = _cv.CvNArrayIterator_stack_set
    __swig_getmethods__["stack"] = _cv.CvNArrayIterator_stack_get
    if _newclass:stack = property(_cv.CvNArrayIterator_stack_get, _cv.CvNArrayIterator_stack_set)
    __swig_setmethods__["hdr"] = _cv.CvNArrayIterator_hdr_set
    __swig_getmethods__["hdr"] = _cv.CvNArrayIterator_hdr_get
    if _newclass:hdr = property(_cv.CvNArrayIterator_hdr_get, _cv.CvNArrayIterator_hdr_set)
    def __init__(self, *args):
        _swig_setattr(self, CvNArrayIterator, 'this', _cv.new_CvNArrayIterator(*args))
        _swig_setattr(self, CvNArrayIterator, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvNArrayIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvNArrayIteratorPtr(CvNArrayIterator):
    def __init__(self, this):
        _swig_setattr(self, CvNArrayIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvNArrayIterator, 'thisown', 0)
        _swig_setattr(self, CvNArrayIterator,self.__class__,CvNArrayIterator)
_cv.CvNArrayIterator_swigregister(CvNArrayIteratorPtr)


cvInitNArrayIterator = _cv.cvInitNArrayIterator

cvNextNArraySlice = _cv.cvNextNArraySlice

cvGetElemType = _cv.cvGetElemType

cvGetDims = _cv.cvGetDims

cvGetDimSize = _cv.cvGetDimSize

cvPtr1D = _cv.cvPtr1D

cvPtr2D = _cv.cvPtr2D

cvPtr3D = _cv.cvPtr3D

cvPtrND = _cv.cvPtrND

cvGet1D = _cv.cvGet1D

cvGet2D = _cv.cvGet2D

cvGet3D = _cv.cvGet3D

cvGetND = _cv.cvGetND

cvGetReal1D = _cv.cvGetReal1D

cvGetReal2D = _cv.cvGetReal2D

cvGetReal3D = _cv.cvGetReal3D

cvGetRealND = _cv.cvGetRealND

cvSet1D = _cv.cvSet1D

cvSet2D = _cv.cvSet2D

cvSet3D = _cv.cvSet3D

cvSetND = _cv.cvSetND

cvSetReal1D = _cv.cvSetReal1D

cvSetReal2D = _cv.cvSetReal2D

cvSetReal3D = _cv.cvSetReal3D

cvSetRealND = _cv.cvSetRealND

cvClearND = _cv.cvClearND

cvGetMat = _cv.cvGetMat

cvGetImage = _cv.cvGetImage

cvReshapeMatND = _cv.cvReshapeMatND

cvReshape = _cv.cvReshape

cvRepeat = _cv.cvRepeat

cvCreateData = _cv.cvCreateData

cvReleaseData = _cv.cvReleaseData

cvSetData = _cv.cvSetData

cvGetRawData = _cv.cvGetRawData

cvGetSize = _cv.cvGetSize

cvCopy = _cv.cvCopy

cvSet = _cv.cvSet

cvSetZero = _cv.cvSetZero

cvSplit = _cv.cvSplit

cvMerge = _cv.cvMerge

cvConvertScale = _cv.cvConvertScale

cvConvertScaleAbs = _cv.cvConvertScaleAbs

cvCheckTermCriteria = _cv.cvCheckTermCriteria

cvAdd = _cv.cvAdd

cvAddS = _cv.cvAddS

cvSub = _cv.cvSub

cvSubS = _cv.cvSubS

cvSubRS = _cv.cvSubRS

cvMul = _cv.cvMul

cvDiv = _cv.cvDiv

cvScaleAdd = _cv.cvScaleAdd

cvAddWeighted = _cv.cvAddWeighted

cvDotProduct = _cv.cvDotProduct

cvAnd = _cv.cvAnd

cvAndS = _cv.cvAndS

cvOr = _cv.cvOr

cvOrS = _cv.cvOrS

cvXor = _cv.cvXor

cvXorS = _cv.cvXorS

cvNot = _cv.cvNot

cvInRange = _cv.cvInRange

cvInRangeS = _cv.cvInRangeS

cvCmp = _cv.cvCmp

cvCmpS = _cv.cvCmpS

cvMin = _cv.cvMin

cvMax = _cv.cvMax

cvMinS = _cv.cvMinS

cvMaxS = _cv.cvMaxS

cvAbsDiff = _cv.cvAbsDiff

cvAbsDiffS = _cv.cvAbsDiffS

cvCartToPolar = _cv.cvCartToPolar

cvPolarToCart = _cv.cvPolarToCart

cvPow = _cv.cvPow

cvExp = _cv.cvExp

cvLog = _cv.cvLog

cvFastArctan = _cv.cvFastArctan

cvCbrt = _cv.cvCbrt

cvCheckArr = _cv.cvCheckArr

cvRandArr = _cv.cvRandArr

cvCrossProduct = _cv.cvCrossProduct

cvGEMM = _cv.cvGEMM

cvTransform = _cv.cvTransform

cvPerspectiveTransform = _cv.cvPerspectiveTransform

cvMulTransposed = _cv.cvMulTransposed

cvTranspose = _cv.cvTranspose

cvFlip = _cv.cvFlip

cvSVD = _cv.cvSVD

cvSVBkSb = _cv.cvSVBkSb

cvInvert = _cv.cvInvert

cvSolve = _cv.cvSolve

cvDet = _cv.cvDet

cvTrace = _cv.cvTrace

cvEigenVV = _cv.cvEigenVV

cvSetIdentity = _cv.cvSetIdentity

cvCalcCovarMatrix = _cv.cvCalcCovarMatrix

cvMahalanobis = _cv.cvMahalanobis

cvSum = _cv.cvSum

cvCountNonZero = _cv.cvCountNonZero

cvAvg = _cv.cvAvg

cvAvgSdv = _cv.cvAvgSdv

cvMinMaxLoc = _cv.cvMinMaxLoc

cvNorm = _cv.cvNorm

cvDFT = _cv.cvDFT

cvMulSpectrums = _cv.cvMulSpectrums

cvGetOptimalDFTSize = _cv.cvGetOptimalDFTSize

cvDCT = _cv.cvDCT

cvSliceLength = _cv.cvSliceLength

cvCreateMemStorage = _cv.cvCreateMemStorage

cvCreateChildMemStorage = _cv.cvCreateChildMemStorage

cvReleaseMemStorage = _cv.cvReleaseMemStorage

cvClearMemStorage = _cv.cvClearMemStorage

cvSaveMemStoragePos = _cv.cvSaveMemStoragePos

cvRestoreMemStoragePos = _cv.cvRestoreMemStoragePos

cvMemStorageAlloc = _cv.cvMemStorageAlloc

cvMemStorageAllocString = _cv.cvMemStorageAllocString

cvCreateSeq = _cv.cvCreateSeq

cvSetSeqBlockSize = _cv.cvSetSeqBlockSize

cvSeqPush = _cv.cvSeqPush

cvSeqPushFront = _cv.cvSeqPushFront

cvSeqPop = _cv.cvSeqPop

cvSeqPopFront = _cv.cvSeqPopFront

cvSeqPushMulti = _cv.cvSeqPushMulti

cvSeqPopMulti = _cv.cvSeqPopMulti

cvSeqInsert = _cv.cvSeqInsert

cvSeqRemove = _cv.cvSeqRemove

cvClearSeq = _cv.cvClearSeq

cvGetSeqElem = _cv.cvGetSeqElem

cvSeqElemIdx = _cv.cvSeqElemIdx

cvStartAppendToSeq = _cv.cvStartAppendToSeq

cvStartWriteSeq = _cv.cvStartWriteSeq

cvEndWriteSeq = _cv.cvEndWriteSeq

cvFlushSeqWriter = _cv.cvFlushSeqWriter

cvStartReadSeq = _cv.cvStartReadSeq

cvGetSeqReaderPos = _cv.cvGetSeqReaderPos

cvSetSeqReaderPos = _cv.cvSetSeqReaderPos

cvCvtSeqToArray = _cv.cvCvtSeqToArray

cvMakeSeqHeaderForArray = _cv.cvMakeSeqHeaderForArray

cvSeqSlice = _cv.cvSeqSlice

cvCloneSeq = _cv.cvCloneSeq

cvSeqRemoveSlice = _cv.cvSeqRemoveSlice

cvSeqInsertSlice = _cv.cvSeqInsertSlice

cvSeqSort = _cv.cvSeqSort

cvSeqSearch = _cv.cvSeqSearch

cvSeqInvert = _cv.cvSeqInvert

cvSeqPartition = _cv.cvSeqPartition

cvChangeSeqBlock = _cv.cvChangeSeqBlock

cvCreateSeqBlock = _cv.cvCreateSeqBlock

cvCreateSet = _cv.cvCreateSet

cvSetAdd = _cv.cvSetAdd

cvSetNew = _cv.cvSetNew

cvSetRemoveByPtr = _cv.cvSetRemoveByPtr

cvSetRemove = _cv.cvSetRemove

cvGetSetElem = _cv.cvGetSetElem

cvClearSet = _cv.cvClearSet

cvCreateGraph = _cv.cvCreateGraph

cvGraphAddVtx = _cv.cvGraphAddVtx

cvGraphRemoveVtx = _cv.cvGraphRemoveVtx

cvGraphRemoveVtxByPtr = _cv.cvGraphRemoveVtxByPtr

cvGraphAddEdge = _cv.cvGraphAddEdge

cvGraphAddEdgeByPtr = _cv.cvGraphAddEdgeByPtr

cvGraphRemoveEdge = _cv.cvGraphRemoveEdge

cvGraphRemoveEdgeByPtr = _cv.cvGraphRemoveEdgeByPtr

cvFindGraphEdge = _cv.cvFindGraphEdge

cvFindGraphEdgeByPtr = _cv.cvFindGraphEdgeByPtr

cvClearGraph = _cv.cvClearGraph

cvGraphVtxDegree = _cv.cvGraphVtxDegree

cvGraphVtxDegreeByPtr = _cv.cvGraphVtxDegreeByPtr
class CvGraphScanner(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvGraphScanner, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvGraphScanner, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvGraphScanner instance at %s>" % (self.this,)
    __swig_setmethods__["vtx"] = _cv.CvGraphScanner_vtx_set
    __swig_getmethods__["vtx"] = _cv.CvGraphScanner_vtx_get
    if _newclass:vtx = property(_cv.CvGraphScanner_vtx_get, _cv.CvGraphScanner_vtx_set)
    __swig_setmethods__["dst"] = _cv.CvGraphScanner_dst_set
    __swig_getmethods__["dst"] = _cv.CvGraphScanner_dst_get
    if _newclass:dst = property(_cv.CvGraphScanner_dst_get, _cv.CvGraphScanner_dst_set)
    __swig_setmethods__["edge"] = _cv.CvGraphScanner_edge_set
    __swig_getmethods__["edge"] = _cv.CvGraphScanner_edge_get
    if _newclass:edge = property(_cv.CvGraphScanner_edge_get, _cv.CvGraphScanner_edge_set)
    __swig_setmethods__["graph"] = _cv.CvGraphScanner_graph_set
    __swig_getmethods__["graph"] = _cv.CvGraphScanner_graph_get
    if _newclass:graph = property(_cv.CvGraphScanner_graph_get, _cv.CvGraphScanner_graph_set)
    __swig_setmethods__["stack"] = _cv.CvGraphScanner_stack_set
    __swig_getmethods__["stack"] = _cv.CvGraphScanner_stack_get
    if _newclass:stack = property(_cv.CvGraphScanner_stack_get, _cv.CvGraphScanner_stack_set)
    __swig_setmethods__["index"] = _cv.CvGraphScanner_index_set
    __swig_getmethods__["index"] = _cv.CvGraphScanner_index_get
    if _newclass:index = property(_cv.CvGraphScanner_index_get, _cv.CvGraphScanner_index_set)
    __swig_setmethods__["mask"] = _cv.CvGraphScanner_mask_set
    __swig_getmethods__["mask"] = _cv.CvGraphScanner_mask_get
    if _newclass:mask = property(_cv.CvGraphScanner_mask_get, _cv.CvGraphScanner_mask_set)
    def __del__(self, destroy=_cv.delete_CvGraphScanner):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvGraphScannerPtr(CvGraphScanner):
    def __init__(self, this):
        _swig_setattr(self, CvGraphScanner, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvGraphScanner, 'thisown', 0)
        _swig_setattr(self, CvGraphScanner,self.__class__,CvGraphScanner)
_cv.CvGraphScanner_swigregister(CvGraphScannerPtr)


cvCreateGraphScanner = _cv.cvCreateGraphScanner

cvReleaseGraphScanner = _cv.cvReleaseGraphScanner

cvNextGraphItem = _cv.cvNextGraphItem

cvCloneGraph = _cv.cvCloneGraph

cvLine = _cv.cvLine

cvRectangle = _cv.cvRectangle

cvCircle = _cv.cvCircle

cvEllipse = _cv.cvEllipse

cvEllipseBox = _cv.cvEllipseBox

cvFillConvexPoly = _cv.cvFillConvexPoly

cvFillPoly = _cv.cvFillPoly

cvPolyLine = _cv.cvPolyLine
class CvFont(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvFont, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvFont, name)
    def __repr__(self):
        return "<C CvFont instance at %s>" % (self.this,)
    __swig_setmethods__["font_face"] = _cv.CvFont_font_face_set
    __swig_getmethods__["font_face"] = _cv.CvFont_font_face_get
    if _newclass:font_face = property(_cv.CvFont_font_face_get, _cv.CvFont_font_face_set)
    __swig_setmethods__["ascii"] = _cv.CvFont_ascii_set
    __swig_getmethods__["ascii"] = _cv.CvFont_ascii_get
    if _newclass:ascii = property(_cv.CvFont_ascii_get, _cv.CvFont_ascii_set)
    __swig_setmethods__["greek"] = _cv.CvFont_greek_set
    __swig_getmethods__["greek"] = _cv.CvFont_greek_get
    if _newclass:greek = property(_cv.CvFont_greek_get, _cv.CvFont_greek_set)
    __swig_setmethods__["cyrillic"] = _cv.CvFont_cyrillic_set
    __swig_getmethods__["cyrillic"] = _cv.CvFont_cyrillic_get
    if _newclass:cyrillic = property(_cv.CvFont_cyrillic_get, _cv.CvFont_cyrillic_set)
    __swig_setmethods__["hscale"] = _cv.CvFont_hscale_set
    __swig_getmethods__["hscale"] = _cv.CvFont_hscale_get
    if _newclass:hscale = property(_cv.CvFont_hscale_get, _cv.CvFont_hscale_set)
    __swig_setmethods__["vscale"] = _cv.CvFont_vscale_set
    __swig_getmethods__["vscale"] = _cv.CvFont_vscale_get
    if _newclass:vscale = property(_cv.CvFont_vscale_get, _cv.CvFont_vscale_set)
    __swig_setmethods__["shear"] = _cv.CvFont_shear_set
    __swig_getmethods__["shear"] = _cv.CvFont_shear_get
    if _newclass:shear = property(_cv.CvFont_shear_get, _cv.CvFont_shear_set)
    __swig_setmethods__["thickness"] = _cv.CvFont_thickness_set
    __swig_getmethods__["thickness"] = _cv.CvFont_thickness_get
    if _newclass:thickness = property(_cv.CvFont_thickness_get, _cv.CvFont_thickness_set)
    __swig_setmethods__["dx"] = _cv.CvFont_dx_set
    __swig_getmethods__["dx"] = _cv.CvFont_dx_get
    if _newclass:dx = property(_cv.CvFont_dx_get, _cv.CvFont_dx_set)
    __swig_setmethods__["line_type"] = _cv.CvFont_line_type_set
    __swig_getmethods__["line_type"] = _cv.CvFont_line_type_get
    if _newclass:line_type = property(_cv.CvFont_line_type_get, _cv.CvFont_line_type_set)
    def __init__(self, *args):
        _swig_setattr(self, CvFont, 'this', _cv.new_CvFont(*args))
        _swig_setattr(self, CvFont, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvFont):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvFontPtr(CvFont):
    def __init__(self, this):
        _swig_setattr(self, CvFont, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvFont, 'thisown', 0)
        _swig_setattr(self, CvFont,self.__class__,CvFont)
_cv.CvFont_swigregister(CvFontPtr)


cvInitFont = _cv.cvInitFont

cvPutText = _cv.cvPutText

cvGetTextSize = _cv.cvGetTextSize

cvColorToScalar = _cv.cvColorToScalar

cvDrawContours = _cv.cvDrawContours

cvLUT = _cv.cvLUT
class CvTreeNodeIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvTreeNodeIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvTreeNodeIterator, name)
    def __repr__(self):
        return "<C CvTreeNodeIterator instance at %s>" % (self.this,)
    __swig_setmethods__["node"] = _cv.CvTreeNodeIterator_node_set
    __swig_getmethods__["node"] = _cv.CvTreeNodeIterator_node_get
    if _newclass:node = property(_cv.CvTreeNodeIterator_node_get, _cv.CvTreeNodeIterator_node_set)
    __swig_setmethods__["level"] = _cv.CvTreeNodeIterator_level_set
    __swig_getmethods__["level"] = _cv.CvTreeNodeIterator_level_get
    if _newclass:level = property(_cv.CvTreeNodeIterator_level_get, _cv.CvTreeNodeIterator_level_set)
    __swig_setmethods__["max_level"] = _cv.CvTreeNodeIterator_max_level_set
    __swig_getmethods__["max_level"] = _cv.CvTreeNodeIterator_max_level_get
    if _newclass:max_level = property(_cv.CvTreeNodeIterator_max_level_get, _cv.CvTreeNodeIterator_max_level_set)
    def __init__(self, *args):
        _swig_setattr(self, CvTreeNodeIterator, 'this', _cv.new_CvTreeNodeIterator(*args))
        _swig_setattr(self, CvTreeNodeIterator, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvTreeNodeIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvTreeNodeIteratorPtr(CvTreeNodeIterator):
    def __init__(self, this):
        _swig_setattr(self, CvTreeNodeIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvTreeNodeIterator, 'thisown', 0)
        _swig_setattr(self, CvTreeNodeIterator,self.__class__,CvTreeNodeIterator)
_cv.CvTreeNodeIterator_swigregister(CvTreeNodeIteratorPtr)


cvInitTreeNodeIterator = _cv.cvInitTreeNodeIterator

cvNextTreeNode = _cv.cvNextTreeNode

cvPrevTreeNode = _cv.cvPrevTreeNode

cvInsertNodeIntoTree = _cv.cvInsertNodeIntoTree

cvRemoveNodeFromTree = _cv.cvRemoveNodeFromTree

cvTreeToNodeSeq = _cv.cvTreeToNodeSeq

cvKMeans2 = _cv.cvKMeans2

cvRegisterModule = _cv.cvRegisterModule

cvUseOptimized = _cv.cvUseOptimized

cvGetModuleInfo = _cv.cvGetModuleInfo

cvGetErrStatus = _cv.cvGetErrStatus

cvSetErrStatus = _cv.cvSetErrStatus

cvGetErrMode = _cv.cvGetErrMode

cvSetErrMode = _cv.cvSetErrMode

cvError = _cv.cvError

cvErrorStr = _cv.cvErrorStr

cvGetErrInfo = _cv.cvGetErrInfo

cvErrorFromIppStatus = _cv.cvErrorFromIppStatus

cvRedirectError = _cv.cvRedirectError

cvNulDevReport = _cv.cvNulDevReport

cvStdErrReport = _cv.cvStdErrReport

cvGuiBoxReport = _cv.cvGuiBoxReport

cvSetMemoryManager = _cv.cvSetMemoryManager

cvSetIPLAllocators = _cv.cvSetIPLAllocators

cvOpenFileStorage = _cv.cvOpenFileStorage

cvReleaseFileStorage = _cv.cvReleaseFileStorage

cvAttrValue = _cv.cvAttrValue

cvStartWriteStruct = _cv.cvStartWriteStruct

cvEndWriteStruct = _cv.cvEndWriteStruct

cvWriteInt = _cv.cvWriteInt

cvWriteReal = _cv.cvWriteReal

cvWriteString = _cv.cvWriteString

cvWriteComment = _cv.cvWriteComment

cvWrite = _cv.cvWrite

cvStartNextStream = _cv.cvStartNextStream

cvWriteRawData = _cv.cvWriteRawData

cvGetHashedKey = _cv.cvGetHashedKey

cvGetRootFileNode = _cv.cvGetRootFileNode

cvGetFileNode = _cv.cvGetFileNode

cvGetFileNodeByName = _cv.cvGetFileNodeByName

cvReadInt = _cv.cvReadInt

cvReadIntByName = _cv.cvReadIntByName

cvReadReal = _cv.cvReadReal

cvReadRealByName = _cv.cvReadRealByName

cvReadString = _cv.cvReadString

cvReadStringByName = _cv.cvReadStringByName

cvRead = _cv.cvRead

cvReadByName = _cv.cvReadByName

cvStartReadRawData = _cv.cvStartReadRawData

cvReadRawDataSlice = _cv.cvReadRawDataSlice

cvReadRawData = _cv.cvReadRawData

cvWriteFileNode = _cv.cvWriteFileNode

cvGetFileNodeName = _cv.cvGetFileNodeName

cvRegisterType = _cv.cvRegisterType

cvUnregisterType = _cv.cvUnregisterType

cvFirstType = _cv.cvFirstType

cvFindType = _cv.cvFindType

cvTypeOf = _cv.cvTypeOf

cvRelease = _cv.cvRelease

cvClone = _cv.cvClone

cvSave = _cv.cvSave

cvLoad = _cv.cvLoad

cvGetTickCount = _cv.cvGetTickCount

cvGetTickFrequency = _cv.cvGetTickFrequency
class CvMoments(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMoments, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMoments, name)
    def __repr__(self):
        return "<C CvMoments instance at %s>" % (self.this,)
    __swig_setmethods__["m00"] = _cv.CvMoments_m00_set
    __swig_getmethods__["m00"] = _cv.CvMoments_m00_get
    if _newclass:m00 = property(_cv.CvMoments_m00_get, _cv.CvMoments_m00_set)
    __swig_setmethods__["m10"] = _cv.CvMoments_m10_set
    __swig_getmethods__["m10"] = _cv.CvMoments_m10_get
    if _newclass:m10 = property(_cv.CvMoments_m10_get, _cv.CvMoments_m10_set)
    __swig_setmethods__["m01"] = _cv.CvMoments_m01_set
    __swig_getmethods__["m01"] = _cv.CvMoments_m01_get
    if _newclass:m01 = property(_cv.CvMoments_m01_get, _cv.CvMoments_m01_set)
    __swig_setmethods__["m20"] = _cv.CvMoments_m20_set
    __swig_getmethods__["m20"] = _cv.CvMoments_m20_get
    if _newclass:m20 = property(_cv.CvMoments_m20_get, _cv.CvMoments_m20_set)
    __swig_setmethods__["m11"] = _cv.CvMoments_m11_set
    __swig_getmethods__["m11"] = _cv.CvMoments_m11_get
    if _newclass:m11 = property(_cv.CvMoments_m11_get, _cv.CvMoments_m11_set)
    __swig_setmethods__["m02"] = _cv.CvMoments_m02_set
    __swig_getmethods__["m02"] = _cv.CvMoments_m02_get
    if _newclass:m02 = property(_cv.CvMoments_m02_get, _cv.CvMoments_m02_set)
    __swig_setmethods__["m30"] = _cv.CvMoments_m30_set
    __swig_getmethods__["m30"] = _cv.CvMoments_m30_get
    if _newclass:m30 = property(_cv.CvMoments_m30_get, _cv.CvMoments_m30_set)
    __swig_setmethods__["m21"] = _cv.CvMoments_m21_set
    __swig_getmethods__["m21"] = _cv.CvMoments_m21_get
    if _newclass:m21 = property(_cv.CvMoments_m21_get, _cv.CvMoments_m21_set)
    __swig_setmethods__["m12"] = _cv.CvMoments_m12_set
    __swig_getmethods__["m12"] = _cv.CvMoments_m12_get
    if _newclass:m12 = property(_cv.CvMoments_m12_get, _cv.CvMoments_m12_set)
    __swig_setmethods__["m03"] = _cv.CvMoments_m03_set
    __swig_getmethods__["m03"] = _cv.CvMoments_m03_get
    if _newclass:m03 = property(_cv.CvMoments_m03_get, _cv.CvMoments_m03_set)
    __swig_setmethods__["mu20"] = _cv.CvMoments_mu20_set
    __swig_getmethods__["mu20"] = _cv.CvMoments_mu20_get
    if _newclass:mu20 = property(_cv.CvMoments_mu20_get, _cv.CvMoments_mu20_set)
    __swig_setmethods__["mu11"] = _cv.CvMoments_mu11_set
    __swig_getmethods__["mu11"] = _cv.CvMoments_mu11_get
    if _newclass:mu11 = property(_cv.CvMoments_mu11_get, _cv.CvMoments_mu11_set)
    __swig_setmethods__["mu02"] = _cv.CvMoments_mu02_set
    __swig_getmethods__["mu02"] = _cv.CvMoments_mu02_get
    if _newclass:mu02 = property(_cv.CvMoments_mu02_get, _cv.CvMoments_mu02_set)
    __swig_setmethods__["mu30"] = _cv.CvMoments_mu30_set
    __swig_getmethods__["mu30"] = _cv.CvMoments_mu30_get
    if _newclass:mu30 = property(_cv.CvMoments_mu30_get, _cv.CvMoments_mu30_set)
    __swig_setmethods__["mu21"] = _cv.CvMoments_mu21_set
    __swig_getmethods__["mu21"] = _cv.CvMoments_mu21_get
    if _newclass:mu21 = property(_cv.CvMoments_mu21_get, _cv.CvMoments_mu21_set)
    __swig_setmethods__["mu12"] = _cv.CvMoments_mu12_set
    __swig_getmethods__["mu12"] = _cv.CvMoments_mu12_get
    if _newclass:mu12 = property(_cv.CvMoments_mu12_get, _cv.CvMoments_mu12_set)
    __swig_setmethods__["mu03"] = _cv.CvMoments_mu03_set
    __swig_getmethods__["mu03"] = _cv.CvMoments_mu03_get
    if _newclass:mu03 = property(_cv.CvMoments_mu03_get, _cv.CvMoments_mu03_set)
    __swig_setmethods__["inv_sqrt_m00"] = _cv.CvMoments_inv_sqrt_m00_set
    __swig_getmethods__["inv_sqrt_m00"] = _cv.CvMoments_inv_sqrt_m00_get
    if _newclass:inv_sqrt_m00 = property(_cv.CvMoments_inv_sqrt_m00_get, _cv.CvMoments_inv_sqrt_m00_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMoments, 'this', _cv.new_CvMoments(*args))
        _swig_setattr(self, CvMoments, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMoments):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMomentsPtr(CvMoments):
    def __init__(self, this):
        _swig_setattr(self, CvMoments, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMoments, 'thisown', 0)
        _swig_setattr(self, CvMoments,self.__class__,CvMoments)
_cv.CvMoments_swigregister(CvMomentsPtr)

class CvHuMoments(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHuMoments, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHuMoments, name)
    def __repr__(self):
        return "<C CvHuMoments instance at %s>" % (self.this,)
    __swig_setmethods__["hu1"] = _cv.CvHuMoments_hu1_set
    __swig_getmethods__["hu1"] = _cv.CvHuMoments_hu1_get
    if _newclass:hu1 = property(_cv.CvHuMoments_hu1_get, _cv.CvHuMoments_hu1_set)
    __swig_setmethods__["hu2"] = _cv.CvHuMoments_hu2_set
    __swig_getmethods__["hu2"] = _cv.CvHuMoments_hu2_get
    if _newclass:hu2 = property(_cv.CvHuMoments_hu2_get, _cv.CvHuMoments_hu2_set)
    __swig_setmethods__["hu3"] = _cv.CvHuMoments_hu3_set
    __swig_getmethods__["hu3"] = _cv.CvHuMoments_hu3_get
    if _newclass:hu3 = property(_cv.CvHuMoments_hu3_get, _cv.CvHuMoments_hu3_set)
    __swig_setmethods__["hu4"] = _cv.CvHuMoments_hu4_set
    __swig_getmethods__["hu4"] = _cv.CvHuMoments_hu4_get
    if _newclass:hu4 = property(_cv.CvHuMoments_hu4_get, _cv.CvHuMoments_hu4_set)
    __swig_setmethods__["hu5"] = _cv.CvHuMoments_hu5_set
    __swig_getmethods__["hu5"] = _cv.CvHuMoments_hu5_get
    if _newclass:hu5 = property(_cv.CvHuMoments_hu5_get, _cv.CvHuMoments_hu5_set)
    __swig_setmethods__["hu6"] = _cv.CvHuMoments_hu6_set
    __swig_getmethods__["hu6"] = _cv.CvHuMoments_hu6_get
    if _newclass:hu6 = property(_cv.CvHuMoments_hu6_get, _cv.CvHuMoments_hu6_set)
    __swig_setmethods__["hu7"] = _cv.CvHuMoments_hu7_set
    __swig_getmethods__["hu7"] = _cv.CvHuMoments_hu7_get
    if _newclass:hu7 = property(_cv.CvHuMoments_hu7_get, _cv.CvHuMoments_hu7_set)
    def __init__(self, *args):
        _swig_setattr(self, CvHuMoments, 'this', _cv.new_CvHuMoments(*args))
        _swig_setattr(self, CvHuMoments, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvHuMoments):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHuMomentsPtr(CvHuMoments):
    def __init__(self, this):
        _swig_setattr(self, CvHuMoments, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHuMoments, 'thisown', 0)
        _swig_setattr(self, CvHuMoments,self.__class__,CvHuMoments)
_cv.CvHuMoments_swigregister(CvHuMomentsPtr)

class CvLineIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvLineIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvLineIterator, name)
    def __repr__(self):
        return "<C CvLineIterator instance at %s>" % (self.this,)
    __swig_setmethods__["ptr"] = _cv.CvLineIterator_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvLineIterator_ptr_get
    if _newclass:ptr = property(_cv.CvLineIterator_ptr_get, _cv.CvLineIterator_ptr_set)
    __swig_setmethods__["err"] = _cv.CvLineIterator_err_set
    __swig_getmethods__["err"] = _cv.CvLineIterator_err_get
    if _newclass:err = property(_cv.CvLineIterator_err_get, _cv.CvLineIterator_err_set)
    __swig_setmethods__["plus_delta"] = _cv.CvLineIterator_plus_delta_set
    __swig_getmethods__["plus_delta"] = _cv.CvLineIterator_plus_delta_get
    if _newclass:plus_delta = property(_cv.CvLineIterator_plus_delta_get, _cv.CvLineIterator_plus_delta_set)
    __swig_setmethods__["minus_delta"] = _cv.CvLineIterator_minus_delta_set
    __swig_getmethods__["minus_delta"] = _cv.CvLineIterator_minus_delta_get
    if _newclass:minus_delta = property(_cv.CvLineIterator_minus_delta_get, _cv.CvLineIterator_minus_delta_set)
    __swig_setmethods__["plus_step"] = _cv.CvLineIterator_plus_step_set
    __swig_getmethods__["plus_step"] = _cv.CvLineIterator_plus_step_get
    if _newclass:plus_step = property(_cv.CvLineIterator_plus_step_get, _cv.CvLineIterator_plus_step_set)
    __swig_setmethods__["minus_step"] = _cv.CvLineIterator_minus_step_set
    __swig_getmethods__["minus_step"] = _cv.CvLineIterator_minus_step_get
    if _newclass:minus_step = property(_cv.CvLineIterator_minus_step_get, _cv.CvLineIterator_minus_step_set)
    def __init__(self, *args):
        _swig_setattr(self, CvLineIterator, 'this', _cv.new_CvLineIterator(*args))
        _swig_setattr(self, CvLineIterator, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvLineIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvLineIteratorPtr(CvLineIterator):
    def __init__(self, this):
        _swig_setattr(self, CvLineIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvLineIterator, 'thisown', 0)
        _swig_setattr(self, CvLineIterator,self.__class__,CvLineIterator)
_cv.CvLineIterator_swigregister(CvLineIteratorPtr)

class CvConnectedComp(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvConnectedComp, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvConnectedComp, name)
    def __repr__(self):
        return "<C CvConnectedComp instance at %s>" % (self.this,)
    __swig_setmethods__["area"] = _cv.CvConnectedComp_area_set
    __swig_getmethods__["area"] = _cv.CvConnectedComp_area_get
    if _newclass:area = property(_cv.CvConnectedComp_area_get, _cv.CvConnectedComp_area_set)
    __swig_setmethods__["value"] = _cv.CvConnectedComp_value_set
    __swig_getmethods__["value"] = _cv.CvConnectedComp_value_get
    if _newclass:value = property(_cv.CvConnectedComp_value_get, _cv.CvConnectedComp_value_set)
    __swig_setmethods__["rect"] = _cv.CvConnectedComp_rect_set
    __swig_getmethods__["rect"] = _cv.CvConnectedComp_rect_get
    if _newclass:rect = property(_cv.CvConnectedComp_rect_get, _cv.CvConnectedComp_rect_set)
    __swig_setmethods__["contour"] = _cv.CvConnectedComp_contour_set
    __swig_getmethods__["contour"] = _cv.CvConnectedComp_contour_get
    if _newclass:contour = property(_cv.CvConnectedComp_contour_get, _cv.CvConnectedComp_contour_set)
    def __init__(self, *args):
        _swig_setattr(self, CvConnectedComp, 'this', _cv.new_CvConnectedComp(*args))
        _swig_setattr(self, CvConnectedComp, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvConnectedComp):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvConnectedCompPtr(CvConnectedComp):
    def __init__(self, this):
        _swig_setattr(self, CvConnectedComp, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvConnectedComp, 'thisown', 0)
        _swig_setattr(self, CvConnectedComp,self.__class__,CvConnectedComp)
_cv.CvConnectedComp_swigregister(CvConnectedCompPtr)

class CvChainPtReader(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvChainPtReader, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvChainPtReader, name)
    def __repr__(self):
        return "<C CvChainPtReader instance at %s>" % (self.this,)
    __swig_setmethods__["header_size"] = _cv.CvChainPtReader_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvChainPtReader_header_size_get
    if _newclass:header_size = property(_cv.CvChainPtReader_header_size_get, _cv.CvChainPtReader_header_size_set)
    __swig_setmethods__["seq"] = _cv.CvChainPtReader_seq_set
    __swig_getmethods__["seq"] = _cv.CvChainPtReader_seq_get
    if _newclass:seq = property(_cv.CvChainPtReader_seq_get, _cv.CvChainPtReader_seq_set)
    __swig_setmethods__["block"] = _cv.CvChainPtReader_block_set
    __swig_getmethods__["block"] = _cv.CvChainPtReader_block_get
    if _newclass:block = property(_cv.CvChainPtReader_block_get, _cv.CvChainPtReader_block_set)
    __swig_setmethods__["ptr"] = _cv.CvChainPtReader_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvChainPtReader_ptr_get
    if _newclass:ptr = property(_cv.CvChainPtReader_ptr_get, _cv.CvChainPtReader_ptr_set)
    __swig_setmethods__["block_min"] = _cv.CvChainPtReader_block_min_set
    __swig_getmethods__["block_min"] = _cv.CvChainPtReader_block_min_get
    if _newclass:block_min = property(_cv.CvChainPtReader_block_min_get, _cv.CvChainPtReader_block_min_set)
    __swig_setmethods__["block_max"] = _cv.CvChainPtReader_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvChainPtReader_block_max_get
    if _newclass:block_max = property(_cv.CvChainPtReader_block_max_get, _cv.CvChainPtReader_block_max_set)
    __swig_setmethods__["delta_index"] = _cv.CvChainPtReader_delta_index_set
    __swig_getmethods__["delta_index"] = _cv.CvChainPtReader_delta_index_get
    if _newclass:delta_index = property(_cv.CvChainPtReader_delta_index_get, _cv.CvChainPtReader_delta_index_set)
    __swig_setmethods__["prev_elem"] = _cv.CvChainPtReader_prev_elem_set
    __swig_getmethods__["prev_elem"] = _cv.CvChainPtReader_prev_elem_get
    if _newclass:prev_elem = property(_cv.CvChainPtReader_prev_elem_get, _cv.CvChainPtReader_prev_elem_set)
    __swig_setmethods__["code"] = _cv.CvChainPtReader_code_set
    __swig_getmethods__["code"] = _cv.CvChainPtReader_code_get
    if _newclass:code = property(_cv.CvChainPtReader_code_get, _cv.CvChainPtReader_code_set)
    __swig_setmethods__["pt"] = _cv.CvChainPtReader_pt_set
    __swig_getmethods__["pt"] = _cv.CvChainPtReader_pt_get
    if _newclass:pt = property(_cv.CvChainPtReader_pt_get, _cv.CvChainPtReader_pt_set)
    __swig_setmethods__["deltas"] = _cv.CvChainPtReader_deltas_set
    __swig_getmethods__["deltas"] = _cv.CvChainPtReader_deltas_get
    if _newclass:deltas = property(_cv.CvChainPtReader_deltas_get, _cv.CvChainPtReader_deltas_set)
    __swig_setmethods__["reserved"] = _cv.CvChainPtReader_reserved_set
    __swig_getmethods__["reserved"] = _cv.CvChainPtReader_reserved_get
    if _newclass:reserved = property(_cv.CvChainPtReader_reserved_get, _cv.CvChainPtReader_reserved_set)
    def __init__(self, *args):
        _swig_setattr(self, CvChainPtReader, 'this', _cv.new_CvChainPtReader(*args))
        _swig_setattr(self, CvChainPtReader, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvChainPtReader):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvChainPtReaderPtr(CvChainPtReader):
    def __init__(self, this):
        _swig_setattr(self, CvChainPtReader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvChainPtReader, 'thisown', 0)
        _swig_setattr(self, CvChainPtReader,self.__class__,CvChainPtReader)
_cv.CvChainPtReader_swigregister(CvChainPtReaderPtr)

class CvContourTree(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvContourTree, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvContourTree, name)
    def __repr__(self):
        return "<C CvContourTree instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvContourTree_flags_set
    __swig_getmethods__["flags"] = _cv.CvContourTree_flags_get
    if _newclass:flags = property(_cv.CvContourTree_flags_get, _cv.CvContourTree_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvContourTree_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvContourTree_header_size_get
    if _newclass:header_size = property(_cv.CvContourTree_header_size_get, _cv.CvContourTree_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvContourTree_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvContourTree_h_prev_get
    if _newclass:h_prev = property(_cv.CvContourTree_h_prev_get, _cv.CvContourTree_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvContourTree_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvContourTree_h_next_get
    if _newclass:h_next = property(_cv.CvContourTree_h_next_get, _cv.CvContourTree_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvContourTree_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvContourTree_v_prev_get
    if _newclass:v_prev = property(_cv.CvContourTree_v_prev_get, _cv.CvContourTree_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvContourTree_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvContourTree_v_next_get
    if _newclass:v_next = property(_cv.CvContourTree_v_next_get, _cv.CvContourTree_v_next_set)
    __swig_setmethods__["total"] = _cv.CvContourTree_total_set
    __swig_getmethods__["total"] = _cv.CvContourTree_total_get
    if _newclass:total = property(_cv.CvContourTree_total_get, _cv.CvContourTree_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvContourTree_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvContourTree_elem_size_get
    if _newclass:elem_size = property(_cv.CvContourTree_elem_size_get, _cv.CvContourTree_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvContourTree_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvContourTree_block_max_get
    if _newclass:block_max = property(_cv.CvContourTree_block_max_get, _cv.CvContourTree_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvContourTree_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvContourTree_ptr_get
    if _newclass:ptr = property(_cv.CvContourTree_ptr_get, _cv.CvContourTree_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvContourTree_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvContourTree_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvContourTree_delta_elems_get, _cv.CvContourTree_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvContourTree_storage_set
    __swig_getmethods__["storage"] = _cv.CvContourTree_storage_get
    if _newclass:storage = property(_cv.CvContourTree_storage_get, _cv.CvContourTree_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvContourTree_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvContourTree_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvContourTree_free_blocks_get, _cv.CvContourTree_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvContourTree_first_set
    __swig_getmethods__["first"] = _cv.CvContourTree_first_get
    if _newclass:first = property(_cv.CvContourTree_first_get, _cv.CvContourTree_first_set)
    __swig_setmethods__["p1"] = _cv.CvContourTree_p1_set
    __swig_getmethods__["p1"] = _cv.CvContourTree_p1_get
    if _newclass:p1 = property(_cv.CvContourTree_p1_get, _cv.CvContourTree_p1_set)
    __swig_setmethods__["p2"] = _cv.CvContourTree_p2_set
    __swig_getmethods__["p2"] = _cv.CvContourTree_p2_get
    if _newclass:p2 = property(_cv.CvContourTree_p2_get, _cv.CvContourTree_p2_set)
    def __init__(self, *args):
        _swig_setattr(self, CvContourTree, 'this', _cv.new_CvContourTree(*args))
        _swig_setattr(self, CvContourTree, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvContourTree):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvContourTreePtr(CvContourTree):
    def __init__(self, this):
        _swig_setattr(self, CvContourTree, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvContourTree, 'thisown', 0)
        _swig_setattr(self, CvContourTree,self.__class__,CvContourTree)
_cv.CvContourTree_swigregister(CvContourTreePtr)

class CvConvexityDefect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvConvexityDefect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvConvexityDefect, name)
    def __repr__(self):
        return "<C CvConvexityDefect instance at %s>" % (self.this,)
    __swig_setmethods__["start"] = _cv.CvConvexityDefect_start_set
    __swig_getmethods__["start"] = _cv.CvConvexityDefect_start_get
    if _newclass:start = property(_cv.CvConvexityDefect_start_get, _cv.CvConvexityDefect_start_set)
    __swig_setmethods__["end"] = _cv.CvConvexityDefect_end_set
    __swig_getmethods__["end"] = _cv.CvConvexityDefect_end_get
    if _newclass:end = property(_cv.CvConvexityDefect_end_get, _cv.CvConvexityDefect_end_set)
    __swig_setmethods__["depth_point"] = _cv.CvConvexityDefect_depth_point_set
    __swig_getmethods__["depth_point"] = _cv.CvConvexityDefect_depth_point_get
    if _newclass:depth_point = property(_cv.CvConvexityDefect_depth_point_get, _cv.CvConvexityDefect_depth_point_set)
    __swig_setmethods__["depth"] = _cv.CvConvexityDefect_depth_set
    __swig_getmethods__["depth"] = _cv.CvConvexityDefect_depth_get
    if _newclass:depth = property(_cv.CvConvexityDefect_depth_get, _cv.CvConvexityDefect_depth_set)
    def __init__(self, *args):
        _swig_setattr(self, CvConvexityDefect, 'this', _cv.new_CvConvexityDefect(*args))
        _swig_setattr(self, CvConvexityDefect, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvConvexityDefect):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvConvexityDefectPtr(CvConvexityDefect):
    def __init__(self, this):
        _swig_setattr(self, CvConvexityDefect, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvConvexityDefect, 'thisown', 0)
        _swig_setattr(self, CvConvexityDefect,self.__class__,CvConvexityDefect)
_cv.CvConvexityDefect_swigregister(CvConvexityDefectPtr)

class CvQuadEdge2D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvQuadEdge2D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvQuadEdge2D, name)
    def __repr__(self):
        return "<C CvQuadEdge2D instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvQuadEdge2D_flags_set
    __swig_getmethods__["flags"] = _cv.CvQuadEdge2D_flags_get
    if _newclass:flags = property(_cv.CvQuadEdge2D_flags_get, _cv.CvQuadEdge2D_flags_set)
    __swig_setmethods__["pt"] = _cv.CvQuadEdge2D_pt_set
    __swig_getmethods__["pt"] = _cv.CvQuadEdge2D_pt_get
    if _newclass:pt = property(_cv.CvQuadEdge2D_pt_get, _cv.CvQuadEdge2D_pt_set)
    __swig_setmethods__["next"] = _cv.CvQuadEdge2D_next_set
    __swig_getmethods__["next"] = _cv.CvQuadEdge2D_next_get
    if _newclass:next = property(_cv.CvQuadEdge2D_next_get, _cv.CvQuadEdge2D_next_set)
    def __init__(self, *args):
        _swig_setattr(self, CvQuadEdge2D, 'this', _cv.new_CvQuadEdge2D(*args))
        _swig_setattr(self, CvQuadEdge2D, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvQuadEdge2D):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvQuadEdge2DPtr(CvQuadEdge2D):
    def __init__(self, this):
        _swig_setattr(self, CvQuadEdge2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvQuadEdge2D, 'thisown', 0)
        _swig_setattr(self, CvQuadEdge2D,self.__class__,CvQuadEdge2D)
_cv.CvQuadEdge2D_swigregister(CvQuadEdge2DPtr)

class CvSubdiv2DPoint(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSubdiv2DPoint, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSubdiv2DPoint, name)
    def __repr__(self):
        return "<C CvSubdiv2DPoint instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvSubdiv2DPoint_flags_set
    __swig_getmethods__["flags"] = _cv.CvSubdiv2DPoint_flags_get
    if _newclass:flags = property(_cv.CvSubdiv2DPoint_flags_get, _cv.CvSubdiv2DPoint_flags_set)
    __swig_setmethods__["first"] = _cv.CvSubdiv2DPoint_first_set
    __swig_getmethods__["first"] = _cv.CvSubdiv2DPoint_first_get
    if _newclass:first = property(_cv.CvSubdiv2DPoint_first_get, _cv.CvSubdiv2DPoint_first_set)
    __swig_setmethods__["pt"] = _cv.CvSubdiv2DPoint_pt_set
    __swig_getmethods__["pt"] = _cv.CvSubdiv2DPoint_pt_get
    if _newclass:pt = property(_cv.CvSubdiv2DPoint_pt_get, _cv.CvSubdiv2DPoint_pt_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSubdiv2DPoint, 'this', _cv.new_CvSubdiv2DPoint(*args))
        _swig_setattr(self, CvSubdiv2DPoint, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSubdiv2DPoint):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSubdiv2DPointPtr(CvSubdiv2DPoint):
    def __init__(self, this):
        _swig_setattr(self, CvSubdiv2DPoint, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSubdiv2DPoint, 'thisown', 0)
        _swig_setattr(self, CvSubdiv2DPoint,self.__class__,CvSubdiv2DPoint)
_cv.CvSubdiv2DPoint_swigregister(CvSubdiv2DPointPtr)

class CvSubdiv2D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvSubdiv2D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvSubdiv2D, name)
    def __repr__(self):
        return "<C CvSubdiv2D instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvSubdiv2D_flags_set
    __swig_getmethods__["flags"] = _cv.CvSubdiv2D_flags_get
    if _newclass:flags = property(_cv.CvSubdiv2D_flags_get, _cv.CvSubdiv2D_flags_set)
    __swig_setmethods__["header_size"] = _cv.CvSubdiv2D_header_size_set
    __swig_getmethods__["header_size"] = _cv.CvSubdiv2D_header_size_get
    if _newclass:header_size = property(_cv.CvSubdiv2D_header_size_get, _cv.CvSubdiv2D_header_size_set)
    __swig_setmethods__["h_prev"] = _cv.CvSubdiv2D_h_prev_set
    __swig_getmethods__["h_prev"] = _cv.CvSubdiv2D_h_prev_get
    if _newclass:h_prev = property(_cv.CvSubdiv2D_h_prev_get, _cv.CvSubdiv2D_h_prev_set)
    __swig_setmethods__["h_next"] = _cv.CvSubdiv2D_h_next_set
    __swig_getmethods__["h_next"] = _cv.CvSubdiv2D_h_next_get
    if _newclass:h_next = property(_cv.CvSubdiv2D_h_next_get, _cv.CvSubdiv2D_h_next_set)
    __swig_setmethods__["v_prev"] = _cv.CvSubdiv2D_v_prev_set
    __swig_getmethods__["v_prev"] = _cv.CvSubdiv2D_v_prev_get
    if _newclass:v_prev = property(_cv.CvSubdiv2D_v_prev_get, _cv.CvSubdiv2D_v_prev_set)
    __swig_setmethods__["v_next"] = _cv.CvSubdiv2D_v_next_set
    __swig_getmethods__["v_next"] = _cv.CvSubdiv2D_v_next_get
    if _newclass:v_next = property(_cv.CvSubdiv2D_v_next_get, _cv.CvSubdiv2D_v_next_set)
    __swig_setmethods__["total"] = _cv.CvSubdiv2D_total_set
    __swig_getmethods__["total"] = _cv.CvSubdiv2D_total_get
    if _newclass:total = property(_cv.CvSubdiv2D_total_get, _cv.CvSubdiv2D_total_set)
    __swig_setmethods__["elem_size"] = _cv.CvSubdiv2D_elem_size_set
    __swig_getmethods__["elem_size"] = _cv.CvSubdiv2D_elem_size_get
    if _newclass:elem_size = property(_cv.CvSubdiv2D_elem_size_get, _cv.CvSubdiv2D_elem_size_set)
    __swig_setmethods__["block_max"] = _cv.CvSubdiv2D_block_max_set
    __swig_getmethods__["block_max"] = _cv.CvSubdiv2D_block_max_get
    if _newclass:block_max = property(_cv.CvSubdiv2D_block_max_get, _cv.CvSubdiv2D_block_max_set)
    __swig_setmethods__["ptr"] = _cv.CvSubdiv2D_ptr_set
    __swig_getmethods__["ptr"] = _cv.CvSubdiv2D_ptr_get
    if _newclass:ptr = property(_cv.CvSubdiv2D_ptr_get, _cv.CvSubdiv2D_ptr_set)
    __swig_setmethods__["delta_elems"] = _cv.CvSubdiv2D_delta_elems_set
    __swig_getmethods__["delta_elems"] = _cv.CvSubdiv2D_delta_elems_get
    if _newclass:delta_elems = property(_cv.CvSubdiv2D_delta_elems_get, _cv.CvSubdiv2D_delta_elems_set)
    __swig_setmethods__["storage"] = _cv.CvSubdiv2D_storage_set
    __swig_getmethods__["storage"] = _cv.CvSubdiv2D_storage_get
    if _newclass:storage = property(_cv.CvSubdiv2D_storage_get, _cv.CvSubdiv2D_storage_set)
    __swig_setmethods__["free_blocks"] = _cv.CvSubdiv2D_free_blocks_set
    __swig_getmethods__["free_blocks"] = _cv.CvSubdiv2D_free_blocks_get
    if _newclass:free_blocks = property(_cv.CvSubdiv2D_free_blocks_get, _cv.CvSubdiv2D_free_blocks_set)
    __swig_setmethods__["first"] = _cv.CvSubdiv2D_first_set
    __swig_getmethods__["first"] = _cv.CvSubdiv2D_first_get
    if _newclass:first = property(_cv.CvSubdiv2D_first_get, _cv.CvSubdiv2D_first_set)
    __swig_setmethods__["free_elems"] = _cv.CvSubdiv2D_free_elems_set
    __swig_getmethods__["free_elems"] = _cv.CvSubdiv2D_free_elems_get
    if _newclass:free_elems = property(_cv.CvSubdiv2D_free_elems_get, _cv.CvSubdiv2D_free_elems_set)
    __swig_setmethods__["active_count"] = _cv.CvSubdiv2D_active_count_set
    __swig_getmethods__["active_count"] = _cv.CvSubdiv2D_active_count_get
    if _newclass:active_count = property(_cv.CvSubdiv2D_active_count_get, _cv.CvSubdiv2D_active_count_set)
    __swig_setmethods__["edges"] = _cv.CvSubdiv2D_edges_set
    __swig_getmethods__["edges"] = _cv.CvSubdiv2D_edges_get
    if _newclass:edges = property(_cv.CvSubdiv2D_edges_get, _cv.CvSubdiv2D_edges_set)
    __swig_setmethods__["quad_edges"] = _cv.CvSubdiv2D_quad_edges_set
    __swig_getmethods__["quad_edges"] = _cv.CvSubdiv2D_quad_edges_get
    if _newclass:quad_edges = property(_cv.CvSubdiv2D_quad_edges_get, _cv.CvSubdiv2D_quad_edges_set)
    __swig_setmethods__["is_geometry_valid"] = _cv.CvSubdiv2D_is_geometry_valid_set
    __swig_getmethods__["is_geometry_valid"] = _cv.CvSubdiv2D_is_geometry_valid_get
    if _newclass:is_geometry_valid = property(_cv.CvSubdiv2D_is_geometry_valid_get, _cv.CvSubdiv2D_is_geometry_valid_set)
    __swig_setmethods__["recent_edge"] = _cv.CvSubdiv2D_recent_edge_set
    __swig_getmethods__["recent_edge"] = _cv.CvSubdiv2D_recent_edge_get
    if _newclass:recent_edge = property(_cv.CvSubdiv2D_recent_edge_get, _cv.CvSubdiv2D_recent_edge_set)
    __swig_setmethods__["topleft"] = _cv.CvSubdiv2D_topleft_set
    __swig_getmethods__["topleft"] = _cv.CvSubdiv2D_topleft_get
    if _newclass:topleft = property(_cv.CvSubdiv2D_topleft_get, _cv.CvSubdiv2D_topleft_set)
    __swig_setmethods__["bottomright"] = _cv.CvSubdiv2D_bottomright_set
    __swig_getmethods__["bottomright"] = _cv.CvSubdiv2D_bottomright_get
    if _newclass:bottomright = property(_cv.CvSubdiv2D_bottomright_get, _cv.CvSubdiv2D_bottomright_set)
    def __init__(self, *args):
        _swig_setattr(self, CvSubdiv2D, 'this', _cv.new_CvSubdiv2D(*args))
        _swig_setattr(self, CvSubdiv2D, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvSubdiv2D):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvSubdiv2DPtr(CvSubdiv2D):
    def __init__(self, this):
        _swig_setattr(self, CvSubdiv2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvSubdiv2D, 'thisown', 0)
        _swig_setattr(self, CvSubdiv2D,self.__class__,CvSubdiv2D)
_cv.CvSubdiv2D_swigregister(CvSubdiv2DPtr)

CV_PTLOC_ERROR = _cv.CV_PTLOC_ERROR
CV_PTLOC_OUTSIDE_RECT = _cv.CV_PTLOC_OUTSIDE_RECT
CV_PTLOC_INSIDE = _cv.CV_PTLOC_INSIDE
CV_PTLOC_VERTEX = _cv.CV_PTLOC_VERTEX
CV_PTLOC_ON_EDGE = _cv.CV_PTLOC_ON_EDGE
CV_NEXT_AROUND_ORG = _cv.CV_NEXT_AROUND_ORG
CV_NEXT_AROUND_DST = _cv.CV_NEXT_AROUND_DST
CV_PREV_AROUND_ORG = _cv.CV_PREV_AROUND_ORG
CV_PREV_AROUND_DST = _cv.CV_PREV_AROUND_DST
CV_NEXT_AROUND_LEFT = _cv.CV_NEXT_AROUND_LEFT
CV_NEXT_AROUND_RIGHT = _cv.CV_NEXT_AROUND_RIGHT
CV_PREV_AROUND_LEFT = _cv.CV_PREV_AROUND_LEFT
CV_PREV_AROUND_RIGHT = _cv.CV_PREV_AROUND_RIGHT
CV_GAUSSIAN_5x5 = _cv.CV_GAUSSIAN_5x5
class CvMatrix3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvMatrix3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvMatrix3, name)
    def __repr__(self):
        return "<C CvMatrix3 instance at %s>" % (self.this,)
    __swig_setmethods__["m"] = _cv.CvMatrix3_m_set
    __swig_getmethods__["m"] = _cv.CvMatrix3_m_get
    if _newclass:m = property(_cv.CvMatrix3_m_get, _cv.CvMatrix3_m_set)
    def __init__(self, *args):
        _swig_setattr(self, CvMatrix3, 'this', _cv.new_CvMatrix3(*args))
        _swig_setattr(self, CvMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvMatrix3Ptr(CvMatrix3):
    def __init__(self, this):
        _swig_setattr(self, CvMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvMatrix3, 'thisown', 0)
        _swig_setattr(self, CvMatrix3,self.__class__,CvMatrix3)
_cv.CvMatrix3_swigregister(CvMatrix3Ptr)

class CvConDensation(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvConDensation, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvConDensation, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvConDensation instance at %s>" % (self.this,)
    __swig_setmethods__["MP"] = _cv.CvConDensation_MP_set
    __swig_getmethods__["MP"] = _cv.CvConDensation_MP_get
    if _newclass:MP = property(_cv.CvConDensation_MP_get, _cv.CvConDensation_MP_set)
    __swig_setmethods__["DP"] = _cv.CvConDensation_DP_set
    __swig_getmethods__["DP"] = _cv.CvConDensation_DP_get
    if _newclass:DP = property(_cv.CvConDensation_DP_get, _cv.CvConDensation_DP_set)
    __swig_setmethods__["DynamMatr"] = _cv.CvConDensation_DynamMatr_set
    __swig_getmethods__["DynamMatr"] = _cv.CvConDensation_DynamMatr_get
    if _newclass:DynamMatr = property(_cv.CvConDensation_DynamMatr_get, _cv.CvConDensation_DynamMatr_set)
    __swig_setmethods__["State"] = _cv.CvConDensation_State_set
    __swig_getmethods__["State"] = _cv.CvConDensation_State_get
    if _newclass:State = property(_cv.CvConDensation_State_get, _cv.CvConDensation_State_set)
    __swig_setmethods__["SamplesNum"] = _cv.CvConDensation_SamplesNum_set
    __swig_getmethods__["SamplesNum"] = _cv.CvConDensation_SamplesNum_get
    if _newclass:SamplesNum = property(_cv.CvConDensation_SamplesNum_get, _cv.CvConDensation_SamplesNum_set)
    __swig_setmethods__["flSamples"] = _cv.CvConDensation_flSamples_set
    __swig_getmethods__["flSamples"] = _cv.CvConDensation_flSamples_get
    if _newclass:flSamples = property(_cv.CvConDensation_flSamples_get, _cv.CvConDensation_flSamples_set)
    __swig_setmethods__["flNewSamples"] = _cv.CvConDensation_flNewSamples_set
    __swig_getmethods__["flNewSamples"] = _cv.CvConDensation_flNewSamples_get
    if _newclass:flNewSamples = property(_cv.CvConDensation_flNewSamples_get, _cv.CvConDensation_flNewSamples_set)
    __swig_setmethods__["flConfidence"] = _cv.CvConDensation_flConfidence_set
    __swig_getmethods__["flConfidence"] = _cv.CvConDensation_flConfidence_get
    if _newclass:flConfidence = property(_cv.CvConDensation_flConfidence_get, _cv.CvConDensation_flConfidence_set)
    __swig_setmethods__["flCumulative"] = _cv.CvConDensation_flCumulative_set
    __swig_getmethods__["flCumulative"] = _cv.CvConDensation_flCumulative_get
    if _newclass:flCumulative = property(_cv.CvConDensation_flCumulative_get, _cv.CvConDensation_flCumulative_set)
    __swig_setmethods__["Temp"] = _cv.CvConDensation_Temp_set
    __swig_getmethods__["Temp"] = _cv.CvConDensation_Temp_get
    if _newclass:Temp = property(_cv.CvConDensation_Temp_get, _cv.CvConDensation_Temp_set)
    __swig_setmethods__["RandomSample"] = _cv.CvConDensation_RandomSample_set
    __swig_getmethods__["RandomSample"] = _cv.CvConDensation_RandomSample_get
    if _newclass:RandomSample = property(_cv.CvConDensation_RandomSample_get, _cv.CvConDensation_RandomSample_set)
    __swig_setmethods__["RandS"] = _cv.CvConDensation_RandS_set
    __swig_getmethods__["RandS"] = _cv.CvConDensation_RandS_get
    if _newclass:RandS = property(_cv.CvConDensation_RandS_get, _cv.CvConDensation_RandS_set)
    def __del__(self, destroy=_cv.delete_CvConDensation):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvConDensationPtr(CvConDensation):
    def __init__(self, this):
        _swig_setattr(self, CvConDensation, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvConDensation, 'thisown', 0)
        _swig_setattr(self, CvConDensation,self.__class__,CvConDensation)
_cv.CvConDensation_swigregister(CvConDensationPtr)

class CvKalman(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvKalman, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvKalman, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvKalman instance at %s>" % (self.this,)
    __swig_setmethods__["MP"] = _cv.CvKalman_MP_set
    __swig_getmethods__["MP"] = _cv.CvKalman_MP_get
    if _newclass:MP = property(_cv.CvKalman_MP_get, _cv.CvKalman_MP_set)
    __swig_setmethods__["DP"] = _cv.CvKalman_DP_set
    __swig_getmethods__["DP"] = _cv.CvKalman_DP_get
    if _newclass:DP = property(_cv.CvKalman_DP_get, _cv.CvKalman_DP_set)
    __swig_setmethods__["CP"] = _cv.CvKalman_CP_set
    __swig_getmethods__["CP"] = _cv.CvKalman_CP_get
    if _newclass:CP = property(_cv.CvKalman_CP_get, _cv.CvKalman_CP_set)
    __swig_setmethods__["PosterState"] = _cv.CvKalman_PosterState_set
    __swig_getmethods__["PosterState"] = _cv.CvKalman_PosterState_get
    if _newclass:PosterState = property(_cv.CvKalman_PosterState_get, _cv.CvKalman_PosterState_set)
    __swig_setmethods__["PriorState"] = _cv.CvKalman_PriorState_set
    __swig_getmethods__["PriorState"] = _cv.CvKalman_PriorState_get
    if _newclass:PriorState = property(_cv.CvKalman_PriorState_get, _cv.CvKalman_PriorState_set)
    __swig_setmethods__["DynamMatr"] = _cv.CvKalman_DynamMatr_set
    __swig_getmethods__["DynamMatr"] = _cv.CvKalman_DynamMatr_get
    if _newclass:DynamMatr = property(_cv.CvKalman_DynamMatr_get, _cv.CvKalman_DynamMatr_set)
    __swig_setmethods__["MeasurementMatr"] = _cv.CvKalman_MeasurementMatr_set
    __swig_getmethods__["MeasurementMatr"] = _cv.CvKalman_MeasurementMatr_get
    if _newclass:MeasurementMatr = property(_cv.CvKalman_MeasurementMatr_get, _cv.CvKalman_MeasurementMatr_set)
    __swig_setmethods__["MNCovariance"] = _cv.CvKalman_MNCovariance_set
    __swig_getmethods__["MNCovariance"] = _cv.CvKalman_MNCovariance_get
    if _newclass:MNCovariance = property(_cv.CvKalman_MNCovariance_get, _cv.CvKalman_MNCovariance_set)
    __swig_setmethods__["PNCovariance"] = _cv.CvKalman_PNCovariance_set
    __swig_getmethods__["PNCovariance"] = _cv.CvKalman_PNCovariance_get
    if _newclass:PNCovariance = property(_cv.CvKalman_PNCovariance_get, _cv.CvKalman_PNCovariance_set)
    __swig_setmethods__["KalmGainMatr"] = _cv.CvKalman_KalmGainMatr_set
    __swig_getmethods__["KalmGainMatr"] = _cv.CvKalman_KalmGainMatr_get
    if _newclass:KalmGainMatr = property(_cv.CvKalman_KalmGainMatr_get, _cv.CvKalman_KalmGainMatr_set)
    __swig_setmethods__["PriorErrorCovariance"] = _cv.CvKalman_PriorErrorCovariance_set
    __swig_getmethods__["PriorErrorCovariance"] = _cv.CvKalman_PriorErrorCovariance_get
    if _newclass:PriorErrorCovariance = property(_cv.CvKalman_PriorErrorCovariance_get, _cv.CvKalman_PriorErrorCovariance_set)
    __swig_setmethods__["PosterErrorCovariance"] = _cv.CvKalman_PosterErrorCovariance_set
    __swig_getmethods__["PosterErrorCovariance"] = _cv.CvKalman_PosterErrorCovariance_get
    if _newclass:PosterErrorCovariance = property(_cv.CvKalman_PosterErrorCovariance_get, _cv.CvKalman_PosterErrorCovariance_set)
    __swig_setmethods__["Temp1"] = _cv.CvKalman_Temp1_set
    __swig_getmethods__["Temp1"] = _cv.CvKalman_Temp1_get
    if _newclass:Temp1 = property(_cv.CvKalman_Temp1_get, _cv.CvKalman_Temp1_set)
    __swig_setmethods__["Temp2"] = _cv.CvKalman_Temp2_set
    __swig_getmethods__["Temp2"] = _cv.CvKalman_Temp2_get
    if _newclass:Temp2 = property(_cv.CvKalman_Temp2_get, _cv.CvKalman_Temp2_set)
    __swig_setmethods__["state_pre"] = _cv.CvKalman_state_pre_set
    __swig_getmethods__["state_pre"] = _cv.CvKalman_state_pre_get
    if _newclass:state_pre = property(_cv.CvKalman_state_pre_get, _cv.CvKalman_state_pre_set)
    __swig_setmethods__["state_post"] = _cv.CvKalman_state_post_set
    __swig_getmethods__["state_post"] = _cv.CvKalman_state_post_get
    if _newclass:state_post = property(_cv.CvKalman_state_post_get, _cv.CvKalman_state_post_set)
    __swig_setmethods__["transition_matrix"] = _cv.CvKalman_transition_matrix_set
    __swig_getmethods__["transition_matrix"] = _cv.CvKalman_transition_matrix_get
    if _newclass:transition_matrix = property(_cv.CvKalman_transition_matrix_get, _cv.CvKalman_transition_matrix_set)
    __swig_setmethods__["control_matrix"] = _cv.CvKalman_control_matrix_set
    __swig_getmethods__["control_matrix"] = _cv.CvKalman_control_matrix_get
    if _newclass:control_matrix = property(_cv.CvKalman_control_matrix_get, _cv.CvKalman_control_matrix_set)
    __swig_setmethods__["measurement_matrix"] = _cv.CvKalman_measurement_matrix_set
    __swig_getmethods__["measurement_matrix"] = _cv.CvKalman_measurement_matrix_get
    if _newclass:measurement_matrix = property(_cv.CvKalman_measurement_matrix_get, _cv.CvKalman_measurement_matrix_set)
    __swig_setmethods__["process_noise_cov"] = _cv.CvKalman_process_noise_cov_set
    __swig_getmethods__["process_noise_cov"] = _cv.CvKalman_process_noise_cov_get
    if _newclass:process_noise_cov = property(_cv.CvKalman_process_noise_cov_get, _cv.CvKalman_process_noise_cov_set)
    __swig_setmethods__["measurement_noise_cov"] = _cv.CvKalman_measurement_noise_cov_set
    __swig_getmethods__["measurement_noise_cov"] = _cv.CvKalman_measurement_noise_cov_get
    if _newclass:measurement_noise_cov = property(_cv.CvKalman_measurement_noise_cov_get, _cv.CvKalman_measurement_noise_cov_set)
    __swig_setmethods__["error_cov_pre"] = _cv.CvKalman_error_cov_pre_set
    __swig_getmethods__["error_cov_pre"] = _cv.CvKalman_error_cov_pre_get
    if _newclass:error_cov_pre = property(_cv.CvKalman_error_cov_pre_get, _cv.CvKalman_error_cov_pre_set)
    __swig_setmethods__["gain"] = _cv.CvKalman_gain_set
    __swig_getmethods__["gain"] = _cv.CvKalman_gain_get
    if _newclass:gain = property(_cv.CvKalman_gain_get, _cv.CvKalman_gain_set)
    __swig_setmethods__["error_cov_post"] = _cv.CvKalman_error_cov_post_set
    __swig_getmethods__["error_cov_post"] = _cv.CvKalman_error_cov_post_get
    if _newclass:error_cov_post = property(_cv.CvKalman_error_cov_post_get, _cv.CvKalman_error_cov_post_set)
    __swig_setmethods__["temp1"] = _cv.CvKalman_temp1_set
    __swig_getmethods__["temp1"] = _cv.CvKalman_temp1_get
    if _newclass:temp1 = property(_cv.CvKalman_temp1_get, _cv.CvKalman_temp1_set)
    __swig_setmethods__["temp2"] = _cv.CvKalman_temp2_set
    __swig_getmethods__["temp2"] = _cv.CvKalman_temp2_get
    if _newclass:temp2 = property(_cv.CvKalman_temp2_get, _cv.CvKalman_temp2_set)
    __swig_setmethods__["temp3"] = _cv.CvKalman_temp3_set
    __swig_getmethods__["temp3"] = _cv.CvKalman_temp3_get
    if _newclass:temp3 = property(_cv.CvKalman_temp3_get, _cv.CvKalman_temp3_set)
    __swig_setmethods__["temp4"] = _cv.CvKalman_temp4_set
    __swig_getmethods__["temp4"] = _cv.CvKalman_temp4_get
    if _newclass:temp4 = property(_cv.CvKalman_temp4_get, _cv.CvKalman_temp4_set)
    __swig_setmethods__["temp5"] = _cv.CvKalman_temp5_set
    __swig_getmethods__["temp5"] = _cv.CvKalman_temp5_get
    if _newclass:temp5 = property(_cv.CvKalman_temp5_get, _cv.CvKalman_temp5_set)
    def __del__(self, destroy=_cv.delete_CvKalman):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvKalmanPtr(CvKalman):
    def __init__(self, this):
        _swig_setattr(self, CvKalman, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvKalman, 'thisown', 0)
        _swig_setattr(self, CvKalman,self.__class__,CvKalman)
_cv.CvKalman_swigregister(CvKalmanPtr)

class CvHaarFeature(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHaarFeature, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHaarFeature, name)
    def __repr__(self):
        return "<C CvHaarFeature instance at %s>" % (self.this,)
    __swig_setmethods__["tilted"] = _cv.CvHaarFeature_tilted_set
    __swig_getmethods__["tilted"] = _cv.CvHaarFeature_tilted_get
    if _newclass:tilted = property(_cv.CvHaarFeature_tilted_get, _cv.CvHaarFeature_tilted_set)
    __swig_getmethods__["rect"] = _cv.CvHaarFeature_rect_get
    if _newclass:rect = property(_cv.CvHaarFeature_rect_get)
    def __init__(self, *args):
        _swig_setattr(self, CvHaarFeature, 'this', _cv.new_CvHaarFeature(*args))
        _swig_setattr(self, CvHaarFeature, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvHaarFeature):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHaarFeaturePtr(CvHaarFeature):
    def __init__(self, this):
        _swig_setattr(self, CvHaarFeature, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHaarFeature, 'thisown', 0)
        _swig_setattr(self, CvHaarFeature,self.__class__,CvHaarFeature)
_cv.CvHaarFeature_swigregister(CvHaarFeaturePtr)

class CvHaarFeature_rect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHaarFeature_rect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHaarFeature_rect, name)
    def __repr__(self):
        return "<C CvHaarFeature_rect instance at %s>" % (self.this,)
    __swig_setmethods__["r"] = _cv.CvHaarFeature_rect_r_set
    __swig_getmethods__["r"] = _cv.CvHaarFeature_rect_r_get
    if _newclass:r = property(_cv.CvHaarFeature_rect_r_get, _cv.CvHaarFeature_rect_r_set)
    __swig_setmethods__["weight"] = _cv.CvHaarFeature_rect_weight_set
    __swig_getmethods__["weight"] = _cv.CvHaarFeature_rect_weight_get
    if _newclass:weight = property(_cv.CvHaarFeature_rect_weight_get, _cv.CvHaarFeature_rect_weight_set)
    def __init__(self, *args):
        _swig_setattr(self, CvHaarFeature_rect, 'this', _cv.new_CvHaarFeature_rect(*args))
        _swig_setattr(self, CvHaarFeature_rect, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvHaarFeature_rect):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHaarFeature_rectPtr(CvHaarFeature_rect):
    def __init__(self, this):
        _swig_setattr(self, CvHaarFeature_rect, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHaarFeature_rect, 'thisown', 0)
        _swig_setattr(self, CvHaarFeature_rect,self.__class__,CvHaarFeature_rect)
_cv.CvHaarFeature_rect_swigregister(CvHaarFeature_rectPtr)

class CvHaarClassifier(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHaarClassifier, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHaarClassifier, name)
    def __repr__(self):
        return "<C CvHaarClassifier instance at %s>" % (self.this,)
    __swig_setmethods__["count"] = _cv.CvHaarClassifier_count_set
    __swig_getmethods__["count"] = _cv.CvHaarClassifier_count_get
    if _newclass:count = property(_cv.CvHaarClassifier_count_get, _cv.CvHaarClassifier_count_set)
    __swig_setmethods__["haar_feature"] = _cv.CvHaarClassifier_haar_feature_set
    __swig_getmethods__["haar_feature"] = _cv.CvHaarClassifier_haar_feature_get
    if _newclass:haar_feature = property(_cv.CvHaarClassifier_haar_feature_get, _cv.CvHaarClassifier_haar_feature_set)
    __swig_setmethods__["threshold"] = _cv.CvHaarClassifier_threshold_set
    __swig_getmethods__["threshold"] = _cv.CvHaarClassifier_threshold_get
    if _newclass:threshold = property(_cv.CvHaarClassifier_threshold_get, _cv.CvHaarClassifier_threshold_set)
    __swig_setmethods__["left"] = _cv.CvHaarClassifier_left_set
    __swig_getmethods__["left"] = _cv.CvHaarClassifier_left_get
    if _newclass:left = property(_cv.CvHaarClassifier_left_get, _cv.CvHaarClassifier_left_set)
    __swig_setmethods__["right"] = _cv.CvHaarClassifier_right_set
    __swig_getmethods__["right"] = _cv.CvHaarClassifier_right_get
    if _newclass:right = property(_cv.CvHaarClassifier_right_get, _cv.CvHaarClassifier_right_set)
    __swig_setmethods__["alpha"] = _cv.CvHaarClassifier_alpha_set
    __swig_getmethods__["alpha"] = _cv.CvHaarClassifier_alpha_get
    if _newclass:alpha = property(_cv.CvHaarClassifier_alpha_get, _cv.CvHaarClassifier_alpha_set)
    def __init__(self, *args):
        _swig_setattr(self, CvHaarClassifier, 'this', _cv.new_CvHaarClassifier(*args))
        _swig_setattr(self, CvHaarClassifier, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvHaarClassifier):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHaarClassifierPtr(CvHaarClassifier):
    def __init__(self, this):
        _swig_setattr(self, CvHaarClassifier, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHaarClassifier, 'thisown', 0)
        _swig_setattr(self, CvHaarClassifier,self.__class__,CvHaarClassifier)
_cv.CvHaarClassifier_swigregister(CvHaarClassifierPtr)

class CvHaarStageClassifier(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHaarStageClassifier, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHaarStageClassifier, name)
    def __repr__(self):
        return "<C CvHaarStageClassifier instance at %s>" % (self.this,)
    __swig_setmethods__["count"] = _cv.CvHaarStageClassifier_count_set
    __swig_getmethods__["count"] = _cv.CvHaarStageClassifier_count_get
    if _newclass:count = property(_cv.CvHaarStageClassifier_count_get, _cv.CvHaarStageClassifier_count_set)
    __swig_setmethods__["threshold"] = _cv.CvHaarStageClassifier_threshold_set
    __swig_getmethods__["threshold"] = _cv.CvHaarStageClassifier_threshold_get
    if _newclass:threshold = property(_cv.CvHaarStageClassifier_threshold_get, _cv.CvHaarStageClassifier_threshold_set)
    __swig_setmethods__["classifier"] = _cv.CvHaarStageClassifier_classifier_set
    __swig_getmethods__["classifier"] = _cv.CvHaarStageClassifier_classifier_get
    if _newclass:classifier = property(_cv.CvHaarStageClassifier_classifier_get, _cv.CvHaarStageClassifier_classifier_set)
    __swig_setmethods__["next"] = _cv.CvHaarStageClassifier_next_set
    __swig_getmethods__["next"] = _cv.CvHaarStageClassifier_next_get
    if _newclass:next = property(_cv.CvHaarStageClassifier_next_get, _cv.CvHaarStageClassifier_next_set)
    __swig_setmethods__["child"] = _cv.CvHaarStageClassifier_child_set
    __swig_getmethods__["child"] = _cv.CvHaarStageClassifier_child_get
    if _newclass:child = property(_cv.CvHaarStageClassifier_child_get, _cv.CvHaarStageClassifier_child_set)
    __swig_setmethods__["parent"] = _cv.CvHaarStageClassifier_parent_set
    __swig_getmethods__["parent"] = _cv.CvHaarStageClassifier_parent_get
    if _newclass:parent = property(_cv.CvHaarStageClassifier_parent_get, _cv.CvHaarStageClassifier_parent_set)
    def __init__(self, *args):
        _swig_setattr(self, CvHaarStageClassifier, 'this', _cv.new_CvHaarStageClassifier(*args))
        _swig_setattr(self, CvHaarStageClassifier, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvHaarStageClassifier):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHaarStageClassifierPtr(CvHaarStageClassifier):
    def __init__(self, this):
        _swig_setattr(self, CvHaarStageClassifier, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHaarStageClassifier, 'thisown', 0)
        _swig_setattr(self, CvHaarStageClassifier,self.__class__,CvHaarStageClassifier)
_cv.CvHaarStageClassifier_swigregister(CvHaarStageClassifierPtr)

class CvHaarClassifierCascade(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvHaarClassifierCascade, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvHaarClassifierCascade, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C CvHaarClassifierCascade instance at %s>" % (self.this,)
    __swig_setmethods__["flags"] = _cv.CvHaarClassifierCascade_flags_set
    __swig_getmethods__["flags"] = _cv.CvHaarClassifierCascade_flags_get
    if _newclass:flags = property(_cv.CvHaarClassifierCascade_flags_get, _cv.CvHaarClassifierCascade_flags_set)
    __swig_setmethods__["count"] = _cv.CvHaarClassifierCascade_count_set
    __swig_getmethods__["count"] = _cv.CvHaarClassifierCascade_count_get
    if _newclass:count = property(_cv.CvHaarClassifierCascade_count_get, _cv.CvHaarClassifierCascade_count_set)
    __swig_setmethods__["orig_window_size"] = _cv.CvHaarClassifierCascade_orig_window_size_set
    __swig_getmethods__["orig_window_size"] = _cv.CvHaarClassifierCascade_orig_window_size_get
    if _newclass:orig_window_size = property(_cv.CvHaarClassifierCascade_orig_window_size_get, _cv.CvHaarClassifierCascade_orig_window_size_set)
    __swig_setmethods__["real_window_size"] = _cv.CvHaarClassifierCascade_real_window_size_set
    __swig_getmethods__["real_window_size"] = _cv.CvHaarClassifierCascade_real_window_size_get
    if _newclass:real_window_size = property(_cv.CvHaarClassifierCascade_real_window_size_get, _cv.CvHaarClassifierCascade_real_window_size_set)
    __swig_setmethods__["scale"] = _cv.CvHaarClassifierCascade_scale_set
    __swig_getmethods__["scale"] = _cv.CvHaarClassifierCascade_scale_get
    if _newclass:scale = property(_cv.CvHaarClassifierCascade_scale_get, _cv.CvHaarClassifierCascade_scale_set)
    __swig_setmethods__["stage_classifier"] = _cv.CvHaarClassifierCascade_stage_classifier_set
    __swig_getmethods__["stage_classifier"] = _cv.CvHaarClassifierCascade_stage_classifier_get
    if _newclass:stage_classifier = property(_cv.CvHaarClassifierCascade_stage_classifier_get, _cv.CvHaarClassifierCascade_stage_classifier_set)
    __swig_setmethods__["hid_cascade"] = _cv.CvHaarClassifierCascade_hid_cascade_set
    __swig_getmethods__["hid_cascade"] = _cv.CvHaarClassifierCascade_hid_cascade_get
    if _newclass:hid_cascade = property(_cv.CvHaarClassifierCascade_hid_cascade_get, _cv.CvHaarClassifierCascade_hid_cascade_set)
    def __del__(self, destroy=_cv.delete_CvHaarClassifierCascade):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvHaarClassifierCascadePtr(CvHaarClassifierCascade):
    def __init__(self, this):
        _swig_setattr(self, CvHaarClassifierCascade, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvHaarClassifierCascade, 'thisown', 0)
        _swig_setattr(self, CvHaarClassifierCascade,self.__class__,CvHaarClassifierCascade)
_cv.CvHaarClassifierCascade_swigregister(CvHaarClassifierCascadePtr)

class CvAvgComp(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvAvgComp, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvAvgComp, name)
    def __repr__(self):
        return "<C CvAvgComp instance at %s>" % (self.this,)
    __swig_setmethods__["rect"] = _cv.CvAvgComp_rect_set
    __swig_getmethods__["rect"] = _cv.CvAvgComp_rect_get
    if _newclass:rect = property(_cv.CvAvgComp_rect_get, _cv.CvAvgComp_rect_set)
    __swig_setmethods__["neighbors"] = _cv.CvAvgComp_neighbors_set
    __swig_getmethods__["neighbors"] = _cv.CvAvgComp_neighbors_get
    if _newclass:neighbors = property(_cv.CvAvgComp_neighbors_get, _cv.CvAvgComp_neighbors_set)
    def __init__(self, *args):
        _swig_setattr(self, CvAvgComp, 'this', _cv.new_CvAvgComp(*args))
        _swig_setattr(self, CvAvgComp, 'thisown', 1)
    def __del__(self, destroy=_cv.delete_CvAvgComp):
        try:
            if self.thisown: destroy(self)
        except: pass

class CvAvgCompPtr(CvAvgComp):
    def __init__(self, this):
        _swig_setattr(self, CvAvgComp, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvAvgComp, 'thisown', 0)
        _swig_setattr(self, CvAvgComp,self.__class__,CvAvgComp)
_cv.CvAvgComp_swigregister(CvAvgCompPtr)


cvCopyMakeBorder = _cv.cvCopyMakeBorder

cvSmooth = _cv.cvSmooth

cvFilter2D = _cv.cvFilter2D

cvIntegral = _cv.cvIntegral

cvPyrDown = _cv.cvPyrDown

cvPyrUp = _cv.cvPyrUp

cvPyrSegmentation = _cv.cvPyrSegmentation

cvSobel = _cv.cvSobel

cvLaplace = _cv.cvLaplace

cvCvtColor = _cv.cvCvtColor

cvResize = _cv.cvResize

cvWarpAffine = _cv.cvWarpAffine

cv2DRotationMatrix = _cv.cv2DRotationMatrix

cvWarpPerspective = _cv.cvWarpPerspective

cvWarpPerspectiveQMatrix = _cv.cvWarpPerspectiveQMatrix

cvRemap = _cv.cvRemap

cvLogPolar = _cv.cvLogPolar

cvCreateStructuringElementEx = _cv.cvCreateStructuringElementEx

cvReleaseStructuringElement = _cv.cvReleaseStructuringElement

cvErode = _cv.cvErode

cvDilate = _cv.cvDilate

cvMorphologyEx = _cv.cvMorphologyEx

cvMoments = _cv.cvMoments

cvGetSpatialMoment = _cv.cvGetSpatialMoment

cvGetCentralMoment = _cv.cvGetCentralMoment

cvGetNormalizedCentralMoment = _cv.cvGetNormalizedCentralMoment

cvGetHuMoments = _cv.cvGetHuMoments

cvInitLineIterator = _cv.cvInitLineIterator

cvSampleLine = _cv.cvSampleLine

cvGetRectSubPix = _cv.cvGetRectSubPix

cvGetQuadrangleSubPix = _cv.cvGetQuadrangleSubPix

cvMatchTemplate = _cv.cvMatchTemplate

cvCalcEMD2 = _cv.cvCalcEMD2

cvFindContours = _cv.cvFindContours

cvStartFindContours = _cv.cvStartFindContours

cvFindNextContour = _cv.cvFindNextContour

cvSubstituteContour = _cv.cvSubstituteContour

cvEndFindContours = _cv.cvEndFindContours

cvApproxChains = _cv.cvApproxChains

cvStartReadChainPoints = _cv.cvStartReadChainPoints

cvReadChainPoint = _cv.cvReadChainPoint

cvCalcOpticalFlowLK = _cv.cvCalcOpticalFlowLK

cvCalcOpticalFlowBM = _cv.cvCalcOpticalFlowBM

cvCalcOpticalFlowHS = _cv.cvCalcOpticalFlowHS

cvCalcOpticalFlowPyrLK = _cv.cvCalcOpticalFlowPyrLK

cvUpdateMotionHistory = _cv.cvUpdateMotionHistory

cvCalcMotionGradient = _cv.cvCalcMotionGradient

cvCalcGlobalOrientation = _cv.cvCalcGlobalOrientation

cvSegmentMotion = _cv.cvSegmentMotion

cvAcc = _cv.cvAcc

cvSquareAcc = _cv.cvSquareAcc

cvMultiplyAcc = _cv.cvMultiplyAcc

cvRunningAvg = _cv.cvRunningAvg

cvCamShift = _cv.cvCamShift

cvMeanShift = _cv.cvMeanShift

cvCreateConDensation = _cv.cvCreateConDensation

cvReleaseConDensation = _cv.cvReleaseConDensation

cvConDensUpdateByTime = _cv.cvConDensUpdateByTime

cvConDensInitSampleSet = _cv.cvConDensInitSampleSet

cvCreateKalman = _cv.cvCreateKalman

cvReleaseKalman = _cv.cvReleaseKalman

cvKalmanPredict = _cv.cvKalmanPredict

cvKalmanCorrect = _cv.cvKalmanCorrect

cvInitSubdivDelaunay2D = _cv.cvInitSubdivDelaunay2D

cvCreateSubdiv2D = _cv.cvCreateSubdiv2D

cvCreateSubdivDelaunay2D = _cv.cvCreateSubdivDelaunay2D

cvSubdivDelaunay2DInsert = _cv.cvSubdivDelaunay2DInsert

cvSubdiv2DLocate = _cv.cvSubdiv2DLocate

cvCalcSubdivVoronoi2D = _cv.cvCalcSubdivVoronoi2D

cvClearSubdivVoronoi2D = _cv.cvClearSubdivVoronoi2D

cvFindNearestPoint2D = _cv.cvFindNearestPoint2D

cvSubdiv2DNextEdge = _cv.cvSubdiv2DNextEdge

cvSubdiv2DRotateEdge = _cv.cvSubdiv2DRotateEdge

cvSubdiv2DSymEdge = _cv.cvSubdiv2DSymEdge

cvSubdiv2DGetEdge = _cv.cvSubdiv2DGetEdge

cvSubdiv2DEdgeOrg = _cv.cvSubdiv2DEdgeOrg

cvSubdiv2DEdgeDst = _cv.cvSubdiv2DEdgeDst

cvTriangleArea = _cv.cvTriangleArea

cvApproxPoly = _cv.cvApproxPoly

cvFindDominantPoints = _cv.cvFindDominantPoints

cvArcLength = _cv.cvArcLength

cvBoundingRect = _cv.cvBoundingRect

cvContourArea = _cv.cvContourArea

cvMinAreaRect2 = _cv.cvMinAreaRect2

cvMinEnclosingCircle = _cv.cvMinEnclosingCircle

cvMatchShapes = _cv.cvMatchShapes

cvCreateContourTree = _cv.cvCreateContourTree

cvContourFromContourTree = _cv.cvContourFromContourTree

cvMatchContourTrees = _cv.cvMatchContourTrees

cvCalcPGH = _cv.cvCalcPGH

cvConvexHull2 = _cv.cvConvexHull2

cvCheckContourConvexity = _cv.cvCheckContourConvexity

cvConvexityDefects = _cv.cvConvexityDefects

cvFitEllipse2 = _cv.cvFitEllipse2

cvMaxRect = _cv.cvMaxRect

cvBoxPoints = _cv.cvBoxPoints

cvCreateHist = _cv.cvCreateHist

cvSetHistBinRanges = _cv.cvSetHistBinRanges

cvMakeHistHeaderForArray = _cv.cvMakeHistHeaderForArray

cvReleaseHist = _cv.cvReleaseHist

cvClearHist = _cv.cvClearHist

cvGetMinMaxHistValue = _cv.cvGetMinMaxHistValue

cvNormalizeHist = _cv.cvNormalizeHist

cvThreshHist = _cv.cvThreshHist

cvCompareHist = _cv.cvCompareHist

cvCopyHist = _cv.cvCopyHist

cvCalcBayesianProb = _cv.cvCalcBayesianProb

cvCalcArrHist = _cv.cvCalcArrHist

cvCalcHist = _cv.cvCalcHist

cvCalcArrBackProject = _cv.cvCalcArrBackProject

cvCalcArrBackProjectPatch = _cv.cvCalcArrBackProjectPatch

cvCalcProbDensity = _cv.cvCalcProbDensity

cvSnakeImage = _cv.cvSnakeImage

cvCalcImageHomography = _cv.cvCalcImageHomography

cvDistTransform = _cv.cvDistTransform

cvThreshold = _cv.cvThreshold

cvAdaptiveThreshold = _cv.cvAdaptiveThreshold

cvFloodFill = _cv.cvFloodFill

cvCanny = _cv.cvCanny

cvPreCornerDetect = _cv.cvPreCornerDetect

cvCornerEigenValsAndVecs = _cv.cvCornerEigenValsAndVecs

cvCornerMinEigenVal = _cv.cvCornerMinEigenVal

cvCornerHarris = _cv.cvCornerHarris

cvFindCornerSubPix = _cv.cvFindCornerSubPix

cvGoodFeaturesToTrack = _cv.cvGoodFeaturesToTrack

cvHoughLines2 = _cv.cvHoughLines2

cvFitLine = _cv.cvFitLine

cvLoadHaarClassifierCascade = _cv.cvLoadHaarClassifierCascade

cvReleaseHaarClassifierCascade = _cv.cvReleaseHaarClassifierCascade

cvHaarDetectObjects = _cv.cvHaarDetectObjects

cvSetImagesForHaarClassifierCascade = _cv.cvSetImagesForHaarClassifierCascade

cvRunHaarClassifierCascade = _cv.cvRunHaarClassifierCascade

cvUnDistortOnce = _cv.cvUnDistortOnce

cvUnDistortInit = _cv.cvUnDistortInit

cvUnDistort = _cv.cvUnDistort

cvConvertMap = _cv.cvConvertMap

cvCalibrateCamera = _cv.cvCalibrateCamera

cvCalibrateCamera_64d = _cv.cvCalibrateCamera_64d

cvFindExtrinsicCameraParams = _cv.cvFindExtrinsicCameraParams

cvFindExtrinsicCameraParams_64d = _cv.cvFindExtrinsicCameraParams_64d

cvRodrigues = _cv.cvRodrigues

cvProjectPoints = _cv.cvProjectPoints

cvProjectPointsSimple = _cv.cvProjectPointsSimple

cvFindChessBoardCornerGuesses = _cv.cvFindChessBoardCornerGuesses

cvCreatePOSITObject = _cv.cvCreatePOSITObject

cvPOSIT = _cv.cvPOSIT

cvReleasePOSITObject = _cv.cvReleasePOSITObject

cvMake2DPoints = _cv.cvMake2DPoints

cvMake3DPoints = _cv.cvMake3DPoints

cvSolveCubic = _cv.cvSolveCubic

cvFindFundamentalMat = _cv.cvFindFundamentalMat

cvComputeCorrespondEpilines = _cv.cvComputeCorrespondEpilines

SendErrorToPython = _cv.SendErrorToPython

function_ptr_generator = _cv.function_ptr_generator

void_ptr_generator = _cv.void_ptr_generator

void_ptrptr_generator = _cv.void_ptrptr_generator
__doc__ = """
OpenCV is the Intel Open CV library, an open source effort to provide
computer vision algorithms for standard PC hardware.

This wrapper was semi-automatically created from the C/C++ headers and therefore
contains no Python documentation. Because all identifiers are identical to their
C/C++ counterparts, you can consult the standard manuals that come with OpenCV.
"""

# this tells OpenCV not to call exit() on errors but throw a python exception instead
cvRedirectError(function_ptr_generator(), void_ptr_generator(), void_ptrptr_generator())



