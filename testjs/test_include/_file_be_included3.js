print('__file__: ' + __file__);
for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + __files__[fn] + '"');
}

print('include("./_file_be_included4.js")');
include('./_file_be_included4.js');
