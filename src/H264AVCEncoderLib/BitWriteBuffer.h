#ifndef _BITWRITEBUFFER_H_
#define _BITWRITEBUFFER_H_

#include "BitWriteBufferIf.h"

namespace JSVM {


class BitWriteBuffer :

public BitWriteBufferIf
{
protected:
    BitWriteBuffer();
    virtual ~BitWriteBuffer();

public:
    BitWriteBufferIf* getNextBitWriteBuffer(Bool bStartNewBitstream);
    Bool nextBitWriteBufferActive() { return NULL != m_pucNextStreamPacket; }
    UChar* getNextBuffersPacket() { return m_pucNextStreamPacket; }

    static ErrVal create( BitWriteBuffer*& rpcBitWriteBuffer);
    ErrVal destroy();

    ErrVal init();
    ErrVal uninit() { return init(); }

    ErrVal initPacket(UInt* pulBits, UInt uiPacketLength);

    ErrVal write(UInt uiBits, UInt uiNumberOfBits = 1);

    UInt getNumberOfWrittenBits() { return  m_uiBitsWritten; }

    ErrVal pcmSamples(const TCoeff* pCoeff, UInt uiNumberOfSamples);

    ErrVal flushBuffer();
    ErrVal writeAlignZero();
    ErrVal writeAlignOne();

    ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos);//FIX_FRAG_CAVLC
    //JVT-X046 {
    UInt   getDWordsLeft(void)  { return m_uiDWordsLeft;   }
    UInt   getBitsWritten(void) { return m_uiBitsWritten;  }
    Int    getValidBits(void)   { return m_iValidBits;     }
    UInt   getCurrentBits(void) { return m_ulCurrentBits;  }
    UInt*  getStreamPacket(void){ return m_pulStreamPacket;}
    void loadBitWriteBuffer(BitWriteBufferIf* pcBitWriteBufferIf)
    {
        BitWriteBuffer* pcBitWriteBuffer = (BitWriteBuffer*)(pcBitWriteBufferIf);
        m_uiDWordsLeft = pcBitWriteBuffer->getDWordsLeft();
        m_uiBitsWritten = pcBitWriteBuffer->getBitsWritten();
        m_iValidBits = pcBitWriteBuffer->getValidBits();
        m_ulCurrentBits = pcBitWriteBuffer->getCurrentBits();
        m_pulStreamPacket = pcBitWriteBuffer->getStreamPacket();
    }
    void loadBitCounter(BitWriteBufferIf* pcBitWriteBufferIf){}
    UInt getBitsWriten() { return m_uiBitsWritten; }
    //JVT-X046 }
protected:
    //交换大小端
    UInt  xSwap( UInt ul )
    {
        // heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
        return ul;
#else
        UInt ul2;

        ul2  = ul>>24;
        ul2 |= (ul>>8) & 0x0000ff00;
        ul2 |= (ul<<8) & 0x00ff0000;
        ul2 |= ul<<24;

        return ul2;
#endif
    }

protected:
    UInt   m_uiDWordsLeft;
    //已写的bit数
    UInt   m_uiBitsWritten;
    Int    m_iValidBits;
    //<4字节时的内容
    UInt   m_ulCurrentBits;
    //>4字节时的内容, 指向m_pucTempBuffer
    UInt*  m_pulStreamPacket;

private:
    UInt   m_uiInitPacketLength;  //当前NAL的长度, 默认1000
    BitWriteBuffer * m_pcNextBitWriteBuffer;
    UChar *m_pucNextStreamPacket;
};



}  //namespace JSVM {


#endif //_BITWRITEBUFFER_H_
