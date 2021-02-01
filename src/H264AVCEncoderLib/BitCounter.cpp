#include "H264AVCEncoderLib.h"
#include "BitCounter.h"


namespace JSVM {


BitCounter::BitCounter():
m_uiBitCounter(0)
{
}

BitCounter::~BitCounter()
{
}

ErrVal BitCounter::create(BitCounter*& rpcBitCounter)
{
    rpcBitCounter = new BitCounter;

    ROT( NULL == rpcBitCounter);

    return Err::m_nOK;
}


ErrVal BitCounter::destroy()
{
    delete this;

    return Err::m_nOK;
}


}  //namespace JSVM {
