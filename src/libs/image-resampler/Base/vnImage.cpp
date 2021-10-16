
#include "vnImage.h"

CVImage::CVImage()
{
    m_uiImageFormat     = VN_IMAGE_FORMAT_NONE;
    m_uiWidthInPixels   = 0;
    m_uiHeightInPixels  = 0;
    m_uiBitsPerPixel    = 0;
    m_uiChannelCount    = 0;
    m_pbyDataBuffer     = 0;
    m_uiDataCapacity    = 0;
}

CVImage::~CVImage()
{
    if ( VN_FAILED( Deallocate() ) )
    {
        vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }
}

UINT32 CVImage::RowPitch() CONST
{
    return ( m_uiWidthInPixels * m_uiBitsPerPixel ) >> 3;
}

UINT32 CVImage::SlicePitch() CONST
{
    return m_uiDataCapacity;
}

UINT32 CVImage::BlockOffset( UINT32 i, UINT32 j ) CONST
{
    return ( RowPitch() * j ) + ( ( i * m_uiBitsPerPixel ) >> 3 );
}


VN_STATUS CVImage::Allocate( UINT32 uiSize )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiSize )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    if ( VN_FAILED( Deallocate() ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    m_pbyDataBuffer = new UINT8[ uiSize ];

    if ( !m_pbyDataBuffer )
    {
        return vnPostError( VN_ERROR_OUTOFMEMORY );
    } 

    m_uiDataCapacity = uiSize;

    //
    // Zero out our initial image memory (for good measure).
    //

    memset( m_pbyDataBuffer, 0, uiSize );

    return VN_SUCCESS;
}

VN_STATUS CVImage::Deallocate()
{
    delete [] m_pbyDataBuffer;

    m_pbyDataBuffer  = 0;
    m_uiDataCapacity = 0;

    return VN_SUCCESS;
}

VN_STATUS CVImage::SetDimension( UINT32 uiNewWidth, UINT32 uiNewHeight )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiNewWidth || 0 == uiNewHeight )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Check for an uninitialized image
    //

    if ( 0 == m_uiBitsPerPixel || VN_IMAGE_FORMAT_NONE == m_uiImageFormat )
    {
        //
        // You must call SetFormat prior to calling this function, so that
        // we know how to allocate the image.
        //

        return vnPostError( VN_ERROR_INVALID_RESOURCE );
    }

    if ( uiNewWidth == QueryWidth() && uiNewHeight == QueryHeight() )
    {
        return VN_SUCCESS;
    }

    //
    // All images are required to use byte aligned pixel rates, so there is 
    // no need to align the allocation size.
    //

    if ( VN_FAILED( Allocate( ( uiNewWidth * uiNewHeight * m_uiBitsPerPixel ) >> 3 ) ) )
    {
        return vnPostError( VN_ERROR_OUTOFMEMORY );
    }

    m_uiWidthInPixels  = uiNewWidth;
    m_uiHeightInPixels = uiNewHeight;

    return VN_SUCCESS;
}

VN_STATUS CVImage::SetFormat( VN_IMAGE_FORMAT format )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == VN_IMAGE_CHANNEL_COUNT( format ) )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    UINT32 uiRateTotal    = VN_IMAGE_PIXEL_RATE( format );
    UINT8  uiChannelCount = VN_IMAGE_CHANNEL_COUNT( format );

    if ( 0 != ( uiRateTotal % 8 ) )
    {
        //
        // The format is invalid -- it does not contain a byte aligned pixel rate.
        //

        return vnPostError( VN_ERROR_INVALIDARG );
    }

    m_uiImageFormat     = format;
    m_uiBitsPerPixel    = uiRateTotal;
    m_uiChannelCount    = uiChannelCount;

    return VN_SUCCESS;
}

UINT32 CVImage::QueryWidth() CONST
{
    return m_uiWidthInPixels;
}

UINT32 CVImage::QueryHeight() CONST
{
    return m_uiHeightInPixels;
}

UINT8 * CVImage::QueryData() CONST
{
    return m_pbyDataBuffer;
}

UINT8 CVImage::QueryBitsPerPixel() CONST
{
    return m_uiBitsPerPixel;
}

VN_IMAGE_FORMAT CVImage::QueryFormat() CONST
{
    return m_uiImageFormat;
}

UINT8 CVImage::QueryChannelCount() CONST
{
    return m_uiChannelCount;
}

VN_STATUS vnCreateImage( VN_IMAGE_FORMAT format, UINT32 uiWidth, UINT32 uiHeight, CVImage * pOutImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( 0 == uiWidth || 0 == uiHeight )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }

        if ( !pOutImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    if ( VN_FAILED( (pOutImage)->SetFormat( format ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    if ( VN_FAILED( (pOutImage)->SetDimension( uiWidth, uiHeight ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    return VN_SUCCESS;
}

VN_STATUS vnDestroyImage( CVImage * pInImage )
{
    if ( !pInImage )
    {
        return VN_SUCCESS;
    }

    if ( VN_FAILED( pInImage->Deallocate() ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    return VN_SUCCESS;
}