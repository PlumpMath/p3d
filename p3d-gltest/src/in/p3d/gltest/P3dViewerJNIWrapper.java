package in.p3d.gltest;

import android.content.res.AssetManager;

public class P3dViewerJNIWrapper {
    static {
        System.loadLibrary("p3dviewer");
    }

    public static native void on_surface_created();
 
    public static native void on_surface_changed(int width, int height);
 
    public static native void on_draw_frame();
    
    public static native void init_asset_manager(AssetManager am);
}
