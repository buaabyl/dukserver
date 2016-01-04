<?php

var_dump($_SERVER['REQUEST_METHOD']);
var_dump($_SERVER['REQUEST_URI']);
var_dump($_SERVER['PATH_INFO']);

if (($stream = fopen('php://input', "rb")) !== FALSE) {
    var_dump(stream_get_contents($stream));
}

?>

