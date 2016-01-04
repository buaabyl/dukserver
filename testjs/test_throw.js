try {
    print("Ok\n");
    not_exist_function();
    throw "Exception";
    print("Not reach\n");

} catch (e) {
    print('typeof(e) = ' + typeof(e));
    if (typeof(e) == 'object') {
        print(e.stack);
        print();
        print(e.name + ": " + e.message);
        print('Location: ' + e.fileName + ":" + e.lineNumber);

    } else {
        print(e);
    }
}

