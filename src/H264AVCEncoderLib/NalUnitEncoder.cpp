
#include "H264AVCEncoderLib.h"
#include "BitWriteBuffer.h"
#include "NalUnitEncoder.h"

#include "CodingParameter.h"

namespace JSVM {


NalUnitEncoder::NalUnitEncoder() :
    m_bIsUnitActive         (false),
    m_pcBitWriteBuffer      (0),
    m_pcHeaderSymbolWriteIf (0),
    m_pcHeaderSymbolTestIf  (0),
    m_pcBinDataAccessor     (0),
    m_pucBuffer             (0),
    m_pucTempBuffer         (0),
    m_pucTempBufferBackup   (0),
    m_uiPacketLength        (MSYS_UINT_MAX),
    m_eNalUnitType          (NAL_UNIT_UNSPECIFIED_0),
    m_eNalRefIdc            (NAL_REF_IDC_PRIORITY_LOWEST)
{
}


NalUnitEncoder::~NalUnitEncoder()
{
}


ErrVal NalUnitEncoder::create (NalUnitEncoder*& rpcNalUnitEncoder)
{
    rpcNalUnitEncoder = new NalUnitEncoder;
    ROT(NULL == rpcNalUnitEncoder);
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::init (BitWriteBuffer*  pcBitWriteBuffer,
                             HeaderSymbolWriteIf*  pcHeaderSymbolWriteIf,
                             HeaderSymbolWriteIf*  pcHeaderSymbolTestIf)
{
    ROT(NULL == pcBitWriteBuffer);
    ROT(NULL == pcHeaderSymbolWriteIf);
    ROT(NULL == pcHeaderSymbolTestIf);

    m_pcBitWriteBuffer      = pcBitWriteBuffer;
    m_pcHeaderSymbolTestIf  = pcHeaderSymbolTestIf;
    m_pcHeaderSymbolWriteIf = pcHeaderSymbolWriteIf;
    m_bIsUnitActive         = false;
    m_pucBuffer             = NULL;
    m_pucTempBuffer         = NULL;
    m_pucTempBufferBackup   = NULL;
    m_uiPacketLength        = MSYS_UINT_MAX;
    m_eNalUnitType          = NAL_UNIT_UNSPECIFIED_0;
    m_eNalRefIdc            = NAL_REF_IDC_PRIORITY_LOWEST;

    return Err::m_nOK;
}


ErrVal NalUnitEncoder::uninit()
{
    delete [] m_pucTempBuffer;
    delete [] m_pucTempBufferBackup;
    m_pucTempBufferBackup   = NULL;

    m_pcBitWriteBuffer      = NULL;
    m_pcHeaderSymbolWriteIf = NULL;
    m_pcHeaderSymbolTestIf  = NULL;
    m_bIsUnitActive         = false;
    m_pucBuffer             = NULL;
    m_pucTempBuffer         = NULL;
    m_uiPacketLength        = MSYS_UINT_MAX;
    m_eNalUnitType          = NAL_UNIT_UNSPECIFIED_0;
    m_eNalRefIdc            = NAL_REF_IDC_PRIORITY_LOWEST;

    return Err::m_nOK;
}


ErrVal NalUnitEncoder::destroy()
{
    uninit();
    delete this;
    return Err::m_nOK;
}


//! 参数1: pcBinDataAccessor: H264AVCEncoderTest::go中的cExtBinDataAccessor, 压缩后的当前帧的二进制文件
//申请m_pucTempBuffer
//m_pucBuffer: 指向bin的数据部分
ErrVal NalUnitEncoder::initNalUnit (BinDataAccessor* pcBinDataAccessor)
{
    ROT(m_bIsUnitActive);
    ROF(pcBinDataAccessor);
    ROT(pcBinDataAccessor->size() < 1);

    m_bIsUnitActive     = true;
    m_pcBinDataAccessor = pcBinDataAccessor;
    //m_pcT
    ///FLQ, 指向m_pucBuffer(Payload)
    m_pucBuffer = pcBinDataAccessor->data();

    //printf("AVC: 该NAL中m_pucBuffer的大小: %d\n", uiPayloadBufferSize);
    printf("AVC: m_uiPacketLength: %d, m_pcBinDataAccessor->size(): %d\n", m_uiPacketLength, m_pcBinDataAccessor->size());

    //第一次: 申请数据
    if(m_uiPacketLength != m_pcBinDataAccessor->size())
    {
        delete[] m_pucTempBuffer;

        m_uiPacketLength = m_pcBinDataAccessor->size();
        m_pucTempBuffer  = new UChar[m_uiPacketLength];
        ROF(m_pucTempBuffer);
    }

    //m_uiPacketLength初始值: 1000或者6266880
    //更新到实际值
    m_pcBitWriteBuffer->initPacket((UInt*)(m_pucTempBuffer), m_uiPacketLength-1);

    return Err::m_nOK;
}



ErrVal NalUnitEncoder::closeAndAppendNalUnits (UInt                    *pauiBits,
                                               ExtBinDataAccessorList  &rcExtBinDataAccessorList,
                                               ExtBinDataAccessor      *pcExtBinDataAccessor,
                                               BinData                 &rcBinData,
                                               H264AVCEncoder          *pcH264AVCEncoder,
                                               UInt                    uiQualityLevelCGSSNR,
                                               UInt                    uiLayerCGSSNR)
{
    ROF(m_bIsUnitActive);
    ROF(pcExtBinDataAccessor);
    ROF(pcExtBinDataAccessor->data());

    ROF(m_pcBinDataAccessor == pcExtBinDataAccessor);

    //===== write trailing bits =====
    if(NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
        NAL_UNIT_END_OF_STREAM   != m_eNalUnitType &&
       (NAL_UNIT_PREFIX          != m_eNalUnitType || m_eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST))
    {
        //写TrailingBits
        xWriteTrailingBits();
    }
    m_pcBitWriteBuffer->flushBuffer();

    //===== convert to payload and add header =====
    UInt  uiHeaderBytes = 1;
    if(m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || m_eNalUnitType == NAL_UNIT_PREFIX)
    {
        uiHeaderBytes += NAL_UNIT_HEADER_SVC_EXTENSION_BYTES;
    }

    BitWriteBufferIf *pcCurrentWriteBuffer = m_pcBitWriteBuffer;
    //m_pucBuffer: 目标, 指向Payload
    UChar            *pucPayload           = m_pucBuffer;
    //m_pucTempBuffer: 源, 指向RBSP
    const UChar      *pucRBSP              = m_pucTempBuffer;
    UInt              uiPayloadBufferSize  = m_uiPacketLength;

    ROF(pcExtBinDataAccessor->data() == pucPayload);
    ROF(pcExtBinDataAccessor->size() == uiPayloadBufferSize);
    printf("SVC: 该NAL中m_pucBuffer的大小: %d\n", uiPayloadBufferSize);

    UInt uiFragment = 0;
    while(true)
    {
        UInt uiBits  = pcCurrentWriteBuffer->getNumberOfWrittenBits();
        UInt uiBytes = (uiBits + 7) >> 3;
        ///赋值指向pcBinDataAccessor->data()的payload
        convertRBSPToPayload(uiBytes,
                             uiHeaderBytes,
                             pucPayload,
                             pucRBSP,
                             uiPayloadBufferSize);
        pauiBits[uiFragment] = 8 * uiBytes;

        UChar* pucNewBuffer = new UChar [uiBytes];
        ROF(pucNewBuffer);
        memcpy(pucNewBuffer, pucPayload, uiBytes * sizeof(UChar));

        if(pcH264AVCEncoder)
        {
            //JVT-W052
            if(pcH264AVCEncoder->getCodingParameter()->getIntegrityCheckSEIEnable() && pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement())
            {
                if(uiQualityLevelCGSSNR + uiFragment > 0)
                {
                    UInt uicrcMsb,uicrcVal;
                    uicrcVal = pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR];
                    uicrcMsb = 0;
                    Bool BitVal = false;
                    for (UInt uiBitIdx = 0; uiBitIdx< uiBytes*8; uiBitIdx++)
                    {
                      uicrcMsb = (uicrcVal >> 15) & 1;
                      BitVal = (pucNewBuffer[uiBitIdx>>3] >> (7-(uiBitIdx&7)))&1;
                      uicrcVal = (((uicrcVal<<1) + BitVal) & 0xffff)^(uicrcMsb*0x1021);
                    }
                    pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR] = uicrcVal;

                    if (pcH264AVCEncoder->m_uiNumofCGS[uiLayerCGSSNR] == uiQualityLevelCGSSNR + uiFragment)
                    {
                        ROT(pcCurrentWriteBuffer->nextBitWriteBufferActive());
                        for (UInt uiBitIdx = 0; uiBitIdx< 16; uiBitIdx++)
                        {
                            uicrcMsb = (uicrcVal >> 15) & 1;
                            BitVal = 0;
                            uicrcVal = (((uicrcVal<<1) + BitVal) & 0xffff)^(uicrcMsb*0x1021);
                        }
                        pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR] = uicrcVal;
                        UInt uiInfoEntry = pcH264AVCEncoder->m_pcIntegrityCheckSEI->getNumInfoEntriesMinus1();
                        if(uiInfoEntry == MSYS_UINT_MAX)
                        {
                            uiInfoEntry = 0;
                        }
                        else
                        {
                            UInt uiLastLayer = pcH264AVCEncoder->m_pcIntegrityCheckSEI->getEntryDependencyId(uiInfoEntry);
                            if(uiLastLayer < uiLayerCGSSNR)
                            {
                                uiInfoEntry++;
                            }
                        }
                        pcH264AVCEncoder->m_pcIntegrityCheckSEI->setNumInfoEntriesMinus1(uiInfoEntry);
                        pcH264AVCEncoder->m_pcIntegrityCheckSEI->setEntryDependencyId(uiInfoEntry,uiLayerCGSSNR);
                        pcH264AVCEncoder->m_pcIntegrityCheckSEI->setQualityLayerCRC(uiInfoEntry,uicrcVal);
                    }
                }
            }
            //JVT-W052
        }

        ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
        ROF(pcNewExtBinDataAccessor);

