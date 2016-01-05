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

    var form = environ['wsgi.post']
    if (form) {
        print("POST:");
        for (var k in form) {
            var v = form[k];
            print(" \"" + k + "\"" + ": " + "\"" + v + "\"");
        };
    }

    start_response(200, {"Name":"jush"});

    var _subprocess = require('_subprocess');
    var ps = _subprocess.open('uptime');
    var stdout;
    while (1) {
        var out = ps.stdout.read();
        if (out == null) {
            break;
        }
        stdout += out;
    }

    return stdout;
}

