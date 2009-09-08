all: CxCore-py.tex CxCore-c.tex CvReference-py.tex CvReference-c.tex HighGui-py.tex HighGui-c.tex

CxCore-py.tex CxCore-c.tex: CxCore.tex.m4
	m4 -DTARGET_LANGUAGE=py CxCore.tex.m4 > CxCore-py.tex
	m4 -DTARGET_LANGUAGE=c CxCore.tex.m4 > CxCore-c.tex

CvReference-py.tex CvReference-c.tex: CvReference.tex.m4
	m4 -DTARGET_LANGUAGE=py CvReference.tex.m4 > CvReference-py.tex
	m4 -DTARGET_LANGUAGE=c CvReference.tex.m4 > CvReference-c.tex

HighGui-py.tex HighGui-c.tex: HighGui.tex.m4
	m4 -DTARGET_LANGUAGE=py HighGui.tex.m4 > HighGui-py.tex
	m4 -DTARGET_LANGUAGE=c HighGui.tex.m4 > HighGui-c.tex
