print("ord: " + ord('a'));
print("chr: " + chr(0x64));
print("hello");

var s = 'sdfsdf.111';
print('s.split = ' + s.split);
print('s.split.name = ' + s.split.name);
print();

var ll = s.split('.');
print('typeof(ll) = ' + typeof(ll));
print(ll);
print();

print("typeof(globals)=", typeof(globals));
print('globals:');
var l = globals();
for (var v in l) {
    print(' ["' + v + '"] = (' + typeof(l[v]) + ') ' + l[v]);
}
print();

print("typeof(Duktape) = " + typeof(Duktape));
print("dir(Duktape):");
var m = dir(Duktape, 2);
for (var k in m) {
    print(' ' + k + ': ' + m[k]);
}


