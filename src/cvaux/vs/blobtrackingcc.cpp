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

#include "_cvaux.h"

static float CalcAverageMask(CvBlob* pBlob, IplImage* pImgFG )
{/* calc summ of mask */
    double  Area, Aver = 0;
    CvRect  r;
    CvMat   mat;

    if(pImgFG==NULL) return 0;

    r.x = cvRound(pBlob->x - pBlob->w*0.5);
    r.y = cvRound(pBlob->y - pBlob->h*0.5);
    r.width = cvRound(pBlob->w);
    r.height = cvRound(pBlob->h);
    Area = r.width*r.height;
    if(r.x<0){r.width += r.x;r.x = 0;}
    if(r.y<0){r.height += r.y;r.y = 0;}
    if((r.x+r.width)>=pImgFG->width){r.width=pImgFG->width-r.x-1;}
    if((r.y+r.height)>=pImgFG->height){r.height=pImgFG->height-r.y-1;}
    if(r.width>0 && r.height>0)
    {
        double Sum = cvSum(cvGetSubRect(pImgFG,&mat,r)).val[0]/255.0;
        assert(Area>0);
        Aver = Sum/Area;
    }
    return (float)Aver;
}/* calc summ of mask */


/*============== BLOB TRACKERCC CLASS DECLARATION =============== */
typedef struct DefBlobTracker
{
    CvBlob                      blob;
    CvBlobTrackPredictor*       pPredictor;
    CvBlob                      BlobPredict;
    int                         Collision;
    CvBlobSeq*                  pBlobHyp;
    float                       AverFG;
}DefBlobTracker;
void cvFindBlobsByCCClasters(IplImage* pFG, CvBlobSeq* pBlobs, CvMemStorage* storage);

class CvBlobTrackerCC : public CvBlobTracker
{
private:
    float           m_AlphaSize;
    float           m_AlphaPos;
    float           m_Alpha;
    int             m_Collision;
    int             m_ConfidenceType;
    char*           m_ConfidenceTypeStr;
    CvBlobSeq       m_BlobList;
    CvBlobSeq       m_BlobListNew;
//    int             m_LastID;
    CvMemStorage*   m_pMem;
    int             m_ClearHyp;
    IplImage*       m_pImg;
    IplImage*       m_pImgFG;
public:
    CvBlobTrackerCC():m_BlobList(sizeof(DefBlobTracker))
    {
//        m_LastID = 0;
        m_ClearHyp = 0;
        m_pMem = cvCreateMemStorage();
        m_Collision = 1; /* if 1 then collistion will be detected and processed */
        AddParam("Collision",&m_Collision);
        CommentParam("Collision", "if 1 then collision cases are processed in special way");

        m_AlphaSize = 0.02f;
        AddParam("AlphaSize",&m_AlphaSize);
        CommentParam("AlphaSize", "Size update speed (0..1)");

        m_AlphaPos = 1.0f;
        AddParam("AlphaPos",&m_AlphaPos);
        CommentParam("AlphaPos", "Pos update speed (0..1)");

        m_Alpha = 0.001f;
        AddParam("Alpha", &m_Alpha);
        CommentParam("Alpha","Coefficient for model histogramm updating (0 - hist is not upated)");

        m_ConfidenceType=0;
        m_ConfidenceTypeStr = "NearestBlob";
        AddParam("ConfidenceType", &m_ConfidenceTypeStr);
        CommentParam("ConfidenceType","Type of calculated Confidence (NearestBlob, AverFG, BC)");

        SetModuleName("CC");
    };
    ~CvBlobTrackerCC()
    {
        if(m_pMem)cvReleaseMemStorage(&m_pMem);
    };

