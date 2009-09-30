import unittest
import random
import time
import math
import sys
import array
import os

import cv

def find_sample(s):
    for d in ["../samples/c/", "../doc/pics/"]:
        path = os.path.join(d, s)
        if os.access(path, os.R_OK):
            return path
    return s

class FrameInterpolator:
    def __init__(self, prev, curr):

        w,h = cv.GetSize(prev)

        self.offx = cv.CreateMat(h, w, cv.CV_32FC1)
        self.offy = cv.CreateMat(h, w, cv.CV_32FC1)
        for y in range(h):
            for x in range(w):
                self.offx[y,x] = x
                self.offy[y,x] = y

        self.maps = [ None, None ]
        for i,a,b in [ (0, prev, curr), (1, curr, prev) ]:
            velx = cv.CreateMat(h, w, cv.CV_32FC1)
            vely = cv.CreateMat(h, w, cv.CV_32FC1)
            cv.CalcOpticalFlowLK(a, b, (15,15), velx, vely)

            for j in range(10):
                cv.Smooth(velx, velx, param1 = 7)
                cv.Smooth(vely, vely, param1 = 7)
            self.maps[i] = (velx, vely)

    def lerp(self, t, prev, curr):

        w,h = cv.GetSize(prev)

        x = cv.CreateMat(h, w, cv.CV_32FC1)
        y = cv.CreateMat(h, w, cv.CV_32FC1)
        d = cv.CloneImage(prev)
        d0 = cv.CloneImage(prev)
        d1 = cv.CloneImage(prev)

        # d0 is curr mapped backwards in time, so 1.0 means exactly curr
        velx,vely = self.maps[0]
        cv.ConvertScale(velx, x, 1.0 - t)
        cv.ConvertScale(vely, y, 1.0 - t)
        cv.Add(x, self.offx, x)
        cv.Add(y, self.offy, y)
        cv.Remap(curr, d0, x, y)

        # d1 is prev mapped forwards in time, so 0.0 means exactly prev
        velx,vely = self.maps[1]
        cv.ConvertScale(velx, x, t)
        cv.ConvertScale(vely, y, t)
        cv.Add(x, self.offx, x)
        cv.Add(y, self.offy, y)
        cv.Remap(prev, d1, x, y)

        cv.AddWeighted(d0, t, d1, 1.0 - t, 0.0, d)
        return d