        rcBinData               .reset          ();
        rcBinData               .set            ( pucNewBuffer, uiBytes);
        rcBinData               .setMemAccessor (*pcNewExtBinDataAccessor);
        rcExtBinDataAccessorList.push_back      ( pcNewExtBinDataAccessor);

        rcBinData               .reset          ();
        rcBinData               .setMemAccessor (*pcExtBinDataAccessor);

        if(!pcCurrentWriteBuffer->nextBitWriteBufferActive())
        {
          break;
        }
        pucRBSP              = pcCurrentWriteBuffer->getNextBuffersPacket();
        pcCurrentWriteBuffer = pcCurrentWriteBuffer->getNextBitWriteBuffer();
        uiFragment++;
    }

    m_pcBitWriteBuffer->uninit();

    //==== reset parameters =====
    m_bIsUnitActive     = false;
    m_pucBuffer         = NULL;
    m_pcBinDataAccessor = NULL;
    m_eNalUnitType      = NAL_UNIT_UNSPECIFIED_0;
    m_eNalRefIdc        = NAL_REF_IDC_PRIORITY_LOWEST;
    return Err::m_nOK;
}



ErrVal NalUnitEncoder::closeNalUnit (UInt& ruiBits)
{
    ROF(m_bIsUnitActive);

    //===== write trailing bits =====
    if(NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
       NAL_UNIT_END_OF_STREAM   != m_eNalUnitType &&
       (NAL_UNIT_PREFIX         != m_eNalUnitType || m_eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST))
    {
        //补0，写拖尾位
        xWriteTrailingBits();
    }
    m_pcBitWriteBuffer->flushBuffer();

    //===== convert to payload and add header =====
    UInt  uiHeaderBytes = 1;
    if(m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || m_eNalUnitType == NAL_UNIT_PREFIX)
    {
        uiHeaderBytes += NAL_UNIT_HEADER_SVC_EXTENSION_BYTES;
    }

    //实际是字节数。 +7: 不足8bit补足
    UInt  uiBits = (m_pcBitWriteBuffer->getNumberOfWrittenBits() + 7) >> 3;//(m_uiBitsWritten + 7)>>3

    ///赋值指向pcBinDataAccessor->data()的payload
    convertRBSPToPayload(uiBits,  //字节数
                         uiHeaderBytes,  //头字节数
                         m_pucBuffer,      //Dst: Payload
                         m_pucTempBuffer,  //Src: RBSP, initNalUnit()中申请的临时buffer
                         m_uiPacketLength);//PayloadBuffer尺寸
    //把uiBits的值赋值给m_pcBinDataAccessor->size(): m_uiSize
    m_pcBinDataAccessor->decreaseEndPos(m_pcBinDataAccessor->size() - uiBits);
    printf("当前nal的bit数uiBits=%d\n", uiBits);
    ruiBits = 8*uiBits;

    //==== reset parameters =====
    m_bIsUnitActive     = false;
    m_pucBuffer         = NULL;
    m_pcBinDataAccessor = NULL;
    m_eNalUnitType      = NAL_UNIT_UNSPECIFIED_0;
    m_eNalRefIdc        = NAL_REF_IDC_PRIORITY_LOWEST;
    return Err::m_nOK;
}

