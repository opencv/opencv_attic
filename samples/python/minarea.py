#!/usr/bin/python 

from opencv.cv import *
from opencv.highgui import *
from random import randint

def parse_point_seq( seq ):
    for i in range(seq.total):
        yield cvGetSeqElemAsPoint( seq, i )

def minarea_array(img, count):
    pointMat = cvCreateMat( count, 1, CV_32SC2 )
    for i in range(count):
        pointMat[i] = cvPoint( randint(img.width/4, img.width*3/4),
                               randint(img.height/4, img.height*3/4) )

    box = cvMinAreaRect2( pointMat )
    box_vtx = cvBoxPoints( box )
    success, center, radius = cvMinEnclosingCircle( pointMat )
    cvZero( img )
    for i in range(count):
        cvCircle( img, cvGet1D(pointMat,i), 2, CV_RGB( 255, 0, 0 ), CV_FILLED, CV_AA, 0 )

    box_vtx = [cvPointFrom32f(box_vtx[0]),
               cvPointFrom32f(box_vtx[1]),
               cvPointFrom32f(box_vtx[2]),
               cvPointFrom32f(box_vtx[3])]
    cvCircle( img, cvPointFrom32f(center), cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 )
    cvPolyLine( img, [box_vtx], 1, CV_RGB(0,255,255), 1, CV_AA ) 
    

    
def minarea_seq(img, count, storage):
    ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof_CvContour, sizeof_CvPoint, storage )
    for i in range(count):
        pt0 = cvPoint( randint(img.width/4, img.width*3/4),
                       randint(img.height/4, img.height*3/4) )
        cvSeqPush( ptseq, pt0 )
    box = cvMinAreaRect2( ptseq )
    box_vtx = cvBoxPoints( box )
    success, center, radius = cvMinEnclosingCircle( ptseq )
    cvZero( img )
    for pt in parse_point_seq(ptseq):
        cvCircle( img, pt, 2, CV_RGB( 255, 0, 0 ), CV_FILLED, CV_AA, 0 )

    box_vtx = [cvPointFrom32f(box_vtx[0]),
               cvPointFrom32f(box_vtx[1]),
               cvPointFrom32f(box_vtx[2]),
               cvPointFrom32f(box_vtx[3])]
    cvCircle( img, cvPointFrom32f(center), cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 )
    cvPolyLine( img, [box_vtx], 1, CV_RGB(0,255,255), 1, CV_AA ) 
    cvClearMemStorage( storage )

if __name__ == "__main__":
    img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );
    storage = cvCreateMemStorage(0);

    cvNamedWindow( "rect & circle", 1 );
        
    use_seq=True

    while True: 
        count = randint(1,100)
        if use_seq:
            minarea_seq(img, count, storage)
        else:
            minarea_array(img, count)

        cvShowImage("rect & circle", img)
        key = cvWaitKey()
        if( key == '\x1b' ):
            break;

        use_seq = not use_seq
        """
        CvPoint pt0, pt;
        CvBox2D box;
        CvPoint2D32f box_vtx[4];
        CvPoint2D32f center;
        CvPoint icenter;


        float radius;
        CvPoint* points = (CvPoint*)malloc( count * sizeof(points[0]));
        CvMat pointMat = cvMat( 1, count, CV_32SC2, points );

        for( i = 0; i < count; i++ )
        {
            pt0.x = rand() % (img->width/2) + img->width/4;
            pt0.y = rand() % (img->height/2) + img->height/4;
            points[i] = pt0;
        }
#ifndef _EiC
        box = cvMinAreaRect2( &pointMat, 0 );
#endif
        cvMinEnclosingCircle( &pointMat, &center, &radius );
#endif
        cvBoxPoints( box, box_vtx );
        cvZero( img );
        for( i = 0; i < count; i++ )
        {
            pt0 = points[i];
            cvCircle( img, pt0, 2, CV_RGB( 255, 0, 0 ), CV_FILLED, CV_AA, 0 );
        }

        pt0.x = cvRound(box_vtx[3].x);
        pt0.y = cvRound(box_vtx[3].y);
        for( i = 0; i < 4; i++ )
        {
            pt.x = cvRound(box_vtx[i].x);
            pt.y = cvRound(box_vtx[i].y);
            cvLine(img, pt0, pt, CV_RGB(0, 255, 0), 1, CV_AA, 0);
            pt0 = pt;
        }
#endif
        icenter.x = cvRound(center.x);
        icenter.y = cvRound(center.y);
        cvCircle( img, icenter, cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 );

        cvShowImage( "rect & circle", img );

        key = (char) cvWaitKey(0);
        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
            break;

#if !ARRAY
        cvClearMemStorage( storage );
#else
        free( points );
#endif
    }
    
    cvDestroyWindow( "rect & circle" );
    return 0;
}

#ifdef _EiC
main(1,"convexhull.c");
#endif
"""
