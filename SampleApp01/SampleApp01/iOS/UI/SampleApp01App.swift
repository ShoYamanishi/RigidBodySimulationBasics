import SwiftUI

@main
struct SampleApp01App: App {

    let worldManager : WorldManagerSwiftEnd!
    
    init() {
        worldManager = WorldManagerSwiftEnd( device: MTLCreateSystemDefaultDevice()! )
    }

    var body: some Scene {
        WindowGroup {
            ContentView().environmentObject( worldManager )
        }
    }
}
