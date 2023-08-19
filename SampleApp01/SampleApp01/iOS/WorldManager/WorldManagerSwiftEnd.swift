import Foundation
import MetalKit
import SwiftUI
import ARKit
import CoreMotion

@objc
class WorldManagerSwiftEnd : NSObject, ObservableObject, TouchMTKViewDelegate {

    var arCoordinator  : ARCoordinator?

    var device:                MTLDevice!
    var screenSize:            CGSize

    var viewMatrix:            float4x4
    var projectionMatrix:      float4x4
    var cameraTransformMatrix: float4x4

    let motion = CMMotionManager()
    var coreMotionTimer: Timer?
    var acceleration: SIMD3<Double>

    @Published var torsionalSpringStrength : Float = 0.0

    var shim: WorldManagerShim?

    public init( device : MTLDevice ) {

        self.device                = device
        self.screenSize            = CGSize( width: 0.0, height: 0.0 )
        self.viewMatrix            = matrix_identity_float4x4
        self.projectionMatrix      = matrix_identity_float4x4
        self.cameraTransformMatrix = matrix_identity_float4x4
        self.acceleration          = SIMD3<Double>( 0.0, 0.0, 0.0 )
        super.init()

        self.shim = WorldManagerShim( device : device, swiftEnd: self )
        startAccelerometers()
    }

    func createPipelineStates( mtkView: MTKView ) {
        self.shim!.createPipelineStates( mtkView.colorPixelFormat )
    }

    func updateScreenSizes( size: CGSize ) {
        screenSize = size
        self.shim!.updateScreenSizes( size.width, height: size.height )
    }

    func updateCamera( viewMatrix: float4x4, projectionMatrix: float4x4, cameraTransformMatrix: float4x4 ) {

        var coordinateSpaceTransform = matrix_identity_float4x4

        self.viewMatrix            = viewMatrix * coordinateSpaceTransform
        self.projectionMatrix      = projectionMatrix
        self.cameraTransformMatrix = cameraTransformMatrix
        var Mview  = toFloatArray16( self.viewMatrix )
        var Mproj  = toFloatArray16( self.projectionMatrix )
        var Mtrans = toFloatArray16( self.cameraTransformMatrix )
        self.shim!.setCameraMatrices( &Mview, proj: &Mproj, transform: &Mtrans );
    }
   
    func updateWorld( ) {
        var accel = toFloatArray3( self.acceleration );
        self.shim!.updateWorld( &accel, torsionalSpringStrength: self.torsionalSpringStrength )
    }
   
    func draw( in view: MTKView, commandBuffer: MTLCommandBuffer ) {

        let descriptor = view.currentRenderPassDescriptor

        // At this point, the image from the camera has been drawn to the drawable, and we should not clear it.
        let oldAction = descriptor!.colorAttachments[0].loadAction
        descriptor!.colorAttachments[0].loadAction = .load

        guard
            let encoder = commandBuffer.makeRenderCommandEncoder( descriptor: descriptor! )
        else {
            return
        }

        self.shim!.encode( encoder )

        encoder.endEncoding()
        
        descriptor!.colorAttachments[0].loadAction = oldAction
    }

    func startAccelerometers() {

        if self.motion.isAccelerometerAvailable {
 
            self.motion.accelerometerUpdateInterval = 1.0 / 50.0  // 50 Hz
            self.motion.startAccelerometerUpdates()

            self.coreMotionTimer = Timer(fire: Date(), interval: (1.0/50.0), repeats: true, block: { (timer) in
                if let data = self.motion.accelerometerData {
                    let x = data.acceleration.x
                    let y = data.acceleration.y
                    let z = data.acceleration.z
                    self.acceleration = SIMD3<Double>(x, y, z)
                }
            })
            RunLoop.current.add(self.coreMotionTimer!, forMode: RunLoop.Mode.default)
        }
    }

    func touchesBegan( location: CGPoint, size: CGRect ) {
        print ( "toutchesBegan at ( \(location.x), \(location.y) ).")
    }

    func touchesMoved( location: CGPoint, size: CGRect ) {
        print ( "toutchesMoved at ( \(location.x), \(location.y) ).")
    }

    func touchesEnded( location: CGPoint, size: CGRect ) {
        print ( "toutchesEnded at ( \(location.x), \(location.y) ).")
    }

    func toFloatArray16( _  m : float4x4 ) -> [Float] {

        var arr = [ Float ]( repeating: 0.0, count: 16 )

        arr[ 0] = m[0][0]
        arr[ 1] = m[0][1]
        arr[ 2] = m[0][2]
        arr[ 3] = m[0][3]

        arr[ 4] = m[1][0]
        arr[ 5] = m[1][1]
        arr[ 6] = m[1][2]
        arr[ 7] = m[1][3]

        arr[ 8] = m[2][0]
        arr[ 9] = m[2][1]
        arr[10] = m[2][2]
        arr[11] = m[2][3]

        arr[12] = m[3][0]
        arr[13] = m[3][1]
        arr[14] = m[3][2]
        arr[15] = m[3][3]

        return arr
    }

    func toFloatArray3( _  m : SIMD3<Double> ) -> [Float] {

        var arr = [ Float ]( repeating: 0.0, count: 3 )

        arr[0] = Float( m[0] )
        arr[1] = Float( m[1] )
        arr[2] = Float( m[2] )

        return arr
    }
}


