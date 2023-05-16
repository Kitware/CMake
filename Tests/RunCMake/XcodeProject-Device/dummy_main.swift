#if os(iOS)
import UIKit

@UIApplicationMain
class MyApp: UIResponder, UIApplicationDelegate {
}

#elseif os(macOS)
import SwiftUI

@main
struct MyApp: App {
    var body: some Scene {
        WindowGroup {
        }
    }
}
#endif
