var modelToLoad = modelToLoad || {
    binUrl: "samples/captain.bin"
};

function mouseup( event ) {
    event.preventDefault();
    event.stopPropagation();
    document.removeEventListener( 'mousemove', mousemove );
    document.removeEventListener( 'mouseup', mouseup );
}

function mousemove(event) {
    event.preventDefault();
    event.stopPropagation();

    var canvas = Module.canvas;
    var x = (event.clientX - canvas.width * 0.5 - canvas.getBoundingClientRect().left) / (canvas.width * 0.5);
    var y = (canvas.height * 0.5 + canvas.getBoundingClientRect().top - event.clientY) / (canvas.height * 0.5);
    //console.log("mouse move", x, y);
    Module._rotateCam(x, y);

}

function mousedown(event) {
    event.preventDefault();
    event.stopPropagation();

    var canvas = Module.canvas;
    var x = (event.clientX - canvas.width * 0.5 - canvas.getBoundingClientRect().left) / (canvas.width * 0.5);
    var y = (canvas.height * 0.5 + canvas.getBoundingClientRect().top - event.clientY) / (canvas.height * 0.5);
    //console.log("mouse down", x, y);
    Module._startRotateCam(x, y);

    document.addEventListener( 'mousemove', mousemove, false );
    document.addEventListener( 'mouseup', mouseup, false );
}

function loadModel(data, extension) {
    var size = data.byteLength;
    Module.print("loaded bytes: " + size);
    var buf = Module._malloc(size);
    Module.HEAPU8.set(new Uint8Array(data), buf);
    Module._loadModel(buf, size, Module.allocate(Module.intArrayFromString(extension), 'i8', Module.ALLOC_STACK));
    Module._free(buf);
    Module.print("material count: " + Module._materialCount());
}

function postRun() {
    console.log("postRun");

    var xhr = new XMLHttpRequest();
    xhr.open('GET', modelToLoad.binUrl, true);
    xhr.responseType = 'arraybuffer';

    xhr.onload = function(e) {
        if (this.readyState === 4 && (this.status === 200 || this.status === 0)) {
            loadModel(this.response, ".bin");
        }
    };

    xhr.send();

    // setup mouse handling
    var canvas = Module.canvas;
    canvas.addEventListener( 'mousedown', mousedown, false );
    canvas.addEventListener( 'dblclick', Module._resetCam, false);
};

function handleFileSelect(evt) {
    var file = evt.target.files[0];
    console.log("loading:", file.name);
    var extension = /\.[^.]+$/.exec(file.name);
    if(!extension) {
        console.log(" filename doesn't have extension");
        return;
    }
    extension = extension[0];
    console.log(" extension:", extension);
    var reader = new FileReader();
    reader.onload = function(e) {
        loadModel(reader.result, extension);
    }
    reader.readAsArrayBuffer(file);
}
var fileElem = document.getElementById('modelFile');
if(fileElem) {
    fileElem.addEventListener('change', handleFileSelect, false);
}

Module.postRun.push(postRun);
Module.TOTAL_MEMORY = 16 * 1024 * 1024;


