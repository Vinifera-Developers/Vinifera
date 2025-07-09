
#include "vnImageBlock.h"

VN_STATUS vnPackFloatChannel( CONST FLOAT64 & pInChannel, UINT8 uiChannelRate, UINT8 * pRawChannel )
{
    //
    // This parameter check is exhaustive, since this function is static and will not presently
    // be called with unchecked parameters, but we redundantly validate the parameters in case 
    // this design changes in the future.
    //

    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiChannelRate || !pRawChannel )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // This function is responsible for writing a 64 bit float value and packing it into a 16 or
    // 32 bit float value for output. 
    //

    switch ( uiChannelRate )
    {
        case 16: *( (FLOAT16 *) pRawChannel ) = (FLOAT32) pInChannel; break;
        case 32: *( (FLOAT32 *) pRawChannel ) = (FLOAT32) pInChannel; break;

        //
        // We only support 16 or 32 bit floating point types.
        //

        default: return vnPostError( VN_ERROR_EXECUTION_FAILURE );

    }

    return VN_SUCCESS;
}

VN_STATUS vnPackFixedChannel( INT64 pInChannel, BOOL bSigned, UINT8 uiChannelRate, UINT32 uiSumOffset, UINT64 * pRawData )
{
    //
    // (!) Note: We are expecting a logical shift here, and further expect that a shift 
    //           by 64 will produce a zero value. Furthermore, it is crucial that we cast our
    //           shift operation to 64 bit precision, otherwise the compiler will infer the precision 
    //           from the operands which will fault our 128 bit pixel support.
    //

    UINT64 uiMaskChannelVal  = ( (UINT64) 1 << uiChannelRate ) - 1;     // bit mask for our destination channel
    UINT64 uiMaxChannelVal   = uiMaskChannelVal;                        // max unsigned value
     INT64  iMaxChannelVal   = (INT64) ( uiMaxChannelVal >> 1 );        // max signed value
     INT64  iMinChannelVal   = -iMaxChannelVal - 1;                     // min signed value

    //
    // If we are a signed format, negative values will be written naturally into the packed
    // buffer. If we are unsigned however, we must clamp to a valid unsigned range.
    //

    INT64 iChannelData      = 0;

    if ( bSigned )
    {
        if ( pInChannel < VN_MIN_INT32 ) pInChannel = VN_MIN_INT32;
        if ( pInChannel > VN_MAX_INT32 ) pInChannel = VN_MAX_INT32;

        if ( pInChannel < 0 )
        {
            iChannelData = (INT64) iMinChannelVal * pInChannel / VN_MIN_INT32;
        }
        else
        {
            iChannelData = (INT64) iMaxChannelVal * pInChannel / VN_MAX_INT32;
        }
    }
    else 
    {
        if ( pInChannel < 0 )             pInChannel = 0;
        if ( pInChannel > VN_MAX_INT32 )  pInChannel = VN_MAX_INT32;

        iChannelData = (INT64) uiMaxChannelVal * pInChannel / VN_MAX_INT32;
    }

    UINT64 uiShiftedValue = (UINT64) ( iChannelData & uiMaskChannelVal ) << uiSumOffset;

    //
    // Next we insert our shifted value into the pRawData stream by first clearing
    // the current destination bits in the stream, and then placing our updated 
    // value.
    //

    (*pRawData) = (*pRawData) & ~( uiMaskChannelVal << uiSumOffset );

    (*pRawData) = (*pRawData) | uiShiftedValue;

    return VN_SUCCESS;
}

