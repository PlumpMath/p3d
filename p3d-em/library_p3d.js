var LibraryP3D = {
    $P3D__deps: ['glGenTextures', 'glBindTexture'],//['$Browser'],
    $P3D: {
        show: function() {
            console.log("ctx", Module.ctx);
        },
    },
    p3d_load_texture: function(url, onload) {
        var _url = Pointer_stringify(url);
        console.log("p3d_load_texture", _url);
        var gl = Module.ctx;
        console.log("gl", gl);
        var img = new Image();
        img.onload = function() {
            console.log("img loaded");
            var texIdPtr = _malloc(4);
            console.log("GL", GL);
            _glGenTextures(1, texIdPtr);
            var texId = getValue(texIdPtr, 'i32');
            _free(texIdPtr);
            console.log('texId', texId);
            _glBindTexture(gl.TEXTURE_2D, texId);

            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, img);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
            gl.generateMipmap(gl.TEXTURE_2D);

            _glBindTexture(gl.TEXTURE_2D, 0);

            Runtime.dynCall('vi', onload, [texId]);
       };
        img.src = _url;
    }
};

autoAddDeps(LibraryP3D, '$P3D');
mergeInto(LibraryManager.library, LibraryP3D);
