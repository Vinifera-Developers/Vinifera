
#include "vnImageCoverage.h"

#include "../Utilities/vnImageBlock.h"

VN_STATUS vnCoverageKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == fRadius || !pRawOutput || !VN_IS_IMAGE_VALID(pSrcImage) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    FLOAT32 fSampleCount            = 0;
    VN_PIXEL_BLOCK gTotalBlock      = {0};
    VN_PIXEL_BLOCK gTempBlock       = {0};
    INT32 iRadius                   = fRadius + 1.0f;

    //
    // Scan the kernel space adding up the pixel values
    //

    FLOAT32 fMaxDistance = sqrtf( (FLOAT32) fRadius * fRadius + fRadius * fRadius );

    for ( INT32 j = -iRadius + 1; j <= iRadius; j++ )
    for ( INT32 i = -iRadius + 1; i <= iRadius; i++ )
    {
        INT32 iX = (INT32) fX + i;
        INT32 iY = (INT32) fY + j;

        if ( iX < 0 || iY < 0 || 
             iX > pSrcImage.QueryWidth() - 1 || 
             iY > pSrcImage.QueryHeight() - 1 )
        {
             continue;
        }

        UINT8 * pSrcPixel   = pSrcImage.QueryData() + pSrcImage.BlockOffset( iX, iY );

        FLOAT32 fXDelta     = (FLOAT32) fX - iX;
        FLOAT32 fYDelta     = (FLOAT32) fY - iY;
        FLOAT32 fDistance   = sqrtf( fXDelta * fXDelta + fYDelta * fYDelta );
        FLOAT32 fWeight     = 0.0f;

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        //
        // The coverage filter performs slightly differently based on whether the image is
        // being magnified or minified. If the image is not undergoing a scaling operation,
        // the coverage filter will pass-through the image without making any modification.
        //

        //
        // Minification: computes a simple distance based weighted average:
        //

        if ( fRadius >= 1.0 )
        {
            fDistance   = VN_MIN2( fMaxDistance, fDistance );

            fWeight     = 1.0f - fDistance / fMaxDistance;
        }

        //
        // Magnification: 
        // 
        //   If the destination pixel is significantly far away from any source pixel, then we
        //   interpolate to find a new value.
        // 
        //   If the destination pixel is sufficiently close to a source pixel, we sample it 
        //   directly and pass it to the output.
        //
    
        else
        {
            if ( fDistance >= 0.5f - fRadius )
            {
                fWeight = 1.0f - fDistance;
            }
            else
            {
                return vnConvertFromBlock( gTempBlock, pSrcImage.QueryFormat(), pRawOutput );
            }        
        }

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += fWeight * gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += fWeight * gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += fWeight * gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += fWeight * gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += fWeight * gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += fWeight * gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += fWeight * gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += fWeight * gTempBlock.iChannelData[3];
        }

        fSampleCount += fWeight;
    }

    //
    // Normalize our simple sum back to the valid pixel range
    //

    FLOAT32 fScaleFactor = 1.0f / fSampleCount;

    gTotalBlock.uiPrecision    = gTempBlock.uiPrecision;
    gTotalBlock.uiChannelCount = gTempBlock.uiChannelCount;

    if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
    {
        gTotalBlock.fChannelData[0] = fScaleFactor * gTotalBlock.fChannelData[0];
        gTotalBlock.fChannelData[1] = fScaleFactor * gTotalBlock.fChannelData[1];
        gTotalBlock.fChannelData[2] = fScaleFactor * gTotalBlock.fChannelData[2];
        gTotalBlock.fChannelData[3] = fScaleFactor * gTotalBlock.fChannelData[3];
    }
    else
    {
        gTotalBlock.iChannelData[0] = fScaleFactor * gTotalBlock.iChannelData[0];
        gTotalBlock.iChannelData[1] = fScaleFactor * gTotalBlock.iChannelData[1];
        gTotalBlock.iChannelData[2] = fScaleFactor * gTotalBlock.iChannelData[2];
        gTotalBlock.iChannelData[3] = fScaleFactor * gTotalBlock.iChannelData[3];
    }

    //
    // Write our weighted sum to our output
    //

    vnConvertFromBlock( gTotalBlock, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;
}

