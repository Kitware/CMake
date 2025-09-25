#import <XCTest/XCTest.h>

#import "FrameworkExample/FrameworkExample.h"

@interface FrameworkExampleTests : XCTestCase

@end

@implementation FrameworkExampleTests

- (void)testFortyTwo {
    // This is an example of a functional test case.
    XCTAssertEqual(42, FortyTwo());
}

@end
