#include "H264AVCEncoderLib.h"
#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/MbData.h"
#include "BitWriteBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"
#include "SliceEncoder.h"
#include "UvlcWriter.h"
#include "MbCoder.h"
#include "MbEncoder.h"
#include "IntraPredictionSearch.h"
#include "CodingParameter.h"
#include "CabacWriter.h"
#include "NalUnitEncoder.h"
#include "Distortion.h"
#include "MotionEstimation.h"
#include "MotionEstimationQuarterPel.h"
#include "RateDistortion.h"
#include "GOPEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "PicEncoder.h"
// JVT-V068 HRD {
#include "Scheduler.h"
#include "H264AVCCommonLib/SequenceParameterSet.h"
// JVT-V068 HRD }

namespace JSVM {



CreaterH264AVCEncoder::CreaterH264AVCEncoder():
    m_pcH264AVCEncoder  (NULL),
    m_pcPicEncoder      (NULL),
    m_pcSliceEncoder    (NULL),
    m_pcControlMng      (NULL),
    m_pcBitWriteBuffer  (NULL),
    m_pcNalUnitEncoder  (NULL),
    m_pcUvlcWriter      (NULL),
    m_pcUvlcTester      (NULL),
    m_pcMbCoder         (NULL),
    m_pcLoopFilter      (NULL),
    m_pcMbEncoder       (NULL),
    m_pcQuarterPelFilter(NULL),
    m_pcCodingParameter (NULL),
    m_pcParameterSetMng (NULL),
    m_pcSampleWeighting (NULL),
    m_pcCabacWriter     (NULL),
    m_pcXDistortion     (NULL),
    m_pcMotionEstimation(NULL),
    m_pcRateDistortion  (NULL),
    m_pcHistory         (NULL),
    m_bTraceEnable      (true)
{
    ::memset(m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*));
    ::memset(m_apcYuvHalfPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*));
    ::memset(m_apcPocCalculator,        0x00, MAX_LAYERS*sizeof(Void*));
    ::memset(m_apcLayerEncoder,         0x00, MAX_LAYERS*sizeof(Void*));
    m_pcReconstructionBypass = NULL;
}


CreaterH264AVCEncoder::~CreaterH264AVCEncoder()
{

}

Bool CreaterH264AVCEncoder::getScalableSeiMessage()
{
    return m_pcH264AVCEncoder->bGetScalableSeiMessage();
}

Void CreaterH264AVCEncoder::SetVeryFirstCall()
{
    m_pcH264AVCEncoder->SetVeryFirstCall();
}

ErrVal CreaterH264AVCEncoder::writeParameterSets (ExtBinDataAccessor* pcExtBinDataAccessor,
                                                  SequenceParameterSet*&  rpcAVCSPS,
                                                  Bool&  rbMoreSets)
{
    //AVC模式
    if(m_pcCodingParameter->getAVCmode())
    {
        //初始化SPS与PPS的入口
        m_pcPicEncoder->writeAndInitParameterSets(pcExtBinDataAccessor, rbMoreSets);
        m_pcH264AVCEncoder->setScalableSEIMessage(); // due to Nokia's (Ye-Kui's) weird implementation
        return Err::m_nOK;
    }
    //SVC模式
    else
    {
        //初始化SPS与PPS的入口
        m_pcH264AVCEncoder->writeParameterSets (pcExtBinDataAccessor, rpcAVCSPS, rbMoreSets);
        return Err::m_nOK;
    }
}


ErrVal CreaterH264AVCEncoder::process (ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                                       PicBuffer*  apcOriginalPicBuffer[MAX_LAYERS],
                                       PicBuffer*  apcReconstructPicBuffer[MAX_LAYERS],
                                       PicBufferList*  apcPicBufferOutputList,
                                       PicBufferList*  apcPicBufferUnusedList)
{
    //Step 3: 开始编码
    //AVC模式
    if (m_pcCodingParameter->getAVCmode())
    {
        apcPicBufferUnusedList->push_back (apcReconstructPicBuffer[0]);
        m_pcPicEncoder->process (apcOriginalPicBuffer[0],
                                 *apcPicBufferOutputList,
                                 *apcPicBufferUnusedList,
                                 rcExtBinDataAccessorList);
        return Err::m_nOK;
    }

    //SVC模式
    else
    {
        m_pcH264AVCEncoder->process (rcExtBinDataAccessorList,
                                     apcOriginalPicBuffer,
                                     apcReconstructPicBuffer,
                                     apcPicBufferOutputList,
                                     apcPicBufferUnusedList);
        return Err::m_nOK;
    }
}


