print("Duktape.version = " + Duktape.version);

function small_test()
{
    var i;
    var n = 2;
    for (i = 0;i < n;i++) {
        print(chr(ord('0') + i));
    }

    v = new Array(1,2,3,4);
    print(v);

    print("list:");
    l = [1,2,"string"];
    for (var val in l) {
        print('', val);
    }

    print("dictory:");
    d = {"name":"fish", "id":122};
    for (var key in d) {
        print('', typeof(key), key, d[key]);
    }

    print("name" in d);
    print("user" in d);
}

small_test();