    /* blob functions*/
    virtual int     GetBlobNum() {return m_BlobList.GetBlobNum();};
    virtual CvBlob* GetBlob(int BlobIndex){return m_BlobList.GetBlob(BlobIndex);};
    virtual void    SetBlob(int BlobIndex, CvBlob* pBlob)
    {
        CvBlob* pB = m_BlobList.GetBlob(BlobIndex);
        if(pB) pB[0] = pBlob[0];
    };
    virtual CvBlob* GetBlobByID(int BlobID){return m_BlobList.GetBlobByID(BlobID);};
    virtual void    DelBlob(int BlobIndex)
    {
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIndex);
        if(pBT==NULL) return;
        if(pBT->pPredictor)
        {
            pBT->pPredictor->Release();
        }
        else
        {
            printf("WARNING!!! Invalid Predictor in CC tracker");
        }
        delete pBT->pBlobHyp;
        m_BlobList.DelBlob(BlobIndex);
    };
#if 0
    virtual void    DelBlobByID(int BlobID)
    {
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlobByID(BlobID);
        pBT->pPredictor->Release();
        delete pBT->pBlobHyp;
        m_BlobList.DelBlobByID(BlobID);
    };
#endif
    virtual void    Release(){delete this;};

    /* Add new blob to track it and assign to this blob personal ID */
    /* pBlob - pinter to structure with blob parameters (ID is ignored)*/
    /* pImg - current image */
    /* pImgFG - current foreground mask */
    /* return pointer to new added blob */
    virtual CvBlob* AddBlob(CvBlob* pB, IplImage* /*pImg*/, IplImage* pImgFG = NULL )
    {
        assert(pImgFG); /* this tracker use only foreground mask */
        DefBlobTracker NewB;
        NewB.blob = pB[0];
//        CV_BLOB_ID(&NewB) = m_LastID;
        NewB.pBlobHyp = new CvBlobSeq;
        NewB.pPredictor = cvCreateModuleBlobTrackPredictKalman(); /* module for predict position */
        NewB.pPredictor->Update(pB);
        NewB.AverFG = pImgFG?CalcAverageMask(pB,pImgFG):0;
        m_BlobList.AddBlob((CvBlob*)&NewB);
        return m_BlobList.GetBlob(m_BlobList.GetBlobNum()-1);
    };
    virtual void    Process(IplImage* pImg, IplImage* pImgFG = NULL)
    {
        CvSeq*      cnts;
        CvSeq*      cnt;
        int i;

        m_pImg = pImg;
        m_pImgFG = pImgFG;

        if(m_BlobList.GetBlobNum() <= 0 ) return;
        
        /* clear blob list for new blobs */
        m_BlobListNew.Clear();

        assert(m_pMem);
        cvClearMemStorage(m_pMem);
        assert(pImgFG);

        
        /* find CC */
#if 0
        {// by contur clastring
            cvFindBlobsByCCClasters(pImgFG, &m_BlobListNew, m_pMem);
        }
#else
        {/* one contour - one blob */
            IplImage* pBin = cvCloneImage(pImgFG);
            assert(pBin);
            cvThreshold(pBin,pBin,128,255,CV_THRESH_BINARY);
            cvFindContours(pBin, m_pMem, &cnts, sizeof(CvContour), CV_RETR_EXTERNAL);
            /* process each contours*/
            for(cnt = cnts;cnt;cnt=cnt->h_next)
            {
                CvBlob  NewBlob;
                /* image moments */
                double      M00,X,Y,XX,YY;
                CvMoments   m;
                CvRect      r = ((CvContour*)cnt)->rect;
                CvMat       mat;
                if(r.height < 3 || r.width < 3) continue;
                cvMoments( cvGetSubRect(pImgFG,&mat,r), &m, 0 );
                M00 = cvGetSpatialMoment( &m, 0, 0 );
                if(M00 <= 0 ) continue;
                X = cvGetSpatialMoment( &m, 1, 0 )/M00;
                Y = cvGetSpatialMoment( &m, 0, 1 )/M00;
                XX = (cvGetSpatialMoment( &m, 2, 0 )/M00) - X*X;
                YY = (cvGetSpatialMoment( &m, 0, 2 )/M00) - Y*Y;
                NewBlob = cvBlob(r.x+(float)X,r.y+(float)Y,(float)(4*sqrt(XX)),(float)(4*sqrt(YY)));
                m_BlobListNew.AddBlob(&NewBlob);
            }/* next contour */
            cvReleaseImage(&pBin);
        }
#endif        
        for(i=m_BlobList.GetBlobNum();i>0;--i)
        {/* predict new blob position */
            CvBlob*         pB=NULL;
            DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(i-1);

            /* update predictor by previouse value of blob*/
            pBT->pPredictor->Update(&(pBT->blob));
            
            /* predict current position */
            pB = pBT->pPredictor->Predict();
            
            if(pB)
            {
                pBT->BlobPredict = pB[0];
            }
            else
            {
                pBT->BlobPredict = pBT->blob;
            }
        }/* predict new blob position */

        if(m_Collision)
        for(i=m_BlobList.GetBlobNum();i>0;--i)
        {/* predict collision */
            int             Collision = 0;
            int             j;
            DefBlobTracker* pF = (DefBlobTracker*)m_BlobList.GetBlob(i-1);
            for(j=m_BlobList.GetBlobNum();j>0;--j)
            {/* predict collision */
                CvBlob* pB1;
                CvBlob* pB2;
                DefBlobTracker* pF2 = (DefBlobTracker*)m_BlobList.GetBlob(j-1);
                if(i==j) continue;
                pB1 = &pF->BlobPredict;
                pB2 = &pF2->BlobPredict;
                if( fabs(pB1->x-pB2->x)<0.6*(pB1->w+pB2->w) &&
                    fabs(pB1->y-pB2->y)<0.6*(pB1->h+pB2->h) ) Collision = 1;
                pB1 = &pF->blob;
                pB2 = &pF2->blob;
                if( fabs(pB1->x-pB2->x)<0.6*(pB1->w+pB2->w) &&
                    fabs(pB1->y-pB2->y)<0.6*(pB1->h+pB2->h) ) Collision = 1;
                if(Collision) break;
            }/* check next blob to cross current*/
            pF->Collision = Collision;
        }/* predict collision */

        for(i=m_BlobList.GetBlobNum();i>0;--i)
        {/* find a neighbour on cur frame for each blob from prev frame */
            CvBlob*         pB = m_BlobList.GetBlob(i-1);
            DefBlobTracker* pBT = (DefBlobTracker*)pB;
            //int             BlobID = CV_BLOB_ID(pB);
            //CvBlob*         pBBest = NULL;
            //double          DistBest = -1;
            //int j;

            if(pBT->pBlobHyp->GetBlobNum()>0)
            {/* track all hypothesis */
                int h,hN = pBT->pBlobHyp->GetBlobNum();
                for(h=0;h<hN;++h)
                {
                    int         j, jN = m_BlobListNew.GetBlobNum();
                    CvBlob*     pB = pBT->pBlobHyp->GetBlob(h);
                    int         BlobID = CV_BLOB_ID(pB);
                    CvBlob*     pBBest = NULL;
                    double      DistBest = -1;
                    for(j=0;j<jN;j++)
                    {/* find best CC */
                        double  Dist = -1;
                        CvBlob* pBNew = m_BlobListNew.GetBlob(j);
                        double  dx = fabs(CV_BLOB_X(pB)-CV_BLOB_X(pBNew));
                        double  dy = fabs(CV_BLOB_Y(pB)-CV_BLOB_Y(pBNew));
                        if(dx > 2*CV_BLOB_WX(pB) || dy > 2*CV_BLOB_WY(pB)) continue;

                        Dist = sqrt(dx*dx+dy*dy);
                        if(Dist < DistBest || pBBest == NULL)
                        {
                            DistBest = Dist;
                            pBBest = pBNew;
                        }
                    }/* find best CC */
                    if(pBBest)
                    {
                        pB[0] = pBBest[0];
                        CV_BLOB_ID(pB) = BlobID;
                    }
                    else
                    { /* delete this hypothesis */
                        pBT->pBlobHyp->DelBlob(h);
                        h--;
                        hN--;
                    }
                }/* next hyp*/
            }/* track all hypothesis */
        }/* track next blob */

        m_ClearHyp = 1;

    }/* Process */

    virtual void ProcessBlob(int BlobIndex, CvBlob* pBlob, IplImage* /*pImg*/, IplImage* /*pImgFG*/ = NULL)
    {
        int             ID = pBlob->ID;
        CvBlob*         pB = m_BlobList.GetBlob(BlobIndex);
        DefBlobTracker* pBT = (DefBlobTracker*)pB;
        //CvBlob*         pBBest = NULL;
        //double          DistBest = -1;
        int             BlobID;
        
        if(pB==NULL) return;
        
        BlobID = pB->ID;

        if(m_Collision && pBT->Collision)
        {/* tracking in collision */
            pB[0]=pBT->BlobPredict;
            CV_BLOB_ID(pB)=BlobID;
        }/* tracking in collision */
        else
        {/* not collision tracking */
            CvBlob* pBBest = GetNearestBlob(pB);
            if(pBBest)
            {
                float   w = pBlob->w*(1-m_AlphaSize)+m_AlphaSize*pBBest->w;
                float   h = pBlob->h*(1-m_AlphaSize)+m_AlphaSize*pBBest->h;
                float   x = pBlob->x*(1-m_AlphaPos)+m_AlphaPos*pBBest->x;
                float   y = pBlob->y*(1-m_AlphaPos)+m_AlphaPos*pBBest->y;
                pB->w = w;
                pB->h = h;
                pB->x = x;
                pB->y = y;
                CV_BLOB_ID(pB) = BlobID;
            }
        }/* not collision tracking */

        pBlob[0] = pB[0];
        pBlob->ID = ID;
    };
    
    virtual double  GetConfidence(int BlobIndex, CvBlob* pBlob, IplImage* /*pImg*/, IplImage* pImgFG = NULL)
    {
        /* define koefficients in exp by exp(-XT*K)=VT */
        static double _KS = -log(0.1)/pow(0.5,2); /* XT = 1, VT = 0.1 - when size is Larger in 2 times Confidence is smoller in 10 times */
        static double _KP = -log(0.1)/pow(m_pImg->width*0.02,2); /* XT = 0.02*ImgWidth, VT = 0.1*/
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIndex);
        float   dx,dy,dw,dh;
        float   dp2,ds2;
        double  W = 1;
        CvBlob* pBC = GetNearestBlob(pBlob);
        if(pBC == NULL ) return 0;

        dx = pBC->x-pBlob->x;
        dy = pBC->y-pBlob->y;
        dw = (pBC->w-pBlob->w)/pBC->w;
        dh = (pBC->h-pBlob->h)/pBC->h;

        dp2 = dx*dx+dy*dy;
        ds2 = dw*dw+dh*dh;

        if(!pBT->Collision) 
        { /* Confidence for size by nearest blob */
            W*=exp(-_KS*ds2);
        }
        
        if(m_ConfidenceType==0 && !pBT->Collision) 
        { /* confinence by nearest blob */
            W*=exp(-_KP*dp2);
        }

        if(m_ConfidenceType==1 && pBT->AverFG>0)
        {/* calc summ of mask */
            float   Aver = CalcAverageMask(pBlob, pImgFG );
            if(Aver < pBT->AverFG)
            {
                float diff = 1+0.9f*(Aver-pBT->AverFG)/pBT->AverFG;
                if(diff < 0.1f) diff = 0.1f;
                W *= diff;
            }
        }/* calc summ of mask */

        if(m_ConfidenceType==2)
        {/* calc BCoeff */
            float   S = 0.2f;
            float   Aver = CalcAverageMask(pBlob, pImgFG );
            double B = sqrt(Aver*pBT->AverFG)+sqrt((1-Aver)*(1-pBT->AverFG));
            
            W *= exp((B-1)/(2*S));
        }/* calc summ of mask */
        return W;
    };

    virtual void UpdateBlob(int BlobIndex, CvBlob* /*pBlob*/, IplImage* /*pImg*/, IplImage* pImgFG = NULL)
    {
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIndex);
        
        if(pImgFG==NULL || pBT==NULL) return;
        
        if(!pBT->Collision)
        {
        //pBT->AverFG = pBT->AverFG * (1-m_Alpha) + m_Alpha * CalcAverageMask(pBlob,pImgFG);
        }
    };

    virtual void ParamUpdate()
    {
        char*   pCT[3] = {"NearestBlob","AverFG","BC"};
        int     i;                
        
        CvBlobTracker::ParamUpdate();
        
        for(i=0;i<3;++i)
        {
            if(cv_stricmp(m_ConfidenceTypeStr,pCT[i])==0)
            {
                m_ConfidenceType = i;
            }
        }
        SetParamStr("ConfidenceType",pCT[m_ConfidenceType]);
    }
    /*  ===============  MULTI HYPOTHESIS INTERFACE ==================  */
    /* return number of position hyposetis of currently tracked blob */
    virtual int     GetBlobHypNum(int BlobIdx)
    {
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIdx);
        assert(pBT->pBlobHyp);
        return pBT->pBlobHyp->GetBlobNum();
    };/* CvBlobtrackerList::GetBlobHypNum() */

    /* return pointer to specified blob hypothesis by index blob */
    virtual CvBlob* GetBlobHyp(int BlobIndex, int hypothesis)
    {
        DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIndex);
        assert(pBT->pBlobHyp);
        return pBT->pBlobHyp->GetBlob(hypothesis);
    };/* CvBlobtrackerList::GetBlobHyp() */
    /* Set new parameters for specified (by index) blob hyp (can be called several times for each hyp )*/
    virtual void    SetBlobHyp(int BlobIndex, CvBlob* pBlob)
    {
        if(m_ClearHyp)
        {/* clear all hypothesis */
            int b, bN = m_BlobList.GetBlobNum();
            for(b=0;b<bN;++b)
            {
                DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(b);
                assert(pBT->pBlobHyp);
                pBT->pBlobHyp->Clear();
            }
            m_ClearHyp = 0;
        }
        {/* add hypothesis */
            DefBlobTracker* pBT = (DefBlobTracker*)m_BlobList.GetBlob(BlobIndex);
            assert(pBT->pBlobHyp);
            pBT->pBlobHyp->AddBlob(pBlob);
        }
    };