ErrVal CreaterH264AVCEncoder::finish (ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                                      PicBufferList*  apcPicBufferOutputList,
                                      PicBufferList*  apcPicBufferUnusedList,
                                      UInt&   ruiNumCodedFrames,
                                      Double& rdHighestLayerOutputRate)
{
    //StepEnd: 结束编码
    //AVC模式
    if(m_pcCodingParameter->getAVCmode())
    {
        m_pcPicEncoder->finish (*apcPicBufferOutputList,
                                *apcPicBufferUnusedList);
        return Err::m_nOK;
    }

    //SVC模式
    else
    {
        m_pcH264AVCEncoder->finish (rcExtBinDataAccessorList,
                                    apcPicBufferOutputList,
                                    apcPicBufferUnusedList,
                                    ruiNumCodedFrames,
                                    rdHighestLayerOutputRate);
        return Err::m_nOK;
    }
}


ErrVal CreaterH264AVCEncoder::create(CreaterH264AVCEncoder*& rpcCreaterH264AVCEncoder)
{
    rpcCreaterH264AVCEncoder = new CreaterH264AVCEncoder;
    ROT(NULL == rpcCreaterH264AVCEncoder);

    rpcCreaterH264AVCEncoder->xCreateEncoder();

    return Err::m_nOK;
}


ErrVal CreaterH264AVCEncoder::xCreateEncoder()
{
    //参数设置
    ParameterSetMng             ::create(m_pcParameterSetMng);
    //重定向输出结果
    BitWriteBuffer              ::create(m_pcBitWriteBuffer);
    BitCounter                  ::create(m_pcBitCounter);
    //NALU
    NalUnitEncoder              ::create(m_pcNalUnitEncoder);
    //Slice
    SliceEncoder                ::create(m_pcSliceEncoder);
    UvlcWriter                  ::create(m_pcUvlcWriter);
    UvlcWriter                  ::create(m_pcUvlcTester, false);
    CabacWriter                 ::create(m_pcCabacWriter);
    //MB
    MbCoder                     ::create(m_pcMbCoder);
    MbEncoder                   ::create(m_pcMbEncoder);
    //De-block
    LoopFilter                  ::create(m_pcLoopFilter);
    //帧内预测
    IntraPredictionSearch       ::create(m_pcIntraPrediction);
    //亚像素级运动估计
    MotionEstimationQuarterPel  ::create(m_pcMotionEstimation);

    //SVC编码器实例
    H264AVCEncoder              ::create(m_pcH264AVCEncoder);
    //AVC编码器实例
    PicEncoder                  ::create(m_pcPicEncoder);

    //encoder控制管理
    ControlMngH264AVCEncoder    ::create(m_pcControlMng);
    //bypass 重建
    ReconstructionBypass        ::create(m_pcReconstructionBypass);
    //1/4像素级Filter
    QuarterPelFilter            ::create(m_pcQuarterPelFilter);
    //transform
    Transform                   ::create(m_pcTransform);
    //权重
    SampleWeighting             ::create(m_pcSampleWeighting);
    //率失真
    XDistortion                 ::create(m_pcXDistortion);

    //SVC相关, 共8层
    for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    {
        LayerEncoder  ::create(m_apcLayerEncoder[uiLayer]);
        PocCalculator ::create(m_apcPocCalculator[uiLayer]);
        YuvBufferCtrl ::create(m_apcYuvFullPelBufferCtrl[uiLayer]);
        YuvBufferCtrl ::create(m_apcYuvHalfPelBufferCtrl[uiLayer]);
    }

    return Err::m_nOK;
}


