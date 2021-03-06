QT += core
QT -= gui

CONFIG += c++11

TARGET = jsvm_mini_encode
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += \
    _DEBUG \
     NO_DEBUG \
#RecPic
    WITH_RECPIC \
#打印Pruefpunkt
#    PRUEF_PUNKT \
    MSYS_LINUX \
    MSYS_UNIX_LARGEFILE \
#    PROPOSED_DEBLOCKING_APRIL2010 \
#追踪
    ENCODER_TRACE \
#deblocking according to the proposal in April 2010 (cp. VCEG_AN10_r3)
    PROPOSED_DEBLOCKING_APRIL2010 \

HEADERS += \
#Include
    include/CodingParameter.h \
    include/CreaterH264AVCDecoder.h \
    include/CreaterH264AVCEncoder.h \
    include/DownConvert.h \
    include/H264AVCCommonIf.h \
    include/H264AVCCommonLib.h \
    include/H264AVCDecoderLib.h \
    include/H264AVCEncoderLib.h \
    include/H264AVCVideoIoLib.h \
    include/H264AVCVideoLib.h \
    include/LargeFile.h \
    include/Macros.h \
    include/MemAccessor.h \
    include/MemCont.h \
    include/MemIf.h \
    include/MemList.h \
    include/RateCtlBase.h \
    include/RateCtlQuadratic.h \
    include/ReadBitstreamFile.h \
    include/ReadBitstreamIf.h \
    include/ReadPEStream.h \
    include/ReadYuvFile.h \
    include/ResizeParameters.h \
    include/Typedefs.h \
    include/WriteBitstreamIf.h \
    include/WriteBitstreamToFile.h \
    include/WriteYuvIf.h \
    include/WriteYuvToFile.h \
    include/H264AVCCommonLib/CabacContextModel.h \
    include/H264AVCCommonLib/CabacContextModel2DBuffer.h \
    include/H264AVCCommonLib/CabacTables.h \
    include/H264AVCCommonLib/CFMO.h \
    include/H264AVCCommonLib/CommonBuffers.h \
    include/H264AVCCommonLib/CommonDefs.h \
    include/H264AVCCommonLib/CommonTypes.h \
    include/H264AVCCommonLib/ContextTables.h \
    include/H264AVCCommonLib/ControlMngIf.h \
    include/H264AVCCommonLib/Frame.h \
    include/H264AVCCommonLib/GlobalFunctions.h \
    include/H264AVCCommonLib/HeaderSymbolReadIf.h \
    include/H264AVCCommonLib/HeaderSymbolWriteIf.h \
    include/H264AVCCommonLib/Hrd.h \
    include/H264AVCCommonLib/IntraPrediction.h \
    include/H264AVCCommonLib/LoopFilter.h \
    include/H264AVCCommonLib/Macros.h \
    include/H264AVCCommonLib/MbData.h \
    include/H264AVCCommonLib/MbDataAccess.h \
    include/H264AVCCommonLib/MbDataCtrl.h \
    include/H264AVCCommonLib/MbDataStruct.h \
    include/H264AVCCommonLib/MbMvData.h \
    include/H264AVCCommonLib/MbTransformCoeffs.h \
    include/H264AVCCommonLib/MotionCompensation.h \
    include/H264AVCCommonLib/MotionVectorCalculation.h \
    include/H264AVCCommonLib/Mv.h \
    include/H264AVCCommonLib/ParameterSetMng.h \
    include/H264AVCCommonLib/PictureParameterSet.h \
    include/H264AVCCommonLib/PocCalculator.h \
    include/H264AVCCommonLib/Quantizer.h \
    include/H264AVCCommonLib/QuarterPelFilter.h \
    include/H264AVCCommonLib/ReconstructionBypass.h \
    include/H264AVCCommonLib/SampleWeighting.h \
    include/H264AVCCommonLib/ScalingMatrix.h \
    include/H264AVCCommonLib/Sei.h \
    include/H264AVCCommonLib/SequenceParameterSet.h \
    include/H264AVCCommonLib/SliceHeader.h \
    include/H264AVCCommonLib/SliceHeaderBase.h \
    include/H264AVCCommonLib/Tables.h \
    include/H264AVCCommonLib/TraceFile.h \
    include/H264AVCCommonLib/Transform.h \
    include/H264AVCCommonLib/Vui.h \
    include/H264AVCCommonLib/YuvBufferCtrl.h \
    include/H264AVCCommonLib/YuvBufferIf.h \
    include/H264AVCCommonLib/YuvMbBuffer.h \
    include/H264AVCCommonLib/YuvPicBuffer.h \
