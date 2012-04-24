from distutils.core import setup, Extension
import os

opencv_base_dir = r'C:\Program Files\OpenCV'

setup(name='OpenCV Python Wrapper',
      version='0.0',
      packages = ['opencv'],
      package_dir = {'opencv': os.path.join (opencv_base_dir,
                                             'interfaces', 'swig', 'python')},
      ext_modules=[Extension('opencv._cv',
                             [os.path.join (opencv_base_dir,
                                            'interfaces', 'swig', 'python',
                                            '_cv.cpp'),
                              os.path.join (opencv_base_dir,
                                            'interfaces', 'swig', 'python',
                                            'error.cpp'),
                              os.path.join (opencv_base_dir,
                                            'interfaces', 'swig', 'python',
                                            'pycvseq.cpp')],
                             include_dirs = [os.path.join (opencv_base_dir,
                                                           'cv', 'include'),
                                             os.path.join (opencv_base_dir,
                                                           'cxcore', 'include'),
                                                           ],
                             library_dirs = [os.path.join (opencv_base_dir,
                                                           'lib')],
                             libraries = ['cv', 'cxcore'],
                             ),
                   Extension('opencv._highgui',
                             [os.path.join (opencv_base_dir,
                                            'interfaces', 'swig', 'python',
                                            '_highgui.cpp')],
                             include_dirs = [os.path.join (opencv_base_dir,
                                                           'otherlibs', 'highgui'),
                                             os.path.join (opencv_base_dir,
                                                           'cxcore', 'include')],
                             library_dirs = [os.path.join (opencv_base_dir,
                                                           'lib')],
                             libraries = ['highgui', 'cv', 'cxcore'],
                             )
                   ]
      )
