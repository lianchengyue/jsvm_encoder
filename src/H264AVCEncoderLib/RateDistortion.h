#ifndef _RATEDISTORTION_H_
#define _RATEDISTORTION_H_


#include "RateDistortionIf.h"
#include "Typedefs.h"

namespace JSVM {

class CodingParameter;



class RateDistortion : public RateDistortionIf
{
protected:
    RateDistortion();
    virtual ~RateDistortion();

public:
    virtual ErrVal  setMbQpLambda( MbDataAccess& rcMbDataAccess, UInt uiQp, Double dLambda);

    static  ErrVal create( RateDistortion *&rpcRateDistortion);
    virtual ErrVal destroy();

    Double  getCost( UInt uiBits, UInt uiDistortion);
    Double  getFCost( UInt uiBits, UInt uiDistortion);
    UInt    getMotionCostShift( Bool bSad) { return (bSad) ? m_uiCostFactorMotionSAD : m_uiCostFactorMotionSSE; }

    ErrVal  fixMacroblockQP( MbDataAccess& rcMbDataAccess);

protected:
    Double m_dCost;
    Double m_dSqrtCost;
    UInt m_uiCostFactorMotionSAD;
    UInt m_uiCostFactorMotionSSE;
};


}  //namespace JSVM {


#endif //_RATEDISTORTION_H_
