#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <simd/simd.h>

inline simd::float4x4 fromArray( const float* arr )
{
    const simd::float4 col0{ arr[ 0], arr[ 1], arr[ 2], arr[ 3] };
    const simd::float4 col1{ arr[ 4], arr[ 5], arr[ 6], arr[ 7] };
    const simd::float4 col2{ arr[ 8], arr[ 9], arr[10], arr[11] };
    const simd::float4 col3{ arr[12], arr[13], arr[14], arr[15] };

    return simd::float4x4{ col0, col1, col2, col3 };
}

inline simd::float4x4 identity()
{
    const simd::float4 col0{ 1.0f, 0.0f, 0.0f, 0.0f };
    const simd::float4 col1{ 0.0f, 1.0f, 0.0f, 0.0f };
    const simd::float4 col2{ 0.0f, 0.0f, 1.0f, 0.0f };
    const simd::float4 col3{ 0.0f, 0.0f, 0.0f, 1.0f };

    return simd::float4x4{ col0, col1, col2, col3 };
}

#endif /*__UTIL_HPP__*/
