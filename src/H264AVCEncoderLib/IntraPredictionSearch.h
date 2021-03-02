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

    //亮度16x16的4种Intra预测模式
    ErrVal predictSLumaMb(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode,               Bool& rbValid);
    //亮度4x4的9种Intra预测模式, block
    ErrVal predictSLumaBlock(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode, LumaIdx cIdx, Bool &rbValid);
    //色度8x8的4种Intra预测模式, block
    ErrVal predictSChromaBlock(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode,               Bool &rbValid);
    //(可选)亮度8x8的9种Intra预测模式
    ErrVal predictSLumaBlock8x8(YuvMbBuffer *pcYuvBuffer, UInt uiPredMode, B8x8Idx cIdx, Bool &rbValid);
};



}  //namespace JSVM {


#endif //_INTRAPREDICTIONSEARCH_H_
