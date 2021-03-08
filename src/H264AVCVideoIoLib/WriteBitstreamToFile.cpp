#include "H264AVCVideoIoLib.h"
#include "WriteBitstreamToFile.h"


ErrVal WriteBitstreamToFile::create(WriteBitstreamToFile*& rpcWriteBitstreamToFile)
{
    rpcWriteBitstreamToFile = new WriteBitstreamToFile;
    ROT(NULL == rpcWriteBitstreamToFile);
    return Err::m_nOK;
}



ErrVal WriteBitstreamToFile::init(const std::string& rcFileName, Bool bNewFileOnNewAu)
{
    m_bNewFileOnNewAu = bNewFileOnNewAu;
    m_cFileName = rcFileName;
    if(Err::m_nOK != m_cFile.open(rcFileName, LargeFile::OM_WRITEONLY))
    {
        std::cerr << "Failed to create output bitstream " << rcFileName.data() << std::endl;
        return Err::m_nERR;
    }
    else
    {
        printf("打开输出文件%s\n", rcFileName.c_str());
    }

    m_uiNumber = 0;
    return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::uninit()
{
    if(m_cFile.is_open())
    {
        m_cFile.close();
    }
    return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::destroy()
{
    ROT(m_cFile.is_open());
    uninit();
    delete this;
    return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::writePacket(BinData* pcBinData, Bool bNewAU)
{
    BinDataAccessor cBinDataAccessor;
    pcBinData->setMemAccessor(cBinDataAccessor);
    writePacket(&cBinDataAccessor, bNewAU);
    return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::writePacket(BinDataAccessor* pcBinDataAccessor, Bool bNewAU)
{
    ROTRS(NULL == pcBinDataAccessor, Err::m_nOK);

    if(bNewAU && m_bNewFileOnNewAu)
    {
        std::cerr << "multiple output bitstreams only supported in Win32";
        AF();
    }

    if(0 != pcBinDataAccessor->size())
    {
        m_cFile.write(pcBinDataAccessor->data(), pcBinDataAccessor->size());
    }

    return Err::m_nOK;
}

ErrVal WriteBitstreamToFile::writePacket(Void* pBuffer, UInt uiLength)
{
    if(uiLength)
    {
        m_cFile.write(pBuffer, uiLength);
    }
    return Err::m_nOK;
}
