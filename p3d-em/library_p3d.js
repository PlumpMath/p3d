var LibraryP3D = {
    $P3D__deps: ['glGenTextures', 'glBindTexture'],//['$Browser'],
    $P3D: {
        pending: [],
        show: function() {
            console.log("ctx", Module.ctx);
        },
    },

    p3d_cancel_textures: function() {
        while(P3D.pending.length > 0) {
            var pending = P3D.pending.pop();
            pending.canceled = true;
            Runtime.dynCall('vii', pending.onload, [pending.arg, 0]);
        }
    },

    p3d_load_texture: function(arg, url, onload) {
        var _url = Pointer_stringify(url);
        console.log("p3d_load_texture", arg, _url);
        var gl = Module.ctx;
        var img = new Image();
        var pending = {
            arg: arg,
            onload: onload,
            canceled: false
        };
        P3D.pending.push(pending);
        img.onload = function() {
            console.log("img loaded");
            if(pending.canceled) return;

            // remove from pending list
            var index = P3D.pending.indexOf(pending);
            console.log("pending index", index);
            if(index >= 0) P3D.pending.splice(index, 1);

            var texIdPtr = _malloc(4);
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

            Runtime.dynCall('vii', pending.onload, [pending.arg, texId]);
       };
        img.src = _url;
    }
};

autoAddDeps(LibraryP3D, '$P3D');
mergeInto(LibraryManager.library, LibraryP3D);
