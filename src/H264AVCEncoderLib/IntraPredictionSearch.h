#ifndef _INTRAPREDICTIONSEARCH_H_
#define _INTRAPREDICTIONSEARCH_H_

#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "DistortionIf.h"


namespace JSVM {


class IntraPredictionSearch :
public IntraPrediction
{
protected:
    IntraPredictionSearch();
    virtual ~IntraPredictionSearch();

public:
    static ErrVal create(IntraPredictionSearch*& rpcIntraPredictionSearch);
    ErrVal destroy();

    ErrVal predictSLumaMb(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode,               Bool& rbValid);
    ErrVal predictSLumaBlock(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode, LumaIdx cIdx, Bool &rbValid);
    ErrVal predictSChromaBlock(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode,               Bool &rbValid);
    ErrVal predictSLumaBlock8x8(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode, B8x8Idx cIdx, Bool &rbValid);
};



}  //namespace JSVM {


#endif //_INTRAPREDICTIONSEARCH_H_
