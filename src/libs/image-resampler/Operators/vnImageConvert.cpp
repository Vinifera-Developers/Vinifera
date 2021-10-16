
#include "vnImageConvert.h"

#include "../Base/vnMath.h"
#include "../Utilities/vnImageBlock.h"
#include "../Operators/vnImageClone.h"

VN_STATUS vnConvertImage( CONST CVImage & pSrcImage, VN_IMAGE_FORMAT destFormat, CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID(pSrcImage) || VN_IMAGE_FORMAT_NONE == destFormat || !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }

        if ( &pSrcImage == pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }
    
    //
    // Check if we're performing a senseless conversion
    //

    if ( pSrcImage.QueryFormat() == destFormat )
    {
        return vnCloneImage( pSrcImage, pDestImage );
    }

    //
    // Our conversion is valid, so we perform it.
    //

    if ( VN_FAILED( vnCreateImage( destFormat, pSrcImage.QueryWidth(), pSrcImage.QueryHeight(), pDestImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    //
    // Check if we're merely converting between color and depth values
    //

    if ( ( pSrcImage.QueryFormat() & ( ~VN_IMAGE_SPACE_MASK ) ) == 
         ( destFormat              & ( ~VN_IMAGE_SPACE_MASK ) ) )
    {
        memcpy( pDestImage->QueryData(), pSrcImage.QueryData(), pSrcImage.SlicePitch() );

        return VN_SUCCESS;
    }

    //
    // Perform an exhaustive conversion
    //

    for ( UINT32 j = 0; j < pSrcImage.QueryHeight(); j++ )
    for ( UINT32 i = 0; i < pSrcImage.QueryWidth(); i++ )
    {
        UINT8 * pSrcPixel  = pSrcImage.QueryData() + pSrcImage.BlockOffset( i, j );
        UINT8 * pDestPixel = pDestImage->QueryData() + pDestImage->BlockOffset( i, j );

        VN_PIXEL_BLOCK sourceBlock = {0};
        VN_PIXEL_BLOCK destBlock   = {0};

        //
        // Unpack the source channel values into its native block.
        //

        if ( VN_FAILED( vnConvertToBlock( pSrcPixel, pSrcImage.QueryFormat(), &sourceBlock ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        //
        // Convert our source image block into a properly precise destination block.
        //

        if ( VN_FAILED( vnConvertBlock( sourceBlock, destFormat, &destBlock ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        //
        // Pack the destination block into our destination buffer.
        //

        if ( VN_FAILED( vnConvertFromBlock( destBlock, destFormat, pDestPixel ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }
    }

    return VN_SUCCESS;
}
