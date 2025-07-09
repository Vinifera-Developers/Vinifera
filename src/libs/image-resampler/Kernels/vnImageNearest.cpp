
#include "vnImageNearest.h"

#include "../Utilities/vnImageBlock.h"

VN_STATUS vnNearestKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID(pSrcImage) || !pRawOutput )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    INT32 iX = (INT32) ( fX + 0.5f );
    INT32 iY = (INT32) ( fY + 0.5f );

    //
    // Floating point pixel coordinates are pixel-center based. Thus, a coordinate 
    // of (0,0) refers to the center of the first pixel in an image, and a coordinate
    // of (0.5,0) refers to the border between the first and second pixels.
    //

    if ( iX < 0 ) iX = 0;
    if ( iX > pSrcImage.QueryWidth() - 1 ) iX = pSrcImage.QueryWidth() - 1;

    if ( iY < 0 ) iY = 0;
    if ( iY > pSrcImage.QueryHeight() - 1 ) iY = pSrcImage.QueryHeight() - 1;

    //
    // Sample our pixel and write it to the output buffer.
    //

    UINT8 * pSrcPixel = pSrcImage.QueryData() + pSrcImage.BlockOffset( iX, iY );

    memcpy( pRawOutput, pSrcPixel, pSrcImage.QueryBitsPerPixel() >> 3 );

    return VN_SUCCESS;
}