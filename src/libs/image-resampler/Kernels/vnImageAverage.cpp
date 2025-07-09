
#include "vnImageAverage.h"

#include "../Utilities/vnImageBlock.h"

VN_STATUS vnAverageKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
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

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += gTempBlock.iChannelData[3];
        }

        fSampleCount++;
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

VN_STATUS vnSampleAverageVertical( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
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

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += gTempBlock.iChannelData[3];
        }

        fSampleCount++;
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

VN_STATUS vnSampleAverageHorizontal( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, FLOAT32 fRadius, UINT8 * pRawOutput )
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

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &gTempBlock );

        if ( VN_IMAGE_PRECISION_FLOAT == gTempBlock.uiPrecision )
        {
            gTotalBlock.fChannelData[0] += gTempBlock.fChannelData[0];
            gTotalBlock.fChannelData[1] += gTempBlock.fChannelData[1];
            gTotalBlock.fChannelData[2] += gTempBlock.fChannelData[2];
            gTotalBlock.fChannelData[3] += gTempBlock.fChannelData[3];
        }
        else
        {
            gTotalBlock.iChannelData[0] += gTempBlock.iChannelData[0];
            gTotalBlock.iChannelData[1] += gTempBlock.iChannelData[1];
            gTotalBlock.iChannelData[2] += gTempBlock.iChannelData[2];
            gTotalBlock.iChannelData[3] += gTempBlock.iChannelData[3];
        }

        fSampleCount++;
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

VN_STATUS vnAverageKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, BOOL bDirection, FLOAT32 fRadius, UINT8 * pRawOutput )
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
        case FALSE: return vnSampleAverageHorizontal( pSrcImage, fX, fY, fRadius, pRawOutput );
        case TRUE:  return vnSampleAverageVertical( pSrcImage, fX, fY, fRadius, pRawOutput );
    }

    return VN_ERROR_NOTIMPL;
}