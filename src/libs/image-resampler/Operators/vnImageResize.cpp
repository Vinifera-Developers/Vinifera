
#include "vnImageResize.h"

#include "../Operators/vnImageClone.h"

VN_STATUS vnResizeImageNonSeparable( CONST CVImage & pSrcImage, VN_IMAGE_KERNEL_TYPE uiKernel, FLOAT32 fHRatio, FLOAT32 fVRatio, CVImage * pDestImage )
{
    FLOAT32 fRadius = sqrtf( (FLOAT32) fHRatio * fHRatio + fVRatio * fVRatio );

    for ( UINT32 j = 0; j < pDestImage->QueryHeight(); j++ )
    for ( UINT32 i = 0; i < pDestImage->QueryWidth(); i++ )
    {
        UINT8 * pOutputData = pDestImage->QueryData() + pDestImage->BlockOffset( i, j );

        //
        // Determine the sub-pixel location of our *target* (i,j) coordinate, in the space
        // of our source image.
        //

        FLOAT32 fX = (FLOAT32) i * fHRatio;
        FLOAT32 fY = (FLOAT32) j * fVRatio;

        if ( VN_FAILED( vnSampleImage( pSrcImage, uiKernel, VN_IMAGE_KERNEL_2D_COMBINED, fX, fY, fRadius, pOutputData ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }
    }

    return VN_SUCCESS;
}

VN_STATUS vnResizeImageSeparable( CONST CVImage & pSrcImage, VN_IMAGE_KERNEL_TYPE uiKernel, FLOAT32 fHRatio, FLOAT32 fVRatio, CVImage * pDestImage )
{
    CVImage tempImage;

    if ( VN_FAILED( vnCreateImage( pSrcImage.QueryFormat(), pDestImage->QueryWidth(), pSrcImage.QueryHeight(), &tempImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    //
    // Perform the horizontal filter sampling.
    //

    for ( UINT32 j = 0; j < pSrcImage.QueryHeight(); j++ )
    for ( UINT32 i = 0; i < pDestImage->QueryWidth(); i++ )
    {
        UINT8 * pOutputData = tempImage.QueryData() + tempImage.BlockOffset( i, j );

        //
        // Determine the sub-pixel location of our *target* (i,j) coordinate, in the space
        // of our source image.
        //

        FLOAT32 fX = (FLOAT32) i * fHRatio;
        FLOAT32 fY = (FLOAT32) j;

        if ( VN_FAILED( vnSampleImage( pSrcImage, uiKernel, VN_IMAGE_KERNEL_1D_HORIZONTAL, fX, fY, fHRatio, pOutputData ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }
    }

    //
    // Perform the vertical filter sampling.
    //

    for ( UINT32 j = 0; j < pDestImage->QueryHeight(); j++ )
    for ( UINT32 i = 0; i < pDestImage->QueryWidth(); i++ )
    {
        UINT8 * pOutputData = pDestImage->QueryData() + pDestImage->BlockOffset( i, j );

        //
        // Determine the sub-pixel location of our *target* (i,j) coordinate, in the space
        // of our temp image.
        //

        FLOAT32 fX = (FLOAT32) i;
        FLOAT32 fY = (FLOAT32) j * fVRatio;

        if ( VN_FAILED( vnSampleImage( tempImage, uiKernel, VN_IMAGE_KERNEL_1D_VERTICAL, fX, fY, fVRatio, pOutputData ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }
    }

    return VN_SUCCESS;
}

VN_STATUS vnResizeImageWithPadding( CONST CVImage & pSrcImage, CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID(pSrcImage) || !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Copy each row of our data over, paying special attention to padding.
    //

    for ( UINT32 j = 0; j < VN_MIN2( pDestImage->QueryHeight(), pSrcImage.QueryHeight() ); j++ )
    {
        UINT8 * pbyDest = pDestImage->QueryData() + pDestImage->BlockOffset( 0, j );
        UINT8 * pbySrc  = pSrcImage.QueryData() + pSrcImage.BlockOffset( 0, j );

        memcpy( pbyDest, pbySrc, VN_MIN2( pDestImage->RowPitch(), pSrcImage.RowPitch() ) );
    }

    return VN_SUCCESS;
}

VN_STATUS vnResizeImage( CONST CVImage & pSrcImage, 
                         VN_IMAGE_KERNEL_TYPE uiKernel, 
                         UINT32 uiWidth, 
                         UINT32 uiHeight, 
                         VN_IMAGE_RESIZE_PARAMETERS uiFlags, 
                         CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID(pSrcImage) || 0 == uiWidth || 0 == uiHeight || !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Check our parameters and state
    //

    if ( VN_IMAGE_RESIZE_POW2 & uiFlags )
    {
        //
        // Guarantee that our dimensions are pow-2 aligned
        //

        uiWidth = vnAlign2( uiWidth );
        uiHeight = vnAlign2( uiHeight );
    }

    if ( VN_IMAGE_RESIZE_SYMMETRIC & uiFlags )
    {
        //
        // Enforce square dimensions
        //
        // (?) Should we change this to aspect scale?
        //

        uiHeight = uiWidth;
    }

    //
    // Verify whether resampling is actually necessary
    //

    if ( uiWidth == pSrcImage.QueryWidth() && uiHeight == pSrcImage.QueryHeight() )
    {
        return vnCloneImage( pSrcImage, pDestImage );
    }

    //
    // Create our destination image.
    //

    if ( VN_FAILED( vnCreateImage( pSrcImage.QueryFormat(), uiWidth, uiHeight, pDestImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    //
    // Verify one final option -- if the caller is requesting a pad extension rather than
    // a resampling, we do it instead.
    //

    if ( VN_IMAGE_RESIZE_PAD_EXTEND & uiFlags )
    {
        //
        // Pad our image to keep the original data in-place and un-scaled.
        //

        return vnResizeImageWithPadding( pSrcImage, pDestImage );
    }

    //
    // Prepare to perform our resample. This is perhaps the most important part of our resizer -- 
    // the calculation of our image ratios. These ratios are responsible for mapping between our 
    // integer pixel locations of the source image and our float sub-pixel coordinates within the
    // source image that represent a reflection of our destination pixels. 
    //
    // Quick visualization:
    //
    // For a source 2x1 image and a destination 4x1 image:
    //
    //        +------------+------------+      o: Note that the center of the first and last pixels
    //  Src:  |      0     |      1     |         in both our src and dst images line up with our
    //        +------------+------------+         float edges of 0.0 and 1.0.
    //               |            |               
    //              0.0          1.0           o: Our sub-pixel interpolated coordinates will always
    //               |           |                be >= 0 and <= src_width
    //             +---+---+---+---+              
    //  Dst:       | 0 | 1 | 2 | 3 |           o: Thus the src pixel coordinate of our final destination
    //             +---+---+---+---+              pixel will always be src_width - 1.
    //                                            

    // josephb: fixme -- check this ? 1.0 (it used to be 0.0).

    FLOAT32 fHorizRatio = ( 1 == uiWidth  ? 1.0f :  (FLOAT32) ( pSrcImage.QueryWidth() - 1 )  / ( uiWidth - 1 ) );
    FLOAT32 fVertRatio  = ( 1 == uiHeight ? 1.0f : (FLOAT32) ( pSrcImage.QueryHeight() - 1 ) / ( uiHeight - 1 ) );

    //
    // The ratio really just needs to be large enough to cover the potentially important pixels. 
    // Note that each kernel will clamp to its own (smaller) kernel. The radii simply need to be 
    // non-zero and large enough to cover the space.
    //

    //
    // If our kernel is non-separable for resizing operations, perform it in 2D
    //

    if ( !( 0x80000000 & uiKernel ) )
    {
        return vnResizeImageNonSeparable( pSrcImage, uiKernel, fHorizRatio, fVertRatio, pDestImage );
    }

    //
    // Our resize filter is separable, so we perform it in the horizontal space first, and 
    // then in the vertical.
    //

    return vnResizeImageSeparable( pSrcImage, uiKernel, fHorizRatio, fVertRatio, pDestImage );
}
