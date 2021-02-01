#ifndef _BITWRITEBUFFERIF_H_
#define _BITWRITEBUFFERIF_H_


namespace JSVM {


class BitWriteBufferIf
{
protected:
    BitWriteBufferIf()
    {
    }
    virtual ~BitWriteBufferIf()
    {
    }

public:
    virtual ErrVal write( UInt uiBits, UInt uiNumberOfBits = 1) = 0;
    virtual BitWriteBufferIf* getNextBitWriteBuffer(Bool bStartNewBitstream = false) = 0;
    virtual Bool nextBitWriteBufferActive() = 0;
    virtual UChar* getNextBuffersPacket() = 0;

    virtual UInt getNumberOfWrittenBits() = 0 ;

    virtual ErrVal writeAlignZero() = 0;
    virtual ErrVal writeAlignOne() = 0;
    virtual ErrVal flushBuffer() = 0;
    virtual ErrVal pcmSamples(const TCoeff* pCoeff, UInt uiNumberOfSamples) = 0;
    virtual ErrVal getLastByte(UChar &uiLastByte, UInt &uiLastBitPos) = 0; //FIX_FRAG_CAVLC
    //JVT-X046 {
    virtual void loadBitWriteBuffer(BitWriteBufferIf* pcBitWriteBufferIf) = 0;
    virtual void loadBitCounter(BitWriteBufferIf* pcBitWriteBufferIf) = 0;
    virtual UInt getBitsWritten(void) = 0;
    //JVT-X046 }
};



}  //namespace JSVM {



#endif //_BITWRITEBUFFERIF_H_
