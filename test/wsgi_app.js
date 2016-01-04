//reserver global name
//  "__file__"           as script file path
//  "__http_response_status__"
//  "__http_response_headers__"
//
//environ is '__cobject_map__', not array!
function application(environ, start_response)
{
    print(__file__);

    for (var k in environ) {
        var v = environ[k];
        print(
            "\"" + k + "\"" + 
            ": " +
            "{" + typeof(v) + "} " +
            "\"" + v + "\""
        );
    };

    var form = environ['FORM']
    if (form) {
        var form = environ['FORM'];
        print("POST:");
        for (var k in form) {
            var v = form[k];
            print(" \"" + k + "\"" + ": " + "\"" + v + "\"");
        };
    }

    start_response(200, {"Name":"jush"});

    try {
        var out = subprocess.check_output('uptime');
    } catch (e) {
        var out = '';
        if (e instanceof subprocess.CalledProcessError) {
            out =
                'subprocess.CalledProcessError:\r\n' + 
                ' e.returncode = ' + e.returncode + '\r\n' +
                ' e.cmd        = ' + e.cmd + '\r\n' + 
                ' e.output     = "' + e.output + '"\r\n';
        } else {
            out = '' + e;
        }
    }

    print('response.length = ' + out.length);
    return out;
}

