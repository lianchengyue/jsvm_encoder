#include "H264AVCEncoderLibTest.h"
#include "H264AVCEncoderTest.h"
#include "H264AVCCommonLib/CommonBuffers.h"


//profile_idc == 100     // High profile
//profile_idc == 110     // High10 profile
//profile_idc == 122     // High422 profile
//profile_idc == 244     // High444 Predictive profile
//profile_idc ==  44     // Cavlc444 profile
//profile_idc ==  83     // Scalable Constrained High profile (SVC)
//profile_idc ==  86     // Scalable High Intra profile (SVC)
//profile_idc == 118     // Stereo High profile (MVC)
//profile_idc == 128     // Multiview High profile (MVC)
//profile_idc == 138     // Multiview Depth High profile (MVCD)
//profile_idc == 144     // old High444 profile

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
