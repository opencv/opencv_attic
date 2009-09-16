import sys

class argument:
  def __init__(self, fields):
    self.ty = fields[0]
    self.nm = fields[1]
    self.flags = ""
    self.init = None

    if len(fields) > 2:
      if fields[2][0] == '/':
        self.flags = fields[2][1:].split(",")
      else:
        self.init = fields[2]

api = []
for l in open("%s/api" % sys.argv[1]):
  if l[0] == '#':
    continue
  l = l.rstrip()
  f = l.split()
  if len(f) != 0:
    if l[0] != ' ':
      if len(f) > 1:
        ty = f[1]
      else:
        ty = None
      api.append((f[0], [], ty))
    else:
      api[-1][1].append(argument(f))

def cname(n):
  if n.startswith("CV"):
    return '_' + n
  elif n[0].isdigit():
    return '_' + n
  else:
    return n

# RHS is how the aggregate gets expanded in the C call
aggregate = {
  'pts_npts_contours' :  '!.pts,!.npts,!.contours',
  'cvarr_count' :        '!.cvarr,!.count',
  'cvarr_plane_count' :  '!.cvarr,!.count',
  'floats' :             '!.f',
  'ints' :               '!.i',
  'CvPoints' :           '!.p,!.count',
  'CvPoint2D32fs' :      '!.p,!.count',
  'CvPoint3D32fs' :      '!.p,!.count',
  'cvarrseq' :           '!.v',
  'CvArrs' :             '!.ims',
  'IplImages' :          '!.ims',
  'intpair' :            '!.pairs,!.count',
  'cvpoint2d32f_count' : '!.points,&!.count'
}
conversion_types = [
'char',
'CvArr',
'CvArrSeq',
'CvBox2D', # '((ff)(ff)f)',
'CvBox2D*',
'CvCapture*',
'CvVideoWriter*',
'CvContourTree*',
'CvFont',
'CvFont*',
'CvHaarClassifierCascade*',
'CvHistogram',
'CvMat',
'CvMatND',
'CvMemStorage',
'CvMoments',
'CvMoments*',
'CvNextEdgeType',
'CvPoint',
'CvPoint*',
'CvPoint2D32f', # '(ff)',
'CvPoint2D32f*',
'CvPoint3D32f*',
'CvPOSITObject*',
'CvRect',
'CvRect*',
'CvRNG*',
'CvScalar',
'CvSeq',
'CvSeqOfCvConvexityDefect',
'CvSize',
'CvSlice',
'CvStarDetectorParams',
'CvSubdiv2D*',
'CvSubdiv2DEdge',
'CvTermCriteria',
'generic',
'IplConvKernel*',
'IplImage',
]

def safename(s):
  return s.replace('*', 'PTR').replace('[', '_').replace(']', '_')

def has_optional(al):
    """ return true if any argument is optional """
    return any([a.init for a in al])

