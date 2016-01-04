var _subprocess = require('_subprocess');
var subprocess = new Object();

subprocess.CalledProcessError = function (returncode, cmd, stdout) {
    this.returncode = returncode;
    this.cmd = cmd;
    this.stdout = stdout;
}

subprocess.call = function() {
    var ps;
    var returncode;
    var out;
    var args = new Array();

    for (var i = 0;i < arguments.length;i++) {
        args.push(arguments[i]);
    }
    ps = _subprocess.open(args);

    while (1) {
        var res = ps.stdout.read();
        if (res == null) {
            break;
        }
        out = out + res;
    }
    returncode = ps.wait();
    if (returncode != 0) {
        throw new subprocess.CalledProcessError(returncode, args, out);
    }

    ps.close();
    return returncode;
}

subprocess.check_output = function() {
    var ps;
    var returncode;
    var out;
    var args = new Array();

    for (var i = 0;i < arguments.length;i++) {
        args.push(arguments[i]);
    }
    ps = _subprocess.open(args);

    while (1) {
        var res = ps.stdout.read();
        if (res == null) {
            break;
        }
        out = out + res;
    }
    returncode = ps.wait();
    if (returncode != 0) {
        throw new subprocess.CalledProcessError(returncode, args, out);
    }

    ps.close();
    return out
}

print('subprocess.call("ls.exe"):');
var ret = subprocess.call("ls.exe");
print('return ' + ret);
print();

print('subprocess.check_output("ls.exe"):');
var ret = subprocess.check_output("ls.exe");
print('return "' + ret + '"');

try {
    subprocess.check_output('test_subprocess_runner', 'error');
} catch (e) {
    if (e instanceof subprocess.CalledProcessError) {
        print('CalledProcessError:');
        print(' ' + e.returncode);
        print(' ' + e.cmd);
        print(' "' + e.stdout + '"');
    } else {
        print(e);
    }
}