VN_STATUS vnSampleCoverageVertical( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == fRadius || !pRawOutput || !VN_IS_IMAGE_VALID(pSrcImage) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    FLOAT32 fSampleCount            = 0;
    VN_PIXEL_BLOCK gTotalBlock      = {0};
    VN_PIXEL_BLOCK gTempBlock       = {0};
    INT32 iRadius                   = fRadius + 1.0f;

    //
    // Scan the kernel space adding up the pixel values
    //

    FLOAT32 fMaxDistance = fRadius;

    for ( INT32 j = -iRadius + 1; j <= iRadius; j++ )
    {
        INT32 iX = (INT32) fX;
        INT32 iY = (INT32) fY + j;

        if ( iX < 0 || iY < 0 || 
             iX > pSrcImage.QueryWidth() - 1 || 
             iY > pSrcImage.QueryHeight() - 1 )
        {
             continue;
        }

        UINT8 * pSrcPixel   = pSrcImage.QueryData() + pSrcImage.BlockOffset( iX, iY );

        FLOAT32 fYDelta     = (FLOAT32) fY - iY;
        FLOAT32 fDistance   = fabs( fYDelta );
        FLOAT32 fWeight     = 0.0f;

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        //
        // If we're minifying, then we can compute a simple distance based weighted average 
        //    using our calculated radius (fRadius)
        //

        if ( fRadius >= 1.0 )
        {
            fDistance   = VN_MIN2( fMaxDistance, fDistance );
            fWeight     = 1.0f - fDistance / fMaxDistance;
        }
        else
        {
            if ( fDistance >= 0.5f - fRadius )
            {
                fWeight = 1.0f - fDistance;
            }
            else
            {
                return vnConvertFromBlock( gTempBlock, pSrcImage.QueryFormat(), pRawOutput );
            }
        }

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += fWeight * gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += fWeight * gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += fWeight * gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += fWeight * gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += fWeight * gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += fWeight * gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += fWeight * gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += fWeight * gTempBlock.iChannelData[3];
        }

        fSampleCount += fWeight;
    }

    //
    // Normalize our sum back to the valid pixel range
    //

    FLOAT32 fScaleFactor = 1.0f / fSampleCount;

    gTotalBlock.uiPrecision    = gTempBlock.uiPrecision;
    gTotalBlock.uiChannelCount = gTempBlock.uiChannelCount;

    if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
    {
        gTotalBlock.fChannelData[0] = fScaleFactor * gTotalBlock.fChannelData[0];
        gTotalBlock.fChannelData[1] = fScaleFactor * gTotalBlock.fChannelData[1];
        gTotalBlock.fChannelData[2] = fScaleFactor * gTotalBlock.fChannelData[2];
        gTotalBlock.fChannelData[3] = fScaleFactor * gTotalBlock.fChannelData[3];
    }
    else
    {
        gTotalBlock.iChannelData[0] = fScaleFactor * gTotalBlock.iChannelData[0];
        gTotalBlock.iChannelData[1] = fScaleFactor * gTotalBlock.iChannelData[1];
        gTotalBlock.iChannelData[2] = fScaleFactor * gTotalBlock.iChannelData[2];
        gTotalBlock.iChannelData[3] = fScaleFactor * gTotalBlock.iChannelData[3];
    }

    //
    // Write our weighted sum to our output
    //

    vnConvertFromBlock( gTotalBlock, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;
}

VN_STATUS vnSampleCoverageHorizontal( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == fRadius || !pRawOutput || !VN_IS_IMAGE_VALID(pSrcImage) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    FLOAT32 fSampleCount            = 0;
    VN_PIXEL_BLOCK gTotalBlock      = {0};
    VN_PIXEL_BLOCK gTempBlock       = {0};
    INT32 iRadius                   = fRadius + 1.0f;
 
    //
    // Scan the kernel space adding up the pixel values
    //

    FLOAT32 fMaxDistance = fRadius;

    for ( INT32 i = -iRadius + 1; i <= iRadius; i++ )
    {
        INT32 iX = (INT32) fX + i;
        INT32 iY = (INT32) fY;

        if ( iX < 0 || iY < 0 || 
                iX > pSrcImage.QueryWidth() - 1 || 
                iY > pSrcImage.QueryHeight() - 1 )
        {
                continue;
        }

        UINT8 * pSrcPixel   = pSrcImage.QueryData() + pSrcImage.BlockOffset( iX, iY );

        FLOAT32 fXDelta     = (FLOAT32) fX - iX;
        FLOAT32 fDistance   = fabs( fXDelta );
        FLOAT32 fWeight     = 0.0f;

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        if ( fRadius >= 1.0 )
        {
            fDistance   = VN_MIN2( fMaxDistance, fDistance );
            fWeight     = 1.0f - fDistance / fMaxDistance;
        }
    
        else
        {
            if ( fDistance >= 0.5f - fRadius )
            {
                fWeight = 1.0f - fDistance;
            }
            else
            {
                return vnConvertFromBlock( gTempBlock, pSrcImage.QueryFormat(), pRawOutput );
            }
        }

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += fWeight * gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += fWeight * gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += fWeight * gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += fWeight * gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += fWeight * gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += fWeight * gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += fWeight * gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += fWeight * gTempBlock.iChannelData[3];
        }

        fSampleCount += fWeight;
    }

    //
    // Normalize our sum back to the valid pixel range
    //

    FLOAT32 fScaleFactor = 1.0f / fSampleCount;

    gTotalBlock.uiPrecision    = gTempBlock.uiPrecision;
    gTotalBlock.uiChannelCount = gTempBlock.uiChannelCount;

    if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
    {
        gTotalBlock.fChannelData[0] = fScaleFactor * gTotalBlock.fChannelData[0];
        gTotalBlock.fChannelData[1] = fScaleFactor * gTotalBlock.fChannelData[1];
        gTotalBlock.fChannelData[2] = fScaleFactor * gTotalBlock.fChannelData[2];
        gTotalBlock.fChannelData[3] = fScaleFactor * gTotalBlock.fChannelData[3];
    }
    else
    {
        gTotalBlock.iChannelData[0] = fScaleFactor * gTotalBlock.iChannelData[0];
        gTotalBlock.iChannelData[1] = fScaleFactor * gTotalBlock.iChannelData[1];
        gTotalBlock.iChannelData[2] = fScaleFactor * gTotalBlock.iChannelData[2];
        gTotalBlock.iChannelData[3] = fScaleFactor * gTotalBlock.iChannelData[3];
    }

    //
    // Write our weighted sum to our output
    //

    vnConvertFromBlock( gTotalBlock, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;
}

VN_STATUS vnCoverageKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, BOOL bDirection, FLOAT32 fRadius, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == fRadius || !pRawOutput || !VN_IS_IMAGE_VALID(pSrcImage) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Compute the horizontal or vertical average sample at the requested pixel coordinate.
    //
    
    switch ( bDirection )
    {
        case FALSE: return vnSampleCoverageHorizontal( pSrcImage, fX, fY, fRadius, pRawOutput );
        case TRUE:  return vnSampleCoverageVertical( pSrcImage, fX, fY, fRadius, pRawOutput );
    }

    return VN_ERROR_NOTIMPL;
}