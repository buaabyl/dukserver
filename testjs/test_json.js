//'jc' is standard json format, 'jx' is extend json format
var fs = require('fs');

try{
    var obj = {name: "fish", devices: ["macbook", "thinkpad", 'hp']};
    //jx = Duktape.enc('jc', obj);
    jx = Duktape.enc('jc', obj, null, 4);
    fs.file_put_content('duk.json', jx);
    print(jx);

    jx = fs.file_get_content('duk.json');
    var dec = Duktape.dec('jc', jx);
    print(dec.name);
    print(dec.devices);
} catch (e) {
    print(e.stack);
}

