function test() {
    return "hello world!";
}

print('__file__: ' + __file__);
for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + __files__[fn] + '"');
}

print('include("test_include/_file_be_included3.js")');
include('test_include/_file_be_included3.js');

var base = 100;
print('base + 1 + 1 = ' + eval('1+1+base'));

test();

