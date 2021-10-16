
#include "vnImageSampler.h"
#include "vnImageBlock.h"

VN_STATUS vnSampleImage( CONST CVImage & pSrcImage, 
                         VN_IMAGE_KERNEL_TYPE uiKernel, 
                         VN_IMAGE_KERNEL_DIRECTION uiDirection, 
                         FLOAT32 fX, 
                         FLOAT32 fY, 
                         FLOAT32 fRadius, 
                         UINT8 * pRawOutput )
{
    if ( VN_PARAM_CHECK )
    {
        if ( !VN_IS_IMAGE_VALID( pSrcImage ) || !pRawOutput )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }

        if ( fX >= pSrcImage.QueryWidth() || fY >= pSrcImage.QueryHeight() )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    //
    // Most of our kernels are internally separable, but care must be taken when writing operators
    // to ensure that we use the appropriate kernel and mode. Not all of our kernels are separable,
    // even though they support a 1D interface.
    // 

    switch ( uiDirection )
    {
        default: break;
            
        case VN_IMAGE_KERNEL_1D_HORIZONTAL:
        case VN_IMAGE_KERNEL_1D_VERTICAL:
        {
            //
            // Here we safeguard against error if our enum should shift or expand. This could
            // be done with a LUT and better parameterization, but we'd like to keep this as 
            // simple and readable as possible.
            //

            BOOL bDirection = ( VN_IMAGE_KERNEL_1D_VERTICAL == uiDirection );

            switch ( uiKernel )
            {
                default: break;
                    
                case VN_IMAGE_KERNEL_NEAREST:   return vnNearestKernel( pSrcImage, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_AVERAGE:   return vnAverageKernel( pSrcImage, fX, fY, bDirection, fRadius, pRawOutput );
                case VN_IMAGE_KERNEL_BILINEAR:  return vnBilinearKernel( pSrcImage, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_GAUSSIAN:  return vnGaussianKernel( pSrcImage, fX, fY, bDirection, fRadius, pRawOutput );
                case VN_IMAGE_KERNEL_BICUBIC:   return vnBicubicKernel( pSrcImage, 0, 1, fX, fY, bDirection, pRawOutput );                
                case VN_IMAGE_KERNEL_CATMULL:   return vnBicubicKernel( pSrcImage, 0, 0.5f, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_MITCHELL:  return vnBicubicKernel( pSrcImage, 1.0f/3.0f, 1.0f/3.0f, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_CARDINAL:  return vnBicubicKernel( pSrcImage, 0, 0.75f, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_BSPLINE:   return vnBicubicKernel( pSrcImage, 1, 0, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_SPLINE:    return vnSplineKernel( pSrcImage, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS:   return vnLanczosKernel( pSrcImage, 1, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS2:  return vnLanczosKernel( pSrcImage, 2, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS3:  return vnLanczosKernel( pSrcImage, 3, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS4:  return vnLanczosKernel( pSrcImage, 4, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS5:  return vnLanczosKernel( pSrcImage, 5, fX, fY, bDirection, pRawOutput );
                case VN_IMAGE_KERNEL_COVERAGE:  return vnCoverageKernel( pSrcImage, fX, fY, bDirection, fRadius, pRawOutput );
            }

        } break;

        case VN_IMAGE_KERNEL_2D_COMBINED:
        {
            switch ( uiKernel )
            {
                default: break;
                    
                case VN_IMAGE_KERNEL_NEAREST:   return vnNearestKernel( pSrcImage, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_AVERAGE:   return vnAverageKernel( pSrcImage, fX, fY, fRadius, pRawOutput );
                case VN_IMAGE_KERNEL_BILINEAR:  return vnBilinearKernel( pSrcImage, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_GAUSSIAN:  return vnGaussianKernel( pSrcImage, fX, fY, fRadius, pRawOutput );
                case VN_IMAGE_KERNEL_BICUBIC:   return vnBicubicKernel( pSrcImage, 0, 1, fX, fY, pRawOutput );                
                case VN_IMAGE_KERNEL_CATMULL:   return vnBicubicKernel( pSrcImage, 0, 0.5f, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_MITCHELL:  return vnBicubicKernel( pSrcImage, 1.0f/3.0f, 1.0f/3.0f, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_CARDINAL:  return vnBicubicKernel( pSrcImage, 0, 0.75f, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_BSPLINE:   return vnBicubicKernel( pSrcImage, 1, 0, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_SPLINE:    return vnSplineKernel( pSrcImage, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS:   return vnLanczosKernel( pSrcImage, 1, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS2:  return vnLanczosKernel( pSrcImage, 2, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS3:  return vnLanczosKernel( pSrcImage, 3, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS4:  return vnLanczosKernel( pSrcImage, 4, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_LANCZOS5:  return vnLanczosKernel( pSrcImage, 5, fX, fY, pRawOutput );
                case VN_IMAGE_KERNEL_COVERAGE:  return vnCoverageKernel( pSrcImage, fX, fY, fRadius, pRawOutput );
            }

        } break;
    }

    return VN_ERROR_NOTIMPL;
}