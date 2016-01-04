print("hello world\n");

function show_method(v)
{
    var ddd = dir(v, 1);
    for (var k in ddd) {
        print(k + ": " + ddd[k]);
    }
    print();
}

var str = "hello $name";
var newstr = str.replace(/\$name/g, 'david');
print(newstr);

var r = /\W*(\d+)\W*x\W*(\d+)/g;
//show_method(r);

var txt = " 1 x8";

var res = r.exec(txt);

print("txt: \"" + txt + "\"");
if (res) {
    print("match");
    print("res.length = " + res.length);

    for (var i = 0;i < res.length;i++) {
        print(" res[" + i + "] = " + res[i]);
    }
    print();
} else {
    print("not match");
}

var params = new Array();
var res = " 1 x8".replace(/\W*(\d+)\W*x\W*(\d+)/g, function (s, s1, s2){
    params.push(s1);
    params.push(s2);
    return s;
});
print(params);


print("replace:");
var res = "${status_code} ${status_message}".replace(/\$\{([^}]+)\}/g, function (s, s1){
    print(s);
    print(s1);
    return s;
});
print("replace-end");

function isdigit(s)
{
    //float
    if (s.match(/^\W*\d+\.\d+\W*$/g)) {
        return true;
    }
    //interger
    if (s.match(/^\W*\d+\W*$/g)) {
        return true;
    }
    return false;
}

print(isdigit("2.34u"));
