s = "hello;world;";
var l = s.split(";");

for (var idx in l) {
    print("index: " + idx + ", \"" + l[idx] + "\"");
}

print(s);
