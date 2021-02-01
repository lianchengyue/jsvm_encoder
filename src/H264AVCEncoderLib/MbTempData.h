#ifndef _MBTEMPDATA_H_)
#define _MBTEMPDATA_H_

#include "H264AVCCommonLib/YuvMbBuffer.h"


namespace JSVM {



class IntMbTempData :
public CostData
, public MbData
, public MbTransformCoeffs
, public YuvMbBuffer
{
public:
    IntMbTempData();
    virtual ~IntMbTempData();

    ErrVal  init(MbDataAccess& rcMbDataAccess);

    ErrVal  uninit();

    Void    clear();
    Void    clearCost();

    UInt&   cbp() { return m_uiMbCbp; }

    Void    copyTo(MbDataAccess&  rcMbDataAccess);
    Void    loadChromaData(IntMbTempData&  rcMbTempData);

    Void    copyResidualDataTo(MbDataAccess&   rcMbDataAccess);

    MbMotionData& getMbMotionData (ListIdx eLstIdx) { return m_acMbMotionData[eLstIdx ]; }
    MbMvData&     getMbMvdData (ListIdx eLstIdx)    { return m_acMbMvdData[eLstIdx ]; }
    YuvMbBuffer&  getTempYuvMbBuffer ()             { return m_cTempYuvMbBuffer; }
    YuvMbBuffer&  getTempBLSkipResBuffer ()         { return m_cTempBLSkipResBuffer; }
    MbDataAccess& getMbDataAccess ()                { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
    const SliceHeader&  getSH () const              { AOF_DBG(m_pcMbDataAccess); return m_pcMbDataAccess->getSH(); }
    const CostData&  getCostData () const           { return *this; }
    operator MbDataAccess& ()                       { AOF_DBG(m_pcMbDataAccess); return *m_pcMbDataAccess; }
    operator YuvMbBuffer* ()                        { return this; }

protected:
    MbDataAccess* m_pcMbDataAccess;
    MbMvData      m_acMbMvdData[2];
    MbMotionData  m_acMbMotionData[2];
    YuvMbBuffer   m_cTempYuvMbBuffer;
    YuvMbBuffer   m_cTempBLSkipResBuffer;
};


}  //namespace JSVM {


#endif //_MBTEMPDATA_H_
