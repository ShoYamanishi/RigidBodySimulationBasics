import SwiftUI

struct ContentView: View {

    @EnvironmentObject var worldManager: WorldManagerSwiftEnd

    @Environment(\.verticalSizeClass) var verticalSizeClass

    var body: some View {

        if verticalSizeClass == .compact {
            HStack {
                MetalView().padding()
                HStack{
                    Text("Torsional Spring Strength:")
                    Slider( value: $worldManager.torsionalSpringStrength, in: 0.0...1.0 ).padding()
                }
            }

        } else {
            VStack(alignment: .center) {
                MetalView().padding()
                HStack{
                    Text("Torsional Spring Strength:")
                    Slider( value: $worldManager.torsionalSpringStrength, in: 0.0...1.0 ).padding()
                }
            }
        }
    }
}

