#include "H264AVCEncoderLibTest.h"
#include "H264AVCEncoderTest.h"
#include "H264AVCCommonLib/CommonBuffers.h"

//!
//1: SVC
//-pf ../jsvm_mini_encode/cfg/encoder.cfg

//2: AVC
//-pf ../jsvm_mini_encode/cfg/MVC.cfg

//!
//1: SVC
// ./H264AVCEncoderLibTestStaticd -pf encoder.cfg
//2: AVC
// ~/git/jsvm/jsvm/bin
// ./H264AVCEncoderLibTestStaticd -pf MVC.cfg
int main( int argc, char** argv)
{
    printf("JSVM %s Encoder\n\n",_JSVM_VERSION_);

    H264AVCEncoderTest* pcH264AVCEncoderTest = NULL;
    H264AVCEncoderTest::create(pcH264AVCEncoderTest);

    //H264AVCEncoderTest的初始化
    pcH264AVCEncoderTest->init (argc, argv);
    pcH264AVCEncoderTest->go ();
    pcH264AVCEncoderTest->destroy ();

  return 0;
}
