
#include "vnImageBilinear.h"

#include "../Utilities/vnImageBlock.h"

VN_STATUS vnLerpBlocks( CONST VN_PIXEL_BLOCK & blockA, CONST VN_PIXEL_BLOCK & blockB, FLOAT32 fDelta, VN_PIXEL_BLOCK * pOutBlock )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pOutBlock )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    if ( blockA.uiPrecision != blockB.uiPrecision )
    {
        return vnPostError( VN_ERROR_INVALID_RESOURCE );
    }

    if ( VN_IMAGE_PRECISION_FLOAT == blockA.uiPrecision )
    {
        pOutBlock->fChannelData[0] = blockA.fChannelData[0] * ( 1.0 - fDelta ) + blockB.fChannelData[0] * fDelta;
        pOutBlock->fChannelData[1] = blockA.fChannelData[1] * ( 1.0 - fDelta ) + blockB.fChannelData[1] * fDelta;
        pOutBlock->fChannelData[2] = blockA.fChannelData[2] * ( 1.0 - fDelta ) + blockB.fChannelData[2] * fDelta;
        pOutBlock->fChannelData[3] = blockA.fChannelData[3] * ( 1.0 - fDelta ) + blockB.fChannelData[3] * fDelta;
    }
    else
    {
        pOutBlock->iChannelData[0] = (UINT64) ( (FLOAT64) blockA.iChannelData[0] * ( 1.0 - fDelta ) + blockB.iChannelData[0] * fDelta );
        pOutBlock->iChannelData[1] = (UINT64) ( (FLOAT64) blockA.iChannelData[1] * ( 1.0 - fDelta ) + blockB.iChannelData[1] * fDelta );
        pOutBlock->iChannelData[2] = (UINT64) ( (FLOAT64) blockA.iChannelData[2] * ( 1.0 - fDelta ) + blockB.iChannelData[2] * fDelta );
        pOutBlock->iChannelData[3] = (UINT64) ( (FLOAT64) blockA.iChannelData[3] * ( 1.0 - fDelta ) + blockB.iChannelData[3] * fDelta );
    }

    return VN_SUCCESS;
}

