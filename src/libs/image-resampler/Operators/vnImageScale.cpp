
#include "vnImageScale.h"

#include "../Operators/vnImageClone.h"

VN_STATUS vnBresenhamScaleLine( UINT8 * pSrcBuffer, UINT32 uiSrcLength, UINT32 uiSrcStride, UINT32 uiPixelPitch, UINT8 * pDestBuffer, UINT32 uiDestLength, UINT32 uiDestStride )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pSrcBuffer || 0 == uiSrcLength || !pDestBuffer || 0 == uiDestLength || 0 == uiPixelPitch )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    UINT32 uiErrorTerm = 0;

    //
    // We avoid the use of division or modulus operators for each iteration, but rely upon 
    // an initial branch per line.
    //

    if ( uiDestLength > uiSrcLength )
    {
        UINT32 uiSrcCoord  = 0;

        for ( UINT32 uiDestCoord = 0; uiDestCoord < uiDestLength; uiDestCoord++ )
        {
            uiErrorTerm += uiSrcLength;

            if ( uiErrorTerm > uiDestLength )
            {
                uiErrorTerm -= uiDestLength;
                uiSrcCoord++;
            }

            //
            // Fill out our current pixel
            //

            UINT8 * pSrcPixel  = pSrcBuffer + uiSrcCoord * uiSrcStride;
            UINT8 * pDestPixel = pDestBuffer + uiDestCoord * uiDestStride;
       
            memcpy( pDestPixel, pSrcPixel, uiPixelPitch );
        }
    }

    //
    // The source line is longer than the destination, so we increment over the source.
    //

    else
    {
        UINT32 uiDestCoord  = 0;

        for ( UINT32 uiSrcCoord = 0; uiSrcCoord < uiSrcLength; uiSrcCoord++ )
        {
            uiErrorTerm += uiDestLength;

            if ( uiErrorTerm > uiSrcLength )
            {
                uiErrorTerm -= uiSrcLength;
                uiDestCoord++;
            }

            //
            // Fill out our current pixel
            //

            UINT8 * pSrcPixel  = pSrcBuffer + uiSrcCoord * uiSrcStride;
            UINT8 * pDestPixel = pDestBuffer + uiDestCoord * uiDestStride;
       
            memcpy( pDestPixel, pSrcPixel, uiPixelPitch );
        }
    }

    return VN_SUCCESS;
}

VN_STATUS vnBresenhamScaleImage( CONST CVImage & pSrcImage, CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Create an image with dimensions of [ dest_width, src_height ] so that we 
    // may scale in the horizontal direction only.
    //

    CVImage tempImage;

    if ( VN_FAILED( vnCreateImage( pSrcImage.QueryFormat(), pDestImage->QueryWidth(), pSrcImage.QueryHeight(), &tempImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    //
    // First we stretch our image in the horizontal direction
    //

    UINT32 uiPixelPitch = VN_IMAGE_PIXEL_RATE( pSrcImage.QueryFormat() ) >> 3;

    for ( UINT32 y = 0; y < pSrcImage.QueryHeight(); y++ )
    {
        UINT8 * pSrcData  = pSrcImage.QueryData() + pSrcImage.BlockOffset( 0, y );
        UINT8 * pDestData = tempImage.QueryData() + tempImage.BlockOffset( 0, y );

        if ( pSrcImage.QueryWidth() == tempImage.QueryWidth() )
        {
            //
            // A simple row copy will suffice
            //

            memcpy( pDestData, pSrcData, pSrcImage.RowPitch() );
        }
        else
        {
            if ( VN_FAILED( vnBresenhamScaleLine( pSrcData, 
                                                  pSrcImage.QueryWidth(), 
                                                  uiPixelPitch, 
                                                  uiPixelPitch, 
                                                  pDestData, 
                                                  tempImage.QueryWidth(), 
                                                  uiPixelPitch ) ) )
            {
                return vnPostError( VN_ERROR_EXECUTION_FAILURE );
            }
        }
    }

    //
    // Check to see whether this image only required horizontal scaling. If so, simply perform a bulk copy 
    // (we're assuming there is zero undisclosed padding.)
    //

    if ( tempImage.QueryHeight() == pDestImage->QueryHeight() )
    {
        memcpy( pDestImage->QueryData(), tempImage.QueryData(), tempImage.SlicePitch() );

        return VN_SUCCESS;
    }

    //
    //  Now scale in the vertical direction -- placing the results into our destination image
    //

    for ( UINT32 x = 0; x < tempImage.QueryWidth(); x++ )
    {
        UINT8 * pSrcData  = tempImage.QueryData()   + tempImage.BlockOffset( x, 0 );
        UINT8 * pDestData = pDestImage->QueryData() + pDestImage->BlockOffset( x, 0 );

        if ( VN_FAILED( vnBresenhamScaleLine( pSrcData, 
                                              tempImage.QueryHeight(), 
                                              tempImage.RowPitch(), 
                                              uiPixelPitch, 
                                              pDestData, 
                                              pDestImage->QueryHeight(), 
                                              pDestImage->RowPitch() ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }
    }

    return VN_SUCCESS;
}

VN_STATUS vnScaleImage( CONST CVImage & pSrcImage, UINT32 uiWidth, UINT32 uiHeight, CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID(pSrcImage) || 0 == uiWidth || 0 == uiHeight || !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    if ( uiWidth == pSrcImage.QueryWidth() && uiHeight == pSrcImage.QueryHeight() )
    {
        //
        // Perform a cloning, no scaling required as our dimensions match.
        //

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
    // Perform our scaling operation using the aliases
    //

    if ( VN_FAILED( vnBresenhamScaleImage( pSrcImage, pDestImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    return VN_SUCCESS;
}