//! RBSP + 0X03
//uiBits:
//uiHeaderBytes:
//m_pucBuffer:
//m_pucTempBuffer:   //initNalUnit()中申请的临时buffer
//m_uiPacketLength:
ErrVal NalUnitEncoder::convertRBSPToPayload (UInt   &ruiBytesWritten,   //待写的字节数
                                             UInt   uiHeaderBytes,      //1
                                             UChar  *pcPayload,         //Dst: payload
                                             const UChar  *pcRBSP,      //Src: RBSP
                                             UInt   uiPayloadBufferSize)//1000
{
    UInt uiZeroCount    = 0;
    UInt uiReadOffset   = uiHeaderBytes;
    UInt uiWriteOffset  = uiHeaderBytes;

    //===== NAL unit header =====
    for(UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++)
    {
        pcPayload[uiIndex] = pcRBSP[uiIndex];
    }

    //===== NAL unit payload =====
    for(; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++)
    {
        ROT(uiWriteOffset >= uiPayloadBufferSize);

        if(2 == uiZeroCount && 0 == (pcRBSP[uiReadOffset] & 0xfc))
        {
            uiZeroCount                = 0;
            pcPayload[uiWriteOffset++] = 0x03;
        }

        pcPayload[uiWriteOffset] = pcRBSP[uiReadOffset];

        if(0 == pcRBSP[uiReadOffset])
        {
            uiZeroCount++;
        }
        else
        {
            uiZeroCount = 0;
        }
    }

    //最后两位为0
    if((0x00 == pcPayload[uiWriteOffset-1]) && (0x00 == pcPayload[uiWriteOffset-2]))
    {
        ROT(uiWriteOffset >= uiPayloadBufferSize);
        pcPayload[uiWriteOffset++] = 0x03;
    }
    ruiBytesWritten = uiWriteOffset;

    return Err::m_nOK;
}

