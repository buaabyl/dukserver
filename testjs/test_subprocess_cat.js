var os = require('os');
var _subprocess = require('_subprocess');

var ps;
if (os.name == 'posix') {
    ps = _subprocess.open('cat');
} else {
    ps = _subprocess.open('cat.exe');
}

var cnt = 10;
while (cnt-- > 0) {
    ps.stdin.write('hello' + cnt + '\n');
    var out = ps.stdout.read();
    if (out == null) {
        break;
    }
    print('"' + out + '"');
}

print('close');
ps.close();


