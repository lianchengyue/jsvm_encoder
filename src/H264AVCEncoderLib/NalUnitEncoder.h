#ifndef _NALUNITENCODER_H_
#define _NALUNITENCODER_H_

#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/Sei.h"

namespace JSVM {


class BitWriteBuffer;

class NalUnitEncoder
{
protected:
    NalUnitEncoder();
    virtual ~NalUnitEncoder();

public:
    static ErrVal create(NalUnitEncoder*&  rpcNalUnitEncoder);
    ErrVal destroy();

    ErrVal init(BitWriteBuffer*  pcBitWriteBuffer,
                HeaderSymbolWriteIf*  pcHeaderSymbolWriteIf,
                HeaderSymbolWriteIf*  pcHeaderSymbolTestIf);
    ErrVal uninit();

    //申请m_pucTempBuffer
    //m_pucBuffer: 指向bin的数据部分
    ErrVal initNalUnit(BinDataAccessor*  pcBinDataAccessor);
    ErrVal closeNalUnit(UInt&  ruiBits);
    ErrVal closeAndAppendNalUnits (UInt  *pauiBits,
                                   ExtBinDataAccessorList  &rcExtBinDataAccessorList,
                                   ExtBinDataAccessor      *pcExtBinDataAccessor,
                                   BinData         &rcBinData,
                                   H264AVCEncoder  *pcH264AVCEncoder,
                                   UInt  uiQualityLevelCGSSNR,
                                   UInt  uiLayerCGSSNR);

    ErrVal write (const SequenceParameterSet& rcSPS);
    ErrVal write (const PictureParameterSet&  rcPPS);
    ErrVal writePrefix (const SliceHeader&  rcSH);
    ErrVal write (const SliceHeader& rcSH);
    ErrVal write (SEI::MessageList&  rcSEIMessageList);

//JVT-V068 {
    ErrVal writeScalableNestingSei(SEI::MessageList&  rcSEIMessageList);
//JVT-V068 }

    static ErrVal convertRBSPToPayload (UInt  &ruiBytesWritten,
                                        UInt   uiHeaderBytes,
                                        UChar *pcPayload,
                                        const UChar *pcRBSP,
                                        UInt   uiPayloadBufferSize);
    BitWriteBuffer* xGetBitsWriteBuffer(void)  {  return m_pcBitWriteBuffer;  }//JVT-X046
protected:
    ErrVal xWriteTrailingBits();

protected:
    Bool                  m_bIsUnitActive;
    //
    BitWriteBuffer*       m_pcBitWriteBuffer;
    HeaderSymbolWriteIf*  m_pcHeaderSymbolWriteIf;
    HeaderSymbolWriteIf*  m_pcHeaderSymbolTestIf;
    //每一次待输出的h264格式的bin文件
    BinDataAccessor*      m_pcBinDataAccessor;
    UChar*                m_pucBuffer;
    //initNalUnit时创建的临时buffer, RBSP
    UChar*                m_pucTempBuffer;
    UChar*                m_pucTempBufferBackup;
    UInt                  m_uiPacketLength;
    NalUnitType           m_eNalUnitType;
    NalRefIdc             m_eNalRefIdc;
};


}  //namespace JSVM {


#endif //_NALUNITENCODER_H_
