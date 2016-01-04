print('__file__: ' + __file__);
for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + Modules.trace[fn] + '"');
}

print('include("_file_be_included2.js")');
include('_file_be_included2.js');


for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + Modules.trace[fn] + '"');
}

var g = globals();
print("globals():");
for (var k in g) {
    print(' ' + k + ': ' + g[k]);
}

var os = require('os');
print(os.path.normpath('dir1/../dir2/test...file/'));
print(os.path.abspath('dir1/../dir2/test...file'));
