#import <Foundation/Foundation.h>
#import "WorldManagerShim.h"
#import "WorldManagerReverseShim.h"
#include "WorldManagerCppEnd.hpp"

// These 2 imports are for satisfying the dependencies in SampleApp01-Swift.h.
#import <MetalKit/MetalKit.h>
#import <ARKit/ARKit.h>
#import "SampleApp01-Swift.h"

@implementation WorldManagerShim {

    WorldManagerCppEnd*      _m_cpp_end;
    WorldManagerSwiftEnd*    _m_swift_end;
    WorldManagerReverseShim* _m_reverse_shim;
}

-(instancetype) initWithDevice: (id<MTLDevice>) device swiftEnd: (WorldManagerSwiftEnd*) swift_end
{
    self = [super init];
    if (self) {
        _m_reverse_shim = new WorldManagerReverseShim( self );
        _m_cpp_end      = new WorldManagerCppEnd( _m_reverse_shim, (__bridge MTL::Device*)device );
        _m_swift_end    = swift_end;
    }
    return self;
}

-(void) createPipelineStates: (MTLPixelFormat) pixelFormat
{
    _m_cpp_end->createPipelineStates( (MTL::PixelFormat)pixelFormat );
}

-(void) updateScreenSizes: (double) width height: (double) height
{
    _m_cpp_end->updateScreenSizes( width, height );
}

-(void) setCameraMatrices: (float*) Mview proj: (float*) Mproj transform: (float*) Mtrans
{
    _m_cpp_end->setCameraMatrices( Mview, Mproj, Mtrans );
}

-(void) updateWorld: (float*) acceleration torsionalSpringStrength: (float) strength;
{
    _m_cpp_end->udpateWorld( acceleration, strength );
}

-(void) encode: ( id<MTLRenderCommandEncoder> ) encoder
{
    _m_cpp_end->encode( (__bridge MTL::RenderCommandEncoder*)encoder );
}

@end