ErrVal CreaterH264AVCEncoder::destroy()
{
    m_pcSliceEncoder        ->destroy();
    m_pcBitWriteBuffer      ->destroy();
    m_pcBitCounter          ->destroy();
    m_pcNalUnitEncoder      ->destroy();
    m_pcUvlcWriter          ->destroy();
    m_pcUvlcTester          ->destroy();
    m_pcMbCoder             ->destroy();
    m_pcLoopFilter          ->destroy();
    m_pcMbEncoder           ->destroy();
    m_pcTransform           ->destroy();
    m_pcIntraPrediction     ->destroy();
    m_pcQuarterPelFilter    ->destroy();
    m_pcCabacWriter         ->destroy();
    m_pcXDistortion         ->destroy();
    m_pcMotionEstimation    ->destroy();
    m_pcSampleWeighting     ->destroy();
    m_pcParameterSetMng     ->destroy();
    m_pcH264AVCEncoder      ->destroy();
    m_pcControlMng          ->destroy();
    m_pcReconstructionBypass->destroy();
    m_pcPicEncoder          ->destroy();

    if(NULL != m_pcRateDistortion)
    {
        m_pcRateDistortion->destroy();
    }

    for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    {
        m_apcLayerEncoder[uiLayer]->destroy();
        m_apcYuvFullPelBufferCtrl[uiLayer]->destroy();
        m_apcYuvHalfPelBufferCtrl[uiLayer]->destroy();
        m_apcPocCalculator[uiLayer]->destroy();
    }

    // JVT-V068 {
    for(UInt uiIndex = 0; uiIndex < MAX_SCALABLE_LAYERS; uiIndex++)
    {
        if (m_apcScheduler[uiIndex])
        {
            m_apcScheduler[uiIndex]->destroy();
        }
    }
    m_apcScheduler.setAll(0);
    // JVT-V068 }

    delete this;

    return Err::m_nOK;
}


ErrVal CreaterH264AVCEncoder::init (CodingParameter* pcCodingParameter)
{
    INIT_ETRACE;

    ROT(NULL == pcCodingParameter);

    m_pcCodingParameter = pcCodingParameter;

    RateDistortion::create(m_pcRateDistortion);

    m_pcBitWriteBuffer ->init();
    m_pcBitCounter     ->init();
    m_pcXDistortion    ->init();
    m_pcSampleWeighting->init();
    m_pcNalUnitEncoder ->init(m_pcBitWriteBuffer,
                              m_pcUvlcWriter,
                              m_pcUvlcTester);
    m_pcUvlcWriter ->init(m_pcBitWriteBuffer);
    m_pcUvlcTester ->init(m_pcBitCounter);
    m_pcCabacWriter->init(m_pcBitWriteBuffer);
    m_pcParameterSetMng->init();

    m_pcSliceEncoder->init (m_pcMbEncoder,
                            m_pcMbCoder,
                            m_pcControlMng,
                            m_pcCodingParameter,
                            m_apcPocCalculator[0],
                            m_pcTransform);
    m_pcReconstructionBypass ->init();

    ErrVal cRet = m_pcLoopFilter->init(m_pcControlMng, m_pcReconstructionBypass, true);
    RNOK(cRet);
    m_pcQuarterPelFilter->init();

    m_pcMbEncoder->init(m_pcTransform,
                        m_pcIntraPrediction,
                        m_pcMotionEstimation,
                        m_pcCodingParameter,
                        m_pcRateDistortion,
                        m_pcXDistortion);
    m_pcMotionEstimation->init(m_pcXDistortion,
                               m_pcCodingParameter,
                               m_pcRateDistortion,
                               m_pcQuarterPelFilter,
                               m_pcTransform,
                               m_pcSampleWeighting);

    m_pcControlMng->init(m_apcLayerEncoder,
                         m_pcSliceEncoder,
                         m_pcControlMng,
                         m_pcBitWriteBuffer,
                         m_pcBitCounter,
                         m_pcNalUnitEncoder,
                         m_pcUvlcWriter,
                         m_pcUvlcTester,
                         m_pcMbCoder,
                         m_pcLoopFilter,
                         m_pcMbEncoder,
                         m_pcTransform,
                         m_pcIntraPrediction,
                         m_apcYuvFullPelBufferCtrl,
                         m_apcYuvHalfPelBufferCtrl,
                         m_pcQuarterPelFilter,
                         m_pcCodingParameter,
                         m_pcParameterSetMng,
                         m_apcPocCalculator,
                         m_pcSampleWeighting,
                         m_pcCabacWriter,
                         m_pcXDistortion,
                         m_pcMotionEstimation,
                         m_pcRateDistortion);

    m_pcPicEncoder->init(m_pcCodingParameter,
                         m_pcControlMng,
                         m_pcSliceEncoder,
                         m_pcLoopFilter,
                         m_apcPocCalculator[0],
                         m_pcNalUnitEncoder,
                         m_apcYuvFullPelBufferCtrl[0],
                         m_apcYuvHalfPelBufferCtrl[0],
                         m_pcQuarterPelFilter,
                         m_pcMotionEstimation);
    m_pcH264AVCEncoder->init(m_apcLayerEncoder,
                             m_pcParameterSetMng,
                             m_apcPocCalculator[0],
                             m_pcNalUnitEncoder,
                             m_pcControlMng,
                             pcCodingParameter,
                             // JVT-V068 {
                             &m_apcScheduler
                             // JVT-V068 }
                             );

    for (UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++)
    {
        m_apcLayerEncoder[uiLayer]->init (m_pcCodingParameter,
                                          &m_pcCodingParameter->getLayerParameters(uiLayer),
                                          m_pcH264AVCEncoder,
                                          m_pcSliceEncoder,
                                          m_pcLoopFilter,
                                          m_apcPocCalculator[uiLayer],
                                          m_pcNalUnitEncoder,
                                          m_apcYuvFullPelBufferCtrl[uiLayer],
                                          m_apcYuvHalfPelBufferCtrl[uiLayer],
                                          m_pcQuarterPelFilter,
                                          m_pcMotionEstimation,
                                          m_pcReconstructionBypass,
                                          &m_apcScheduler);
    }

    //Bug_Fix JVT-R057{
    if(m_pcCodingParameter->getLARDOEnable())
    {
        Bool bFlag=false;
        for(UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++)
        {
            if(!m_apcLayerEncoder[uiLayer]->getLARDOEnable())
            {
               bFlag=true;
               break;
            }
        }
        if(bFlag)
        {
            for(UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++)
            {
                m_apcLayerEncoder[uiLayer]->setLARDOEnable(false);
            }
        }
    }
    //Bug_Fix JVT-R057}
    return Err::m_nOK;
}


