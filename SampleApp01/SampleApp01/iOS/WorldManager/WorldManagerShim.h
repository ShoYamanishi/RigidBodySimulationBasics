#ifndef __WORLD_MANAGER_SHIM_H__
#define __WORLD_MANAGER_SHIM_H__

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@class WorldManagerSwiftEnd;

@interface WorldManagerShim : NSObject

-(instancetype) initWithDevice: (id<MTLDevice>) device swiftEnd: (WorldManagerSwiftEnd*) swift_end;
-(void) createPipelineStates: (MTLPixelFormat) pixelFormat;
-(void) updateScreenSizes: (double) width height: (double) height;
-(void) setCameraMatrices: (float*) Mview proj: (float*) Mproj transform: (float*) Mtrans;
-(void) updateWorld: (float*) acceleration torsionalSpringStrength: (float) strength;
-(void) encode: ( id<MTLRenderCommandEncoder> ) encoder;
@end

#endif /* __WORLD_MANAGER_SHIM_H__ */