class TestDirected(unittest.TestCase):

    depths = [ cv.IPL_DEPTH_8U, cv.IPL_DEPTH_8S, cv.IPL_DEPTH_16U, cv.IPL_DEPTH_16S, cv.IPL_DEPTH_32S, cv.IPL_DEPTH_32F, cv.IPL_DEPTH_64F ]

    mat_types = [
        cv.CV_8UC1,
        cv.CV_8UC2,
        cv.CV_8UC3,
        cv.CV_8UC4,
        cv.CV_8SC1,
        cv.CV_8SC2,
        cv.CV_8SC3,
        cv.CV_8SC4,
        cv.CV_16UC1,
        cv.CV_16UC2,
        cv.CV_16UC3,
        cv.CV_16UC4,
        cv.CV_16SC1,
        cv.CV_16SC2,
        cv.CV_16SC3,
        cv.CV_16SC4,
        cv.CV_32SC1,
        cv.CV_32SC2,
        cv.CV_32SC3,
        cv.CV_32SC4,
        cv.CV_32FC1,
        cv.CV_32FC2,
        cv.CV_32FC3,
        cv.CV_32FC4,
        cv.CV_64FC1,
        cv.CV_64FC2,
        cv.CV_64FC3,
        cv.CV_64FC4,
    ]

    def depthsize(self, d):
        return { cv.IPL_DEPTH_8U : 1,
                         cv.IPL_DEPTH_8S : 1,
                         cv.IPL_DEPTH_16U : 2,
                         cv.IPL_DEPTH_16S : 2,
                         cv.IPL_DEPTH_32S : 4,
                         cv.IPL_DEPTH_32F : 4,
                         cv.IPL_DEPTH_64F : 8 }[d]

    def expect_exception(self, func, exception):
        tripped = False
        try:
            func()
        except exception:
            tripped = True
        self.assert_(tripped)

    def test_LoadImage(self):
        self.expect_exception(lambda: cv.LoadImage(), TypeError)
        self.expect_exception(lambda: cv.LoadImage(4), TypeError)
        self.expect_exception(lambda: cv.LoadImage('foo.jpg', 1, 1), TypeError)
        self.expect_exception(lambda: cv.LoadImage('foo.jpg', xiscolor=cv.CV_LOAD_IMAGE_COLOR), TypeError)

    def test_CreateMat(self):
        for rows in [2, 4, 16, 64, 512, 640]: # XXX - 1 causes bug in OpenCV
            for cols in [1, 2, 4, 16, 64, 512, 640]:
                for t in self.mat_types:
                    m = cv.CreateMat(rows, cols, t)

    def test_CreateImage(self):
        for w in [ 1, 4, 64, 512, 640]:
            for h in [ 1, 4, 64, 480, 512]:
                for c in [1, 2, 3, 4]:
                    for d in self.depths:
                        a = cv.CreateImage((w,h), d, c);
                        self.assert_(a.width == w)
                        self.assert_(a.height == h)
                        self.assert_(a.nChannels == c)
                        self.assert_(a.depth == d)
                        self.assert_(cv.GetSize(a) == (w, h))
                        # self.assert_(cv.GetElemType(a) == d)

    def test_types(self):
        self.assert_(type(cv.CreateImage((7,5), cv.IPL_DEPTH_8U, 1)) == cv.iplimage)
        self.assert_(type(cv.CreateMat(5, 7, cv.CV_32FC1)) == cv.cvmat)

    def test_GetSize(self):
        self.assert_(cv.GetSize(cv.CreateMat(5, 7, cv.CV_32FC1)) == (7,5))
        self.assert_(cv.GetSize(cv.CreateImage((7,5), cv.IPL_DEPTH_8U, 1)) == (7,5))

    def test_GetAffineTransform(self):
        mapping = cv.CreateMat(2, 3, cv.CV_32FC1)
        cv.GetAffineTransform([ (0,0), (1,0), (0,1) ], [ (0,0), (17,0), (0,17) ], mapping)
        self.assertAlmostEqual(mapping[0,0], 17, 2)
        self.assertAlmostEqual(mapping[1,1], 17, 2)

    def test_MinMaxLoc(self):
        scribble = cv.CreateImage((640,480), cv.IPL_DEPTH_8U, 1)
        los = [ (random.randrange(480), random.randrange(640)) for i in range(100) ]
        his = [ (random.randrange(480), random.randrange(640)) for i in range(100) ]
        for (lo,hi) in zip(los,his):
            cv.Set(scribble, 128)
            scribble[lo] = 0
            scribble[hi] = 255
            r = cv.MinMaxLoc(scribble)
            self.assert_(r == (0, 255, tuple(reversed(lo)), tuple(reversed(hi))))

    def failing_test_exception(self):
        a = cv.CreateImage((640,480), cv.IPL_DEPTH_8U, 1)
        b = cv.CreateImage((640,480), cv.IPL_DEPTH_8U, 1)
        self.expect_exception(lambda: cv.Laplace(a, b), cv.error)

    def test_tostring(self):
        for w in [ 1, 4, 64, 512, 640]:
            for h in [ 1, 4, 64, 480, 512]:
                for c in [1, 2, 3, 4]:
                    for d in self.depths:
                        a = cv.CreateImage((w,h), d, c);
                        self.assert_(len(a.tostring()) == w * h * c * self.depthsize(d))

    def test_cvmat_accessors(self):
        cvm = cv.CreateMat(20, 10, cv.CV_32FC1)

    def test_depths(self):
        """ Make sure that the depth enums are unique """
        self.assert_(len(self.depths) == len(set(self.depths)))

    def test_leak(self):
        """ If CreateImage is not releasing image storage, then the loop below should use ~4GB of memory. """
        for i in range(4000):
            a = cv.CreateImage((1024,1024), cv.IPL_DEPTH_8U, 1)

    def test_avg(self):
        m = cv.CreateMat(1, 8, cv.CV_32FC1)
        for i,v in enumerate([2, 4, 4, 4, 5, 5, 7, 9]):
            m[0,i] = (v,)
        self.assertAlmostEqual(cv.Avg(m)[0], 5.0, 3)
        avg,sdv = cv.AvgSdv(m)
        self.assertAlmostEqual(avg[0], 5.0, 3)
        self.assertAlmostEqual(sdv[0], 2.0, 3)

    def test_histograms(self):
        def split(im):
            nchans = cv.CV_MAT_CN(cv.GetElemType(im)) 
            c = [ cv.CreateImage(cv.GetSize(im), cv.IPL_DEPTH_8U, 1) for i in range(nchans) ] + [None] * (4 - nchans)
            cv.Split(im, c[0], c[1], c[2], c[3])
            return c[:nchans]
        def imh(im):
            s = split(im)
            hist = cv.CreateHist([256] * len(s), cv.CV_HIST_ARRAY, [ (0,255) ] * len(s), 1)
            cv.CalcHist(s, hist, 0)
            return hist

        src = cv.LoadImage(find_sample("lena.jpg"), 0)
        h = imh(src)
        (minv, maxv, minl, maxl) = cv.GetMinMaxHistValue(h)
        self.assert_(cv.QueryHistValue_nD(h, minl) == minv)
        self.assert_(cv.QueryHistValue_nD(h, maxl) == maxv)
        bp = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_8U, 1)
        cv.CalcBackProject(split(src), bp, h)
        bp = cv.CreateImage((cv.GetSize(src)[0]-2, cv.GetSize(src)[1]-2), cv.IPL_DEPTH_32F, 1)
        cv.CalcBackProjectPatch(split(src), bp, (3,3), h, cv.CV_COMP_INTERSECT, 1)

    def test_remap(self):

        raw = cv.CreateImage((640, 480), cv.IPL_DEPTH_8U, 1)
        for x in range(0, 640, 20):
            cv.Line(raw, (x,0), (x,480), 255, 1)
        for y in range(0, 480, 20):
            cv.Line(raw, (0,y), (640,y), 255, 1)
        intrinsic_mat = cv.CreateMat(3, 3, cv.CV_32FC1);
        distortion_coeffs = cv.CreateMat(1, 4, cv.CV_32FC1);

        cv.SetZero(intrinsic_mat)
        intrinsic_mat[0,2] = 320.0
        intrinsic_mat[1,2] = 240.0
        intrinsic_mat[0,0] = 320.0
        intrinsic_mat[1,1] = 320.0
        intrinsic_mat[2,2] = 1.0
        cv.SetZero(distortion_coeffs)
        distortion_coeffs[0,0] = 1e-1
        mapx = cv.CreateImage((640, 480), cv.IPL_DEPTH_32F, 1)
        mapy = cv.CreateImage((640, 480), cv.IPL_DEPTH_32F, 1)
        cv.SetZero(mapx)
        cv.SetZero(mapy)
        cv.InitUndistortMap(intrinsic_mat, distortion_coeffs, mapx, mapy)
        rect = cv.CreateImage((640, 480), cv.IPL_DEPTH_8U, 1)

        (w,h) = (640,480)
        rMapxy = cv.CreateMat(h, w, cv.CV_16SC2)
        rMapa  = cv.CreateMat(h, w, cv.CV_16UC1)
        cv.ConvertMaps(mapx,mapy,rMapxy,rMapa);

        cv.Remap(raw, rect, mapx, mapy)
        cv.Remap(raw, rect, rMapxy, rMapa)
        cv.Undistort2(raw, rect, intrinsic_mat, distortion_coeffs)

        for w in [1, 4, 4095, 4096, 4097, 4100]:
            p = cv.CreateImage((w,256), 8, 1)
            cv.Undistort2(p, p, intrinsic_mat, distortion_coeffs);
        #print p

        fptypes = [cv.CV_32FC1, cv.CV_64FC1]
        for t0 in fptypes:
            for t1 in fptypes:
                for t2 in fptypes:
                    for t3 in fptypes:
                        rotation_vector = cv.CreateMat(1, 3, t0)
                        translation_vector = cv.CreateMat(1, 3, t1)
                        object_points = cv.CreateMat(7, 3, t2)
                        image_points = cv.CreateMat(7, 2, t3)
                        cv.ProjectPoints2(object_points, rotation_vector, translation_vector, intrinsic_mat, distortion_coeffs, image_points)

        return

        started = time.time()
        for i in range(10):
            if 1:
                cv.Remap(raw, rect, mapx, mapy)
            else:
                cv.Remap(raw,rect,rMapxy,rMapa)
        print "took", time.time() - started

        print
        print "mapx", mapx[0,0]
        print "mapy", mapx[0,0]
        self.snap(rect)

    def test_arithmetic(self):
        a = cv.CreateMat(4, 4, cv.CV_8UC1)
        a[0,0] = 50.0
        b = cv.CreateMat(4, 4, cv.CV_8UC1)
        b[0,0] = 4.0
        d = cv.CreateMat(4, 4, cv.CV_8UC1)
        cv.Add(a, b, d)
        self.assertEqual(d[0,0], 54.0)
        cv.Mul(a, b, d)
        self.assertEqual(d[0,0], 200.0)
        
    def test_inrange(self):

        sz = (256,256)
        Igray1 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)
        Ilow1 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)
        Ihi1 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)
        Igray2 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)
        Ilow2 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)
        Ihi2 = cv.CreateImage(sz,cv.IPL_DEPTH_32F,1)

        Imask = cv.CreateImage(sz, cv.IPL_DEPTH_8U,1)
        Imaskt = cv.CreateImage(sz,cv.IPL_DEPTH_8U,1)

        cv.InRange(Igray1, Ilow1, Ihi1, Imask);
        cv.InRange(Igray2, Ilow2, Ihi2, Imaskt);

        cv.Or(Imask, Imaskt, Imask);

    def failing_test_cvtcolor(self):
        src3 = cv.LoadImage(find_sample("lena.jpg"))
        src1 = cv.LoadImage(find_sample("lena.jpg"), 0)
        dst8u = dict([(c,cv.CreateImage(cv.GetSize(src1), cv.IPL_DEPTH_8U, c)) for c in (1,2,3,4)])
        dst16u = dict([(c,cv.CreateImage(cv.GetSize(src1), cv.IPL_DEPTH_16U, c)) for c in (1,2,3,4)])
        dst32f = dict([(c,cv.CreateImage(cv.GetSize(src1), cv.IPL_DEPTH_32F, c)) for c in (1,2,3,4)])

        for srcf in ["BGR", "RGB"]:
            for dstf in ["Luv"]:
                cv.CvtColor(src3, dst8u[3], eval("cv.CV_%s2%s" % (srcf, dstf)))
                cv.CvtColor(src3, dst32f[3], eval("cv.CV_%s2%s" % (srcf, dstf)))
                cv.CvtColor(src3, dst8u[3], eval("cv.CV_%s2%s" % (dstf, srcf)))
            
        for srcf in ["BayerBG", "BayerGB", "BayerGR"]:
            for dstf in ["RGB", "BGR"]:
                cv.CvtColor(src1, dst8u[3], eval("cv.CV_%s2%s" % (srcf, dstf)))

    def test_voronoi(self):
        w,h = 500,500

        storage = cv.CreateMemStorage(0)

        def facet_edges(e0):
            e = e0
            while True:
                e = cv.Subdiv2DGetEdge(e, cv.CV_NEXT_AROUND_LEFT)
                yield e
                if e == e0:
                    break

        def areas(edges):
            seen = []
            seensorted = []
            for edge in edges:
                pts = [ cv.Subdiv2DEdgeOrg(e) for e in facet_edges(edge) ]
                if not (None in pts):
                    l = [p.pt for p in pts]
                    ls = sorted(l)
                    if not(ls in seensorted):
                        seen.append(l)
                        seensorted.append(ls)
            return seen

        for npoints in range(1, 200):
            points = [ (random.randrange(w), random.randrange(h)) for i in range(npoints) ]
            subdiv = cv.CreateSubdivDelaunay2D( (0,0,w,h), storage )
            for p in points:
                cv.SubdivDelaunay2DInsert( subdiv, p)
            cv.CalcSubdivVoronoi2D(subdiv)
            ars = areas([ cv.Subdiv2DRotateEdge(e, 1) for e in subdiv.edges ] + [ cv.Subdiv2DRotateEdge(e, 3) for e in subdiv.edges ])
            self.assert_(len(ars) == len(set(points)))

            if False:
                img = cv.CreateImage((w,h), cv.IPL_DEPTH_8U, 3)
                cv.SetZero(img)
                def T(x): return int(x) # int(300+x/16)
                for pts in ars:
                    cv.FillConvexPoly( img, [(T(x),T(y)) for (x,y) in pts], cv.RGB(100+random.randrange(156),random.randrange(256),random.randrange(256)), cv.CV_AA, 0 );
                for x,y in points:
                    cv.Circle(img, (T(x), T(y)), 3, cv.RGB(0,0,0), -1)

                cv.ShowImage("snap", img)
                if cv.WaitKey(10) > 0:
                    break

    def test_lineclip(self):
        w,h = 640,480
        img = cv.CreateImage((w,h), cv.IPL_DEPTH_8U, 1)
        cv.SetZero(img)
        tricky = [ -8000, -2, -1, 0, 1, h/2, h-1, h, h+1, w/2, w-1, w, w+1, 8000]
        for x0 in tricky:
            for y0 in tricky:
                for x1 in tricky:
                    for y1 in tricky:
                        for thickness in [ 0, 1, 8 ]:
                            for line_type in [0, 4, 8, cv.CV_AA ]:
                                cv.Line(img, (x0,y0), (x1,y1), 255, thickness, line_type)
        # just check that something was drawn
        self.assert_(cv.Sum(img)[0] > 0)

    def test_inpaint(self):
        src = cv.LoadImage(find_sample("building.jpg"))
        msk = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_8U, 1)
        damaged = cv.CloneImage(src)
        repaired = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_8U, 3)
        difference = cv.CloneImage(repaired)
        cv.SetZero(msk)
        for method in [ cv.CV_INPAINT_NS, cv.CV_INPAINT_TELEA ]:
            for (p0,p1) in [ ((10,10), (400,400)) ]:
                cv.Line(damaged, p0, p1, cv.RGB(255, 0, 255), 2)
                cv.Line(msk, p0, p1, 255, 2)
            cv.Inpaint(damaged, msk, repaired, 10., cv.CV_INPAINT_NS)
        cv.AbsDiff(src, repaired, difference)
        #self.snapL([src, damaged, repaired, difference])

    def test_GetSubRect(self):
        src = cv.CreateImage((100,100), 8, 1)
        data = "z" * (100 * 100)

        cv.SetData(src, data, 100)
        start_count = sys.getrefcount(data)

        iter = 77
        subs = []
        for i in range(iter):
            sub = cv.GetSubRect(src, (0, 0, 10, 10))
            subs.append(sub)
        self.assert_(sys.getrefcount(data) == (start_count + iter))

        src = cv.LoadImage(find_sample("lena.jpg"), 0)
        made = cv.CreateImage(cv.GetSize(src), 8, 1)
        sub = cv.CreateMat(32, 32, cv.CV_8UC1)
        for x in range(0, 512, 32):
            for y in range(0, 512, 32):
                sub = cv.GetSubRect(src, (x, y, 32, 32))
                cv.SetImageROI(made, (x, y, 32, 32))
                cv.Copy(sub, made)
        cv.ResetImageROI(made)
        cv.AbsDiff(made, src, made)
        self.assert_(cv.CountNonZero(made) == 0)

    def perf_test_pow(self):
        mt = cv.CreateMat(1000, 1000, cv.CV_32FC1)
        dst = cv.CreateMat(1000, 1000, cv.CV_32FC1)
        rng = cv.RNG(0)
        cv.RandArr(rng, mt, cv.CV_RAND_UNI, 0, 1000.0)
        mt[0,0] = 10
        print
        for a in [0.5, 2.0, 2.3, 2.4, 3.0, 37.1786] + [2.4]*10:
            started = time.time()
            for i in range(10):
                cv.Pow(mt, dst, a)
            took = (time.time() - started) / 1e7
            print "%4.1f took %f ns" % (a, took * 1e9)
        print dst[0,0], 10 ** 2.4

    def test_GetRowCol(self):
        src = cv.CreateImage((8,3), 8, 1)
        # Put these words
        #     Achilles
        #     Benedict
        #     Congreve
        # in an array (3 rows, 8 columns).
        # Then extract the array in various ways.

        for r,w in enumerate(("Achilles", "Benedict", "Congreve")):
            for c,v in enumerate(w):
                src[r,c] = ord(v)
        self.assertEqual(src.tostring(), "AchillesBenedictCongreve")
        self.assertEqual(src[:,:].tostring(), "AchillesBenedictCongreve")
        self.assertEqual(src[:,:4].tostring(), "AchiBeneCong")
        self.assertEqual(src[:,0].tostring(), "ABC")
        self.assertEqual(src[:,4:].tostring(), "llesdictreve")
        self.assertEqual(src[::2,:].tostring(), "AchillesCongreve")
        self.assertEqual(src[1:,:].tostring(), "BenedictCongreve")
        self.assertEqual(src[1:2,:].tostring(), "Benedict")
        self.assertEqual(src[::2,:4].tostring(), "AchiCong")
        # The mats share the same storage, so updating one should update them all
        lastword = src[2]
        self.assertEqual(lastword.tostring(), "Congreve")
        src[2,0] = ord('K')
        self.assertEqual(lastword.tostring(), "Kongreve")

        # ABCD
        # EFGH
        # IJKL
        #
        # MNOP
        # QRST
        # UVWX

        mt = cv.CreateMatND([2,3,4], cv.CV_8UC1)
        for i in range(2):
            for j in range(3):
                for k in range(4):
                    mt[i,j,k] = ord('A') + k + 4 * (j + 3 * i)
        self.assertEqual(mt[:,:,:1].tostring(), "AEIMQU")
        self.assertEqual(mt[:,:1,:].tostring(), "ABCDMNOP")
        self.assertEqual(mt[:1,:,:].tostring(), "ABCDEFGHIJKL")
        self.assertEqual(mt[1,1].tostring(), "QRST")
        self.assertEqual(mt[:,::2,:].tostring(), "ABCDIJKLMNOPUVWX")

    def test_addS_3D(self):
        for dim in [ [1,1,4], [2,2,3], [7,4,3] ]:
            for ty,ac in [ (cv.CV_32FC1, 'f'), (cv.CV_64FC1, 'd')]:
                mat = cv.CreateMatND(dim, ty)
                mat2 = cv.CreateMatND(dim, ty)
                for increment in [ 0, 3, -1 ]:
                    cv.SetData(mat, array.array(ac, range(dim[0] * dim[1] * dim[2])), 0)
                    cv.AddS(mat, increment, mat2)
                    for i in range(dim[0]):
                        for j in range(dim[1]):
                            for k in range(dim[2]):
                                self.assert_(mat2[i,j,k] == mat[i,j,k] + increment)

    def test_Buffers(self):
        ar = array.array('f', [7] * (360*640))

        m = cv.CreateMat(360, 640, cv.CV_32FC1)
        cv.SetData(m, ar, 4 * 640)
        self.assert_(m[0,0] == 7.0)

        m = cv.CreateMatND((360, 640), cv.CV_32FC1)
        cv.SetData(m, ar, 4 * 640)
        self.assert_(m[0,0] == 7.0)

        m = cv.CreateImage((640, 360), cv.IPL_DEPTH_32F, 1)
        cv.SetData(m, ar, 4 * 640)
        self.assert_(m[0,0] == 7.0)

    def xxtest_Filters(self):
        print
        m = cv.CreateMat(360, 640, cv.CV_32FC1)
        d = cv.CreateMat(360, 640, cv.CV_32FC1)
        for k in range(3, 21, 2):
            started = time.time()
            for i in range(1000):
                cv.Smooth(m, m, param1=k)
            print k, "took", time.time() - started

    def assertSame(self, a, b):
        w,h = cv.GetSize(a)
        d = cv.CreateMat(h, w, cv.CV_8UC1)
        cv.AbsDiff(a, b, d)
        self.assert_(cv.CountNonZero(d) == 0)

    def test_GetStarKeypoints(self):
        src = cv.LoadImage(find_sample("lena.jpg"), 0)
        storage = cv.CreateMemStorage()
        kp = cv.GetStarKeypoints(src, storage)
        self.assert_(len(kp) > 0)
        for (x,y),scale,r in kp:
            self.assert_(0 <= x)
            self.assert_(x <= cv.GetSize(src)[0])
            self.assert_(0 <= y)
            self.assert_(y <= cv.GetSize(src)[1])
        return
        scribble = cv.CreateImage(cv.GetSize(src), 8, 3)
        cv.CvtColor(src, scribble, cv.CV_GRAY2BGR)
        for (x,y),scale,r in kp:
            print x,y,scale,r
            cv.Circle(scribble, (x,y), scale, cv.RGB(255,0,0))
        self.snap(scribble)

    def test_Threshold(self):
        """ directed test for bug 2790622 """
        src = cv.LoadImage(find_sample("lena.jpg"), 0)
        results = set()
        for i in range(10):
            dst = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_8U, 1)
            cv.Threshold(src, dst, 128, 128, cv.CV_THRESH_BINARY)
            results.add(dst.tostring())
        # Should have produced the same answer every time, so results set should have size 1
        self.assert_(len(results) == 1)

    def failing_test_Circle(self):
        """ smoke test to draw circles, many clipped """
        for w,h in [(2,77), (77,2), (256, 256), (640,480)]:
            img = cv.CreateImage((w,h), cv.IPL_DEPTH_8U, 1)
            cv.SetZero(img)
            tricky = [ -8000, -2, -1, 0, 1, h/2, h-1, h, h+1, w/2, w-1, w, w+1, 8000]
            for x0 in tricky:
                for y0 in tricky:
                    for r in [ 0, 1, 2, 3, 4, 5, w/2, w-1, w, w+1, h/2, h-1, h, h+1, 8000 ]:
                        for thick in [1, 2, 10]:
                            for t in [0, 8, 4, cv.CV_AA]:
                                cv.Circle(img, (x0,y0), r, 255, thick, t)
        # just check that something was drawn
        self.assert_(cv.Sum(img)[0] > 0)

    def test_text(self):
        img = cv.CreateImage((640,40), cv.IPL_DEPTH_8U, 1)
        cv.SetZero(img)
        font = cv.InitFont(cv.CV_FONT_HERSHEY_SIMPLEX, 1, 1)
        message = "XgfooX"
        cv.PutText(img, message, (320,30), font, 255)
        ((w,h),bl) = cv.GetTextSize(message, font)

        # Find nonzero in X and Y
        Xs = []
        for x in range(640):
            cv.SetImageROI(img, (x, 0, 1, 40))
            Xs.append(cv.Sum(img)[0] > 0)
        def firstlast(l):
            return (l.index(True), len(l) - list(reversed(l)).index(True))

        Ys = []
        for y in range(40):
            cv.SetImageROI(img, (0, y, 640, 1))
            Ys.append(cv.Sum(img)[0] > 0)

        x0,x1 = firstlast(Xs)
        y0,y1 = firstlast(Ys)
        actual_width = x1 - x0
        actual_height = y1 - y0

        # actual_width can be up to 8 pixels smaller than GetTextSize says
        self.assert_(actual_width <= w)
        self.assert_((w - actual_width) <= 8)

        # actual_height can be up to 4 pixels smaller than GetTextSize says
        self.assert_(actual_height <= (h + bl))
        self.assert_(((h + bl) - actual_height) <= 4)

        cv.ResetImageROI(img)
        self.assert_(w != 0)
        self.assert_(h != 0)

    def test_sizes(self):
        sizes = [ 1, 2, 3, 97, 255, 256, 257, 947 ]
        for w in sizes:
            for h in sizes:
                # Create an IplImage 
                im = cv.CreateImage((w,h), cv.IPL_DEPTH_8U, 1)
                cv.Set(im, 1)
                self.assert_(cv.Sum(im)[0] == (w * h))
                del im
                # Create a CvMat
                mt = cv.CreateMat(h, w, cv.CV_8UC1)
                cv.Set(mt, 1)
                self.assert_(cv.Sum(mt)[0] == (w * h))

        random.seed(7)
        for dim in range(1, cv.CV_MAX_DIM + 1):
            for attempt in range(10):
                dims = [ random.choice([1,1,1,1,2,3]) for i in range(dim) ]
                mt = cv.CreateMatND(dims, cv.CV_8UC1)
                cv.SetZero(mt)
                self.assert_(cv.Sum(mt)[0] == 0)
                # Set to all-ones, verify the sum
                cv.Set(mt, 1)
                expected = 1
                for d in dims:
                    expected *= d
                self.assert_(cv.Sum(mt)[0] == expected)

    def test_random(self):
        seeds = [ 0, 1, 2**48, 2**48 + 1 ]
        sequences = set()
        for s in seeds:
            rng = cv.RNG(s)
            sequences.add(str([cv.RandInt(rng) for i in range(10)]))
        self.assert_(len(seeds) == len(sequences))
            
        rng = cv.RNG(0)
        im = cv.CreateImage((1024,1024), cv.IPL_DEPTH_8U, 1)
        cv.RandArr(rng, im, cv.CV_RAND_UNI, 0, 256)
        cv.RandArr(rng, im, cv.CV_RAND_NORMAL, 128, 30)
        if 1:
            hist = cv.CreateHist([ 256 ], cv.CV_HIST_ARRAY, [ (0,255) ], 1)
            cv.CalcHist([im], hist)

        rng = cv.RNG()
        for i in range(1000):
            v = cv.RandReal(rng)
            self.assert_(0 <= v)
            self.assert_(v < 1)

        for mode in [ cv.CV_RAND_UNI, cv.CV_RAND_NORMAL ]:
            for fmt in self.mat_types:
                mat = cv.CreateMat(64, 64, fmt)
                cv.RandArr(cv.RNG(), mat, mode, (0,0,0,0), (1,1,1,1))

    def failing_test_mixchannels(self):
        rgba = cv.CreateMat(100, 100, cv.CV_8UC4)
        bgr = cv.CreateMat(100, 100, cv.CV_8UC3)
        alpha = cv.CreateMat(100, 100, cv.CV_8UC1)
        cv.Set(rgba, (1,2,3,4))
        cv.MixChannels([rgba,rgba,rgba,rgba], [bgr, bgr, bgr, alpha], [
           (0, 2),    # rgba[0] -> bgr[2]
           (1, 1),    # rgba[1] -> bgr[1]
           (2, 0),    # rgba[2] -> bgr[0]
           (3, 0)     # rgba[3] -> alpha[0]
        ])
        self.assert_(bgr[0,0] == (3,2,1))
        self.assert_(alpha[0,0] == 4)

        cv.MixChannels([rgba,rgba,rgba,None], [bgr, bgr, bgr, alpha], [
           (0, 0),    # rgba[0] -> bgr[0]
           (1, 1),    # rgba[1] -> bgr[1]
           (2, 2),    # rgba[2] -> bgr[2]
           (77, 0)    # 0 -> alpha[0]
        ])
        self.assert_(bgr[0,0] == (1,2,3))
        self.assert_(alpha[0,0] == 0)

    def test_access(self):
        cnames = { 1:cv.CV_32FC1, 2:cv.CV_32FC2, 3:cv.CV_32FC3, 4:cv.CV_32FC4 }

        for w in range(1,11):
            for h in range(2,11):
                for c in [1,2]:
                    for o in [ cv.CreateMat(h, w, cnames[c]), cv.CreateImage((w,h), cv.IPL_DEPTH_32F, c) ][1:]:
                        pattern = [ (i,j) for i in range(w) for j in range(h) ]
                        random.shuffle(pattern)
                        for k,(i,j) in enumerate(pattern):
                            if c == 1:
                                o[j,i] = k
                            else:
                                o[j,i] = (k,) * c
                        for k,(i,j) in enumerate(pattern):
                            if c == 1:
                                self.assert_(o[j,i] == k)
                            else:
                                self.assert_(o[j,i] == (k,)*c)

        test_mat = cv.CreateMat(2, 3, cv.CV_32FC1)
        cv.SetData(test_mat, array.array('f', range(6)), 12)
        self.assertEqual(cv.GetDims(test_mat[0]), (1, 3))
        self.assertEqual(cv.GetDims(test_mat[1]), (1, 3))
        self.assertEqual(cv.GetDims(test_mat[0:1]), (1, 3))
        self.assertEqual(cv.GetDims(test_mat[1:2]), (1, 3))
        self.assertEqual(cv.GetDims(test_mat[-1:]), (1, 3))
        self.assertEqual(cv.GetDims(test_mat[-1]), (1, 3))

    def test_InitLineIterator(self):
        scribble = cv.CreateImage((640,480), cv.IPL_DEPTH_8U, 1)
        self.assert_(len(list(cv.InitLineIterator(scribble, (20,10), (30,10)))) == 11)

    def test_CalcEMD2(self):
        cc = {}
        for r in [ 5, 10, 37, 38 ]:
            scratch = cv.CreateImage((100,100), 8, 1)
            cv.SetZero(scratch)
            cv.Circle(scratch, (50,50), r, 255, -1)
            storage = cv.CreateMemStorage()
            seq = cv.FindContours(scratch, storage, cv.CV_RETR_TREE, cv.CV_CHAIN_APPROX_SIMPLE)
            arr = cv.CreateMat(len(seq), 3, cv.CV_32FC1)
            for i,e in enumerate(seq):
                arr[i,0] = 1
                arr[i,1] = e[0]
                arr[i,2] = e[1]
            cc[r] = arr
        def myL1(A, B, D):
            return abs(A[0]-B[0]) + abs(A[1]-B[1])
        def myL2(A, B, D):
            return math.sqrt((A[0]-B[0])**2 + (A[1]-B[1])**2)
        def myC(A, B, D):
            return max(abs(A[0]-B[0]), abs(A[1]-B[1]))
        contours = set(cc.values())
        for c0 in contours:
            for c1 in contours:
                self.assert_(abs(cv.CalcEMD2(c0, c1, cv.CV_DIST_L1) - cv.CalcEMD2(c0, c1, cv.CV_DIST_USER, myL1)) < 1e-3)
                self.assert_(abs(cv.CalcEMD2(c0, c1, cv.CV_DIST_L2) - cv.CalcEMD2(c0, c1, cv.CV_DIST_USER, myL2)) < 1e-3)
                self.assert_(abs(cv.CalcEMD2(c0, c1, cv.CV_DIST_C) - cv.CalcEMD2(c0, c1, cv.CV_DIST_USER, myC)) < 1e-3)

    def test_FindContours(self):
        random.seed(0)

        storage = cv.CreateMemStorage()
        for trial in range(10):
            scratch = cv.CreateImage((800,800), 8, 1)
            cv.SetZero(scratch)
            def plot(center, radius, mode):
                cv.Circle(scratch, center, radius, mode, -1)
                if radius < 20:
                    return 0
                else:
                    newmode = 255 - mode
                    subs = random.choice([1,2,3])
                    if subs == 1:
                        return [ plot(center, radius - 5, newmode) ]
                    else:
                        newradius = int({ 2: radius / 2, 3: radius / 2.3 }[subs] - 5)
                        r = radius / 2
                        ret = []
                        for i in range(subs):
                            th = i * (2 * math.pi) / subs
                            ret.append(plot((int(center[0] + r * math.cos(th)), int(center[1] + r * math.sin(th))), newradius, newmode))
                        return sorted(ret)

            actual = plot((400,400), 390, 255 )

            seq = cv.FindContours(scratch, storage, cv.CV_RETR_TREE, cv.CV_CHAIN_APPROX_SIMPLE)

            def traverse(s):
                if s == None:
                    return 0
                else:
                    self.assert_(abs(cv.ContourArea(s)) > 0.0)
                    ((x,y),(w,h),th) = cv.MinAreaRect2(s, cv.CreateMemStorage())
                    self.assert_(((w / h) - 1.0) < 0.01)
                    self.assert_(abs(cv.ContourArea(s)) > 0.0)
                    r = []
                    while s:
                        r.append(traverse(s.v_next()))
                        s = s.h_next()
                    return sorted(r)
            self.assert_(traverse(seq.v_next()) == actual)

    def test_ConvexHull2(self):
        # Draw a series of N-pointed stars, find contours, assert the contour is not convex,
        # assert the hull has N segments, assert that there are N convexity defects.

        def polar2xy(th, r):
            return (int(400 + r * math.cos(th)), int(400 + r * math.sin(th)))
        storage = cv.CreateMemStorage(0)
        for way in ['CvSeq', 'CvMat', 'list']:
            for points in range(3,20):
                scratch = cv.CreateImage((800,800), 8, 1)
                sides = 2 * points
                cv.FillPoly(scratch, [ [ polar2xy(i * 2 * math.pi / sides, [100,350][i&1]) for i in range(sides) ] ], 255)

                seq = cv.FindContours(scratch, storage, cv.CV_RETR_TREE, cv.CV_CHAIN_APPROX_SIMPLE)

                if way == 'CvSeq':
                    # pts is a CvSeq
                    pts = seq
                elif way == 'CvMat':
                    # pts is a CvMat
                    arr = cv.CreateMat(len(seq), 1, cv.CV_32SC2)
                    for i,e in enumerate(seq):
                        arr[i,0] = e
                    pts = arr
                elif way == 'list':
                    # pts is a list of 2-tuples
                    pts = list(seq)
                else:
                    assert False

                self.assert_(cv.CheckContourConvexity(pts) == 0)
                hull = cv.ConvexHull2(pts, storage, return_points = 1)
                self.assert_(cv.CheckContourConvexity(hull) == 1)
                self.assert_(len(hull) == points)

                if way in [ 'CvSeq', 'CvMat' ]:
                    defects = cv.ConvexityDefects(pts, cv.ConvexHull2(pts, storage), storage)
                    self.assert_(len([depth for (_,_,_,depth) in defects if (depth > 5)]) == points)

    def xxxtest_corners(self):
        a = cv.LoadImage("foo-mono.png", 0)
        cv.AdaptiveThreshold(a, a, 255, param1=5)
        scribble = cv.CreateImage(cv.GetSize(a), 8, 3)
        cv.CvtColor(a, scribble, cv.CV_GRAY2BGR)
        if 0:
            eig_image = cv.CreateImage(cv.GetSize(a), cv.IPL_DEPTH_32F, 1)
            temp_image = cv.CreateImage(cv.GetSize(a), cv.IPL_DEPTH_32F, 1)
            pts = cv.GoodFeaturesToTrack(a, eig_image, temp_image, 100, 0.04, 2, use_harris=1)
            for p in pts:
                cv.Circle( scribble, p, 1, cv.RGB(255,0,0), -1 )
            self.snap(scribble)
        canny = cv.CreateImage(cv.GetSize(a), 8, 1)
        cv.SubRS(a, 255, canny)
        self.snap(canny)
        li = cv.HoughLines2(canny,
                                                cv.CreateMemStorage(),
                                                cv.CV_HOUGH_STANDARD,
                                                1,
                                                math.pi/180,
                                                60,
                                                0,
                                                0)
        for (rho,theta) in li:
            print rho,theta
            c = math.cos(theta)
            s = math.sin(theta)
            x0 = c*rho
            y0 = s*rho
            cv.Line(scribble,
                            (x0 + 1000*(-s), y0 + 1000*c),
                            (x0 + -1000*(-s), y0 - 1000*c),
                            (0,255,0))
        self.snap(scribble)

    def test_CalcOpticalFlowBM(self):
        a = cv.LoadImage(find_sample("lena.jpg"), 0)
        b = cv.LoadImage(find_sample("lena.jpg"), 0)
        (w,h) = cv.GetSize(a)
        vel_size = (w - 8, h - 8)
        velx = cv.CreateImage(vel_size, cv.IPL_DEPTH_32F, 1)
        vely = cv.CreateImage(vel_size, cv.IPL_DEPTH_32F, 1)
        cv.CalcOpticalFlowBM(a, b, (8,8), (1,1), (8,8), 0, velx, vely)

    def test_tostring(self):
        for w in [ 32, 96, 480 ]:
            for h in [ 32, 96, 480 ]:
                depth_size = {
                    cv.IPL_DEPTH_8U : 1,
                    cv.IPL_DEPTH_8S : 1,
                    cv.IPL_DEPTH_16U : 2,
                    cv.IPL_DEPTH_16S : 2,
                    cv.IPL_DEPTH_32S : 4,
                    cv.IPL_DEPTH_32F : 4,
                    cv.IPL_DEPTH_64F : 8
                }
                for f in  self.depths:
                    for channels in (1,2,3,4):
                        img = cv.CreateImage((w, h), f, channels)
                        esize = (w * h * channels * depth_size[f])
                        self.assert_(len(img.tostring()) == esize)
                        cv.SetData(img, " " * esize, w * channels * depth_size[f])
                        self.assert_(len(img.tostring()) == esize)

                mattype_size = {
                    cv.CV_8UC1 : 1,
                    cv.CV_8UC2 : 1,
                    cv.CV_8UC3 : 1,
                    cv.CV_8UC4 : 1,
                    cv.CV_8SC1 : 1,
                    cv.CV_8SC2 : 1,
                    cv.CV_8SC3 : 1,
                    cv.CV_8SC4 : 1,
                    cv.CV_16UC1 : 2,
                    cv.CV_16UC2 : 2,
                    cv.CV_16UC3 : 2,
                    cv.CV_16UC4 : 2,
                    cv.CV_16SC1 : 2,
                    cv.CV_16SC2 : 2,
                    cv.CV_16SC3 : 2,
                    cv.CV_16SC4 : 2,
                    cv.CV_32SC1 : 4,
                    cv.CV_32SC2 : 4,
                    cv.CV_32SC3 : 4,
                    cv.CV_32SC4 : 4,
                    cv.CV_32FC1 : 4,
                    cv.CV_32FC2 : 4,
                    cv.CV_32FC3 : 4,
                    cv.CV_32FC4 : 4,
                    cv.CV_64FC1 : 8,
                    cv.CV_64FC2 : 8,
                    cv.CV_64FC3 : 8,
                    cv.CV_64FC4 : 8
                }

                for t in self.mat_types:
                    im = cv.CreateMat(h, w, t)
                    elemsize = cv.CV_MAT_CN(cv.GetElemType(im)) * mattype_size[cv.GetElemType(im)]
                    cv.SetData(im, " " * (w * h * elemsize), (w * elemsize))
                    esize = (w * h * elemsize)
                    self.assert_(len(im.tostring()) == esize)
                    cv.SetData(im, " " * esize, w * elemsize)
                    self.assert_(len(im.tostring()) == esize)

    def xxx_test_Disparity(self):
        print
        for t in ["8U", "8S", "16U", "16S", "32S", "32F", "64F" ]:
          for c in [1,2,3,4]:
            nm = "%sC%d" % (t, c)
            print "int32 CV_%s=%d" % (nm, eval("cv.CV_%s" % nm))
        return
        integral = cv.CreateImage((641,481), cv.IPL_DEPTH_32S, 1)
        L = cv.LoadImage("f0-left.png", 0)
        R = cv.LoadImage("f0-right.png", 0)
        d = cv.CreateImage(cv.GetSize(L), cv.IPL_DEPTH_8U, 1)
        Rn = cv.CreateImage(cv.GetSize(L), cv.IPL_DEPTH_8U, 1)
        started = time.time()
        for i in range(100):
            cv.AbsDiff(L, R, d)
            cv.Integral(d, integral)
            cv.SetImageROI(R, (1, 1, 639, 479))
            cv.SetImageROI(Rn, (0, 0, 639, 479))
            cv.Copy(R, Rn)
            R = Rn
            cv.ResetImageROI(R)
        print 1e3 * (time.time() - started) / 100, "ms"
        # self.snap(d)

    def local_test_lk(self):
        seq = [cv.LoadImage("track/%06d.png" % i, 0) for i in range(40)]
        crit = (cv.CV_TERMCRIT_ITER, 100, 0.1)
        crit = (cv.CV_TERMCRIT_EPS, 0, 0.001)

        for i in range(1,40):
            r = cv.CalcOpticalFlowPyrLK(seq[0], seq[i], None, None, [(32,32)], (7,7), 0, crit, 0)
            pos = r[0][0]
            #print pos, r[2]

            a = cv.CreateImage((1024,1024), 8, 1)
            b = cv.CreateImage((1024,1024), 8, 1)
            cv.Resize(seq[0], a, cv.CV_INTER_NN)
            cv.Resize(seq[i], b, cv.CV_INTER_NN)
            cv.Line(a, (0, 512), (1024, 512), 255)
            cv.Line(a, (512,0), (512,1024), 255)
            x,y = [int(c) for c in pos]
            cv.Line(b, (0, y*16), (1024, y*16), 255)
            cv.Line(b, (x*16,0), (x*16,1024), 255)
            #self.snapL([a,b])

    def xxx_test_CalcOpticalFlowBM(self):
        a = cv.LoadImage("ab/0.tiff", 0)

        if 0:
            # create b, just a shifted 2 pixels in X
            b = cv.CreateImage(cv.GetSize(a), 8, 1)
            m = cv.CreateMat(2, 3, cv.CV_32FC1)
            cv.SetZero(m)
            m[0,0] = 1
            m[1,1] = 1
            m[0,2] = 2
            cv.WarpAffine(a, b, m)
        else:
            b = cv.LoadImage("ab/1.tiff", 0)

        if 1:
            factor = 2
            for i in range(50):
                print i
                o0 = cv.LoadImage("again3_2245/%06d.tiff" % i, 1)
                o1 = cv.LoadImage("again3_2245/%06d.tiff" % (i+1), 1)
                a = cv.CreateImage((640,360), 8, 3)
                b = cv.CreateImage((640,360), 8, 3)
                cv.Resize(o0, a)
                cv.Resize(o1, b)
                am = cv.CreateImage(cv.GetSize(a), 8, 1)
                bm = cv.CreateImage(cv.GetSize(b), 8, 1)
                cv.CvtColor(a, am, cv.CV_RGB2GRAY)
                cv.CvtColor(b, bm, cv.CV_RGB2GRAY)
                fi = FrameInterpolator(am, bm)
                for k in range(factor):
                    on = (i * factor) + k
                    cv.SaveImage("/Users/jamesb/Desktop/foo/%06d.png" % on, fi.lerp(k / float(factor), a, b))
            return

        if 0:
            # Run FlowBM
            w,h = cv.GetSize(a)
            wv = (w - 6) / 8
            hv = (h - 6) / 8
            velx = cv.CreateMat(hv, wv, cv.CV_32FC1)
            vely = cv.CreateMat(hv, wv, cv.CV_32FC1)
            cv.CalcOpticalFlowBM(a, b, (6,6), (8,8), (32,32), 0, velx, vely)

            if 1:
                scribble = cv.CreateImage(cv.GetSize(a), 8, 3)
                cv.CvtColor(a, scribble, cv.CV_GRAY2BGR)
                for y in range(0,360, 4):
                    for x in range(0,640, 4):
                        cv.Line(scribble, (x, y), (x+velx[y,x], y + vely[y,x]), (0,255,0))
                cv.Line(a, (640/5,0), (640/5,480), 255)
                cv.Line(a, (0,360/5), (640,360/5), 255)
                self.snap(scribbe)
                return 0
                ivx = cv.CreateMat(h, w, cv.CV_32FC1)
                ivy = cv.CreateMat(h, w, cv.CV_32FC1)
                cv.Resize(velx, ivx)
                cv.Resize(vely, ivy)

            cv.ConvertScale(ivx, ivx, 0.5)
            cv.ConvertScale(ivy, ivy, 0.5)
        
        if 1:
            w,h = cv.GetSize(a)
            velx = cv.CreateMat(h, w, cv.CV_32FC1)
            vely = cv.CreateMat(h, w, cv.CV_32FC1)
            cv.CalcOpticalFlowLK(a, b, (7,7), velx, vely)

            for i in range(10):
                cv.Smooth(velx, velx, param1 = 7)
                cv.Smooth(vely, vely, param1 = 7)
            scribble = cv.CreateImage(cv.GetSize(a), 8, 3)
            cv.CvtColor(a, scribble, cv.CV_GRAY2BGR)
            for y in range(0, 360, 8):
                for x in range(0, 640, 8):
                    cv.Line(scribble, (x, y), (x+velx[y,x], y + vely[y,x]), (0,255,0))
            self.snapL((a,scribble,b))
            ivx = velx
            ivy = vely

        offx = cv.CreateMat(h, w, cv.CV_32FC1)
        offy = cv.CreateMat(h, w, cv.CV_32FC1)
        for y in range(360):
            for x in range(640):
                offx[y,x] = x
                offy[y,x] = y

        x = cv.CreateMat(h, w, cv.CV_32FC1)
        y = cv.CreateMat(h, w, cv.CV_32FC1)
        d = cv.CreateImage(cv.GetSize(a), 8, 1)
        cv.ConvertScale(velx, x, 1.0)
        cv.ConvertScale(vely, y, 1.0)
        cv.Add(x, offx, x)
        cv.Add(y, offy, y)

        cv.Remap(b, d, x, y)
        cv.Merge(d, d, a, None, scribble)
        original = cv.CreateImage(cv.GetSize(a), 8, 3)
        cv.Merge(b, b, a, None, original)
        self.snapL((original, scribble))

    def snap(self, img):
        self.snapL([img])

    def snapL(self, L):
        for i,img in enumerate(L):
            cv.NamedWindow("snap-%d" % i, 1)
            cv.ShowImage("snap-%d" % i, img)
        cv.WaitKey()
        cv.DestroyAllWindows()

    def yield_line_image(self):
        src = cv.LoadImage(find_sample("building.jpg"), 0)
        dst = cv.CreateImage(cv.GetSize(src), 8, 1)
        cv.Canny(src, dst, 50, 200, 3)
        return dst

    def test_HoughLines2_STANDARD(self):
        li = cv.HoughLines2(self.yield_line_image(),
                                                cv.CreateMemStorage(),
                                                cv.CV_HOUGH_STANDARD,
                                                1,
                                                math.pi/180,
                                                100,
                                                0,
                                                0)
        self.assert_(len(li) > 0)
        self.assert_(li[0] != None)

    def test_HoughLines2_PROBABILISTIC(self):
        li = cv.HoughLines2(self.yield_line_image(),
                                                cv.CreateMemStorage(),
                                                cv.CV_HOUGH_PROBABILISTIC,
                                                1,
                                                math.pi/180,
                                                50,
                                                50,
                                                10)
        self.assert_(len(li) > 0)
        self.assert_(li[0] != None)

    def test_Save(self):
        for o in [ cv.CreateImage((128,128), cv.IPL_DEPTH_8U, 1), cv.CreateMat(16, 16, cv.CV_32FC1) ]:
            cv.Save("test.save", o)
            loaded = cv.Load("test.save", cv.CreateMemStorage())
            self.assert_(type(o) == type(loaded))

    def test_ExtractSURF(self):
        img = cv.LoadImage(find_sample("lena.jpg"), 0)
        w,h = cv.GetSize(img)
        for hessthresh in [ 300,400,500]:
            for dsize in [0,1]:
                for layers in [1,3,10]:
                    kp,desc = cv.ExtractSURF(img, None, cv.CreateMemStorage(), (dsize, hessthresh, 3, layers))
                    self.assert_(len(kp) == len(desc))
                    for d in desc:
                        self.assert_(len(d) == {0:64, 1:128}[dsize])
                    for pt,laplacian,size,dir,hessian in kp:
                        self.assert_((0 <= pt[0]) and (pt[0] <= w))
                        self.assert_((0 <= pt[1]) and (pt[1] <= h))
                        self.assert_(laplacian in [-1, 0, 1])
                        self.assert_((0 <= dir) and (dir <= 360))
                        self.assert_(hessian >= hessthresh)

    def local_test_Haar(self):
        import os
        hcfile = os.environ['OPENCV_ROOT'] + '/share/opencv/haarcascades/haarcascade_frontalface_default.xml'
        hc = cv.Load(hcfile)
        img = cv.LoadImage('Stu.jpg', 0)
        faces = cv.HaarDetectObjects(img, hc, cv.CreateMemStorage())
        self.assert_(len(faces) > 0)
        for (x,y,w,h),n in faces:
            cv.Rectangle(img, (x,y), (x+w,y+h), 255)
        #self.snap(img)

    def test_FindChessboardCorners(self):
        im = cv.CreateImage((512,512), cv.IPL_DEPTH_8U, 1)
        cv.Set(im, 128)

        # Empty image run
        status,corners = cv.FindChessboardCorners( im, (7,7) )

        # Perfect checkerboard
        def xf(i,j, o):
            return ((96 + o) + 40 * i, (96 + o) + 40 * j)
        for i in range(8):
            for j in range(8):
                color = ((i ^ j) & 1) * 255
                cv.Rectangle(im, xf(i,j, 0), xf(i,j, 39), color, cv.CV_FILLED)
        status,corners = cv.FindChessboardCorners( im, (7,7) )
        self.assert_(status)
        self.assert_(len(corners) == (7 * 7))

        # Exercise corner display
        im3 = cv.CreateImage(cv.GetSize(im), cv.IPL_DEPTH_8U, 3)
        cv.Merge(im, im, im, None, im3)
        cv.DrawChessboardCorners(im3, (7,7), corners, status)

        if 0:
            self.snap(im3)

        # Run it with too many corners
        cv.Set(im, 128)
        for i in range(40):
            for j in range(40):
                color = ((i ^ j) & 1) * 255
                x = 30 + 6 * i
                y = 30 + 4 * j
                cv.Rectangle(im, (x, y), (x+4, y+4), color, cv.CV_FILLED)
        status,corners = cv.FindChessboardCorners( im, (7,7) )

        # XXX - this is very slow
        if 0:
            rng = cv.RNG(0)
            cv.RandArr(rng, im, cv.CV_RAND_UNI, 0, 255.0)
            self.snap(im)
            status,corners = cv.FindChessboardCorners( im, (7,7) )

    def test_FillPoly(self):
        scribble = cv.CreateImage((640,480), cv.IPL_DEPTH_8U, 1)
        random.seed(0)
        for i in range(50):
            cv.SetZero(scribble)
            self.assert_(cv.CountNonZero(scribble) == 0)
            cv.FillPoly(scribble, [ [ (random.randrange(640), random.randrange(480)) for i in range(100) ] ], (255,))
            self.assert_(cv.CountNonZero(scribble) != 0)

    def test_create(self):
        """ CvCreateImage, CvCreateMat and the header-only form """
        for (w,h) in [ (320,400), (640,480), (1024, 768) ]:
            data = "z" * (w * h)

            im = cv.CreateImage((w,h), 8, 1)
            cv.SetData(im, data, w)
            im2 = cv.CreateImageHeader((w,h), 8, 1)
            cv.SetData(im2, data, w)
            self.assertSame(im, im2)

            m = cv.CreateMat(h, w, cv.CV_8UC1)
            cv.SetData(m, data, w)
            m2 = cv.CreateMatHeader(h, w, cv.CV_8UC1)
            cv.SetData(m2, data, w)
            self.assertSame(m, m2)

            self.assertSame(im, m)
            self.assertSame(im2, m2)

    def test_reshape(self):
        """ Exercise Reshape """
        # 97 rows
        # 12 cols
        rows = 97
        cols = 12
        im = cv.CreateMat( rows, cols, cv.CV_32FC1 )
        elems = rows * cols * 1
        def crd(im):
            return cv.GetSize(im) + (cv.CV_MAT_CN(cv.GetElemType(im)),)

        for c in (1, 2, 3, 4):
            nc,nr,nd = crd(cv.Reshape(im, c))
            self.assert_(nd == c)
            self.assert_((nc * nr * nd) == elems)

        nc,nr,nd = crd(cv.Reshape(im, 0, 97*2))
        self.assert_(nr == 97*2)
        self.assert_((nc * nr * nd) == elems)

        nc,nr,nd = crd(cv.Reshape(im, 3, 97*2))
        self.assert_(nr == 97*2)
        self.assert_(nd == 3)
        self.assert_((nc * nr * nd) == elems)

    def test_casts(self):
        """ Exercise Reshape """
        im = cv.LoadImage(find_sample("lena.jpg"), 0)
        data = im.tostring()
        cv.SetData(im, data, cv.GetSize(im)[0])

        start_count = sys.getrefcount(data)

        # Conversions should produce same data
        self.assertSame(im, cv.GetImage(im))
        m = cv.GetMat(im)
        self.assertSame(im, m)
        self.assertSame(m, cv.GetImage(m))
        im2 = cv.GetImage(m)
        self.assertSame(im, im2)
        
        self.assertEqual(sys.getrefcount(data), start_count + 2)
        del im2
        self.assertEqual(sys.getrefcount(data), start_count + 1)
        del m
        self.assertEqual(sys.getrefcount(data), start_count)
        del im
        self.assertEqual(sys.getrefcount(data), start_count - 1)

    def test_clipline(self):
        self.assert_(cv.ClipLine((100,100), (-100,0), (500,0)) == ((0,0), (99,0)))
        self.assert_(cv.ClipLine((100,100), (-100,0), (-200,0)) == None)

    def test_smoke_image_processing(self):
        src = cv.LoadImage(find_sample("lena.jpg"), cv.CV_LOAD_IMAGE_GRAYSCALE)
        #dst = cv.CloneImage(src)
        for aperture_size in [1, 3, 5, 7]:
          dst_16s = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_16S, 1)
          dst_32f = cv.CreateImage(cv.GetSize(src), cv.IPL_DEPTH_32F, 1)

          cv.Sobel(src, dst_16s, 1, 1, aperture_size)
          cv.Laplace(src, dst_16s, aperture_size)
          cv.PreCornerDetect(src, dst_32f)
          eigendst = cv.CreateImage((6*cv.GetSize(src)[0], cv.GetSize(src)[1]), cv.IPL_DEPTH_32F, 1)
          cv.CornerEigenValsAndVecs(src, eigendst, 8, aperture_size)
          cv.CornerMinEigenVal(src, dst_32f, 8, aperture_size)
          cv.CornerHarris(src, dst_32f, 8, aperture_size)
          cv.CornerHarris(src, dst_32f, 8, aperture_size, 0.1)

        #self.snap(dst)

    def test_fitline(self):
        cv.FitLine([ (1,1), (10,10) ], cv.CV_DIST_L2, 0, 0.01, 0.01)
        cv.FitLine([ (1,1,1), (10,10,10) ], cv.CV_DIST_L2, 0, 0.01, 0.01)
        a = cv.LoadImage(find_sample("lena.jpg"), 0)
        eig_image = cv.CreateImage(cv.GetSize(a), cv.IPL_DEPTH_32F, 1)
        temp_image = cv.CreateImage(cv.GetSize(a), cv.IPL_DEPTH_32F, 1)
        pts = cv.GoodFeaturesToTrack(a, eig_image, temp_image, 100, 0.04, 2, use_harris=1)
        hull = cv.ConvexHull2(pts, cv.CreateMemStorage(), return_points = 1)
        cv.FitLine(hull, cv.CV_DIST_L2, 0, 0.01, 0.01)

    def test_moments(self):
        im = cv.LoadImage(find_sample("lena.jpg"), 0)
        mo = cv.Moments(im)
        orders = []
        for x_order in range(4):
          for y_order in range(4 - x_order):
            orders.append((x_order, y_order))
        
        # Just a smoke test for these three functions
        [ cv.GetSpatialMoment(mo, xo, yo) for (xo,yo) in orders ]
        [ cv.GetCentralMoment(mo, xo, yo) for (xo,yo) in orders ]
        [ cv.GetNormalizedCentralMoment(mo, xo, yo) for (xo,yo) in orders ]

        # Hu Moments we can do slightly better.  Check that the first
        # six are invariant wrt image reflection, and that the 7th
        # is negated.

        hu0 = cv.GetHuMoments(cv.Moments(im))
        cv.Flip(im, im, 1)
        hu1 = cv.GetHuMoments(cv.Moments(im))
        self.assert_(len(hu0) == 7)
        self.assert_(len(hu1) == 7)
        for i in range(5):
          self.assert_(abs(hu0[i] - hu1[i]) < 1e-6)
        self.assert_(abs(hu0[i] + hu1[i]) < 1e-6)

    def temp_test(self):
        cv.temp_test()

    def failing_test_rand_GetStarKeypoints(self):
        #GetStarKeypoints [<cvmat(type=4242400d rows=64 cols=64 step=512 )>, <cv.cvmemstorage object at 0xb7cc40d0>, (45, 0.73705234376883488, 0.64282591451367344, 0.1567738743689836, 3)]
        print cv.CV_MAT_CN(0x4242400d)
        mat = cv.CreateMat( 64, 64, cv.CV_32FC2)
        cv.GetStarKeypoints(mat, cv.CreateMemStorage(), (45, 0.73705234376883488, 0.64282591451367344, 0.1567738743689836, 3))
        print mat

    def test_rand_PutText(self):
        """ Test for bug 2829336 """
        mat = cv.CreateMat( 64, 64, cv.CV_8UC1)
        font = cv.InitFont(cv.CV_FONT_HERSHEY_SIMPLEX, 1, 1)
        cv.PutText(mat, chr(127), (20, 20), font, 255)

    def failing_test_rand_FindNearestPoint2D(self):
        subdiv = cv.CreateSubdivDelaunay2D((0,0,100,100), cv.CreateMemStorage())
        cv.SubdivDelaunay2DInsert( subdiv, (50, 50))
        cv.CalcSubdivVoronoi2D(subdiv)
        print
        for e in subdiv.edges:
            print e, 
            print "  ", cv.Subdiv2DEdgeOrg(e)
            print "  ", cv.Subdiv2DEdgeOrg(cv.Subdiv2DRotateEdge(e, 1)), cv.Subdiv2DEdgeDst(cv.Subdiv2DRotateEdge(e, 1))
        print "nearest", cv.FindNearestPoint2D(subdiv, (1.0, 1.0))

if __name__ == '__main__':
    random.seed(0)
    if len(sys.argv) == 1:
        suite = unittest.TestLoader().loadTestsFromTestCase(TestDirected)
        unittest.TextTestRunner(verbosity=2).run(suite)
    else:
        suite = unittest.TestSuite()
        suite.addTest(TestDirected(sys.argv[1]))
        unittest.TextTestRunner(verbosity=2).run(suite)
