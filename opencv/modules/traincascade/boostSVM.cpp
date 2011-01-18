#include "boostSVM.h"
#include "cascadeclassifier.h"

//----------------------------- CascadeBoostParams -------------------------------------------------

CvCascadeBoostSVMParams::CvCascadeBoostSVMParams() : minHitRate(0.995F), maxFalseAlarm(0.5F)
{
    //boost_type = CvBoost::GENTLE;
    boost_type = CvBoost::DISCRETE;
}

CvCascadeBoostSVMParams::CvCascadeBoostSVMParams(int boostType, float minHitRate, float maxFalseAlarm, 
                                                 double weightTrimRate, int maxWeakCount, double cost, TermCriteria termCrit)
                                                 : CvBoostSVMParams( boostType, maxWeakCount, weightTrimRate, cost, NULL, termCrit  )
{
    //boost_type = CvBoost::GENTLE;
    boost_type = CvBoost::DISCRETE;
    minHitRate = minHitRate;
    maxFalseAlarm = maxFalseAlarm;
}

void CvCascadeBoostSVMParams::write(FileStorage &fs) const
{
    String boostTypeStr = boost_type == CvBoost::DISCRETE ? CC_DISCRETE_BOOST : 
                          boost_type == CvBoost::REAL ? CC_REAL_BOOST :
                          boost_type == CvBoost::LOGIT ? CC_LOGIT_BOOST :
                          boost_type == CvBoost::GENTLE ? CC_GENTLE_BOOST : String();
    CV_Assert( !boostTypeStr.empty() );
    fs << CC_BOOST_TYPE << boostTypeStr;
    fs << CC_MINHITRATE << minHitRate;
    fs << CC_MAXFALSEALARM << maxFalseAlarm;
    fs << CC_TRIM_RATE << weight_trim_rate;
    fs << CC_WEAK_COUNT << weak_count;

    fs << CC_SVMCOST_FACTOR << C;
}

bool CvCascadeBoostSVMParams::read(const FileNode &node)
{
    String boostTypeStr;
    FileNode rnode = node[CC_BOOST_TYPE];
    rnode >> boostTypeStr;
    boost_type = !boostTypeStr.compare( CC_DISCRETE_BOOST ) ? CvBoost::DISCRETE :
                 !boostTypeStr.compare( CC_REAL_BOOST ) ? CvBoost::REAL :
                 !boostTypeStr.compare( CC_LOGIT_BOOST ) ? CvBoost::LOGIT :
                 !boostTypeStr.compare( CC_GENTLE_BOOST ) ? CvBoost::GENTLE : -1;
    if (boost_type == -1)
        CV_Error( CV_StsBadArg, "unsupported Boost type" );
    node[CC_MINHITRATE] >> minHitRate;
    node[CC_MAXFALSEALARM] >> maxFalseAlarm;
    node[CC_TRIM_RATE] >> weight_trim_rate ;
    node[CC_WEAK_COUNT] >> weak_count ;
    node[CC_MAX_DEPTH] >> C;
    if ( minHitRate <= 0 || minHitRate > 1 ||
         maxFalseAlarm <= 0 || maxFalseAlarm > 1 ||
         weight_trim_rate <= 0 || weight_trim_rate > 1 ||
         weak_count <= 0 || C < 0)
        CV_Error( CV_StsBadArg, "bad parameters range");
    return true;
}

void CvCascadeBoostSVMParams::printDefaults() const
{
    cout << "--boostSVMParams--" << endl;
    cout << "  [-bt <{" << CC_DISCRETE_BOOST << ", " 
                        << CC_REAL_BOOST << ", "
                        << CC_LOGIT_BOOST ", "
                        << CC_GENTLE_BOOST << "(default)}>]" << endl;
    cout << "  [-minHitRate <min_hit_rate> = " << minHitRate << ">]" << endl;
    cout << "  [-maxFalseAlarmRate <max_false_alarm_rate = " << maxFalseAlarm << ">]" << endl;
    cout << "  [-weightTrimRate <weight_trim_rate = " << weight_trim_rate << ">]" << endl;
    cout << "  [-maxWeakCount <max_weak_tree_count = " << weak_count << ">]" << endl;
    cout << "  [-C <SVM_cost_factor = " << C << ">]" << endl;
}

void CvCascadeBoostSVMParams::printAttrs() const
{
    String boostTypeStr = boost_type == CvBoost::DISCRETE ? CC_DISCRETE_BOOST : 
                          boost_type == CvBoost::REAL ? CC_REAL_BOOST :
                          boost_type == CvBoost::LOGIT  ? CC_LOGIT_BOOST :
                          boost_type == CvBoost::GENTLE ? CC_GENTLE_BOOST : String();
    CV_Assert( !boostTypeStr.empty() );
    cout << "boostType: " << boostTypeStr << endl;
    cout << "minHitRate: " << minHitRate << endl;
    cout << "maxFalseAlarmRate: " <<  maxFalseAlarm << endl;
    cout << "weightTrimRate: " << weight_trim_rate << endl;
    cout << "maxWeakCount: " << weak_count << endl;
    cout << "C: " << C << endl;
}

bool CvCascadeBoostSVMParams::scanAttr(const String prmName, const String val)
{
    bool res = true;

    if( !prmName.compare( "-bt" ) )
    {
        boost_type = !val.compare( CC_DISCRETE_BOOST ) ? CvBoost::DISCRETE :
                     !val.compare( CC_REAL_BOOST ) ? CvBoost::REAL :
                     !val.compare( CC_LOGIT_BOOST ) ? CvBoost::LOGIT :
                     !val.compare( CC_GENTLE_BOOST ) ? CvBoost::GENTLE : -1;
        if (boost_type == -1)
            res = false;
    }
    else if( !prmName.compare( "-minHitRate" ) )
    {
        minHitRate = (float) atof( val.c_str() );
    }
    else if( !prmName.compare( "-maxFalseAlarmRate" ) )
    {
        maxFalseAlarm = (float) atof( val.c_str() );
    }
    else if( !prmName.compare( "-weightTrimRate" ) )
    {
        weight_trim_rate = (float) atof( val.c_str() );
    }
    else if( !prmName.compare( "-C" ) )
    {
        C = atoi( val.c_str() );
    }
    else if( !prmName.compare( "-maxWeakCount" ) )
    {
        weak_count = atoi( val.c_str() );
    }
    else
    {
        res = false;
    }
    return res;
}





