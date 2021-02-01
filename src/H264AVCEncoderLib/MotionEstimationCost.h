#ifndef _MOTIONESTIMATIONCOST_H_
#define _MOTIONESTIMATIONCOST_H_

#include "RateDistortionIf.h"
#include <vector>

namespace JSVM {

class MotionEstimationCost
{
protected:
    MotionEstimationCost();
    virtual ~MotionEstimationCost();

    ErrVal  xInit (const Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf);
    ErrVal  xUninit ();

    ErrVal  xSetMEPars (const UInt uiMvScaleShift, const Bool bSad);
    ErrVal  xSetPredictor (const Mv&  rcMv);

    UInt  xGetBits(Int iMvX, Int iMvY) const { return m_puiHorCost[iMvX << m_uiMvScaleShift] + m_puiVerCost[iMvY << m_uiMvScaleShift]; }
    UInt  xGetCost(UInt uiBits) const        { return (m_uiCostFactor * uiBits) >> 16; }
    UInt  xGetCost(Int iMvX, Int iMvY) const { return (m_uiCostFactor * xGetBits(iMvX, iMvY)) >> 16; }
    UInt  xGetCost(Mv& rcMv ) const          { return (m_uiCostFactor * xGetBits(rcMv.getHor(), rcMv.getVer())) >> 16; }

private:
    UInt  xGetComponentBits(Int iPos) const;

protected:
    RateDistortionIf* m_pcRateDistortionIf;
    UInt*  m_puiComponentCostAlloc;
    UInt*  m_puiComponentCost;
    UInt*  m_puiHorCost;
    UInt*  m_puiVerCost;
    UInt   m_uiMvScaleShift;
    UInt   m_uiCostFactor;
    Int    m_iSubPelSearchLimit;
};

}  //namespace JSVM {


#endif //_MOTIONESTIMATIONCOST_H_
