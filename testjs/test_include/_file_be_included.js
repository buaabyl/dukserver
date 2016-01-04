print('__file__:' + __file__);
for (var fn in Modules.trace) {
    print(' .' + fn + ': "' + __files__[fn] + '"');
}
print('hello world\n');
print('sys.args: ' + Modules.sys.args);

//Duktape内部的字符编码是UTF-8的，
//并且这个源文件也是UTF-8的。
//下面将他写为UCS-2（Unicode 2字节编码小端模式）
var f = Modules.fs.open('测试.tmp', 'wb');
f.write(chr(0xFF));
f.write(chr(0xFE));
f.write('测试一下'.decode('UTF-8'));
f.close();

print('ord("1"): ' + ord('1'));
print('chr(72): ' + chr(72));
print('hex(72): ' + hex(72));

print('include("_file_be_included1.js")');
include('_file_be_included1.js');

