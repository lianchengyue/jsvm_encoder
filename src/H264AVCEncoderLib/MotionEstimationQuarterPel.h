#ifndef _MOTIONESTIMATIONQUARTERPEL_H_
#define _MOTIONESTIMATIONQUARTERPEL_H_


#include "MotionEstimation.h"
#define X1 24


namespace JSVM {

class MotionEstimationQuarterPel :
public MotionEstimation
{
protected:
    MotionEstimationQuarterPel();
    virtual ~MotionEstimationQuarterPel();

public:
    static ErrVal create(MotionEstimation*& rpcMotionEstimation);

    ErrVal compensateBlock(YuvMbBuffer *pcRecPelData, UInt uiBlk, UInt uiMode, YuvMbBuffer *pcRefPelData2 = NULL);

    Void xSubPelSearch(YuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiBlk, UInt uiMode);

protected:
    Void xCompensateBlocksHalf (XPel *pPelDes, YuvPicBuffer *pcRefPelData, Mv cMv, UInt uiMode, UInt uiYSize, UInt uiXSize);
    Void xGetSizeFromMode (UInt& ruiXSize, UInt& ruiYSize, UInt uiMode);
    Void xInitBuffer();

protected:
    UInt m_uiBestMode;
    XPel m_aXHPelSearch[17*X1*4];
    XPel m_aXQPelSearch[16*16*9];
    XPel *m_apXHPelSearch[9];
    XPel *m_apXQPelSearch[9];
};


}  //namespace JSVM {


#endif //_MOTIONESTIMATIONQUARTERPEL_H_