ErrVal CreaterH264AVCEncoder::uninit()
{
    m_pcQuarterPelFilter    ->uninit();
    m_pcSampleWeighting     ->uninit();
    m_pcParameterSetMng     ->uninit();
    m_pcSliceEncoder        ->uninit();
    m_pcNalUnitEncoder      ->uninit();
    m_pcBitWriteBuffer      ->uninit();
    m_pcBitCounter          ->uninit();
    m_pcUvlcWriter          ->uninit();
    m_pcUvlcTester          ->uninit();
    m_pcMbCoder             ->uninit();
    m_pcLoopFilter          ->uninit();
    m_pcMbEncoder           ->uninit();
    m_pcIntraPrediction     ->uninit();
    m_pcMotionEstimation    ->uninit();
    m_pcCabacWriter         ->uninit();
    m_pcMotionEstimation    ->uninit();
    m_pcXDistortion         ->uninit();
    m_pcH264AVCEncoder      ->uninit();
    m_pcControlMng          ->uninit();
    m_pcReconstructionBypass->uninit();
    m_pcPicEncoder          ->uninit();

    for(UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++)
    {
        m_apcLayerEncoder[uiLayer]->uninit();
        m_apcYuvFullPelBufferCtrl[uiLayer]->uninit();
        m_apcYuvHalfPelBufferCtrl[uiLayer]->uninit();
    }

    UNINIT_ETRACE;

    return Err::m_nOK;
}

// JVT-W043 {
CodingParameter* CreaterH264AVCEncoder::getCodingParameter (void)
{
    return  m_pcCodingParameter;
}
// JVT-W043 }
// JVT-V068 {
ErrVal CreaterH264AVCEncoder::writeAVCCompatibleHRDSEI (ExtBinDataAccessor* pcExtBinDataAccessor, SequenceParameterSet* pcAVCSPS)
{
    m_pcH264AVCEncoder->writeAVCCompatibleHRDSEI (pcExtBinDataAccessor, *pcAVCSPS);
    return Err::m_nOK;
}
// JVT-V068 }

}  //namespace JSVM {
