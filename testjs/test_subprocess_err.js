include('test_subprocess.js');

try {
    var out = subprocess.check_output("test_subprocess_runner");
    var out = subprocess.check_output("test_subprocess_runner", "error");
    print(out);
} catch (e) {
    if (e instanceof subprocess.CalledProcessError) {
        print('subprocess.CalledProcessError:');
        print(' e.returncode = ' + e.returncode);
        print(' e.cmd        = ' + e.cmd);
        print(' e.output     = "' + e.output + '"');
    }
}

