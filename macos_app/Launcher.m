#import <Cocoa/Cocoa.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

@interface BasicPPDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, copy) NSString *fileToOpen;
@end

@implementation BasicPPDelegate

- (void)application:(NSApplication *)app openFiles:(NSArray<NSString *> *)filenames
{
    if ([filenames count] > 0)
    {
        self.fileToOpen = filenames[0];
    }
    [self launch];
    [app stop:nil];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    if ([args count] > 1)
    {
        self.fileToOpen = args[1];
    }
    
    [self launch];
    [[NSApplication sharedApplication] stop:nil];
}

- (void)launch
{
    NSString *launcherPath = [[NSProcessInfo processInfo] arguments][0];
    NSString *baseDir = [launcherPath stringByDeletingLastPathComponent];
    NSString *basicppPath = [baseDir stringByAppendingPathComponent:@"basicpp"];

    NSMutableArray *taskArgs = [NSMutableArray array];
    if (self.fileToOpen)
        [taskArgs addObject:self.fileToOpen];

    NSMutableDictionary *env = [[[NSProcessInfo processInfo] environment] mutableCopy];
    env[@"DYLD_LIBRARY_PATH"] = @"/opt/homebrew/lib:/opt/homebrew/opt/sdl2/lib:/opt/homebrew/opt/sdl2_ttf/lib";
    env[@"DYLD_FALLBACK_LIBRARY_PATH"] = @"/opt/homebrew/lib:/usr/lib";

    NSTask *task = [[NSTask alloc] init];
    [task setExecutableURL:[NSURL fileURLWithPath:basicppPath]];
    [task setArguments:taskArgs];
    [task setEnvironment:env];

    /* Log stderr to /tmp for debugging */
    NSFileHandle *errLog = [NSFileHandle fileHandleForWritingAtPath:@"/tmp/basicpp_sdl_debug.log"];
    if (!errLog)
    {
        [@"" writeToFile:@"/tmp/basicpp_sdl_debug.log" atomically:YES encoding:NSUTF8StringEncoding error:nil];
        errLog = [NSFileHandle fileHandleForWritingAtPath:@"/tmp/basicpp_sdl_debug.log"];
    }
    if (errLog)
        [task setStandardError:errLog];
    [task setStandardInput:[NSFileHandle fileHandleWithNullDevice]];

    NSError *error = nil;
    if (![task launchAndReturnError:&error])
    {
        NSLog(@"Failed to launch %@: %@", basicppPath, error);
        exit(1);
    }

    /* Give the SDL2 process a moment to create its window, then activate it */
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
        pid_t pid = [task processIdentifier];
        NSRunningApplication *child =
            [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
        [child activateWithOptions:NSApplicationActivateIgnoringOtherApps];
    });

    [task waitUntilExit];
    exit([task terminationStatus]);
}

@end

int main(int argc, char *argv[])
{
    @autoreleasepool
    {
        NSApplication *app = [NSApplication sharedApplication];
        BasicPPDelegate *delegate = [[BasicPPDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}
