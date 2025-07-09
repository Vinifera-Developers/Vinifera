
#include "vnImageBlock.h"

VN_STATUS vnUnpackFloatChannel( UINT8 uiChannelRate, UINT8 * pRawChannel, FLOAT64 * pOutChannel )
{
    //
    // This parameter check is exhaustive, since this function is static and will not presently
    // be called with unchecked parameters, but we redundantly validate the parameters in case 
    // this design changes in the future.
    //

    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiChannelRate || !pOutChannel )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // This function unpacks a 16 or 32 bit float value, and stores it into a 64 bit float output.
    //

    switch ( uiChannelRate )
    {
        case 16:
        {
            FLOAT16 * pHalf = ( (FLOAT16 *) pRawChannel );

            (*pOutChannel)  = (FLOAT64) FLOAT16::ToFloat32( *pHalf );

        } break;

        case 32:
        {
            FLOAT32 * pSingle = ( (FLOAT32 *) pRawChannel );

            (*pOutChannel)    = (FLOAT64) *pSingle;

        } break;

        //
        // We only support 16 or 32 bit floating point types.
        //

        default: return vnPostError( VN_ERROR_EXECUTION_FAILURE );

    }

    return VN_SUCCESS;
}

VN_STATUS vnUnpackFixedChannel( UINT8 uiChannelRate, BOOL bSigned, UINT64 uiRawData, UINT64 * pOutChannel )
{
    //
    // This parameter check is exhaustive, since this function is static and will not presently
    // be called with unchecked parameters, but we redundantly validate the parameters in case 
    // this design changes in the future.
    //

    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiChannelRate || !pOutChannel )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // (!) We are expecting a logical shift here, and further expect that a shift 
    //     by 64 will produce a zero value. 
    //

    UINT64 uiMaskChannelVal  = ( (UINT64) 1 << uiChannelRate ) - 1;     // bit mask for our channel
    UINT64 uiMaxChannelVal   = uiMaskChannelVal;                        // max unsigned value
     INT64  iMaxChannelVal   = (INT64) ( uiMaxChannelVal >> 1 );        // max signed value
     INT64  iMinChannelVal   = -iMaxChannelVal - 1;                     // min signed value

    //
    // This function unpacks fixed integer values and stores them in the lower 
    // 32 bits of a signed 64 bit output. Note that unlike our float unpack equivalent, 
    // this function returns the number of *bits* that were processed, rather than bytes.
    //

    if ( bSigned )
    {
        INT64 iRawData = (INT64) uiRawData;

        //
        // First we check our sign bit, and perform an extention if necessary.
        //

        if ( iRawData & ( (INT64) 1 << ( uiChannelRate - 1 ) ) )
        {
            iRawData |= ~uiMaskChannelVal;
        }

        if ( iRawData < 0 )
        {
            (*pOutChannel)  = (INT64) VN_MIN_INT32 * iRawData / iMinChannelVal; 
        }
        else
        {
            (*pOutChannel)  = (INT64) VN_MAX_INT32 * iRawData / iMaxChannelVal; 
        }
    }
    
    //
    // Otherwise, handle the unsigned channel case
    //

    else
    {
        (*pOutChannel) = (INT64) VN_MAX_INT32 * uiRawData / uiMaxChannelVal;
    }

    return VN_SUCCESS;
}

