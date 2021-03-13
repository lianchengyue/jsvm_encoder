#ifndef  _PIC_ENCODER_INCLUDED_
#define  _PIC_ENCODER_INCLUDED_

#include "InputPicBuffer.h"
#include "SequenceStructure.h"
#include "RecPicBuffer.h"

namespace JSVM {

class CodingParameter;
class ParameterSetMng;
class SliceEncoder;
class LoopFilter;
class PocCalculator;
class NalUnitEncoder;
class YuvBufferCtrl;
class QuarterPelFilter;
class MotionEstimation;
class ControlMngIf;


class PicEncoder
{
public:
    PicEncoder();
    virtual ~PicEncoder();

    static ErrVal   create(PicEncoder*&  rpcPicEncoder);
    ErrVal  destroy();

    ErrVal  init(CodingParameter*   pcCodingParameter,
                 ControlMngIf*      pcControlMng,
                 SliceEncoder*      pcSliceEncoder,
                 LoopFilter*        pcLoopFilter,
                 PocCalculator*     pcPocCalculator,
                 NalUnitEncoder*    pcNalUnitEncoder,
                 YuvBufferCtrl*     pcYuvBufferCtrlFullPel,
                 YuvBufferCtrl*     pcYuvBufferCtrlHalfPel,
                 QuarterPelFilter*  pcQuarterPelFilter,
                 MotionEstimation*  pcMotionEstimation);
    ErrVal  uninit();

    ErrVal  writeAndInitParameterSets(ExtBinDataAccessor*  pcExtBinDataAccessor,
                                      Bool&  rbMoreSets);
    ErrVal  process(PicBuffer*  pcInputPicBuffer,
                    PicBufferList&  rcOutputList,
                    PicBufferList&  rcUnusedList,
                    ExtBinDataAccessorList&  rcExtBinDataAccessorList);
    ErrVal  finish(PicBufferList&  rcOutputList,
                   PicBufferList&  rcUnusedList);

private:
    //===== initializations =====
    ErrVal  xInitSPS();
    ErrVal  xInitPPS();
    ErrVal  xInitParameterSets();
    ErrVal  xInitSliceHeader(SliceHeader*&  rpcSliceHeader,
                             const FrameSpec&  rcFrameSpec,
                             Double&  dLambda);
    ErrVal  xInitPredWeights(SliceHeader&  rcSliceHeader);

    //===== create and delete memory =====
    ErrVal  xCreateData();
    ErrVal  xDeleteData();

    //===== packet management =====
    ErrVal  xInitExtBinDataAccessor(ExtBinDataAccessor&  rcExtBinDataAccessor);
    ErrVal  xAppendNewExtBinDataAccessor(ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                         ExtBinDataAccessor* pcExtBinDataAccessor);

    //===== encoding =====
    ErrVal  xStartPicture(RecPicBufUnit& rcRecPicBufUnit,
                          SliceHeader&   rcSliceHeader,
                          RefFrameList&  rcList0,
                          RefFrameList&  rcList1);
    ErrVal  xEncodePicture(ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                           RecPicBufUnit&  rcRecPicBufUnit,
                           SliceHeader&    rcSliceHeader,
                           Double dLambda,
                           UInt&  ruiBits);
    ErrVal  xFinishPicture(RecPicBufUnit&  rcRecPicBufUnit,
                           SliceHeader&    rcSliceHeader,
                           RefFrameList&   rcList0,
                           RefFrameList&   rcList1,
                           UInt  uiBits);
    ErrVal  xGetPSNR(RecPicBufUnit&  rcRecPicBufUnit,
                     Double*  adPSNR);

private:
    Bool  m_bInit;
    Bool  m_bInitParameterSets;

    //===== members =====
    BinData                m_cBinData;
    //单独的一帧.h264,结束后会push进rcExtBinDataAccessorList
    ExtBinDataAccessor     m_cExtBinDataAccessor;
    FrameSpec              m_cFrameSpecification;
    SequenceStructure*     m_pcSequenceStructure;
    InputPicBuffer*        m_pcInputPicBuffer;
    SequenceParameterSet*  m_pcSPS;
    PictureParameterSet*   m_pcPPS;
    RecPicBuffer*          m_pcRecPicBuffer;

    //===== references =====
    CodingParameter*  m_pcCodingParameter;
    ControlMngIf*     m_pcControlMng;
    SliceEncoder*     m_pcSliceEncoder;
    LoopFilter*       m_pcLoopFilter;
    PocCalculator*    m_pcPocCalculator;
    NalUnitEncoder*   m_pcNalUnitEncoder;
    //像素级别
    YuvBufferCtrl*    m_pcYuvBufferCtrlFullPel;
    //1/2亚像素
    YuvBufferCtrl*    m_pcYuvBufferCtrlHalfPel;
    //1/4亚像素
    QuarterPelFilter*  m_pcQuarterPelFilter;
    MotionEstimation*  m_pcMotionEstimation;

    //===== fixed coding parameters =====
    UInt  m_uiFrameWidthInMb;
    UInt  m_uiFrameHeightInMb;
    UInt  m_uiMbNumber;

    //===== variable parameters =====
    UInt  m_uiFrameNum;
    UInt  m_uiIdrPicId;
    UInt  m_uiWrittenBytes;
    UInt  m_uiCodedFrames;
    Double  m_dSumYPSNR;
    Double  m_dSumUPSNR;
    Double  m_dSumVPSNR;

    //===== auxiliary buffers =====
    UInt m_uiWriteBufferSize;  // size of temporary write buffer
    UChar* m_pucWriteBuffer;  // write buffer

    Bool m_bTraceEnable;
};


}  //namespace JSVM {


#endif //_PIC_ENCODER_INCLUDED_