def gen(name, args, ty):
  yield ""
  if has_optional(args):
      yield "static PyObject *pycv%s(PyObject *self, PyObject *args, PyObject *kw)" % cname(name) 
  else:
      yield "static PyObject *pycv%s(PyObject *self, PyObject *args)" % cname(name)
  yield "{"

  destinations = []
  for a in args:
    remap = {
     'CvArr' : 'CvArr*',
     'CvMat' : 'CvMat*',
     'CvMatND' : 'CvMatND*',
     'IplImage' : 'IplImage*',
     'CvMemStorage' : 'CvMemStorage*',
     'CvHistogram':'CvHistogram*',
     'CvSeq':'CvSeq*',
     'CvHaarClassifierCascade' : 'CvHaarClassifierCascade*'
    }
    ctype = remap.get(a.ty, a.ty)
    if a.init:
      init = " = %s" % a.init
    else:
      init = ''
    yield "  %s %s%s;" % (ctype, a.nm, init)
    if 'O' in a.flags:
      continue
    if a.ty in (conversion_types + aggregate.keys()):
      yield '  PyObject *pyobj_%s = NULL;' % (a.nm)
      destinations.append('&pyobj_%s' % (a.nm))
    elif a.ty in [ 'CvPoint2D32f' ]:
      destinations.append('&%s.x, &%s.y' % (a.nm, a.nm))
    elif a.ty in [ 'CvTermCriteria' ]:
      destinations.append('&%s.type, &%s.max_iter, &%s.epsilon' % ((a.nm,)*3))
    elif a.ty in [ 'CvSURFParams' ]:
      destinations.append('&%s.extended, &%s.hessianThreshold, &%s.nOctaves, &%s.nOctaveLayers' % ((a.nm,)*4))
    elif a.nm in [ 'CvBox2D' ]:
      s = ", ".join([('&' + a.nm +'.' + fld) for fld in [ 'center.x', 'center.y', 'size.width', 'size.height', 'angle' ] ])
      destinations.append(s)
    else:
      destinations.append('&%s' % a.nm)
  fmap = {
    'CvSURFParams' : '(idii)',
    'double' : 'd',
    'float' : 'f',
    'int' : 'i',
    'int64' : 'L',
    'char*' : 's',
  }
  for k in (conversion_types + aggregate.keys()):
    fmap[k] = 'O'
  in_args = [ a for a in args if not 'O' in a.flags ]
  fmt0 = "".join([ fmap[a.ty] for a in in_args if not a.init])
  fmt1 = "".join([ fmap[a.ty] for a in in_args if a.init])
      
  yield ''
  if len(fmt0 + fmt1) > 0:
    if len(fmt1) > 0:
      yield '  const char *keywords[] = { %s };' % (", ".join([ '"%s"' % arg.nm for arg in args if not 'O' in arg.flags ] + ['NULL']))
      yield '  if (!PyArg_ParseTupleAndKeywords(args, kw, "%s|%s", %s))' % (fmt0, fmt1, ", ".join(['(char**)keywords'] + destinations))
      if '(' in (fmt0 + fmt1):
        print "Tuple with kwargs is not allowed, function", name
        sys.exit(1)
    else:
      yield '  if (!PyArg_ParseTuple(args, "%s", %s))' % (fmt0, ", ".join(destinations))
    yield '    return NULL;'

  # Do the conversions:
  for a in args:
    joinwith = [f[2:] for f in a.flags if f.startswith("J:")]
    if len(joinwith) > 0:
      yield 'preShareData(%s, &%s);' % (joinwith[0], a.nm)
    if 'O' in a.flags:
      continue
    if a.ty in (conversion_types + aggregate.keys()):
      if a.init:
        pred = '(pyobj_%s != NULL) && ' % a.nm
      else:
        pred = ''
      yield '  if (%s!convert_to_%s(pyobj_%s, &%s, "%s")) return NULL;' % (pred, safename(a.ty), a.nm, a.nm, a.nm)

  def invokename(a):
    if 'K' in a.flags:
      prefix = "(const CvArr **)"
    elif 'O' in a.flags and not 'A' in a.flags:
      prefix = "&"
    else:
      prefix = ""
    if a.ty in aggregate:
      return prefix + aggregate[a.ty].replace('!', a.nm)
    else:
      return prefix + a.nm

  def funcname(s):
    # The name by which the function is called, in C
    if s.startswith("CV"):
      return s
    else:
      return "cv" + s
  tocall = '%s(%s)' % (funcname(name), ", ".join(invokename(a) for a in args))
  if ty == None:
    yield '  ERRWRAP(%s);' % tocall
    yield '  Py_RETURN_NONE;'
  else:
    Rtypes = [
      'int',
      'double',
      'CvCapture*',
      'CvVideoWriter*',
      'CvPOSITObject*',
      'CvScalar',
      'CvSize',
      'CvRect',
      'CvSeq*',
      'CvBox2D',
      'CvSeqOfCvAvgComp*',
      'CvSeqOfCvConvexityDefect*',
      'CvSeqOfCvStarKeypoint*',
      'CvSeqOfCvSURFPoint*',
      'CvSeqOfCvSURFDescriptor*',
      'CvContourTree*',
      'IplConvKernel*',
      'IplImage*',
      'CvMat*',
      'CvMatND*',
      'CvPoint2D32f_4',
      'CvRNG',
      'CvSubdiv2D*',
      'CvSubdiv2DPoint*',
      'CvSubdiv2DEdge',
      'ROIplImage*',
      'float',
      'generic',
      'unsigned' ]

    if ty in Rtypes:
      yield '  %s r;' % (ty)
      yield '  ERRWRAP(r = %s);' % (tocall)
      yield '  return FROM_%s(r);' % safename(ty)
    else:
      all_returns = ty.split(",")
      return_value_from_call = len(set(Rtypes) & set(all_returns)) != 0
      if return_value_from_call:
        yield '  %s r;' % list(set(Rtypes) & set(all_returns))[0]
        yield '  ERRWRAP(r = %s);' % (tocall)
      else:
        yield '  ERRWRAP(%s);' % (tocall)
      typed = dict([ (a.nm,a.ty) for a in args])
      for i in range(len(all_returns)):
        if all_returns[i] in Rtypes:
          typed['r'] = all_returns[i]
          all_returns[i] = "r"
      if len(all_returns) == 1:
        af = dict([ (a.nm,a.flags) for a in args])
        joinwith = [f[2:] for f in af.get(all_returns[0], []) if f.startswith("J:")]
        if len(joinwith) > 0:
            yield '  return shareData(pyobj_%s, %s, %s);' % (joinwith[0], joinwith[0], all_returns[0])
        else:
            yield '  return FROM_%s(%s);' % (safename(typed[all_returns[0]]), all_returns[0])
      else:
        yield '  return Py_BuildValue("%s", %s);' % ("N" * len(all_returns), ", ".join(["FROM_%s(%s)" % (safename(typed[n]), n) for n in all_returns]))

  yield '}'

gen_c = [ open("generated%d.i" % i, "w") for i in range(3) ]

print "Generated %d functions" % len(api)
for nm,args,ty in sorted(api):

  # Figure out docstring into ds_*
  ds_args = []
  mandatory = [a.nm for a in args if not ('O' in a.flags) and not a.init]
  optional = [a.nm for a in args if not ('O' in a.flags) and a.init]
  ds_args = ", ".join(mandatory)
  def o2s(o):
    if o == []:
        return ""
    else:
        return ' [, %s%s]' % (o[0], o2s(o[1:]))
  ds_args += o2s(optional)

  ds = "%s(%s) -> %s" % (nm, ds_args, str(ty))
  print ds

  if has_optional(args):
      entry = '{"%%s", (PyCFunction)pycv%s, METH_KEYWORDS, "%s"},' % (cname(nm), ds)
  else:
      entry = '{"%%s", pycv%s, METH_VARARGS, "%s"},' % (cname(nm), ds)
  print >>gen_c[1], entry % (nm)
  if nm.startswith('CV_'):
    print >>gen_c[1], entry % (nm[3:])
  for l in gen(nm,args,ty):
    print >>gen_c[0], l

for l in open("%s/defs" % sys.argv[1]):
  print >>gen_c[2], "PUBLISH(%s);" % l.split()[1]

for f in gen_c:
  f.close()
