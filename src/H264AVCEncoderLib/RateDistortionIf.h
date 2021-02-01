#ifndef _RATEDISTORTIONIF_H_
#define _RATEDISTORTIONIF_H_

#include "Typedefs.h"

namespace JSVM {


class RateDistortionIf
{
protected:
    RateDistortionIf() {}
    virtual ~RateDistortionIf() {}

public:
    virtual ErrVal setMbQpLambda (MbDataAccess& rcMbDataAccess, UInt uiQp, Double dLambda) = 0;

    virtual Double getCost (UInt uiBits, UInt uiDistortion) = 0;
    virtual Double getFCost (UInt uiBits, UInt uiDistortion) = 0;
    virtual UInt   getMotionCostShift (Bool bSad ) = 0;

    virtual ErrVal fixMacroblockQP (MbDataAccess& rcMbDataAccess) = 0;
};


}  //namespace JSVM {


#endif //_RATEDISTORTIONIF_H_
