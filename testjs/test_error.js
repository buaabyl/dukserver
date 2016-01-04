var sys = require('sys');

var s = 'sys.args: ';
for (var i = 0;i < sys.args.length;i++) {
    s += sys.args[i] + ", ";
}
print(s);

try {
    print('throw "exception"');
    throw Array("throw Array");
    throw "throw string: exception";
    print("ok");
} catch (estr) {
    if (estr instanceof Object) {
        print('catched object: ' + estr);
    } else {
        print('catched: ' + estr);
    }
    This_will_cause_ReferenceError;
}


