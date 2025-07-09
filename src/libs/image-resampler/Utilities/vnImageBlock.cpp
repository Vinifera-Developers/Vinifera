
#include "vnImageBlock.h"

VN_STATUS vnConvertBlock( CONST VN_PIXEL_BLOCK & pSrc, VN_IMAGE_FORMAT destFormat, VN_PIXEL_BLOCK * pDest )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pDest )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }
    
    //
    // (!) Note: this function is written to support aliased conversion. That is, conversions where pSrc
    //           and pDest are aliases of the same block.
    //
    
    //
    // Our next step is to copy over the block channel data with proper conversions for the
    // (potential) precision change.
    //
    
    VN_IMAGE_PRECISION destPrecision = ( VN_IS_FLOAT_FORMAT( destFormat ) ) ? VN_IMAGE_PRECISION_FLOAT : VN_IMAGE_PRECISION_FIXED;
    
    //
    // Note that we store the destination channel count but do not take it into consideration
    // when performing the channel conversion. In this way we allow zero values to propagate from
    // the source to destination in the event of a channel count mismatch.
    //
    
    if ( pSrc.uiPrecision == destPrecision )
    {
        //
        // Same precision, simply copy over the data and format
        //
        
        memcpy( pDest->uiChannelBytes, pSrc.uiChannelBytes, 32 );
    }
    
    //
    // Conversion between fixed/float format
    //
    
    else
    {
        if ( VN_IMAGE_PRECISION_FLOAT == destPrecision )
        {
            //
            // fixed -> float, we perform a normalizing scale
            //
            
            for ( UINT8 i = 0; i < 4; i++ )
            {
                if ( pSrc.iChannelData[i] < 0 )
                {
                    pDest->fChannelData[i] = ( (FLOAT64) -1 * pSrc.iChannelData[i] / VN_MIN_INT32 );
                }
                else
                {
                    pDest->fChannelData[i] = (FLOAT64) pSrc.iChannelData[i] / VN_MAX_INT32;
                }

                pDest->fChannelData[i] = vnClipRange64( pDest->fChannelData[i], -1.0, 1.0 );
            }
        }
        else
        {
            //
            // float -> fixed, we perform a truncating scale
            //
            
            FLOAT64 fTempChannels[4] = {0};
            
            for ( UINT8 i = 0; i < 4; i++ )
            {
                fTempChannels[i]       = vnClipRange64( pSrc.fChannelData[i], -1.0f, 1.0f );

                if ( fTempChannels[i] < 0 )
                {
                    pDest->iChannelData[i] = (INT64) ( (FLOAT64) -1.0 * fTempChannels[i] * VN_MIN_INT32 );
                }
                else
                {
                    pDest->iChannelData[i] = (INT64) ( (FLOAT64) fTempChannels[i] * VN_MAX_INT32 );
                }
            }
        }
    }
    
    pDest->uiChannelCount            = VN_IMAGE_CHANNEL_COUNT( destFormat );
    pDest->uiPrecision               = destPrecision;
    
    return VN_SUCCESS;
}