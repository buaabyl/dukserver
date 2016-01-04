var time = require('time');

print(" time.time(): " + time.time());
print(" time.localtime(): " + time.localtime());

print("strftime: " + time.strftime("%Y-%m-%d"));
print(time.ctime());