VN_STATUS vnBilinearKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( fX < 0 || fY < 0 || !VN_IS_IMAGE_VALID(pSrcImage) || !pRawOutput )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Initially we perform a horizontal bilinear kernel on our top row of source pixels
    //

    VN_PIXEL_BLOCK biBlocks[2]      = {0};
    VN_PIXEL_BLOCK biTotalBlockA    = {0};
    VN_PIXEL_BLOCK biTotalBlockB    = {0};
    VN_PIXEL_BLOCK biTotalBlockOut  = {0};

    //
    // We do not bias our float coordinate by 0.5 because we wish
    // to sample using the nearest 2 pixels to our coordinate.  
    //

    INT32 iSampleX = fX;
    INT32 iSampleY = fY;

    FLOAT32 fXDelta = (FLOAT32) fX - iSampleX;
    FLOAT32 fYDelta = (FLOAT32) fY - iSampleY;

    for ( UINT32 i = 0; i < 2; i++ )
    {
        INT32 iSourceX = vnClipRange( iSampleX + i, 0, pSrcImage.QueryWidth() - 1 );
        INT32 iSourceY = vnClipRange( iSampleY, 0, pSrcImage.QueryHeight() - 1 );

        UINT8 * pSrcPixel = pSrcImage.QueryData() + pSrcImage.BlockOffset( iSourceX, iSourceY );

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &biBlocks[ i ] );
    }

    biTotalBlockA.uiPrecision    = biBlocks[0].uiPrecision;
    biTotalBlockA.uiChannelCount = biBlocks[0].uiChannelCount;

    if ( VN_FAILED( vnLerpBlocks( biBlocks[0], biBlocks[1], fXDelta, &biTotalBlockA ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }   

    //
    // Next we perform a horizontal bilinear kernel on our bottom row of source pixels
    //

    for ( UINT32 i = 0; i < 2; i++ )
    {
        INT32 iSourceX = vnClipRange( iSampleX + i, 0, pSrcImage.QueryWidth() - 1 );
        INT32 iSourceY = vnClipRange( iSampleY + 1, 0, pSrcImage.QueryHeight() - 1 );

        UINT8 * pSrcPixel = pSrcImage.QueryData() + pSrcImage.BlockOffset( iSourceX, iSourceY );

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &biBlocks[ i ] );
    }

    biTotalBlockB.uiPrecision    = biBlocks[0].uiPrecision;
    biTotalBlockB.uiChannelCount = biBlocks[0].uiChannelCount;

    if ( VN_FAILED( vnLerpBlocks( biBlocks[0], biBlocks[1], fXDelta, &biTotalBlockB ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }   

    //
    // Interpolate our new pixel color using the two source blocks
    //

    if ( VN_FAILED( vnLerpBlocks( biTotalBlockA, biTotalBlockB, fYDelta, &biTotalBlockOut ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }  

    //
    // Write our filtered result out
    //

    vnConvertFromBlock( biTotalBlockOut, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;  
}

VN_STATUS vnSampleBilinearHorizontal( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( fX < 0 || fY < 0 || !VN_IS_IMAGE_VALID(pSrcImage) || !pRawOutput )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Perform a bilinear kernel at the appointed location.
    //

    VN_PIXEL_BLOCK biBlocks[2]      = {0};
    VN_PIXEL_BLOCK biTotalBlock     = {0};

    //
    // We do not bias our float coordinate by 0.5 because we wish
    // to sample using the nearest 2 pixels to our coordinate.  
    //

    INT32 iSampleX = fX;
    INT32 iSampleY = fY;

    for ( UINT32 i = 0; i < 2; i++ )
    {
        INT32 iSourceX = vnClipRange( iSampleX + i, 0, pSrcImage.QueryWidth() - 1 );
        INT32 iSourceY = vnClipRange( iSampleY, 0, pSrcImage.QueryHeight() - 1 );

        UINT8 * pSrcPixel = pSrcImage.QueryData() + pSrcImage.BlockOffset( iSourceX, iSourceY );

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &biBlocks[ i ] );
    }

    //
    // Interpolate our new pixel color using the two source blocks
    //
    
    biTotalBlock.uiPrecision    = biBlocks[0].uiPrecision;
    biTotalBlock.uiChannelCount = biBlocks[0].uiChannelCount;

    //
    // Calculate our interpolation parameter
    //

    FLOAT32 fXDelta = (FLOAT32) fX - iSampleX;

    if ( VN_FAILED( vnLerpBlocks( biBlocks[0], biBlocks[1], fXDelta, &biTotalBlock ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }   

    //
    // Write our filtered result out
    //

    vnConvertFromBlock( biTotalBlock, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;  
}


VN_STATUS vnSampleBilinearVertical( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( fX < 0 || fY < 0 || !VN_IS_IMAGE_VALID(pSrcImage) || !pRawOutput )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Perform a bilinear kernel at the appointed location.
    //

    VN_PIXEL_BLOCK biBlocks[2]      = {0};
    VN_PIXEL_BLOCK biTotalBlock     = {0};

    //
    // We do not bias our float coordinate by 0.5 because we wish
    // to sample using the nearest 2 pixels to our coordinate.  
    //

    INT32 iSampleX = fX;
    INT32 iSampleY = fY;

    for ( UINT32 i = 0; i < 2; i++ )
    {
        INT32 iSourceX = vnClipRange( iSampleX, 0, pSrcImage.QueryWidth() - 1 );
        INT32 iSourceY = vnClipRange( iSampleY + i, 0, pSrcImage.QueryHeight() - 1 );

        UINT8 * pSrcPixel = pSrcImage.QueryData() + pSrcImage.BlockOffset( iSourceX, iSourceY );

        vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &biBlocks[ i ] );
    }

    //
    // Interpolate our new pixel color using the two source blocks
    //
    
    biTotalBlock.uiPrecision    = biBlocks[0].uiPrecision;
    biTotalBlock.uiChannelCount = biBlocks[0].uiChannelCount;

    //
    // Calculate our interpolation parameter
    //

    FLOAT32 fYDelta = (FLOAT32) fY - iSampleY;

    if ( VN_FAILED( vnLerpBlocks( biBlocks[0], biBlocks[1], fYDelta, &biTotalBlock ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }   

    //
    // Write our filtered result out
    //

    vnConvertFromBlock( biTotalBlock, pSrcImage.QueryFormat(), pRawOutput );

    return VN_SUCCESS;  
}

VN_STATUS vnBilinearKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, BOOL bDirection, UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pRawOutput || !VN_IS_IMAGE_VALID(pSrcImage) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Compute the horizontal or vertical gaussian sample at the requested pixel coordinate.
    //
    
    switch ( bDirection )
    {
        case FALSE: return vnSampleBilinearHorizontal( pSrcImage, fX, fY, pRawOutput );
        case TRUE:  return  vnSampleBilinearVertical( pSrcImage, fX, fY, pRawOutput );
    }

    return VN_ERROR_NOTIMPL;
}