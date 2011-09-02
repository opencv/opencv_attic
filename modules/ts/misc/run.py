import testlog_parser, sys, os, xml, re
from table_formatter import *
from optparse import OptionParser
from subprocess import Popen, PIPE

hostos = os.name
nameprefix = "opencv_perf_"

parse_patterns = (
  {'name': "has_perf_tests",     'default': "OFF",      'pattern': re.compile("^BUILD_PERF_TESTS:BOOL=(ON)$")},
  {'name': "cmake_home",         'default': None,       'pattern': re.compile("^CMAKE_HOME_DIRECTORY:INTERNAL=(.+)$")},
  {'name': "opencv_home",        'default': None,       'pattern': re.compile("^OpenCV_SOURCE_DIR:STATIC=(.+)$")},
  {'name': "tests_dir",          'default': None,       'pattern': re.compile("^EXECUTABLE_OUTPUT_PATH:PATH=(.+)$")},
  {'name': "build_type",         'default': "Release",  'pattern': re.compile("^CMAKE_BUILD_TYPE:STRING=(.*)$")},
  {'name': "svnversion_path",    'default': None,       'pattern': re.compile("^SVNVERSION_PATH:FILEPATH=(.*)$")},
  {'name': "cxx_flags",          'default': None,       'pattern': re.compile("^CMAKE_CXX_FLAGS:STRING=(.*)$")},
  {'name': "cxx_flags_debug",    'default': None,       'pattern': re.compile("^CMAKE_CXX_FLAGS_DEBUG:STRING=(.*)$")},
  {'name': "cxx_flags_release",  'default': None,       'pattern': re.compile("^CMAKE_CXX_FLAGS_RELEASE:STRING=(.*)$")},
  {'name': "ndk_path",           'default': None,       'pattern': re.compile("^ANDROID_NDK(?:_TOOLCHAIN_ROOT)?:PATH=(.*)$")},
  {'name': "arm_target",         'default': None,       'pattern': re.compile("^ARM_TARGET:INTERNAL=(.*)$")},
  {'name': "android_executable", 'default': None,       'pattern': re.compile("^ANDROID_EXECUTABLE:FILEPATH=(.*android.*)$")},
)

class RunInfo(object):
    def __init__(self, path):
        self.path = path
        for p in parse_patterns:
            setattr(self, p["name"], p["default"])
        cachefile = open(os.path.join(path, "CMakeCache.txt"), "rt")
        try:
            for l in cachefile.readlines():
                ll = l.strip()
                if not ll or ll.startswith("#"):
                    continue
                for p in parse_patterns:
                    match = p["pattern"].match(ll)
                    if match:
                        value = match.groups()[0]
                        if value and not value.endswith("-NOTFOUND"):
                            setattr(self, p["name"], value)
        except:
            pass
        cachefile.close()
        # add path to adb
        if self.android_executable:
            self.adb = os.path.join(os.path.dirname(os.path.dirname(self.android_executable)), ("platform-tools/adb","platform-tools/adb.exe")[hostos == 'nt'])
        else:
            self.adb = None
        # detect target platform    
        if self.android_executable or self.arm_target or self.ndk_path:
            self.targetos = "android"
        else:
            self.targetos = hostos
        # fix has_perf_tests param
        self.has_perf_tests = self.has_perf_tests == "ON"
        
        self.getSvnVersion(self.cmake_home, "cmake_home_svn")
        self.getSvnVersion(self.opencv_home, "opencv_home_svn")
        
    def getSvnVersion(self, path, name):
        if not self.has_perf_tests or not self.svnversion_path or not os.path.isdir(path):
            setattr(self, name, None)
            return
        svnprocess = Popen([self.svnversion_path, "-n", path], stdout=PIPE, stderr=PIPE)
        output = svnprocess.communicate()
        setattr(self, name, output[0])

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-t", "--tests", dest="tests", help="comma-separated list of modules to test", metavar="SUITS", default="")
    (options, args) = parser.parse_args()
    
    test_args = [a for a in args if a.startswith("--perf_") or a.startswith("--gtest_")]
    run_args2 = [a for a in args if not(a.startswith("--perf_") or a.startswith("--gtest_"))]
    
    run_args = []
    
    for path in run_args2:
        path = os.path.abspath(path)
        while (True):
            if os.path.isdir(path) and os.path.isfile(os.path.join(path, "CMakeCache.txt")):
                run_args.append(path)
                break
            npath = os.path.dirname(path)
            if npath == path:
                break
            path = npath
    #run_args = [ a for a in run_args if os.path.isdir(a) and os.path.isfile(os.path.join(a, "CMakeCache.txt"))]
    
    if len(run_args) == 0:
        print >> sys.stderr, "Usage:\n", os.path.basename(sys.argv[0]), "<build_path>"
        exit(1)
        
    tests = [s.strip() for s in options.tests.split(",")]
    for i in range(len(tests)):
        name = tests[i]
        if not name.startswith(nameprefix):
            tests[i] = nameprefix + name
    
    infos = []
    for path in run_args:
        infos.append(RunInfo(path))
        print vars(infos[len(infos)-1]),"\n"