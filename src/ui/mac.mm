#import <Cocoa/Cocoa.h>

extern "C" void DisableDockIcon() 
{
    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyAccessory];
}