#Common
    src/H264AVCCommonLib/resource.h \
#Encoder
    src/H264AVCEncoderLib/BitCounter.h \
    src/H264AVCEncoderLib/BitWriteBuffer.h \
    src/H264AVCEncoderLib/BitWriteBufferIf.h \
    src/H264AVCEncoderLib/CabacWriter.h \
    src/H264AVCEncoderLib/CabaEncoder.h \
    src/H264AVCEncoderLib/ControlMngH264AVCEncoder.h \
    src/H264AVCEncoderLib/Distortion.h \
    src/H264AVCEncoderLib/DistortionIf.h \
    src/H264AVCEncoderLib/GOPEncoder.h \
    src/H264AVCEncoderLib/H264AVCEncoder.h \
    src/H264AVCEncoderLib/H264AVCEncoderLib.rc \
    src/H264AVCEncoderLib/InputPicBuffer.h \
    src/H264AVCEncoderLib/IntraPredictionSearch.h \
    src/H264AVCEncoderLib/MbCoder.h \
    src/H264AVCEncoderLib/MbEncoder.h \
    src/H264AVCEncoderLib/MbSymbolWriteIf.h \
    src/H264AVCEncoderLib/MbTempData.h \
    src/H264AVCEncoderLib/MotionEstimation.h \
    src/H264AVCEncoderLib/MotionEstimationCost.h \
    src/H264AVCEncoderLib/MotionEstimationQuarterPel.h \
    src/H264AVCEncoderLib/NalUnitEncoder.h \
    src/H264AVCEncoderLib/PicEncoder.h \
    src/H264AVCEncoderLib/RateDistortion.h \
    src/H264AVCEncoderLib/RateDistortionIf.h \
    src/H264AVCEncoderLib/RecPicBuffer.h \
    src/H264AVCEncoderLib/resource.h \
    src/H264AVCEncoderLib/Scheduler.h \
    src/H264AVCEncoderLib/SequenceStructure.h \
    src/H264AVCEncoderLib/SliceEncoder.h \
    src/H264AVCEncoderLib/UvlcWriter.h \
#Video
    src/H264AVCVideoIoLib/H264AVCVideoIoLib.rc \
    src/H264AVCVideoIoLib/resource.h \
    src/H264AVCVideoIoLib/WriteYuvaToRgb.h \
##EncoderTest
    main/H264AVCEncoderLibTest/EncoderCodingParameter.h \
    main/H264AVCEncoderLibTest/H264AVCEncoderLibTest.h \
    main/H264AVCEncoderLibTest/H264AVCEncoderTest.h \
    main/H264AVCEncoderLibTest/img_process_mux.h \


SOURCES += \
#Common
    src/H264AVCCommonLib/CabacContextModel.cpp \
    src/H264AVCCommonLib/CabacContextModel2DBuffer.cpp \
    src/H264AVCCommonLib/CFMO.cpp \
    src/H264AVCCommonLib/DownConvert.cpp \
    src/H264AVCCommonLib/Frame.cpp \
    src/H264AVCCommonLib/H264AVCCommonLib.cpp \
    src/H264AVCCommonLib/Hrd.cpp \
    src/H264AVCCommonLib/IntraPrediction.cpp \
    src/H264AVCCommonLib/LoopFilter.cpp \
    src/H264AVCCommonLib/MbData.cpp \
    src/H264AVCCommonLib/MbDataAccess.cpp \
    src/H264AVCCommonLib/MbDataCtrl.cpp \
    src/H264AVCCommonLib/MbDataStruct.cpp \
    src/H264AVCCommonLib/MbMvData.cpp \
    src/H264AVCCommonLib/MbTransformCoeffs.cpp \
    src/H264AVCCommonLib/MotionCompensation.cpp \
    src/H264AVCCommonLib/MotionVectorCalculation.cpp \
    src/H264AVCCommonLib/Mv.cpp \
    src/H264AVCCommonLib/ParameterSetMng.cpp \
    src/H264AVCCommonLib/PictureParameterSet.cpp \
    src/H264AVCCommonLib/PocCalculator.cpp \
    src/H264AVCCommonLib/Quantizer.cpp \
    src/H264AVCCommonLib/QuarterPelFilter.cpp \
    src/H264AVCCommonLib/ReconstructionBypass.cpp \
    src/H264AVCCommonLib/ResizeParameters.cpp \
    src/H264AVCCommonLib/SampleWeighting.cpp \
    src/H264AVCCommonLib/ScalingMatrix.cpp \
    src/H264AVCCommonLib/Sei.cpp \
    src/H264AVCCommonLib/SequenceParameterSet.cpp \
    src/H264AVCCommonLib/SliceHeader.cpp \
    src/H264AVCCommonLib/SliceHeaderBase.cpp \
    src/H264AVCCommonLib/Tables.cpp \
    src/H264AVCCommonLib/TraceFile.cpp \
    src/H264AVCCommonLib/Transform.cpp \
    src/H264AVCCommonLib/Vui.cpp \
    src/H264AVCCommonLib/YuvBufferCtrl.cpp \
    src/H264AVCCommonLib/YuvMbBuffer.cpp \
    src/H264AVCCommonLib/YuvPicBuffer.cpp \
