var os  = require('os');
var sys = require('sys');
var _subprocess = require('_subprocess');

var args = Array();

for (var i = 1;i < sys.args.length;i++) {
    args.push(sys.args[i]);
}

var res = _subprocess.open(args);

var m = dir(res);
print('dir(res):');
for (var k in m) {
    print(' ' + k + ': ' + m[k]);
}
print();

//read stdout
while (1) {
    var out = res.stdout.read();
    if (out == null) {
        break;
    }
    print('"' + out + '"');
}

print('commandline: ' + res.cmd);
print('returncode: ' + res.wait());


