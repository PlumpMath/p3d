package in.p3d.mobile;

import java.nio.ByteBuffer;

import android.content.res.AssetManager;
import android.util.Log;

public class P3dViewerJNIWrapper {
	private static final String TAG = "P3dViewerJNIWrapper";
    static {
    	Log.d(TAG, "Loading .so");
        System.loadLibrary("p3dviewer");
    }

    public static void init() {
    	// dummy to force library load
    	Log.d(TAG, "init");
    };
    
    public static native void on_surface_created();
 
    public static native void on_surface_changed(int width, int height);
 
    public static native void on_draw_frame();
    
    public static native void init_asset_manager(AssetManager am);
    
    public static native void load_binary(ByteBuffer data, int size);
    
    public static native void start_rotate_cam(float x, float y);
    
    public static native void rotate_cam(float x, float y);
    
    public static native void reset_cam();
}