VN_STATUS vnPackFloatPixel( CONST VN_PIXEL_BLOCK & pSourceBlock, VN_IMAGE_FORMAT format, UINT8 * pRawPixel )
{
    UINT32 uiSumOffset   = 0;
    UINT8 uiChannelCount = VN_IMAGE_CHANNEL_COUNT( format );
    UINT8 uiPixelRate    = VN_IMAGE_PIXEL_RATE( format );

    for ( UINT32 i = 0; i < uiChannelCount; i++ )
    {
        UINT8 uiRate        = VN_IMAGE_CHANNEL_RATE( i, format );
        UINT8 * pRawChannel = pRawPixel + uiSumOffset;

        //
        // Convert the type (if necessary), and safeguard against invalid formats.
        //

        if ( VN_FAILED( vnPackFloatChannel( pSourceBlock.fChannelData[i], uiRate, pRawChannel ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        uiSumOffset += ( uiRate >> 3 );
    }

    return VN_SUCCESS;
}

VN_STATUS vnPackFixedPixel( CONST VN_PIXEL_BLOCK & pSourceBlock, VN_IMAGE_FORMAT format, UINT8 * pRawPixel )
{
    //
    // (!) Note: our summation offset is a *bit* offset for fixed channels, not 
    //           a byte offset like we use for float channels.
    //

    UINT64 uiRawData     = 0;
    UINT32 uiSumOffset   = 0; 
    UINT8 uiChannelCount = VN_IMAGE_CHANNEL_COUNT( format );
    UINT8 uiPixelRate    = VN_IMAGE_PIXEL_RATE( format );

    for ( UINT32 i = 0; i < uiChannelCount; i++ )
    {
        UINT8 uiChannelRate   = VN_IMAGE_CHANNEL_RATE( i, format );

        //
        // The sum of all channel rates of a pixel must be byte aligned, with a maximum
        // of 8 bytes per pixel. Additionally, our individual channels are limited to 
        // 32 bits. 
        // 
        // We therefore write our pixel data to the output in batches of 32 or fewer bits.
        //

        if ( VN_FAILED( vnPackFixedChannel( pSourceBlock.iChannelData[i], VN_IS_SIGNED_FORMAT( format ), uiChannelRate, uiSumOffset, &uiRawData ) ) )
        {
            return vnPostError( VN_ERROR_EXECUTION_FAILURE );
        }

        uiSumOffset += uiChannelRate;

        //
        // If we've packed 32 or more bits so far, we write out 4 bytes and shift our counters.
        // This will enable us to pack and store more than 64 bits per pixel.
        //

        if ( uiSumOffset >= 32 )
        {
            //
            // Copy out four bytes and advance our counters.
            //

            memcpy( pRawPixel, &uiRawData, 4 );

            uiSumOffset = ( uiSumOffset % 32 );
            uiRawData   = (UINT64) uiRawData >> 32;
            pRawPixel   = pRawPixel + 4;            
        }
    }

    //
    // If there are any remaining pixels in our raw data buffer, they must be 
    // byte aligned.
    //

    if ( uiSumOffset )
    {
        memcpy( pRawPixel, &uiRawData, ( uiSumOffset >> 3 ) );
    }

    return VN_SUCCESS;
}

VN_STATUS vnConvertFromBlock( CONST VN_PIXEL_BLOCK & pSourceBlock, VN_IMAGE_FORMAT format, UINT8 * pRawPixel )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !pRawPixel || !VN_IS_FORMAT_VALID( format ) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Check to verify that the precision matches between our source block and the destination
    // format. This function does not perform precision conversion (it is assumed to have already
    // been performed).
    //

    BOOL bFloatFormat                   = !!( VN_IS_FLOAT_FORMAT( format ) );

    VN_IMAGE_PRECISION formatPrecision  = bFloatFormat ? VN_IMAGE_PRECISION_FLOAT : VN_IMAGE_PRECISION_FIXED;

    if ( formatPrecision != pSourceBlock.uiPrecision )
    {
        //
        // Perform a sudden block conversion to match the required precision.
        //
        
        VN_PIXEL_BLOCK tempBlock;
        
        vnConvertBlock( pSourceBlock, format, &tempBlock );
        
        if ( bFloatFormat ) return vnPackFloatPixel( tempBlock, format, pRawPixel );
        else                return vnPackFixedPixel( tempBlock, format, pRawPixel );
    }
    else
    {
        if ( bFloatFormat ) return vnPackFloatPixel( pSourceBlock, format, pRawPixel );
        else                return vnPackFixedPixel( pSourceBlock, format, pRawPixel );
    }

    return VN_SUCCESS;
}