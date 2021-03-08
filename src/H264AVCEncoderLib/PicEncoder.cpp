#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/CFMO.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "CodingParameter.h"
#include "PicEncoder.h"
#include "RecPicBuffer.h"
#include "NalUnitEncoder.h"
#include "SliceEncoder.h"


namespace JSVM {


PicEncoder::PicEncoder() :
    m_bInit                   (false),
    m_bInitParameterSets      (false),
    m_pcSequenceStructure     (NULL),
    m_pcInputPicBuffer        (NULL),
    m_pcSPS                   (NULL),
    m_pcPPS                   (NULL),
    m_pcRecPicBuffer          (NULL),
    m_pcCodingParameter       (NULL),
    m_pcControlMng            (NULL),
    m_pcSliceEncoder          (NULL),
    m_pcLoopFilter            (NULL),
    m_pcPocCalculator         (NULL),
    m_pcNalUnitEncoder        (NULL),
    m_pcYuvBufferCtrlFullPel  (NULL),
    m_pcYuvBufferCtrlHalfPel  (NULL),
    m_pcQuarterPelFilter      (NULL),
    m_pcMotionEstimation      (NULL),
    m_uiFrameWidthInMb        (0),
    m_uiFrameHeightInMb       (0),
    m_uiMbNumber              (0),
    m_uiFrameNum              (0),
    m_uiIdrPicId              (0),
    m_uiCodedFrames           (0),
    m_dSumYPSNR               (0.0),
    m_dSumUPSNR               (0.0),
    m_dSumVPSNR               (0.0),
    m_uiWrittenBytes          (0),
    m_uiWriteBufferSize       (0),
    m_pucWriteBuffer          (NULL),
    m_bTraceEnable            (true)
{
}


PicEncoder::~PicEncoder()
{
  uninit();
}


ErrVal PicEncoder::create(PicEncoder*& rpcPicEncoder)
{
    rpcPicEncoder = new PicEncoder;
    ROF(rpcPicEncoder);
    return Err::m_nOK;
}

ErrVal PicEncoder::destroy()
{
    delete this;
    return Err::m_nOK;
}

ErrVal PicEncoder::init (CodingParameter*    pcCodingParameter,
                         ControlMngIf*       pcControlMng,
                         SliceEncoder*       pcSliceEncoder,
                         LoopFilter*         pcLoopFilter,
                         PocCalculator*      pcPocCalculator,
                         NalUnitEncoder*     pcNalUnitEncoder,
                         YuvBufferCtrl*      pcYuvBufferCtrlFullPel,
                         YuvBufferCtrl*      pcYuvBufferCtrlHalfPel,
                         QuarterPelFilter*   pcQuarterPelFilter,
                         MotionEstimation*   pcMotionEstimation)
{
    ROF(pcCodingParameter    );
    ROF(pcControlMng         );
    ROF(pcSliceEncoder       );
    ROF(pcLoopFilter         );
    ROF(pcPocCalculator      );
    ROF(pcNalUnitEncoder     );
    ROF(pcYuvBufferCtrlFullPel);
    ROF(pcYuvBufferCtrlHalfPel);
    ROF(pcQuarterPelFilter   );
    ROF(pcMotionEstimation   );

    m_pcCodingParameter       = pcCodingParameter;
    m_pcControlMng            = pcControlMng;
    m_pcSliceEncoder          = pcSliceEncoder;
    m_pcLoopFilter            = pcLoopFilter;
    m_pcPocCalculator         = pcPocCalculator;
    m_pcNalUnitEncoder        = pcNalUnitEncoder;
    m_pcYuvBufferCtrlFullPel  = pcYuvBufferCtrlFullPel;
    m_pcYuvBufferCtrlHalfPel  = pcYuvBufferCtrlHalfPel;
    m_pcQuarterPelFilter      = pcQuarterPelFilter;
    m_pcMotionEstimation      = pcMotionEstimation;

    //----- create objects -----
    RecPicBuffer      ::create(m_pcRecPicBuffer);
    SequenceStructure ::create(m_pcSequenceStructure);
    InputPicBuffer    ::create(m_pcInputPicBuffer);

    //----- init objects -----
    m_pcRecPicBuffer->init(m_pcYuvBufferCtrlFullPel, m_pcYuvBufferCtrlHalfPel);
    m_pcInputPicBuffer->init();

    m_pcSliceEncoder->getMbEncoder()->setLowComplexMbEnable(0, false);

    //----- init parameters -----
    m_uiWrittenBytes          = 0;
    m_uiCodedFrames           = 0;
    m_dSumYPSNR               = 0.0;
    m_dSumVPSNR               = 0.0;
    m_dSumUPSNR               = 0.0;
    m_bInit                   = true;

    return Err::m_nOK;
}


ErrVal PicEncoder::uninit()
{
    //----- free allocated memory -----
    xDeleteData();

    //----- free allocated member -----
    if(m_pcRecPicBuffer)
    {
        m_pcRecPicBuffer->uninit();
        m_pcRecPicBuffer->destroy();
    }
    if(m_pcSPS)
    {
        m_pcSPS->destroy();
    }
    if(m_pcPPS)
    {
        m_pcPPS->destroy();
    }
    if(m_pcSequenceStructure)
    {
        m_pcSequenceStructure->uninit();
        m_pcSequenceStructure->destroy();
    }
    if(m_pcInputPicBuffer)
    {
        m_pcInputPicBuffer->uninit();
        m_pcInputPicBuffer->destroy();
    }

    m_pcRecPicBuffer      = NULL;
    m_pcSequenceStructure = NULL;
    m_pcInputPicBuffer    = NULL;
    m_pcSPS               = NULL;
    m_pcPPS               = NULL;
    m_bInitParameterSets  = false;
    m_bInit               = false;

    return Err::m_nOK;
}

///AVC模式专用
//对NAL的三连:
//m_pcNalUnitEncoder->initNalUnit ()
//m_pcNalUnitEncoder->write       ()
//m_pcNalUnitEncoder->closeNalUnit()
ErrVal PicEncoder::writeAndInitParameterSets(ExtBinDataAccessor* pcExtBinDataAccessor,
                                             Bool&  rbMoreSets)
{
    if(!m_pcSPS)
    {
        //===== create SPS =====
        xInitSPS();

        //===== write SPS =====
        UInt uiSPSBits = 0;
        //! flq, pcExtBinDataAccessor: 在这里对bin进行初始化
        m_pcNalUnitEncoder->initNalUnit (pcExtBinDataAccessor);

        ///写SPS到 TraceEncoder_DQId000.txt + nal_ref_idc赋值 + nal_unit_type赋值
        m_pcNalUnitEncoder->write (*m_pcSPS);
        m_pcNalUnitEncoder->closeNalUnit (uiSPSBits);
        m_uiWrittenBytes += ((uiSPSBits >> 3) + 4);
    }
    else if(!m_pcPPS)
    {
        //===== create PPS =====
        xInitPPS();

        //===== write PPS =====
        UInt uiPPSBits = 0;
        m_pcNalUnitEncoder->initNalUnit(pcExtBinDataAccessor);

        ///写PPS到 TraceEncoder_DQId000.txt + nal_ref_idc赋值 + nal_unit_type赋值
        m_pcNalUnitEncoder->write (*m_pcPPS);
        m_pcNalUnitEncoder->closeNalUnit (uiPPSBits);
        m_uiWrittenBytes += ((uiPPSBits >> 3) + 4);

        //===== init pic encoder with parameter sets =====
        xInitParameterSets();
    }

    //===== set status =====
    rbMoreSets = !m_bInitParameterSets;

    return Err::m_nOK;
}


ErrVal PicEncoder::process (PicBuffer*  pcInputPicBuffer,  //读取到的yuv
                            PicBufferList&  rcOutputList,
                            PicBufferList&  rcUnusedList,
                            ExtBinDataAccessorList&  rcExtBinDataAccessorList)  //输出的编码后的h264 bin
{
    ROF(m_bInitParameterSets);

    //===== add picture to input picture buffer =====
    m_pcInputPicBuffer->add(pcInputPicBuffer);

    //===== encode following access units that are stored in input picture buffer =====
    while(true)
    {
        InputAccessUnit* pcInputAccessUnit = NULL;

        //----- get next frame specification and input access unit -----
        if (!m_pcInputPicBuffer->empty())
        {
            if (!m_cFrameSpecification.isInitialized())
            {
                //更新m_cFrameSpecification, 其中包含m_eSliceType: I_SLICE, B_SLICE, P_SLICE
                m_cFrameSpecification = m_pcSequenceStructure->getNextFrameSpec();
            }
            pcInputAccessUnit = m_pcInputPicBuffer->remove (m_cFrameSpecification.getContFrameNumber());
        }

        //输入结束，退出循环
        if (!pcInputAccessUnit)
        {
            break;
        }

        //----- initialize picture -----
        Double          dLambda = 0;
        UInt            uiPictureBits = 0;
        SliceHeader*    pcSliceHeader = 0;
        RecPicBufUnit*  pcRecPicBufUnit = 0;
        PicBuffer*      pcOrigPicBuffer = pcInputAccessUnit->getInputPicBuffer();

        ///Step 3: important, 初始化一帧图像的Slice Header中的的参数值
        /***********************/
        /***初始化slice header***/
        /***********************/
        xInitSliceHeader (pcSliceHeader, m_cFrameSpecification, dLambda);
        //Rec到这里rcOutputList
#ifdef WITH_RECPIC
        m_pcRecPicBuffer->initCurrRecPicBufUnit(pcRecPicBufUnit,
                                                pcOrigPicBuffer,
                                                pcSliceHeader,
                                                rcOutputList,
                                                rcUnusedList);
#endif



        ///Step 4: 对一帧图像进行encode
        /***********************/
        /***对一帧图像进行encode**/
        /***********************/
        //----- encoding -----
        xEncodePicture (rcExtBinDataAccessorList,
                        *pcRecPicBufUnit,
                        *pcSliceHeader,
                        dLambda,
                        uiPictureBits);
        m_uiWrittenBytes += (uiPictureBits >> 3);



        ///Step 5: 保存输出后的h264文件
        ///----- store picture -----
#ifdef WITH_RECPIC
        m_pcRecPicBuffer->store (pcRecPicBufUnit, pcSliceHeader, rcOutputList, rcUnusedList);
#endif

        //----- reset -----
        delete pcInputAccessUnit;
        delete pcSliceHeader;
        m_cFrameSpecification.uninit();
    }

    return Err::m_nOK;
}


ErrVal PicEncoder::finish (PicBufferList&  rcOutputList,
                           PicBufferList&  rcUnusedList)
{
    m_pcRecPicBuffer->clear(rcOutputList, rcUnusedList);

    //===== output summary =====
    printf ("\n\n PicEncoder::finish"
            "\n %5d frames encoded:  Y %7.4lf dB  U %7.4lf dB  V %7.4lf dB\n"
            "\n     average bit rate:  %.4lf kbit/s  [%d byte for %.3lf sec]\n\n",
            m_uiCodedFrames,
            m_dSumYPSNR / (Double)m_uiCodedFrames,
            m_dSumUPSNR / (Double)m_uiCodedFrames,
            m_dSumVPSNR / (Double)m_uiCodedFrames,
            0.008 * (Double)m_uiWrittenBytes/(Double)m_uiCodedFrames*m_pcCodingParameter->getMaximumFrameRate(),
            m_uiWrittenBytes,
            (Double)m_uiCodedFrames/m_pcCodingParameter->getMaximumFrameRate()
            );

    return Err::m_nOK;
}


//为SPS赋值
ErrVal PicEncoder::xInitSPS()
{
    ROF(m_bInit);
    ROT(m_pcSPS);

    //===== determine parameters =====
    UInt uiSPSId    = 0;
    UInt uiMbX      = (m_pcCodingParameter->getFrameWidth () + 15) >> 4;
    UInt uiMbY      = (m_pcCodingParameter->getFrameHeight() + 15) >> 4;
    UInt uiOutFreq  = (UInt)ceil(m_pcCodingParameter->getMaximumFrameRate());
    UInt uiMvRange  = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange();
    UInt uiDPBSize  = m_pcCodingParameter->getDPBSize();
    UInt uiLevelIdc = SequenceParameterSet::getLevelIdc(uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize, 0);

    //===== create parameter sets =====
    SequenceParameterSet::create(m_pcSPS);

    //===== set SPS parameters =====
    m_pcSPS->setNalUnitType                    (NAL_UNIT_SPS);
    m_pcSPS->setAVCHeaderRewriteFlag(false);
    m_pcSPS->setDependencyId                   (0);
    m_pcSPS->setProfileIdc                     (HIGH_PROFILE);  //TQQ, HIGH_PROFILE,    MAIN_PROFILE is wrong
    m_pcSPS->setConstrainedSet0Flag            (false);
    m_pcSPS->setConstrainedSet1Flag            (false);
    m_pcSPS->setConstrainedSet2Flag            (false);
    m_pcSPS->setConstrainedSet3Flag            (false);
    m_pcSPS->setConvertedLevelIdc              (uiLevelIdc);
    m_pcSPS->setSeqParameterSetId              (uiSPSId);
    m_pcSPS->setSeqScalingMatrixPresentFlag    (m_pcCodingParameter->getScalingMatricesPresent() > 0);
    m_pcSPS->setLog2MaxFrameNum                (m_pcCodingParameter->getLog2MaxFrameNum());
    m_pcSPS->setLog2MaxPicOrderCntLsb          (m_pcCodingParameter->getLog2MaxPocLsb());
    m_pcSPS->setNumRefFrames                   (m_pcCodingParameter->getNumDPBRefFrames());
    m_pcSPS->setGapsInFrameNumValueAllowedFlag (true);
    m_pcSPS->setFrameWidthInMbs                (uiMbX);
    m_pcSPS->setFrameHeightInMbs               (uiMbY);
    m_pcSPS->setDirect8x8InferenceFlag         (true);

//JVT-V068 {
    m_pcSPS->setVUI (m_pcSPS);
//JVT-V068 }
    return Err::m_nOK;
}


//为PPS赋值
ErrVal PicEncoder::xInitPPS()
{
    ROF(m_bInit);
    ROF(m_pcSPS);
    ROT(m_pcPPS);

    //===== determine parameters =====
    UInt  uiPPSId = 0;

    //===== create PPS =====
    PictureParameterSet::create(m_pcPPS);

    //===== set PPS parameters =====
    m_pcPPS->setNalUnitType                           (NAL_UNIT_PPS);
    m_pcPPS->setPicParameterSetId                     (uiPPSId);
    m_pcPPS->setSeqParameterSetId                     (m_pcSPS->getSeqParameterSetId());
    m_pcPPS->setEntropyCodingModeFlag                 (m_pcCodingParameter->getSymbolMode() != 0);
    m_pcPPS->setPicOrderPresentFlag                   (true);
    m_pcPPS->setNumRefIdxActive                       (LIST_0, m_pcCodingParameter->getMaxRefIdxActiveBL0());
    m_pcPPS->setNumRefIdxActive                       (LIST_1, m_pcCodingParameter->getMaxRefIdxActiveBL1());
    m_pcPPS->setPicInitQp                             (gMin(51, gMax(0, (Int)m_pcCodingParameter->getBasisQp())));
    m_pcPPS->setChromaQpIndexOffset                   (0);
    m_pcPPS->setDeblockingFilterParametersPresentFlag (!m_pcCodingParameter->getLoopFilterParams().isDefault());
    m_pcPPS->setConstrainedIntraPredFlag              (m_pcCodingParameter->getConstrainedIntraPred() > 0);
    m_pcPPS->setRedundantPicCntPresentFlag            (false);  //JVT-Q054 Red. Picture
    m_pcPPS->setTransform8x8ModeFlag                  (m_pcCodingParameter->getEnable8x8Trafo() > 0);
    m_pcPPS->setPicScalingMatrixPresentFlag           (false);
    m_pcPPS->set2ndChromaQpIndexOffset                (0);
    m_pcPPS->setNumSliceGroupsMinus1                  (0);

    //===== prediction weights =====
//    m_pcPPS->setWeightedPredFlag                      (WEIGHTED_PRED_FLAG);
//    m_pcPPS->setWeightedBiPredIdc                     (WEIGHTED_BIPRED_IDC);

//TMM_WP
      m_pcPPS->setWeightedPredFlag                   (m_pcCodingParameter->getIPMode()!=0);
      m_pcPPS->setWeightedBiPredIdc                  (m_pcCodingParameter->getBMode());
//TMM_WP

    return Err::m_nOK;
}


ErrVal PicEncoder::xInitParameterSets()
{
    //===== init control manager =====
    m_pcSPS->setAllocFrameMbsX(m_pcSPS->getFrameWidthInMbs());
    m_pcSPS->setAllocFrameMbsY(m_pcSPS->getFrameHeightInMbs());
    //在ControlMng中初始化SPS与PPS
    m_pcControlMng->initParameterSets (*m_pcSPS, *m_pcPPS);

    //===== set fixed parameters =====
    m_uiFrameWidthInMb  = m_pcSPS->getFrameWidthInMbs();
    m_uiFrameHeightInMb = m_pcSPS->getFrameHeightInMbs();
    m_uiMbNumber = m_uiFrameWidthInMb * m_uiFrameHeightInMb;

    //===== re-allocate dynamic memory =====
    xDeleteData();
    xCreateData();

    //===== init objects =====
    m_pcRecPicBuffer->initSPS (*m_pcSPS);
    m_pcSequenceStructure->init (//编码序列的String
                                 m_pcCodingParameter->getSequenceFormatString(),
                                 //待编码的总帧数
                                 m_pcCodingParameter->getTotalFrames());

    //==== initialize variable parameters =====
    m_uiFrameNum = 0;
    m_uiIdrPicId = 0;
    m_bInitParameterSets = true;

    return Err::m_nOK;
}


ErrVal PicEncoder::xCreateData()
{
    //===== write buffer =====
    UInt  uiNum4x4Blocks = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
    m_uiWriteBufferSize  = 3 * (uiNum4x4Blocks * 4 * 4);
    ROFS((m_pucWriteBuffer = new UChar [m_uiWriteBufferSize]));

    return Err::m_nOK;
}

ErrVal PicEncoder::xDeleteData()
{
    //===== write buffer =====
    delete[] m_pucWriteBuffer;
    m_pucWriteBuffer    = 0;
    m_uiWriteBufferSize = 0;

    return Err::m_nOK;
}


ErrVal PicEncoder::xInitSliceHeader (SliceHeader*&  rpcSliceHeader,
                                     const FrameSpec&  rcFrameSpec,
                                     Double& rdLambda)
{
    ROF(m_bInitParameterSets);

    //===== create new slice header =====
    rpcSliceHeader = new SliceHeader(*m_pcSPS, *m_pcPPS);
    ROF(rpcSliceHeader);

    //===== determine parameters =====
    //获取QP值
    Double dQp = m_pcCodingParameter->getBasisQp () + m_pcCodingParameter->getDeltaQpLayer (rcFrameSpec.getTemporalLayer());
    Int iQp = gMin(51, gMax(0, (Int)dQp));

    //根据QP值，计算得到lambda
    rdLambda = 0.85 * pow(2.0, gMin(52.0, dQp) / 3.0 - 4.0);
    //uiSizeL0: 1
    UInt uiSizeL0 = (rcFrameSpec.getSliceType() == I_SLICE ? 0 :
                     rcFrameSpec.getSliceType() == P_SLICE ? m_pcCodingParameter->getMaxRefIdxActiveP () :
                     rcFrameSpec.getSliceType() == B_SLICE ? m_pcCodingParameter->getMaxRefIdxActiveBL0() : MSYS_UINT_MAX);
    //uiSizeL1: 0
    UInt uiSizeL1 = (rcFrameSpec.getSliceType() == I_SLICE ? 0 :
                     rcFrameSpec.getSliceType() == P_SLICE ? 0 :
                     rcFrameSpec.getSliceType() == B_SLICE ? m_pcCodingParameter->getMaxRefIdxActiveBL1() : MSYS_UINT_MAX);

    //===== set NAL unit header =====
    rpcSliceHeader->setNalRefIdc           (rcFrameSpec.getNalRefIdc());
    rpcSliceHeader->setNalUnitType         (rcFrameSpec.getNalUnitType());
    rpcSliceHeader->setDependencyId        (0);
    rpcSliceHeader->setTemporalId          (rcFrameSpec.getTemporalLayer());
    rpcSliceHeader->setUseRefBasePicFlag   (false);
    rpcSliceHeader->setStoreRefBasePicFlag (false);
    rpcSliceHeader->setPriorityId          (0);
    rpcSliceHeader->setDiscardableFlag     (false);
    rpcSliceHeader->setQualityId           (0);
    rpcSliceHeader->setIdrFlag             (rcFrameSpec.getNalUnitType  () == NAL_UNIT_CODED_SLICE_IDR);


    //===== set general parameters =====
    rpcSliceHeader->setFirstMbInSlice          (0);
    rpcSliceHeader->setLastMbInSlice           (m_uiMbNumber - 1);
    rpcSliceHeader->setSliceType               (rcFrameSpec.getSliceType());
    rpcSliceHeader->setFrameNum                (m_uiFrameNum);
    rpcSliceHeader->setNumMbsInSlice           (m_uiMbNumber);
    rpcSliceHeader->setIdrPicId                (m_uiIdrPicId);
    rpcSliceHeader->setDirectSpatialMvPredFlag (true);
    rpcSliceHeader->setRefLayer                (MSYS_UINT_MAX, 15);
    rpcSliceHeader->setNoInterLayerPredFlag    (true);
    rpcSliceHeader->setNoOutputOfPriorPicsFlag (false);
    rpcSliceHeader->setCabacInitIdc            (0);
    rpcSliceHeader->setSliceHeaderQp           (iQp);

    //Reference List0 与 List1
    //===== reference picture list ===== (init with default data, later updated)
    rpcSliceHeader->setNumRefIdxActiveOverrideFlag (false);
    rpcSliceHeader->setNumRefIdxActive (LIST_0, uiSizeL0);  //uiSizeL0: 1
    rpcSliceHeader->setNumRefIdxActive (LIST_1, uiSizeL1);  //uiSizeL1: 0

    //===== set deblocking filter parameters =====
    if(rpcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag())
    {
        rpcSliceHeader->getDeblockingFilterParameter().setDisableDeblockingFilterIdc(  m_pcCodingParameter->getLoopFilterParams().getFilterIdc   ());
        rpcSliceHeader->getDeblockingFilterParameter().setSliceAlphaC0Offset        (2*m_pcCodingParameter->getLoopFilterParams().getAlphaOffset ());
        rpcSliceHeader->getDeblockingFilterParameter().setSliceBetaOffset           (2*m_pcCodingParameter->getLoopFilterParams().getBetaOffset  ());
    }

    //===== set picture order count =====
    m_pcPocCalculator->setPoc (*rpcSliceHeader, rcFrameSpec.getContFrameNumber());

    //===== set MMCO commands =====
    //MM: Marking MODE
    //MMCO  内存管理控制操作 Marking MODE Control Operation
    if(rcFrameSpec.getMmcoBuf())
    {
        rpcSliceHeader->getDecRefPicMarking().copy(*rcFrameSpec.getMmcoBuf());
        rpcSliceHeader->getDecRefPicMarking().setAdaptiveRefPicMarkingModeFlag(true);
    }

    //===== set RPRL commands =====
    if(rcFrameSpec.getRplrBuf(LIST_0))
    {
        rpcSliceHeader->getRefPicListReordering(LIST_0).copy(*rcFrameSpec.getRplrBuf(LIST_0));
    }
    if(rcFrameSpec.getRplrBuf(LIST_1))
    {
        rpcSliceHeader->getRefPicListReordering(LIST_1).copy(*rcFrameSpec.getRplrBuf(LIST_1));
    }

    //************************/
    //*****自定义FMO，使用6*****/
    //************************/
    //===== flexible macroblock ordering =====
    rpcSliceHeader->setSliceGroupChangeCycle(1);
    rpcSliceHeader->FMOInit();


    //===== update parameters =====
    if(rpcSliceHeader->getNalRefIdc())
    {
        m_uiFrameNum  = (m_uiFrameNum + 1) % (1 << rpcSliceHeader->getSPS().getLog2MaxFrameNum());
    }
    if(rpcSliceHeader->getIdrFlag())
    {
        m_uiIdrPicId  = (m_uiIdrPicId + 1) % 3;
    }

    return Err::m_nOK;
}


ErrVal PicEncoder::xInitPredWeights(SliceHeader& rcSliceHeader)
{
  if(rcSliceHeader.isPSlice())
  {
      if(rcSliceHeader.getPPS().getWeightedPredFlag())
      {
          rcSliceHeader.setLumaLog2WeightDenom  (6);
          rcSliceHeader.setChromaLog2WeightDenom(6);
          rcSliceHeader.getPredWeightTableL0().initDefaults(rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom());
          rcSliceHeader.getPredWeightTableL0().initRandomly();
      }
  }
  else if(rcSliceHeader.isBSlice())
  {
      if(rcSliceHeader.getPPS().getWeightedBiPredIdc() == 1)
      {
          rcSliceHeader.setLumaLog2WeightDenom(6);
          rcSliceHeader.setChromaLog2WeightDenom(6);
          rcSliceHeader.getPredWeightTableL0().initDefaults(rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom());
          rcSliceHeader.getPredWeightTableL1().initDefaults(rcSliceHeader.getLumaLog2WeightDenom(), rcSliceHeader.getChromaLog2WeightDenom());
          rcSliceHeader.getPredWeightTableL0().initRandomly();
          rcSliceHeader.getPredWeightTableL1().initRandomly();
      }
  }
  return Err::m_nOK;
}


ErrVal PicEncoder::xInitExtBinDataAccessor(ExtBinDataAccessor& rcExtBinDataAccessor)
{
    ROF(m_pucWriteBuffer);
    m_cBinData.reset();
    m_cBinData.set(m_pucWriteBuffer, m_uiWriteBufferSize);
    m_cBinData.setMemAccessor(rcExtBinDataAccessor);

    return Err::m_nOK;
}


ErrVal PicEncoder::xAppendNewExtBinDataAccessor (ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                                 ExtBinDataAccessor*  pcExtBinDataAccessor)
{
    ROF(pcExtBinDataAccessor);
    ROF(pcExtBinDataAccessor->data());

    UInt    uiNewSize    = pcExtBinDataAccessor->size();
    UChar*  pucNewBuffer = new UChar [uiNewSize];
    ROF(pucNewBuffer);
    memcpy(pucNewBuffer, pcExtBinDataAccessor->data(), uiNewSize * sizeof(UChar));

    ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
    ROF(pcNewExtBinDataAccessor);

    m_cBinData.reset();
    m_cBinData.set (pucNewBuffer, uiNewSize);
    m_cBinData.setMemAccessor (*pcNewExtBinDataAccessor);
    rcExtBinDataAccessorList.push_back (pcNewExtBinDataAccessor);

    m_cBinData.reset();
    m_cBinData.setMemAccessor (*pcExtBinDataAccessor);

    return Err::m_nOK;
}


//编码一帧NV21图像
//对NAL的三连:
//m_pcNalUnitEncoder->initNalUnit ()
//m_pcNalUnitEncoder->write       ()
//m_pcNalUnitEncoder->closeNalUnit()
ErrVal PicEncoder::xEncodePicture (ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                   RecPicBufUnit&  rcRecPicBufUnit,
                                   SliceHeader&    rcSliceHeader,
                                   Double   dLambda,
                                   UInt&    ruiBits)
{
    UInt  uiBits = 0;

    //===== start picture =====
    //每一帧更新一次 已编码帧列表list0, list1
    RefFrameList  cList0, cList1;
    //Step1: 初始化list0, list1
    xStartPicture(rcRecPicBufUnit, rcSliceHeader, cList0, cList1);

    RefListStruct cRefListStruct;
    cRefListStruct.acRefFrameListME[0].copy(cList0);  //运动估计
    cRefListStruct.acRefFrameListMC[0].copy(cList0);  //运动补偿
    cRefListStruct.acRefFrameListRC[0].copy(cList0);  //??????
    cRefListStruct.acRefFrameListME[1].copy(cList1);
    cRefListStruct.acRefFrameListMC[1].copy(cList1);
    cRefListStruct.acRefFrameListRC[1].copy(cList1);
    cRefListStruct.bMCandRClistsDiffer = false;
    cRefListStruct.uiFrameIdCol = MSYS_UINT_MAX;

    //TMM_WP
    if(rcSliceHeader.getSliceType() == P_SLICE)
    {
        m_pcSliceEncoder->xSetPredWeights(rcSliceHeader,
                                          rcRecPicBufUnit.getRecFrame(),  //返回m_pcReconstructedFrame
                                          cRefListStruct);
    }
    else if(rcSliceHeader.getSliceType() == B_SLICE)
    {
        m_pcSliceEncoder->xSetPredWeights (rcSliceHeader,
                                           rcRecPicBufUnit.getRecFrame(),
                                           cRefListStruct);
    }
    //TMM_WP

    //===== encoding of slice groups =====
    for(Int iSliceGroupID = rcSliceHeader.getFMO()->getFirstSliceGroupId(); !rcSliceHeader.getFMO()->SliceGroupCompletelyCoded(iSliceGroupID); iSliceGroupID = rcSliceHeader.getFMO()->getNextSliceGroupId(iSliceGroupID))
    {
        UInt  uiBitsSlice = 0;

        //----- init slice size -----
        ///1:初始化
        //rcSliceHeader.getFMO()->getFirstMacroblockInSlice(iSliceGroupID) = 0
        rcSliceHeader.setFirstMbInSlice (rcSliceHeader.getFMO()->getFirstMacroblockInSlice(iSliceGroupID));
        //rcSliceHeader.getFMO()->getLastMBInSliceGroup(iSliceGroupID) = 8159
        rcSliceHeader.setLastMbInSlice (rcSliceHeader.getFMO()->getLastMBInSliceGroup(iSliceGroupID));

        //----- init NAL unit -----
        ///2:初始化Nalu单元
        xInitExtBinDataAccessor (m_cExtBinDataAccessor);
        m_pcNalUnitEncoder->initNalUnit (&m_cExtBinDataAccessor);

        //----- write slice header -----
        ///3:写slice头
        m_pcNalUnitEncoder->write (rcSliceHeader);

        //----- real coding -----
        ///4:逐个对Slice进行编码
        m_pcSliceEncoder->encodeSlice (rcSliceHeader,
                                       rcRecPicBufUnit.getRecFrame  (),
                                       rcRecPicBufUnit.getMbDataCtrl(),//MbDataCtrl m_pcMbDataCtrl
                                       cRefListStruct,
                                       m_pcCodingParameter->getMCBlks8x8Disable() > 0,//blocks smaller than 8x8 are disabled
                                       m_uiFrameWidthInMb,  //120=1920/16
                                       dLambda);

        //----- close NAL unit -----
        m_pcNalUnitEncoder->closeNalUnit(uiBitsSlice);
        //???
        xAppendNewExtBinDataAccessor(rcExtBinDataAccessorList, &m_cExtBinDataAccessor);
        uiBitsSlice += 4*8;
        uiBits += uiBitsSlice;
    }


    //===== finish =====
    ///~在此处做环路滤波
    xFinishPicture (rcRecPicBufUnit, rcSliceHeader, cList0, cList1, uiBits);
    ruiBits += uiBits;

    return Err::m_nOK;
}

//list0与list1
ErrVal PicEncoder::xStartPicture (RecPicBufUnit& rcRecPicBufUnit,
                                  SliceHeader&   rcSliceHeader,
                                  RefFrameList&  rcList0,
                                  RefFrameList&  rcList1)
{
    //===== initialize reference picture lists and update slice header =====
    m_pcRecPicBuffer->getRefLists (rcList0, rcList1, rcSliceHeader);
    rcSliceHeader.setRefFrameList (&rcList0, FRAME, LIST_0);
    rcSliceHeader.setRefFrameList (&rcList1, FRAME, LIST_1);

    //===== init half-pel buffers =====
    UInt uiPos;
    for(uiPos = 0; uiPos < rcList0.getActive(); uiPos++)
    {
        Frame* pcRefFrame = rcList0.getEntry(uiPos);
        if(!pcRefFrame->isHalfPel())
        {
            pcRefFrame->initHalfPel();
            pcRefFrame->extendFrame(m_pcQuarterPelFilter);
        }
        else
        if(!pcRefFrame->isExtended())
        {
            pcRefFrame->extendFrame(m_pcQuarterPelFilter);
        }
    }
    for(uiPos = 0; uiPos < rcList1.getActive(); uiPos++)
    {
        Frame* pcRefFrame = rcList1.getEntry(uiPos);
        if(!pcRefFrame->isHalfPel())
        {
            pcRefFrame->initHalfPel();
        }
        if(!pcRefFrame->isExtended())
        {
            pcRefFrame->extendFrame(m_pcQuarterPelFilter);
        }
    }

    //===== reset macroblock data =====
#ifdef WITH_RECPIC
    rcRecPicBufUnit.getMbDataCtrl()->reset();
    rcRecPicBufUnit.getMbDataCtrl()->clear();
#endif

    return Err::m_nOK;
}


ErrVal PicEncoder::xFinishPicture (RecPicBufUnit&  rcRecPicBufUnit,
                                   SliceHeader&    rcSliceHeader,
                                   RefFrameList&   rcList0,
                                   RefFrameList&   rcList1,
                                   UInt  uiBits)
{
    //===== uninit half-pel data =====
    UInt uiPos;
    for(uiPos = 0; uiPos < rcList0.getActive(); uiPos++)
    {
        Frame* pcRefFrame = rcList0.getEntry(uiPos);
        if(pcRefFrame->isExtended())
        {
            pcRefFrame->clearExtended();
        }
        if(pcRefFrame->isHalfPel())
        {
            pcRefFrame->uninitHalfPel();
        }
    }
    for(uiPos = 0; uiPos < rcList1.getActive(); uiPos++)
    {
        Frame* pcRefFrame = rcList1.getEntry(uiPos);
        if(pcRefFrame->isExtended())
        {
            pcRefFrame->clearExtended();
        }
        if(pcRefFrame->isHalfPel())
        {
            pcRefFrame->uninitHalfPel();
        }
    }

    //===== deblocking =====
    ///TQQ, 环路滤波
    m_pcLoopFilter->process (rcSliceHeader, rcRecPicBufUnit.getRecFrame(), NULL, rcRecPicBufUnit.getMbDataCtrl(), 0, false);

    //===== get PSNR =====
    Double dPSNR[3];
    xGetPSNR (rcRecPicBufUnit, dPSNR);

    //===== output =====
    printf ("PicEncoder::xFinishPicture, current_frame: %d\n"
            "%4d %c %s %4d  QP%3d  Y %7.4lf dB  U %7.4lf dB  V %7.4lf dB   bits%8d\n",
            m_uiFrameNum,
            m_uiCodedFrames,
            rcSliceHeader.getSliceType  ()==I_SLICE ? 'I' :
            rcSliceHeader.getSliceType  ()==P_SLICE ? 'P' : 'B',
            rcSliceHeader.getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR ? "IDR" :
            rcSliceHeader.getNalRefIdc  ()==NAL_REF_IDC_PRIORITY_LOWEST ? "   " : "REF",
            rcSliceHeader.getPoc(),
            rcSliceHeader.getSliceQp(),
            dPSNR[0],
            dPSNR[1],
            dPSNR[2],
            uiBits
            );

    //===== update parameters =====
    m_uiCodedFrames++;
    ETRACE_NEWPIC;

    return Err::m_nOK;
}


ErrVal PicEncoder::xGetPSNR (RecPicBufUnit&  rcRecPicBufUnit,
                             Double*  adPSNR)
{
    //===== reset buffer control =====
    m_pcYuvBufferCtrlFullPel->initMb();

    //===== set parameters =====
    const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvBufferCtrlFullPel->getBufferParameter();
    Frame* pcFrame = rcRecPicBufUnit.getRecFrame();
    PicBuffer* pcPicBuffer = rcRecPicBufUnit.getPicBuffer();

    //===== calculate PSNR =====
    Pel*  pPelOrig = pcPicBuffer->getBuffer() + cBufferParam.getMbLum();
    XPel* pPelRec  = pcFrame->getFullPelYuvBuffer()->getMbLumAddr();
    Int   iStride  = cBufferParam.getStride();
    Int   iWidth   = cBufferParam.getWidth ();
    Int   iHeight  = cBufferParam.getHeight();
    UInt  uiSSDY = 0;
    UInt  uiSSDU = 0;
    UInt  uiSSDV = 0;
    Int   x, y;

    for(y = 0; y < iHeight; y++)
    {
        for(x = 0; x < iWidth; x++)
        {
            Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
            uiSSDY   += iDiff * iDiff;
        }
        pPelOrig += iStride;
        pPelRec  += iStride;
    }

    iHeight >>= 1;
    iWidth  >>= 1;
    iStride >>= 1;
    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCb();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCbAddr();

    for(y = 0; y < iHeight; y++)
    {
        for(x = 0; x < iWidth; x++)
        {
            Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
            uiSSDU   += iDiff * iDiff;
        }
        pPelOrig += iStride;
        pPelRec  += iStride;
    }

    pPelOrig = pcPicBuffer->getBuffer() + cBufferParam.getMbCr();
    pPelRec  = pcFrame->getFullPelYuvBuffer()->getMbCrAddr();

    for(y = 0; y < iHeight; y++)
    {
        for(x = 0; x < iWidth; x++)
        {
            Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
            uiSSDV   += iDiff * iDiff;
        }
        pPelOrig += iStride;
        pPelRec  += iStride;
    }

    Double fRefValueY = 255.0 * 255.0 * 16.0 * 16.0 * (Double)m_uiMbNumber;
    Double fRefValueC = fRefValueY / 4.0;
    adPSNR[0]  = (uiSSDY ? 10.0 * log10(fRefValueY / (Double)uiSSDY) : 99.99);
    adPSNR[1]  = (uiSSDU ? 10.0 * log10(fRefValueC / (Double)uiSSDU) : 99.99);
    adPSNR[2]  = (uiSSDV ? 10.0 * log10(fRefValueC / (Double)uiSSDV) : 99.99);
    m_dSumYPSNR += adPSNR[0];
    m_dSumUPSNR += adPSNR[1];
    m_dSumVPSNR += adPSNR[2];

    return Err::m_nOK;
}

}  //namespace JSVM {