VN_STATUS vnUnpackFloatPixel( UINT8 * pRawPixel, VN_IMAGE_FORMAT format, VN_PIXEL_BLOCK * pOutBlock )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pRawPixel || !VN_IS_FORMAT_VALID( format ) || !pOutBlock )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    UINT8 uiPixelRate            = VN_IMAGE_PIXEL_RATE( format );
    UINT8 uiChannelCount         = VN_IMAGE_CHANNEL_COUNT( format );
    UINT32 uiSumOffset           = 0;

    for ( UINT32 i = 0; i < uiChannelCount; i++ )
    {
        UINT8 uiRate        = VN_IMAGE_CHANNEL_RATE( i, format );
        UINT8 * pRawChannel = pRawPixel + uiSumOffset;

        //
        // Convert the type (if necessary), and safeguard against invalid formats.
        //

        if ( VN_FAILED( vnUnpackFloatChannel( uiRate, pRawChannel, &(pOutBlock->fChannelData[i]) ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        uiSumOffset += ( uiRate >> 3 );
    }

    pOutBlock->uiPrecision     = VN_IMAGE_PRECISION_FLOAT;
    pOutBlock->uiChannelCount  = uiChannelCount;

    return VN_SUCCESS;
}

VN_STATUS vnUnpackFixedPixel( UINT8 * pRawPixel, VN_IMAGE_FORMAT format, VN_PIXEL_BLOCK * pOutBlock )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pRawPixel || !VN_IS_FORMAT_VALID( format ) || !pOutBlock )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    UINT8 uiPixelRate            = VN_IMAGE_PIXEL_RATE( format );
    UINT8 uiChannelCount         = VN_IMAGE_CHANNEL_COUNT( format );

    //
    // (!) Note: our summation offset is a *bit* offset for fixed channels, not 
    //           a byte offset like we use for float channels.
    //

    UINT32 uiSumOffset       = 0;

    for ( UINT32 i = 0; i < uiChannelCount; i++ )
    {
        UINT64 uiRawData    = 0;
        UINT32 uiBytesRead  = ( uiSumOffset >> 3 );
        UINT8  uiChannelRate = VN_IMAGE_CHANNEL_RATE( i, format );

        //
        // The maximum allowable size per channel is currently 32 bits. We
        // move a dynamic sliding window along the pixel buffer and select
        // the appropriate bits for each channel.
        //

        memcpy( &uiRawData, pRawPixel + uiBytesRead, VN_MIN2( 8, ( uiPixelRate >> 3 ) - uiBytesRead ) );
            
        //
        // Each channel may contain, at most, 8 bytes of data, so we temporarily
        // store channel data in a 64 bit unsigned integer, and select the bits 
        // that are relevant for this channel.
        //

        //
        // (!) Note: it is crucial that we cast our shift operation to 64 bit precision, otherwise
        //           the compiler will infer the precision from the operands which will fault our
        //           128 bit pixel support.
        //

        uiRawData = ( uiRawData >> ( uiSumOffset % 8 ) ) & ( ( (UINT64) 1 << uiChannelRate ) - 1 );

        //
        // Convert the type (if necessary), and safeguard against invalid formats.
        //

        if ( VN_FAILED( vnUnpackFixedChannel( uiChannelRate, VN_IS_SIGNED_FORMAT( format ), uiRawData, (UINT64 *) &( pOutBlock->iChannelData[i] ) ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        uiSumOffset += uiChannelRate;
    }

    pOutBlock->uiPrecision     = VN_IMAGE_PRECISION_FIXED;
    pOutBlock->uiChannelCount  = uiChannelCount;

    return VN_SUCCESS;
}

VN_STATUS vnConvertToBlock( UINT8 * pRawPixel, VN_IMAGE_FORMAT internalFormat, VN_PIXEL_BLOCK * pOutBlock )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pRawPixel || !VN_IS_FORMAT_VALID( internalFormat ) || !pOutBlock )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // The design of this function aimed at balancing simplicity and ease of readability, with performance. We try
    // to push as many branches out as possible (with the channel inner branch a necessity due to formats with 
    // heterogeneous channel rates).
    //

    BOOL bFloatFormat = !!( VN_IS_FLOAT_FORMAT( internalFormat ) );

    if ( bFloatFormat ) return vnUnpackFloatPixel( pRawPixel, internalFormat, pOutBlock );
    else                return vnUnpackFixedPixel( pRawPixel, internalFormat, pOutBlock );

    return VN_SUCCESS;
}
