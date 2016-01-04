//create an unnamed object, not create dict!
d = {"name0":"fish", "name1":"ken"};
print(d instanceof Array);
print(Object.prototype.toString.call(d));
print("Object.getOwnPropertyNames({}):", Object.getOwnPropertyNames(d));
print("Object.keys({}):", Object.keys(d));
print();

//create array
l0 = [1,2,3,4];
print(l0 instanceof Array);
print(Object.prototype.toString.call(l0));
print("Object.getOwnPropertyNames(l0):", Object.getOwnPropertyNames(l0));
print("Object.keys(l0):", Object.keys(l0));
print();

//create array
l1 = new Array(1,2,3,4);
print(l1 instanceof Array);
print(Object.prototype.toString.call(l1));
print("Object.getOwnPropertyNames(l1):", Object.getOwnPropertyNames(l1));
print("Object.keys(l1):", Object.keys(l1));
print();

//create array
dict = new Array();
dict["name0"] = "david";
dict["name1"] = "john";
print(dict instanceof Array);
print(Object.prototype.toString.call(dict));
print("Object.getOwnPropertyNames(dict):", Object.getOwnPropertyNames(dict));
print("Object.keys(dict):", Object.keys(dict));
print();

buf = Duktape.Buffer("string");
print(typeof(buf));
for (var c in buf) {
    print('', c);
}
print("==");
res = new Array();
for (i = 0;i < 5;i++) {
    res.push(i);
}
print(Object.prototype.toString.call(res));
print(res.join(''));