private:
    CvBlob* GetNearestBlob(CvBlob* pB)
    {
        //DefBlobTracker* pBT = (DefBlobTracker*)pB;
        CvBlob*         pBBest = NULL;
        double          DistBest = -1;
        int             j,BlobID;
        
        if(pB==NULL) return NULL;
        
        BlobID = pB->ID;

        for(j=m_BlobListNew.GetBlobNum();j>0;--j)
        {/* find best CC */
            double  Dist = -1;
            CvBlob* pBNew = m_BlobListNew.GetBlob(j-1);
            double  dx = fabs(CV_BLOB_X(pB)-CV_BLOB_X(pBNew));
            double  dy = fabs(CV_BLOB_Y(pB)-CV_BLOB_Y(pBNew));
            if(dx > 2*CV_BLOB_WX(pB) || dy > 2*CV_BLOB_WY(pB)) continue;

            Dist = sqrt(dx*dx+dy*dy);
            if(Dist < DistBest || pBBest == NULL)
            {
                DistBest = Dist;
                pBBest = pBNew;
            }
        }/* find best CC */
        return pBBest;
    }; /* GetNearestBlob */

};

CvBlobTracker* cvCreateBlobTrackerCC()
{
    return (CvBlobTracker*) new CvBlobTrackerCC;
}
/*============== BLOB TRACKERCC CLASS DECLARATION =============== */
