#ifndef _DISTORTIONIF_H_
#define _DISTORTIONIF_H_

#include "Typedefs.h"

namespace JSVM {


class YuvMbBuffer;
class YuvPicBuffer;
class XDistortion;
class XDistSearchStruct;

typedef UInt (*XDistortionFunc)( XDistSearchStruct*);


class XDistSearchStruct
{
public:
    XPel*  pYOrg;
    XPel*  pYFix;
    XPel*  pYSearch;
    Int    iYStride;
    XPel*  pUOrg;
    XPel*  pVOrg;
    XPel*  pUFix;
    XPel*  pVFix;
    XPel*  pUSearch;
    XPel*  pVSearch;
    Int    iCStride;
    Int    iRows;
    XDistortionFunc Func;
};

class XDistortionIf
{
protected:
    XDistortionIf() {}
    virtual ~XDistortionIf() {}

public:
    virtual Void  loadOrgMbPelData(const YuvPicBuffer* pcOrgYuvBuffer, YuvMbBuffer*& rpcOrgMbBuffer) = 0;

    virtual UInt  get8x8Cb   (XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD) = 0;
    virtual UInt  get8x8Cr   (XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD) = 0;
    virtual UInt  getLum16x16(XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD) = 0;
    virtual UInt  getLum8x8  (XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD) = 0;
    virtual UInt  getLum4x4  (XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD) = 0;
//TMM_WP
    ErrVal getLumaWeight   (YuvPicBuffer* pcOrgPicBuffer, YuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiLumaLog2WeightDenom);
    ErrVal getChromaWeight (YuvPicBuffer* pcOrgPicBuffer, YuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiChromaLog2WeightDenom, Bool bCb);

    ErrVal getLumaOffsets  (YuvPicBuffer* pcOrgPicBuffer, YuvPicBuffer* pcRefPicBuffer, Double& rfOffset);
    ErrVal getChromaOffsets(YuvPicBuffer* pcOrgPicBuffer, YuvPicBuffer* pcRefPicBuffer, Double& rfOffset, Bool bCb);
//TMM_WP
};



}  //namespace JSVM {


#endif //_DISTORTIONIF_H_
