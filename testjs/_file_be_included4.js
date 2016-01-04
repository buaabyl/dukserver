print('__file__: ' + __file__);
for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + __files__[fn] + '"');
}

