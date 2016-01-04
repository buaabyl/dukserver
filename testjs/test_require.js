/*
function modSearch(id, require, exports, module) {
    print("*modSearch(" + id + "," + require + "," + exports + "," + module + ")");
    print("dir(module):");
    for (var k in dir(module, 1)) {
        print(" *" + k + ':' + module[k]);
    }
    print("END");

    if (id === 'foo') {
        return 'exports.hello = function() { print("Hello from foo!"); };';
    } else if (id === 'bar') {
        return 'exports.hello = function() { print("Hello from bar!"); };';
    } else if (id === 'fun') {
        return 'exports = function() { print("Hello from fun!"); };';
    } else {
        //try load script
        var dirname = os.path.dirname(__file__);
        var fn = os.path.join(dirname, id);
        var absfn = os.path.abspath(fn);
        if (!os.path.isfile(absfn)) {
            dirname = os.getcwd();
            fn = os.path.join(dirname, id);
            absfn = os.path.abspath(fn);
            if (!os.path.isfile(absfn)) {
                throw new Error('module not found: ' + id);
            }
        }

        var text = fs.file_get_content(absfn);
        print("load: " + absfn);

        return text;
    }
    throw new Error('module not found: ' + id);
};

Duktape.modSearch = modSearch;
*/

print("globals():");
for (var k in globals()) {
    print(k + ':' + globals()[k]);
}
print("END");

var obj = require('_mod_be_require.js');
if (obj) {
    var m = dir(obj);
    for (var k in m) {
        print(k + ":" + m[k]);
    }
    obj.hello();
}




