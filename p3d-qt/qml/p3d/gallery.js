function json(url, cb) {
    console.log("fetch");
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.HEADERS_RECEIVED) {
//            console.log("Headers -->");
//            console.log(xhr.getAllResponseHeaders ());
//            console.log("Last modified -->");
//            console.log(xhr.getResponseHeader ("Last-Modified"));

        } else if (xhr.readyState === XMLHttpRequest.DONE) {
//            console.log("Headers -->");
//            console.log(xhr.getAllResponseHeaders ());
//            console.log("Last modified -->");
//            console.log(xhr.getResponseHeader ("Last-Modified"));
            var data = JSON.parse(xhr.responseText);
            cb(null, data);

        }
    }

    //xhr.responseType = "json";
    xhr.open("GET", url);
    xhr.send();
}

