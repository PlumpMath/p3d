var Module = Module || {
    preRun: [],
    postRun: [],
    print: function(text) {
        console.log(text);
    },
    printErr: function(text) {
        console.log(text);
    }
};

var P3DViewer = (function(){
    var p3d = {};
    var oldMouseX = 0;
    var oldMouseY = 0;

    p3d.loadBin = function(binUrl, jsonUrl) {
        if(moduleRunning) {
            loadBin(binUrl, jsonUrl);
        } else {
            Module.postRun.push(loadBin.bind(this, binUrl, jsonUrl));
        }
    }

    p3d.loadBlend = function(blendUrl) {
        if(moduleRunning) {
            loadBlend(blendUrl);
        } else {
            Module.postRun.push(loadBlend.bind(this, blendUrl));
        }
    }

    var moduleRunning = false;

    // wrapped c functions
    var setMaterialProperty;
    var setUrlPrefix;

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
        if(event.button === 0) {
            Module._rotateCam(x, y);
        } else if(event.button === 1) {
            Module._zoomCam((y - oldMouseY) * 5);
        } else if(event.button === 2) {
            Module._panCam((x - oldMouseX) * (canvas.width / canvas.height) * 0.205, (oldMouseY - y) * 0.205);
        }

        oldMouseX = x;
        oldMouseY = y;
    }

    function mousedown(event) {
        event.preventDefault();
        event.stopPropagation();

        var canvas = Module.canvas;
        var x = (event.clientX - canvas.width * 0.5 - canvas.getBoundingClientRect().left) / (canvas.width * 0.5);
        var y = (canvas.height * 0.5 + canvas.getBoundingClientRect().top - event.clientY) / (canvas.height * 0.5);
        oldMouseX = x;
        oldMouseY = y;
        //console.log("mouse down", x, y);
        if(event.button === 0) {
            Module._startRotateCam(x, y);
        }

        document.addEventListener( 'mousemove', mousemove, false );
        document.addEventListener( 'mouseup', mouseup, false );
    }

    function mousewheel(event) {
        event.preventDefault();
        event.stopPropagation();
        Module._zoomCam((event.deltaY/Module.canvas.height) * -5);
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

    function handleMaterials(json)
    {
        for(var matIndex = 0, matIndexL = json.materials.length; matIndex < matIndexL; ++matIndex) {
            var mat = json.materials[matIndex];
            var matSettings = JSON.parse(mat.settings);
            var keys = Object.keys(matSettings);
            for(var key = 0, keyl = keys.length; key < keyl; ++key) {
                var propName = keys[key];
                var propValue = "" + matSettings[propName];
                setMaterialProperty(matIndex, propName, propValue);
            }

            for(var i = 0, il = mat.texture_assignment_ids.length; i < il; ++i) {
                var taId = mat.texture_assignment_ids[i];
                for(var j = 0, jl = json.texture_assignments.length; j < jl; ++j) {
                    var ta = json.texture_assignments[j];
                    if(ta.id === taId) {
                        for(var k = 0, kl = json.textures.length; k < kl; ++k) {
                            var tex = json.textures[k];
                            if(tex.id === ta.texture_id) {
                                console.log(matIndex, ta.texture_type, tex.url);
                                if(ta.texture_type === "diff") {
                                    setMaterialProperty(matIndex, "diffuseTexture", tex.url);
                                } else if(ta.texture_type === "spec") {
                                    setMaterialProperty(matIndex, "specTexture", tex.url);
                                }
                            }
                            continue;
                        }
                        continue;
                    }
                }
            }
        }
    }

    function postRun() {
        moduleRunning = true;

        setMaterialProperty = Module.cwrap('setMaterialProperty', 'void', ['number', 'string', 'string']);
        setUrlPrefix = Module.cwrap('setUrlPrefix', 'void', ['string']);

        // setup mouse handling
        var canvas = Module.canvas;
        canvas.addEventListener( 'mousedown', mousedown, false );
        canvas.addEventListener( 'wheel', mousewheel, false );
        canvas.addEventListener( 'dblclick', Module._resetCam, false);
    };

    function loadBin(binUrl, jsonUrl) {
        var pending = {
            json: null,
            dataDone: false,
            cb: handleMaterials,
            check: function() {
                if(pending.json && pending.dataDone) {
                    pending.cb(pending.json);
                }
            }
        };
        var xhr = new XMLHttpRequest();
        xhr.open('GET', binUrl, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function(e) {
            if (this.readyState === 4 && (this.status === 200 || this.status === 0)) {
                loadModel(this.response, ".bin");
                pending.dataDone = true;
                pending.check();
            }
        };
        xhr.send();

        if(jsonUrl) {
            xhr = new XMLHttpRequest();
            xhr.open('GET', jsonUrl, true);
            xhr.responseType = 'json';
            xhr.onload = function(e) {
                if (this.readyState === 4 && (this.status === 200 || this.status === 0)) {
                    pending.json = this.response;
                    pending.check();
                }
            };
            xhr.send();
        }
    }

    function loadBlend(blendUrl) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', blendUrl, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function(e) {
            if (this.readyState === 4 && (this.status === 200 || this.status === 0)) {
                setUrlPrefix(blendUrl.substr(0, blendUrl.lastIndexOf('/') + 1));
                loadModel(this.response, ".blend");
            }
        };
        xhr.send();
    }

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

    return p3d;
}());

