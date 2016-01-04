var os = require('os');

print(" os.getcwd(): " + 
        os.getcwd());

print(" os.path.split('C:'): " + 
        os.path.split('C:'));
print(" os.path.split('C:/'): " + 
        os.path.split('C:/'));
print(" os.path.split('/ewfwef/ffff.js'): " + 
        os.path.split('/ewfwef/ffff.js'));

print(" os.path.splitext('/ewfwef/ffffjs'): " + 
        os.path.splitext('/ewfwef/ffffjs'));
print(" os.path.splitext('c:\\ewfwef\\ffff.js'): " + 
        os.path.splitext('c:\\ewfwef\\ffff.js'));
print(" os.path.splitext('c:/ewfwef/ffff.js'): " + 
        os.path.splitext('c:/ewfwef/ffff.js'));

print(" os.path.normpath('test/../.././hello.js'): " + 
        os.path.normpath('test/../.././hello.js'));

print(" os.path.abspath('test/../.././hello.js'): " + 
        os.path.abspath('test/../.././hello.js'));

print(" os.path.join('C:/', 'test', 'file.js')" +
        os.path.join('C:/', 'test', 'file.js'));

