print("__file__:" + __file__);

var res = globals();
print("globals():");
for (var k in res) {
    print(" " + k + ": " + res[k]);
}
print();

print("include('./test_include/_file_be_included.js')");
include("./test_include/_file_be_included.js");

var sys = require('sys');
if (sys.args.length > 1) {
    if (os.path.exists(sys.args[1])) {
        var st = os.stat(sys.args[1]);
        print(sys.args[1].decode('UTF-8').encode('GBK'));
        print(st);
    }
}