//closeNalUnit时，补0
ErrVal NalUnitEncoder::xWriteTrailingBits()
{
    m_pcBitWriteBuffer->write(1);
    m_pcBitWriteBuffer->writeAlignZero();

    BitWriteBufferIf* pcCurrentBitWriter = m_pcBitWriteBuffer;
    while(pcCurrentBitWriter->nextBitWriteBufferActive())
    {
        pcCurrentBitWriter = pcCurrentBitWriter->getNextBitWriteBuffer();
        pcCurrentBitWriter->write(1);
        pcCurrentBitWriter->writeAlignZero();
    }
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::write(const SequenceParameterSet& rcSPS)
{
    //写SPS所有信息到TraceEncoder_DQId000.txt 与 bin缓存中
    rcSPS.write(m_pcHeaderSymbolWriteIf);

    ///nal_unit_type: JSVM::NAL_UNIT_SPS (7)
    m_eNalUnitType  = rcSPS.getNalUnitType();
    ///nal_ref_idc: JSVM::NAL_REF_IDC_PRIORITY_HIGHEST (3)
    m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_HIGHEST;
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::write(const PictureParameterSet& rcPPS)
{
    //写PPS所有信息到TraceEncoder_DQId000.txt 与 bin缓存中
    rcPPS.write(m_pcHeaderSymbolWriteIf);

    ///nal_unit_type: JSVM::NAL_UNIT_PPS (8)
    m_eNalUnitType  = rcPPS.getNalUnitType();
    ///nal_ref_idc: JSVM::NAL_REF_IDC_PRIORITY_HIGHEST (3)
    m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_HIGHEST;
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::writePrefix(const SliceHeader& rcSH)
{
    rcSH.writePrefix(*m_pcHeaderSymbolWriteIf);

    m_eNalUnitType = NAL_UNIT_PREFIX;
    m_eNalRefIdc = rcSH.getNalRefIdc();
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::write(const SliceHeader& rcSH)
{
    SliceHeader cSH = rcSH;
    HeaderSymbolWriteIf*  pcCurrWriteIf = m_pcHeaderSymbolWriteIf;

    for(UInt uiMGSFragment = 0; true; uiMGSFragment++)
    {
        //----- modify copy of slice header -----
        cSH.setDependencyId                   (rcSH.getLayerCGSSNR        ());
        cSH.setQualityId                      (rcSH.getQualityLevelCGSSNR () + uiMGSFragment);
        cSH.setDiscardableFlag                (rcSH.getDiscardableFlag    ());
        cSH.setNoInterLayerPredFlag           (rcSH.getNoInterLayerPredFlag() && cSH.getQualityId() == 0);
        cSH.setScanIdxStart                   (rcSH.getSPS().getMGSCoeffStart(uiMGSFragment));
        cSH.setScanIdxStop                    (rcSH.getSPS().getMGSCoeffStop (uiMGSFragment));
        cSH.setRefLayerDQId                   (uiMGSFragment == 0 ? rcSH.getRefLayerDQId                   () : (rcSH.getLayerCGSSNR() << 4) + rcSH.getQualityLevelCGSSNR() + uiMGSFragment - 1);
        cSH.setAdaptiveBaseModeFlag           (uiMGSFragment == 0 ? rcSH.getAdaptiveBaseModeFlag           () : false );
        cSH.setAdaptiveMotionPredictionFlag   (uiMGSFragment == 0 ? rcSH.getAdaptiveMotionPredictionFlag   () : false );
        cSH.setAdaptiveResidualPredictionFlag (uiMGSFragment == 0 ? rcSH.getAdaptiveResidualPredictionFlag () : false );
        cSH.setDefaultBaseModeFlag            (uiMGSFragment == 0 ? rcSH.getDefaultBaseModeFlag            () : true  );
        cSH.setDefaultMotionPredictionFlag    (uiMGSFragment == 0 ? rcSH.getDefaultMotionPredictionFlag    () : true  );
        cSH.setDefaultResidualPredictionFlag  (uiMGSFragment == 0 ? rcSH.getDefaultResidualPredictionFlag  () : true  );

        //----- write copy of slice header -----
        cSH.write(*pcCurrWriteIf);
        if(rcSH.getSPS().getMGSCoeffStop(uiMGSFragment) == 16)
        {
            break;
        }

        //----- update -----
        pcCurrWriteIf = pcCurrWriteIf->getHeaderSymbolWriteIfNextSlice(true);
    }

    m_eNalUnitType  = rcSH.getNalUnitType ();
    m_eNalRefIdc    = rcSH.getNalRefIdc   ();
    return Err::m_nOK;
}


ErrVal NalUnitEncoder::write(SEI::MessageList& rcSEIMessageList)
{
    SEI::write(m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList);

    m_eNalUnitType  = NAL_UNIT_SEI;
    m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_LOWEST;
    return Err::m_nOK;
}


// JVT-V068 {
ErrVal NalUnitEncoder::writeScalableNestingSei(SEI::MessageList& rcSEIMessageList)
{
    SEI::writeScalableNestingSei(m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList);
    m_eNalUnitType  = NAL_UNIT_SEI;
    m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_LOWEST;
    return Err::m_nOK;
}
// JVT-V068 }

}  //namespace JSVM {
