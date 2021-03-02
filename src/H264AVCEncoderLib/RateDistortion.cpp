#include "H264AVCEncoderLib.h"
#include "CodingParameter.h"
#include "math.h"
#include "RateDistortion.h"

namespace JSVM {


RateDistortion::RateDistortion() :
    m_uiCostFactorMotionSAD(0),
    m_uiCostFactorMotionSSE(0)
{
}

RateDistortion::~RateDistortion()
{
}

ErrVal RateDistortion::create(RateDistortion *&rpcRateDistortion)
{
    rpcRateDistortion = new RateDistortion;

    ROT(NULL == rpcRateDistortion);

    return Err::m_nOK;
}

//根据位数 与 distort值, 四舍五入, 计算Cost
Double RateDistortion::getCost(UInt uiBits, UInt uiDistortion)
{
    Double d = ((Double)uiDistortion + (Double)(Int)uiBits * m_dCost+.5);
    return (Double)(UInt)floor(d);
}

//根据位数 与 distort值, 计算FCost
Double RateDistortion::getFCost(UInt uiBits, UInt uiDistortion)
{
    Double d = (((Double)uiDistortion) + ((Double)uiBits * m_dCost));
    return d;
}


ErrVal RateDistortion::destroy()
{
    delete this;
    return Err::m_nOK;
}


//修正宏块QP值
ErrVal RateDistortion::fixMacroblockQP(MbDataAccess& rcMbDataAccess)
{
    if(!(rcMbDataAccess.getMbData().getMbCbp() || rcMbDataAccess.getMbData().isIntra16x16())) // has no coded texture
    {
        rcMbDataAccess.getMbData().setQp(rcMbDataAccess.getLastQp());
    }
    return Err::m_nOK;
}


ErrVal RateDistortion::setMbQpLambda(MbDataAccess& rcMbDataAccess, UInt uiQp, Double dLambda)
{
    rcMbDataAccess.getMbData().setQp(uiQp);

    //lambda 就是cost值
    m_dCost = dLambda;
    //平方根
    m_dSqrtCost = sqrt(dLambda);
    //SAD
    m_uiCostFactorMotionSAD = (UInt)floor(65536.0 * sqrt(m_dCost));
    //SSE
    m_uiCostFactorMotionSSE = (UInt)floor(65536.0 * m_dCost);

    return Err::m_nOK;
}


}  //namespace JSVM {