#Encoder
    src/H264AVCEncoderLib/BitCounter.cpp \
    src/H264AVCEncoderLib/BitWriteBuffer.cpp \
    src/H264AVCEncoderLib/CabacWriter.cpp \
    src/H264AVCEncoderLib/CabaEncoder.cpp \
    src/H264AVCEncoderLib/CodingParameter.cpp \
    src/H264AVCEncoderLib/ControlMngH264AVCEncoder.cpp \
    src/H264AVCEncoderLib/CreaterH264AVCEncoder.cpp \
    src/H264AVCEncoderLib/Distortion.cpp \
    src/H264AVCEncoderLib/GOPEncoder.cpp \
    src/H264AVCEncoderLib/H264AVCEncoder.cpp \
    src/H264AVCEncoderLib/H264AVCEncoderLib.cpp \
    src/H264AVCEncoderLib/InputPicBuffer.cpp \
    src/H264AVCEncoderLib/IntraPredictionSearch.cpp \
    src/H264AVCEncoderLib/MbCoder.cpp \
    src/H264AVCEncoderLib/MbEncoder.cpp \
    src/H264AVCEncoderLib/MbTempData.cpp \
    src/H264AVCEncoderLib/MotionEstimation.cpp \
    src/H264AVCEncoderLib/MotionEstimationCost.cpp \
    src/H264AVCEncoderLib/MotionEstimationQuarterPel.cpp \
    src/H264AVCEncoderLib/NalUnitEncoder.cpp \
    src/H264AVCEncoderLib/PicEncoder.cpp \
    src/H264AVCEncoderLib/RateCtlBase.cpp \
    src/H264AVCEncoderLib/RateCtlQuadratic.cpp \
    src/H264AVCEncoderLib/RateDistortion.cpp \
    src/H264AVCEncoderLib/RecPicBuffer.cpp \
    src/H264AVCEncoderLib/Scheduler.cpp \
    src/H264AVCEncoderLib/SequenceStructure.cpp \
    src/H264AVCEncoderLib/SliceEncoder.cpp \
    src/H264AVCEncoderLib/UvlcWriter.cpp \
#Video
    src/H264AVCVideoIoLib/H264AVCVideoIoLib.cpp \
    src/H264AVCVideoIoLib/LargeFile.cpp \
    src/H264AVCVideoIoLib/ReadBitstreamFile.cpp \
    src/H264AVCVideoIoLib/ReadYuvFile.cpp \
    src/H264AVCVideoIoLib/WriteBitstreamToFile.cpp \
    src/H264AVCVideoIoLib/WriteYuvaToRgb.cpp \
    src/H264AVCVideoIoLib/WriteYuvToFile.cpp \
#EncoderTest
    main/H264AVCEncoderLibTest/EncoderCodingParameter.cpp \
    main/H264AVCEncoderLibTest/H264AVCEncoderLibTest.cpp \
    main/H264AVCEncoderLibTest/H264AVCEncoderTest.cpp \
    main/H264AVCEncoderLibTest/img_process_mux_sbs.cpp \
    main/H264AVCEncoderLibTest/img_process_mux_tab.cpp \

INCLUDEPATH += \
    include \
    src/lib \
#Common
    src/H264AVCCommonLib \
#Encoder
    src/H264AVCEncoderLib \
#Video
    src/H264AVCVideoIoLib \
