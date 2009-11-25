import cv
import numpy

def clamp(a, x, b):
    return numpy.maximum(a, numpy.minimum(x, b))

def norm(v):
    mag = numpy.sqrt(sum([e * e for e in v]))
    return v / mag

class Ray:
    def __init__(self, o, d):
        self.o = o
        self.d = d

class Camera:
    
    def __init__(self, F):
        R = (1., 0., 0.)
        U = (0, 1., 0)
        self.center = (0, 0, 0)
        self.pcenter = (0, 0, F)
        self.up = U
        self.right = R

    def genray(self, x, y):
        """ -1 <= y <= 1 """
        o = self.center
        r = tuple([(self.pcenter[c] + x * self.right[c] + y * self.up[c]) - o[c] for c in range(3)])
        return Ray(o, norm(r))

class Sphere:

    def __init__(self, center, radius):
        self.center = center
        self.radius = radius

    def hit(self, r):
        # a = mag2(r.d)
        a = 1.
        v = [r.o[c] - self.center[c] for c in range(3)]
        b = 2 * (r.d.x * v.x + r.d.y * v.y + r.d.z * v.z)
        c = mag2(self.center) + mag2(r.o) + 2 * (-self.center.x * r.o.x - self.center.y * r.o.y - self.center.z * r.o.z) - self.radius * self.radius
        det = (b * b) - (4 * c)
        pred = 0 < det

        if not sometrue(pred):
            return (pred, MAXFLOAT, dummyNormal)
        sq = safesqrt(det)
        h0 = (-b - sq) / (2)
        h1 = (-b + sq) / (2)

        h = minimum(h0, h1)

        pred = pred & (h > 0)
        normal = (r.project(h) - self.center) * (1.0 / self.radius)
        return (pred, where(pred, h, MAXFLOAT), normal)

cam = Camera(7.0)

x = numpy.arange(640*480) % 640
y = numpy.floor(numpy.arange(640*480) / 640)
nx = (x - 320) / 320.
ny = (y - 240) / 240.
r = cam.genray(nx, ny)

img = cv.CreateMat(480, 640, cv.CV_8UC1)
cv.SetData(img, (clamp(0, l, 1) * 255).astype(numpy.uint8).tostring(), 640)
cv.NamedWindow("snap")
cv.ShowImage("snap", img)
cv.WaitKey()
