#ifndef _H264AVCENCODERTEST_H_
#define _H264AVCENCODERTEST_H_

#include <algorithm>
#include <list>

#include "WriteBitstreamToFile.h"
#include "ReadYuvFile.h"
#include "WriteYuvToFile.h"
#include "Typedefs.h"


class EncoderCodingParameter;



typedef struct
{
    UInt    uiNumberOfLayers;
    std::string cBitstreamFilename;
    Int     nResult;
    UInt    nFrames;
} EncoderIoParameter;




class H264AVCEncoderTest
{
private:
    H264AVCEncoderTest();
    virtual ~H264AVCEncoderTest();

public:
    static ErrVal create (H264AVCEncoderTest*& rpcH264AVCEncoderTest );

    ErrVal init (Int argc, Char** argv);
    ErrVal go ();
    ErrVal destroy ();
    ErrVal ScalableDealing ();

protected:
    ErrVal xGetNewPicBuffer(PicBuffer*& rpcPicBuffer,
                            UInt uiLayer,
                            UInt uiSize );
    ErrVal xRemovePicBuffer(PicBufferList& rcPicBufferUnusedList,
                            UInt uiLayer);

    ErrVal xWrite (ExtBinDataAccessorList& rcList,
                   UInt& ruiBytesInFrame);
    ErrVal xRelease (ExtBinDataAccessorList& rcList);

    ErrVal xWrite (PicBufferList& rcList,
                   UInt uiLayer);
    ErrVal xRelease (PicBufferList& rcList,
                     UInt uiLayer);
#if DOLBY_ENCMUX_ENABLE
    //Dolby muxing functions
    void sbsMux(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height, Int offset0=0, Int offset1=1, Int iFilterIdx=7);
    ErrVal padBuf(UChar *output, Int iStrideOut, Int width, Int height, Int width_out, Int height_out, Int fillMode);
    void sbsMuxFR(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height);
    void tabMux(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height, Int offset0=0, Int offset1=1, Int iFilterIdx=7);
    void tabMuxFR(UChar *output, Int iStrideOut, UChar *input0, UChar *input1, Int iStrideIn, Int width, Int height);
#endif

protected:
    EncoderIoParameter            m_cEncoderIoParameter;
    EncoderCodingParameter*       m_pcEncoderCodingParameter;
    JSVM::CreaterH264AVCEncoder*  m_pcH264AVCEncoder;
    //保存压缩后的输出
    WriteBitstreamToFile*         m_pcWriteBitstreamToFile;
    //保存reconstruct后的yuv文件
    WriteYuvToFile*               m_apcWriteYuv[MAX_LAYERS];
    //加载的yuv输入文件
    ReadYuvFile*                  m_apcReadYuv[MAX_LAYERS];

    PicBufferList  m_acActivePicBufferList[MAX_LAYERS];
    PicBufferList  m_acUnusedPicBufferList[MAX_LAYERS];
    UInt           m_auiLumOffset[MAX_LAYERS];
    UInt           m_auiCbOffset[MAX_LAYERS];
    UInt           m_auiCrOffset[MAX_LAYERS];
    UInt           m_auiHeight[MAX_LAYERS];
    UInt           m_auiWidth [MAX_LAYERS];
    UInt           m_auiStride[MAX_LAYERS];
    UInt           m_aauiCropping[MAX_LAYERS][4];

    UChar          m_aucStartCodeBuffer[5];
    //0x00 0x00 0x00 0x01
    BinData        m_cBinDataStartCode;
    //输出的h264文件名
    std::string    m_cWriteToBitFileName;
    //输出的h264文件名 + .temp
    std::string    m_cWriteToBitFileTempName;
};




#endif //_H264AVCENCODERTEST_H_
